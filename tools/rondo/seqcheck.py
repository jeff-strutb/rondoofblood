#!/usr/bin/env python3
"""seqcheck.py -- memory accesses C leaves unordered.

The checks compare the C's memory accesses with the original's, in order.
C fixes no order between the operands of most operators (+, |, <<, ==,
=, [] ...) nor between a call's arguments, so two accesses in such sibling
subexpressions of one full expression come out in whatever order the
compiler picks, which need not be the original's. This finds them.

It parses the C the decomp's files use: tokens, then statements and a
recursive-descent expression parser. A call touches the machine when it
calls a console access the headers declare (rd8, wr8 ... and every other
function declared in the include folders without a body there: the
machine's side of machine.h, rd8 and wr8 and the rest), calls through a pointer, or calls
a function defined in the C whose body does (to a fixed point). A
finding is an operator or call with two or more operands that each hold
such a call. Not findings: the operands of &&, || and the comma operator,
and ?:'s branches (each sequenced), nested calls (an argument runs before
its call), and separate declarators or statements.

    seqcheck.py                  every .c and .h of src/rondo and syscard (include folders too)
    seqcheck.py FILE...          these files (the definitions still come from all)
    seqcheck.py --list           also print the calls that touch the machine
Exit 1 when anything is found; each finding prints as path:line: text.
"""
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SRCS = ('src/rondo', 'syscard/src', 'syscard/include')
INCS = ('src/rondo/include/', 'syscard/include/')

PUNCT = sorted('''>>= <<= ... -> ++ -- << >> <= >= == != && || *= /= %= += -= &= ^= |= ##
    [ ] ( ) { } . & * + - ~ ! / % < > ^ | ? : ; = , #'''.split(), key=len, reverse=True)
TOKEN = re.compile(r'''
    (?P<ws>\s+)
  | (?P<comment>//[^\n]*|/\*.*?\*/)
  | (?P<str>"(?:\\.|[^"\\\n])*")
  | (?P<chr>'(?:\\.|[^'\\\n])+')
  | (?P<num>\.?[0-9](?:[eEpP][+-]|[0-9a-zA-Z_.])*)
  | (?P<id>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<p>''' + '|'.join(re.escape(p) for p in PUNCT) + r''')
''', re.S | re.X)

TYPE_WORDS = set('''void char short int long float double signed unsigned _Bool const volatile
    static extern inline register auto restrict struct union enum typedef _Noreturn'''.split())
BASE_TYPEDEFS = set('''uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t size_t
    uintptr_t intptr_t ptrdiff_t bool'''.split())
ASSIGN_OPS = set('= *= /= %= += -= <<= >>= &= ^= |='.split())
BINARY = [['||'], ['&&'], ['|'], ['^'], ['&'], ['==', '!='], ['<', '>', '<=', '>='],
          ['<<', '>>'], ['+', '-'], ['*', '/', '%']]
SEQUENCED = {'||', '&&', ','}


class Tok:
    __slots__ = ('k', 'v', 'line')

    def __init__(self, k, v, line):
        self.k, self.v, self.line = k, v, line

    def __repr__(self):
        return '%s@%d' % (self.v, self.line)


def strip_pp(text):
    """Blank preprocessor lines (with their continuations), keeping line numbers.
    Returns the text and the #define lines."""
    out, defines, lines, i = [], [], text.split('\n'), 0
    while i < len(lines):
        line = lines[i]
        if line.lstrip().startswith('#'):
            block = [line]
            while block[-1].endswith('\\') and i + 1 < len(lines):
                i += 1
                block.append(lines[i])
            defines.append(' '.join(b.rstrip('\\') for b in block))
            out.extend([''] * len(block))
        else:
            out.append(line)
        i += 1
    return '\n'.join(out), defines


def tokenize(text):
    toks, pos, line = [], 0, 1
    while pos < len(text):
        m = TOKEN.match(text, pos)
        if not m:
            raise SyntaxError('line %d: cannot read %r' % (line, text[pos:pos + 20]))
        k = m.lastgroup
        v = m.group(0)
        if k not in ('ws', 'comment'):
            toks.append(Tok(k, v, line))
        line += v.count('\n')
        pos = m.end()
    return toks


