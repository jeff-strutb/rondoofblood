"""typedrec.py -- the records file and the C structs written from it.

Used by syscard/tools/ramnames.py. The game has its own copy (tools/rondo/typedrec.py): the card keeps nothing
of the game's, and the two change together. A records
file describes how some of a list of symbols are laid out as records; this module checks the
description against the symbols and writes one C header of structs, with a compile-time check for
every member that it sits at the address its symbol has.

Records file (ASCII, one row a line, `#` starts a comment, numbers in hex as in symbols.csv):

    record,NAME,LAYOUT,BASE,COUNT,SIZE,POINTER
        LAYOUT soa: parallel byte arrays, the shape the object table has. BASE is the logical address
            of the first byte, COUNT the slots in each column, SIZE the bytes the whole table spans.
            A member is a column of COUNT bytes; its offset is the column's start.
        LAYOUT aos: an array of COUNT structs of SIZE bytes from BASE. A member's offset is within one
            struct. The struct type describes one element; POINTER points at element 0.
        POINTER is the C name that stands for the record placed at BASE: ((NAME *)BASE). The struct is
            never instantiated or dereferenced; its members' addresses are the access.
    bits,SET,BIT,MASK,NOTE
        A named bit of a bit set, for members of type bits:SET.
    member,RECORD,NAME,TYPE,OFFSET,SYMBOLS,NOTE
        TYPE: u8, s8, u16le, u16be, u24le, u24be (the byte order in storage; the order of the
            accesses is the macro's, RD16_LH and the like), bits:SET, or u8[N] (aos only).
        SYMBOLS: the symbol of each byte in storage order, joined by |; a wide member may name only
            its first byte's symbol. Every member names at least one symbol: a field the assembly
            names is a field of a record, and the other way round.

    alias,RECORD,ALIAS,TARGET,SYMBOL
        A second name for a member, kept while code still uses the old one: both names are the same bytes (an
        anonymous union in the struct), and both are checked against the symbol. Drop the row when the last user moves.
        TARGET may be MEMBER.SUB (SUB one of lo, mid, hi) to name one byte of a wide member: a scratch byte another
        routine uses under its own name; or MEMBER+N, MEMBER+N[K] or MEMBER+N:TYPE, the byte, K bytes or a wide value (u16le and so on) at offset N
        of an array member (a u8[N] array two routines see differently). SYMBOL is then the symbol of that byte, and is checked
        against its address.

A row that disagrees with the symbols, two members that overlap, a symbol claimed twice, a name used
twice, or a type that collides with a symbol makes the generator fail with the row's line.
"""
import re
import sys

WIDE = {"u16le": ("U16LE", ("lo", "hi")), "u16be": ("U16BE", ("hi", "lo")),
        "u24le": ("U24LE", ("lo", "mid", "hi")), "u24be": ("U24BE", ("hi", "mid", "lo"))}
IDENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class SpecError(Exception):
    pass


def _num(text, where):
    try:
        return int(text, 16)
    except ValueError:
        raise SpecError(f"{where}: {text!r} is not a hex number")


def load_spec(path):
    """(records, bits, members): records {name: dict}, bits {set: [(bit, mask, note, line)]}, members [dict]."""
    records, bits, members, aliases = {}, {}, [], []
    with open(path, encoding="ascii") as f:
        for n, line in enumerate(f, 1):
            line = line.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            where = f"{path}:{n}"
            kind, _, rest = line.partition(",")
            if kind == "record":
                p = rest.split(",")
                if len(p) != 6:
                    raise SpecError(f"{where}: record needs NAME,LAYOUT,BASE,COUNT,SIZE,POINTER")
                name, layout, base, count, size, ptr = p
                if layout not in ("soa", "aos"):
                    raise SpecError(f"{where}: layout {layout!r} is not soa or aos")
                if not IDENT.match(name) or not IDENT.match(ptr):
                    raise SpecError(f"{where}: bad name")
                if name in records:
                    raise SpecError(f"{where}: record {name} twice")
                records[name] = dict(name=name, layout=layout, base=_num(base, where), count=_num(count, where),
                                     size=_num(size, where), ptr=ptr, line=n, members=[])
            elif kind == "bits":
                p = rest.split(",", 3)
                if len(p) != 4:
                    raise SpecError(f"{where}: bits needs SET,BIT,MASK,NOTE")
                mask = _num(p[2], where)
                if mask == 0 or mask > 0xFF:
                    raise SpecError(f"{where}: mask must be 1 to FF")
                bits.setdefault(p[0], []).append((p[1], mask, p[3], n))
            elif kind == "member":
                p = rest.split(",", 5)
                if len(p) != 6:
                    raise SpecError(f"{where}: member needs RECORD,NAME,TYPE,OFFSET,SYMBOLS,NOTE")
                rec, name, typ, off, syms, note = p
                members.append(dict(rec=rec, name=name, type=typ, offset=_num(off, where),
                                    syms=[s for s in syms.split("|") if s], note=note, line=n, where=where))
            elif kind == "alias":
                p = rest.split(",", 3)
                if len(p) != 4:
                    raise SpecError(f"{where}: alias needs RECORD,ALIAS,TARGET,SYMBOL")
                aliases.append(dict(rec=p[0], alias=p[1], member=p[2], symbol=p[3].strip(), line=n, where=where))
            else:
                raise SpecError(f"{where}: unknown row kind {kind!r}")
    return records, bits, members, aliases