class Node:
    __slots__ = ('kind', 'op', 'kids', 'line', 'name')

    def __init__(self, kind, op, kids, line, name=None):
        self.kind, self.op, self.kids, self.line, self.name = kind, op, kids, line, name


class Parser:
    def __init__(self, toks, typedefs):
        self.t, self.i, self.typedefs = toks, 0, typedefs

    def peek(self, n=0):
        j = self.i + n
        return self.t[j] if j < len(self.t) else Tok('eof', '', self.t[-1].line if self.t else 0)

    def next(self):
        tok = self.peek()
        self.i += 1
        return tok

    def expect(self, v):
        tok = self.next()
        if tok.v != v:
            raise SyntaxError('line %d: expected %r, found %r' % (tok.line, v, tok.v))
        return tok

    def is_type_start(self, tok):
        return tok.k == 'id' and (tok.v in TYPE_WORDS or tok.v in self.typedefs)

    def skip_balanced(self):
        """Skip one token, or a whole (...), [...] or {...} group."""
        open_ = self.next().v
        close = {'(': ')', '[': ']', '{': '}'}.get(open_)
        if not close:
            return
        depth = 1
        while depth:
            v = self.next().v
            if v in '([{' and len(v) == 1:
                depth += 1
            elif v in ')]}' and len(v) == 1:
                depth -= 1
            elif v == '':
                raise SyntaxError('unbalanced %s' % open_)

    # ---- statements: yields each full expression (Node) ----
    def statement(self, out):
        tok = self.peek()
        v = tok.v
        if v == '{':
            self.next()
            while self.peek().v != '}':
                self.statement(out)
            self.next()
        elif v == ';':
            self.next()
        elif v in ('if', 'while', 'switch'):
            self.next()
            self.expect('(')
            out.append(self.expr())
            self.expect(')')
            self.statement(out)
            if v == 'if' and self.peek().v == 'else':
                self.next()
                self.statement(out)
        elif v == 'do':
            self.next()
            self.statement(out)
            self.expect('while')
            self.expect('(')
            out.append(self.expr())
            self.expect(')')
            self.expect(';')
        elif v == 'for':
            self.next()
            self.expect('(')
            if self.is_type_start(self.peek()):
                self.declaration(out)
            else:
                if self.peek().v != ';':
                    out.append(self.expr())
                self.expect(';')
            if self.peek().v != ';':
                out.append(self.expr())
            self.expect(';')
            if self.peek().v != ')':
                out.append(self.expr())
            self.expect(')')
            self.statement(out)
        elif v == 'case':
            self.next()
            self.cond()
            self.expect(':')
        elif v == 'default' and self.peek(1).v == ':':
            self.next()
            self.next()
        elif v == 'return':
            self.next()
            if self.peek().v != ';':
                out.append(self.expr())
            self.expect(';')
        elif v in ('break', 'continue'):
            self.next()
            self.expect(';')
        elif v == 'goto':
            self.next()
            self.next()
            self.expect(';')
        elif tok.k == 'id' and self.peek(1).v == ':' and v not in TYPE_WORDS:
            self.next()
            self.next()
        elif self.is_type_start(tok) and not (tok.v in self.typedefs and self.peek(1).v in ('(', '.', '->', '=')):
            self.declaration(out)
        else:
            out.append(self.expr())
            self.expect(';')

    def declaration(self, out):
        """Specifiers, then declarators; each initializer is a full expression."""
        while True:
            tok = self.peek()
            if tok.v in ('struct', 'union', 'enum'):
                self.next()
                if self.peek().k == 'id':
                    self.next()
                if self.peek().v == '{':
                    self.skip_balanced()
            elif self.is_type_start(tok):
                self.next()
            else:
                break
        while True:
            depth = 0
            while True:
                tok = self.peek()
                if depth == 0 and tok.v in ('=', ',', ';'):
                    break
                if tok.v in ('(', '['):
                    depth += 1
                elif tok.v in (')', ']'):
                    depth -= 1
                elif tok.k == 'eof':
                    raise SyntaxError('unterminated declaration')
                self.next()
            if self.peek().v == '=':
                self.next()
                out.append(self.initializer())
            if self.next().v == ';':
                return

    def initializer(self):
        if self.peek().v != '{':
            return self.assign()
        line = self.next().line
        kids = []
        while self.peek().v != '}':
            while self.peek().v in ('.', '['):  # designators
                if self.next().v == '.':
                    self.next()
                else:
                    self.cond()
                    self.expect(']')
                if self.peek().v == '=':
                    self.next()
            kids.append(self.initializer())
            if self.peek().v == ',':
                self.next()
        self.next()
        return Node('init', '{}', kids, line)

    # ---- expressions ----
    def expr(self):
        node = self.assign()
        while self.peek().v == ',':
            line = self.next().line
            node = Node('bin', ',', [node, self.assign()], line)
        return node

    def assign(self):
        node = self.cond()
        if self.peek().v in ASSIGN_OPS:
            tok = self.next()
            node = Node('assign', tok.v, [node, self.assign()], tok.line)
        return node

    def cond(self):
        node = self.binary(0)
        if self.peek().v == '?':
            line = self.next().line
            a = self.expr()
            self.expect(':')
            b = self.cond()
            node = Node('cond', '?:', [node, a, b], line)
        return node

    def binary(self, level):
        if level == len(BINARY):
            return self.unary()
        node = self.binary(level + 1)
        while self.peek().v in BINARY[level]:
            tok = self.next()
            node = Node('bin', tok.v, [node, self.binary(level + 1)], tok.line)
        return node

    def type_name(self):
        """After '(' at a type start: skip to the matching ')'."""
        depth = 1
        while depth:
            v = self.next().v
            if v == '(':
                depth += 1
            elif v == ')':
                depth -= 1

    def unary(self):
        tok = self.peek()
        if tok.v in ('++', '--', '&', '*', '+', '-', '~', '!'):
            self.next()
            return Node('un', tok.v, [self.unary()], tok.line)
        if tok.v == 'sizeof':
            self.next()
            if self.peek().v == '(' and self.is_type_start(self.peek(1)):
                self.next()
                self.type_name()
            else:
                self.unary()
            return Node('num', 'sizeof', [], tok.line)
        if tok.v == '(' and self.is_type_start(self.peek(1)):
            self.next()
            self.type_name()
            if self.peek().v == '{':
                return self.postfix(self.initializer())
            return Node('cast', '()', [self.unary()], tok.line)
        return self.postfix(self.primary())

    def primary(self):
        tok = self.next()
        if tok.k == 'id':
            return Node('id', tok.v, [], tok.line, tok.v)
        if tok.k in ('num', 'chr'):
            return Node('num', tok.v, [], tok.line)
        if tok.k == 'str':
            while self.peek().k == 'str':
                self.next()
            return Node('num', 'str', [], tok.line)
        if tok.v == '(':
            node = self.expr()
            self.expect(')')
            return node
        raise SyntaxError('line %d: unexpected %r' % (tok.line, tok.v))

    def postfix(self, node):
        while True:
            tok = self.peek()
            if tok.v == '(':
                self.next()
                args = []
                while self.peek().v != ')':
                    args.append(self.assign())
                    if self.peek().v == ',':
                        self.next()
                self.next()
                node = Node('call', '()', [node] + args, tok.line)
            elif tok.v == '[':
                self.next()
                idx = self.expr()
                self.expect(']')
                node = Node('bin', '[]', [node, idx], tok.line)
            elif tok.v in ('.', '->'):
                self.next()
                name = self.next().v
                node = Node('member', tok.v, [node], tok.line, name)
            elif tok.v in ('++', '--'):
                self.next()
                node = Node('un', 'post' + tok.v, [node], tok.line)
            else:
                return node


def read(path):
    with open(path, encoding='utf-8', errors='replace') as f:
        return f.read()


def typedef_names(texts):
    """Every typedef's name: the last identifier outside braces before its ';'
    (for a function pointer type, the one after '(*')."""
    names = set(BASE_TYPEDEFS)
    for text in texts:
        t = tokenize(strip_pp(text)[0])
        for i, tok in enumerate(t):
            if tok.v != 'typedef':
                continue
            depth, ids, j = 0, [], i + 1
            while j < len(t) and not (depth == 0 and t[j].v == ';'):
                if t[j].v == '{':
                    depth += 1
                elif t[j].v == '}':
                    depth -= 1
                elif depth == 0 and t[j].k == 'id':
                    if t[j - 1].v == '*' and t[j - 2].v == '(':
                        ids.append((1, t[j].v))
                    else:
                        ids.append((0, t[j].v))
                j += 1
            fp = [v for k, v in ids if k]
            names.add(fp[0] if fp else ids[-1][1])
    return names


class Unit:
    """A file's functions: name -> (tokens of the body, line), and its prototypes."""

    def __init__(self, rel, text, typedefs):
        self.rel = rel
        clean, self.defines = strip_pp(text)
        self.toks = tokenize(clean)
        self.typedefs = typedefs
        self.bodies = {}     # name -> (start, end) token span of the body, braces included
        self.protos = set()  # names declared with no body here
        t, i, depth = self.toks, 0, 0
        while i < len(t):
            v = t[i].v
            if v == '{':
                if i > 0 and t[i - 1].v == ')':
                    j, d = i - 1, 0
                    while True:
                        if t[j].v == ')':
                            d += 1
                        elif t[j].v == '(':
                            d -= 1
                            if d == 0:
                                break
                        j -= 1
                    name = t[j - 1].v
                    end = self.match(i)
                    self.bodies[name] = (i, end)
                    i = end + 1
                    continue
                i = self.match(i) + 1
                continue
            if v == '(' and i > 0 and t[i - 1].k == 'id' and t[i - 1].v not in TYPE_WORDS:
                end = self.match(i)
                if end + 1 < len(t) and t[end + 1].v == ';':
                    self.protos.add(t[i - 1].v)
                i = end + 1
                continue
            i += 1

    def match(self, i):
        t, depth, open_ = self.toks, 0, self.toks[i].v
        close = {'(': ')', '{': '}', '[': ']'}[open_]
        while True:
            if t[i].v == open_:
                depth += 1
            elif t[i].v == close:
                depth -= 1
                if depth == 0:
                    return i
            i += 1

    def full_exprs(self, name):
        s, e = self.bodies[name]
        p = Parser(self.toks[s:e + 1], self.typedefs)
        out = []
        try:
            p.statement(out)
        except SyntaxError as e:
            raise SyntaxError('%s: %s: %s' % (self.rel, name, e))
        return out