def _width(m, rec):
    t = m["type"]
    if t in ("u8", "s8") or t.startswith("bits:"):
        return 1, "byte"
    if t in WIDE:
        return len(WIDE[t][1]), "wide"
    mm = re.match(r"^u8\[(\d+)\]$", t)
    if mm:
        return int(mm.group(1)), "array"
    raise SpecError(f"{m['where']}: unknown type {t!r}")


def build(path, symbols, taken_names):
    """Check the spec against `symbols` [(logical address, name)] and return (header text, report lines).

    taken_names: every identifier the header must not collide with (the symbols' names and others)."""
    records, bits, members, aliases = load_spec(path)
    by_name = {}
    for addr, name in symbols:
        by_name.setdefault(name, []).append(addr)
    for name, addrs in by_name.items():
        if len(set(addrs)) > 1:
            raise SpecError(f"symbol {name} has two addresses")
    addr_of = {n: a[0] for n, a in by_name.items()}
    reserved = set(taken_names)
    seen_idents = {}

    def claim_ident(ident, where):
        if ident in reserved or ident in seen_idents:
            raise SpecError(f"{where}: {ident} is already a symbol or another type's name")
        seen_idents[ident] = where

    for r in records.values():
        claim_ident(r["name"], f"{path}:{r['line']}")
        claim_ident(r["ptr"], f"{path}:{r['line']}")
    for sname, rows in bits.items():
        bn = set()
        for bit, mask, note, ln in rows:
            if bit in bn:
                raise SpecError(f"{path}:{ln}: bit {bit} of {sname} twice")
            bn.add(bit)
            claim_ident(f"{sname}_{bit}", f"{path}:{ln}")
    for m in members:
        if m["rec"] not in records:
            raise SpecError(f"{m['where']}: no record {m['rec']}")
        records[m["rec"]]["members"].append(m)

    return records, bits, addr_of, reserved, aliases


def generate(path, symbols, taken_names, guard, include, origin):
    """The header's lines and a report, or SpecError."""
    records, bits, addr_of, reserved, member_aliases = build(path, symbols, taken_names)
    claimed, report = {}, []
    out = [f"/* generated by {origin}; do not edit */"]
    all_syms = sorted((a, n) for a, n in symbols)
    out += [f"#ifndef {guard}", f"#define {guard}", "", "#include <stddef.h>", "#include <stdint.h>",
            f'#include "{include}"', "",
            "/* Wide fields: the bytes in storage order, each named. The order of an access is the macro's. */",
            "typedef struct { uint8_t lo, hi; } U16LE;", "typedef struct { uint8_t hi, lo; } U16BE;",
            "typedef struct { uint8_t lo, mid, hi; } U24LE;", "typedef struct { uint8_t hi, mid, lo; } U24BE;", ""]
    for sname in sorted(bits):
        out.append(f"/* bit set {sname} */")
        for bit, mask, note, ln in sorted(bits[sname], key=lambda b: (b[1], b[0])):
            out.append(f"#define {sname}_{bit} 0x{mask:02X} /* {note} */")
        out.append("")
    for rname in sorted(records, key=lambda k: records[k]["line"]):
        r = records[rname]
        ms = r["members"]
        soa = r["layout"] == "soa"
        for m in ms:
            w, shape = _width(m, r)
            m["w"], m["shape"] = w, shape
            if not IDENT.match(m["name"]):
                raise SpecError(f"{m['where']}: bad member name {m['name']}")
            if shape == "wide" and soa:
                raise SpecError(f"{m['where']}: a wide member cannot be a column; name its bytes and use the two-field macros")
            if shape == "array" and soa:
                raise SpecError(f"{m['where']}: an array member is for aos")
            if m["type"].startswith("bits:") and m["type"][5:] not in bits:
                raise SpecError(f"{m['where']}: no bit set {m['type'][5:]}")
            m["span"] = w * r["count"] if soa else w
            if not m["syms"]:
                raise SpecError(f"{m['where']}: member {m['name']} names no symbol")
            if len(m["syms"]) > (1 if soa else w):
                raise SpecError(f"{m['where']}: more symbols than bytes")
            for i, s in enumerate(m["syms"]):
                if s not in addr_of:
                    raise SpecError(f"{m['where']}: no symbol {s}")
                if s in claimed:
                    raise SpecError(f"{m['where']}: symbol {s} is already member {claimed[s]}")
                claimed[s] = f"{rname}.{m['name']}"
                want = r["base"] + m["offset"] + i
                if addr_of[s] != want:
                    raise SpecError(f"{m['where']}: {s} is at {addr_of[s]:04X}, the record puts {rname}.{m['name']}"
                                    f" byte {i} at {want:04X}")
            if m["offset"] + m["span"] > r["size"]:
                raise SpecError(f"{m['where']}: {m['name']} runs past the record's {r['size']:X} bytes")
        ms.sort(key=lambda m: (m["offset"], m["name"]))
        names = set()
        end = 0
        for m in ms:
            if m["name"] in names or m["name"].startswith("_gap"):
                raise SpecError(f"{m['where']}: member name {m['name']} twice or reserved")
            names.add(m["name"])
            if m["offset"] < end:
                raise SpecError(f"{m['where']}: {rname}.{m['name']} overlaps the member before it")
            end = m["offset"] + m["span"]
        # Symbols that sit inside a member without being one of its own bytes' names.
        aliases, inside_gap = [], 0
        lo_all, hi_all = r["base"], r["base"] + r["size"]
        for a, s in all_syms:
            if not (lo_all <= a < hi_all) or s in claimed or s in {x["symbol"] for x in member_aliases if x["symbol"]}:
                continue
            owner = None
            for m in ms:
                if r["base"] + m["offset"] <= a < r["base"] + m["offset"] + m["span"]:
                    owner = m
            if owner is None:
                inside_gap += 1
            elif owner["shape"] == "wide" and a != r["base"] + owner["offset"]:
                raise SpecError(f"{path}: symbol {s} ({a:04X}) lies inside {rname}.{owner['name']} and is not one of its bytes")
            else:
                aliases.append((s, f"{rname}.{owner['name']}"))
        for al in member_aliases:
            if al["rec"] == rname and re.split(r"[.+]", al["member"])[0] not in names:
                raise SpecError(f"{al['where']}: alias of {al['member']}, which is not a member of {rname}")
        # The struct.
        out.append(f"/* {rname}: {'parallel byte arrays' if soa else 'one struct'} at ${r['base']:04X}, "
                   f"{r['count']} {'slots' if soa else 'element(s)'}, {r['size']} bytes */")
        out.append(f"#define {rname}_BASE 0x{r['base']:04X}")
        out.append(f"#define {rname}_COUNT {r['count']}")
        out.append("typedef struct {")
        pos, gaps, gapbytes = 0, 0, 0
        for m in ms:
            if m["offset"] > pos:
                n = m["offset"] - pos
                out.append(f"    uint8_t _gap_{pos:04X}[{n}]; /* no name yet: ${r['base'] + pos:04X}-${r['base'] + pos + n - 1:04X} */")
                gaps += 1
                gapbytes += n
            t = m["type"]
            if soa:
                decl = f"uint8_t {m['name']}[{r['count']}]"
            elif m["shape"] == "array":
                decl = f"uint8_t {m['name']}[{m['w']}]"
            elif m["shape"] == "wide":
                decl = f"{WIDE[t][0]} {m['name']}"
            elif t == "s8":
                decl = f"int8_t {m['name']}"
            else:
                decl = f"uint8_t {m['name']}"
            subs = []
            for al in [x for x in member_aliases if x["rec"] == rname and re.split(r"[.+]", x["member"])[0] == m["name"]]:
                if not IDENT.match(al["alias"]) or al["alias"] in names or al["alias"] == m["name"]:
                    raise SpecError(f"{al['where']}: alias {al['alias']} is not a free name")
                names.add(al["alias"])
                m.setdefault("aliasnames", []).append(al["alias"])
                tm = re.match(r"^\w+(?:\.(\w+)|\+(\d+)(?:\[(\d+)\])?(?::(\w+))?)?$", al["member"])
                if tm is None:
                    raise SpecError(f"{al['where']}: cannot read alias target {al['member']}")
                if tm.group(1) or tm.group(2) is not None:
                    if tm.group(1):
                        if m["shape"] != "wide" or tm.group(1) not in WIDE[m["type"]][1]:
                            raise SpecError(f"{al['where']}: {m['name']} has no byte {tm.group(1)}")
                        idx, cnt = WIDE[m["type"]][1].index(tm.group(1)), 0
                    else:
                        if m["shape"] != "array":
                            raise SpecError(f"{al['where']}: {m['name']} is not an array")
                        idx, cnt = int(tm.group(2)), int(tm.group(3) or 0)
                        wt = tm.group(4)
                        if wt and wt not in WIDE:
                            raise SpecError(f"{al['where']}: {wt} is not a wide type")
                        if idx + (len(WIDE[wt][1]) if wt else max(cnt, 1)) > m["w"]:
                            raise SpecError(f"{al['where']}: {al['member']} runs past {m['name']}'s {m['w']} bytes")
                    if al["symbol"]:
                        if al["symbol"] not in addr_of:
                            raise SpecError(f"{al['where']}: no symbol {al['symbol']}")
                        want = r["base"] + m["offset"] + idx
                        if addr_of[al["symbol"]] != want:
                            raise SpecError(f"{al['where']}: {al['symbol']} is at {addr_of[al['symbol']]:04X}, {al['member']} of "
                                            f"{rname}.{m['name']} is at {want:04X}")
                        claimed[al["symbol"]] = f"{rname}.{al['alias']}"
                    sub_decl = ("%s %s" % (WIDE[wt][0], al["alias"])) if tm.group(4) else "uint8_t %s%s" % (al["alias"], ("[%d]" % cnt) if cnt else "")
                    subs.append("struct { %s%s; }" % (("uint8_t _pad_%s[%d]; " % (al["alias"], idx)) if idx else "", sub_decl))
                    m.setdefault("aliassyms", []).append((al["alias"], al["symbol"]))
                else:
                    decl = "union { %s; %s; }" % (decl, decl.replace(m["name"], al["alias"], 1))
                    if al["symbol"]:
                        m.setdefault("aliassyms", []).append((al["alias"], al["symbol"]))
            if subs:
                decl = "union { %s; %s; }" % (decl, "; ".join(subs))
            note = m["note"].strip()
            tail = f" /* {m['syms'][0]}{(' +' + str(len(m['syms']) - 1)) if len(m['syms']) > 1 else ''}"
            tail += (": " + note if note else "") + " */"
            out.append(f"    {decl};{tail}")
            pos = m["offset"] + m["span"]
        if r["size"] > pos:
            n = r["size"] - pos
            out.append(f"    uint8_t _gap_{pos:04X}[{n}]; /* no name yet: ${r['base'] + pos:04X}-${r['base'] + pos + n - 1:04X} */")
            gaps += 1
            gapbytes += n
        out.append(f"}} {rname};")
        out.append(f"_Static_assert(sizeof({rname}) == 0x{r['size']:X}, \"{rname} size\");")
        for m in ms:
            for i, s in enumerate(m["syms"]):
                for nm in [m["name"]] + [a for a in m.get("aliasnames", []) if a not in [x for x, _ in m.get("aliassyms", [])]]:
                    out.append(f"_Static_assert({rname}_BASE + offsetof({rname}, {nm}) + {i} == {s}, "
                               f"\"{rname}.{nm} is not at {s}\");")
                for nm, sy in m.get("aliassyms", []):
                    if i == 0 and sy:
                        out.append(f"_Static_assert({rname}_BASE + offsetof({rname}, {nm}) == {sy}, "
                                   f"\"{rname}.{nm} is not at {sy}\");")
        out.append(f"#define {r['ptr']} (({rname} *){rname}_BASE)")
        out.append("")
        nsym = sum(len(m["syms"]) for m in ms)
        report.append(f"{rname}: {len(ms)} members, {nsym} symbols, {gaps} gaps ({gapbytes} bytes), "
                      f"{inside_gap} plain names in its gaps, {len(aliases)} aliases"
                      + ("".join(f"\n    alias {s} in {where}" for s, where in aliases)))
    out += ["#endif", ""]
    return out, report, claimed