def walk(node):
    yield node
    for k in node.kids:
        yield from walk(k)


def callee_name(call):
    f = call.kids[0]
    return f.name if f.kind == 'id' else None


class Checker:
    def __init__(self, root=ROOT, staged=None):
        """staged: rel -> text, read in place of the work tree's files."""
        self.root = root
        staged = staged or {}
        files = []
        for src in SRCS:
            for dp, dn, fn in os.walk(os.path.join(root, src)):
                for f in sorted(fn):
                    if f.endswith(('.c', '.h')):
                        files.append(os.path.relpath(os.path.join(dp, f), root))
        self.files = sorted(set(files) | set(staged))
        texts = {rel: staged[rel] if rel in staged else read(os.path.join(root, rel)) for rel in self.files}
        self.typedefs = typedef_names(texts.values())
        self.units = {rel: Unit(rel, texts[rel], self.typedefs) for rel in self.files}
        # Definitions: a file's own (static) functions first, then the shared
        # headers' inline ones, then other files' (the linker's).
        self.defined = {}
        for rel in sorted(self.units, key=lambda r: (not r.startswith(INCS), r)):
            for name in self.units[rel].bodies:
                self.defined.setdefault(name, rel)
        # The console's accesses: declared in the shared headers with no body here.
        self.console = set()
        for rel, u in self.units.items():
            if rel.startswith(INCS):
                self.console |= {n for n in u.protos if n not in self.defined}
        macros = {}
        for u in self.units.values():
            for d in u.defines:
                m = re.match(r'\s*#\s*define\s+([A-Za-z_]\w*)\(([^)]*)\)(.*)', d)
                if m:
                    macros.setdefault(m.group(1), set(re.findall(r'[A-Za-z_]\w*', m.group(3))))
        self.macro_names = set(macros)
        self.exprs, self.calls_of = {}, {}
        for rel, u in self.units.items():
            for name in u.bodies:
                ex = u.full_exprs(name)
                self.exprs[(rel, name)] = ex
                self.calls_of[(rel, name)] = {self.resolve(u, callee_name(n)) for x in ex for n in walk(x)
                                              if n.kind == 'call'}
        # Function-like macros (ops65.h's ACC8, MEM8 ...): the functions their bodies name.
        for name, ids in macros.items():
            self.calls_of[('#', name)] = {self.resolve(None, i) for i in ids
                                          if i in self.defined or i in self.console or i in macros}
            self.calls_of[('#', name)].discard(('#', name))
        self.unknown = {k[1] for calls in self.calls_of.values() for k in calls if k[0] == '?'}
        # A pointer call, a console access, or a name defined nowhere here (a
        # parameter holding a routine) touches the machine; so does whatever calls one.
        self.touch = {k for calls in self.calls_of.values() for k in calls if k[0] in '*!?'}
        changed = True
        while changed:
            changed = False
            for key, calls in self.calls_of.items():
                if key not in self.touch and calls & self.touch:
                    self.touch.add(key)
                    changed = True

    def resolve(self, unit, name):
        if name is None:
            return ('*', '')
        if unit is not None and name in unit.bodies:
            return (unit.rel, name)
        if name in self.defined:
            return (self.defined[name], name)
        if name in self.console:
            return ('!', name)
        if name in self.macro_names:
            return ('#', name)
        return ('?', name)

    def touching_calls(self, node, unit):
        if node.kind == 'num':
            return []
        out = []
        if node.kind == 'call':
            key = self.resolve(unit, callee_name(node))
            if key in self.touch:
                out.append(key[1] or '(*)')
        for k in node.kids:
            out += self.touching_calls(k, unit)
        return out

    def findings_in(self, node, unit):
        """(line, op, [calls of each touching operand]) for every unordered group."""
        found = []
        for n in walk(node):
            if n.kind in ('bin', 'assign', 'call', 'init') and n.op not in SEQUENCED:
                groups = [c for c in (self.touching_calls(k, unit) for k in n.kids) if c]
                if len(groups) >= 2:
                    found.append((n.line, n.op, groups))
        return found

    def check(self, rels=None):
        out = []
        for rel in (rels or self.files):
            u = self.units[rel]
            for name in u.bodies:
                for x in self.exprs[(rel, name)]:
                    for line, op, groups in self.findings_in(x, u):
                        out.append((rel, line, name, op, groups))
        return sorted(set((r, l, n, o, tuple(tuple(g) for g in gs)) for r, l, n, o, gs in out))


def check_texts(texts, root=ROOT):
    """For the commit check: findings in these files' staged texts (rel -> text),
    with every definition read from the staged texts and the rest of the tree."""
    return Checker(root, texts).check(list(texts))


def fmt(f):
    rel, line, name, op, groups = f
    return '%s:%d: %s: %s with operands calling %s' % (
        rel, line, name, {'()': 'call', '[]': 'index', '{}': 'initializer'}.get(op, op),
        ' | '.join(','.join(g) for g in groups))


def main(argv):
    lst = '--list' in argv
    args = [a for a in argv if a != '--list']
    ck = Checker()
    if lst:
        print('console accesses:', ' '.join(sorted(ck.console)))
        print('called, defined nowhere here:', ' '.join(sorted(ck.unknown)))
        print('touching functions defined here: %d of %d' % (
            len([k for k in ck.touch if k in ck.exprs]), len(ck.exprs)))
        print('macros that touch:', ' '.join(sorted(k[1] for k in ck.touch if k[0] == '#')))
    rels = [os.path.relpath(os.path.abspath(a), ROOT) for a in args] or None
    found = ck.check(rels)
    for f in found:
        print(fmt(f))
    stmts = {(f[0], f[1]) for f in found}
    print('seqcheck: %d unordered groups on %d lines in %d files' % (
        len(found), len(stmts), len({f[0] for f in found})), file=sys.stderr)
    return 1 if found else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
