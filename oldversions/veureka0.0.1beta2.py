#!/usr/bin/env python3
"""
Veureka - Un linguaggio di programmazione moderno, semplice ma potente.

Uso:
    python veureka.py script.ver          # Esegue un file
    python veureka.py                      # Avvia REPL interattivo
    python veureka.py --examples           # Esegue gli esempi
"""

import sys
import re
import os
from enum import Enum, auto
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Callable

# ============ LEXER ============

class TT(Enum):
    # Keywords
    LET = auto()
    FN = auto()
    CLASS = auto()
    NEW = auto()
    SELF = auto()
    IF = auto()
    ELIF = auto()
    ELSE = auto()
    FOR = auto()
    IN = auto()
    WHILE = auto()
    RETURN = auto()
    BREAK = auto()
    CONTINUE = auto()
    MATCH = auto()
    CASE = auto()
    END = auto()
    TRUE = auto()
    FALSE = auto()
    NIL = auto()
    AND = auto()
    OR = auto()
    NOT = auto()
    INCLUDE = auto()
    
    # Literals
    NUMBER = auto()
    STRING = auto()
    IDENT = auto()
    
    # Operators
    PLUS = auto()
    MINUS = auto()
    STAR = auto()
    SLASH = auto()
    PERCENT = auto()
    POWER = auto()
    
    EQ = auto()
    NE = auto()
    LT = auto()
    LE = auto()
    GT = auto()
    GE = auto()
    
    ASSIGN = auto()
    PLUS_EQ = auto()
    MINUS_EQ = auto()
    STAR_EQ = auto()
    SLASH_EQ = auto()
    INCREMENT = auto()
    DECREMENT = auto()
    ARROW = auto()
    
    # Delimiters
    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    LBRACKET = auto()
    RBRACKET = auto()
    COMMA = auto()
    COLON = auto()
    DOT = auto()
    NEWLINE = auto()
    
    EOF = auto()

@dataclass
class Token:
    type: TT
    value: Any
    line: int
    col: int

class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.col = 1
        
        self.keywords = {
            'let': TT.LET, 'fn': TT.FN, 'class': TT.CLASS, 'new': TT.NEW,
            'self': TT.SELF, 'if': TT.IF, 'elif': TT.ELIF,
            'else': TT.ELSE, 'for': TT.FOR, 'in': TT.IN, 'while': TT.WHILE,
            'return': TT.RETURN, 'break': TT.BREAK, 'continue': TT.CONTINUE,
            'match': TT.MATCH, 'case': TT.CASE, 'end': TT.END,
            'true': TT.TRUE, 'false': TT.FALSE, 'nil': TT.NIL,
            'and': TT.AND, 'or': TT.OR, 'not': TT.NOT, 'include': TT.INCLUDE
        }
    
    def tokenize(self) -> List[Token]:
        tokens = []
        while self.pos < len(self.source):
            self.skip_whitespace_except_newline()
            
            if self.pos >= len(self.source):
                break
            
            # Comments
            if self.current() == '#':
                self.skip_line()
                continue
            
            # Newlines (significativi)
            if self.current() == '\n':
                tokens.append(Token(TT.NEWLINE, '\n', self.line, self.col))
                self.advance()
                continue
            
            char = self.current()
            
            # Numbers
            if char.isdigit():
                tokens.append(self.read_number())
            # Strings
            elif char in '"\'':
                tokens.append(self.read_string())
            # Identifiers/Keywords
            elif char.isalpha() or char == '_':
                tokens.append(self.read_identifier())
            # Operators
            elif char == '+':
                if self.peek() == '+':
                    self.advance()
                    tokens.append(self.make_token(TT.INCREMENT, '++'))
                elif self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.PLUS_EQ, '+='))
                else:
                    tokens.append(self.make_token(TT.PLUS, '+'))
            elif char == '-':
                if self.peek() == '-':
                    self.advance()
                    tokens.append(self.make_token(TT.DECREMENT, '--'))
                elif self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.MINUS_EQ, '-='))
                else:
                    tokens.append(self.make_token(TT.MINUS, '-'))
            elif char == '*':
                if self.peek() == '*':
                    self.advance()
                    tokens.append(self.make_token(TT.POWER, '**'))
                elif self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.STAR_EQ, '*='))
                else:
                    tokens.append(self.make_token(TT.STAR, '*'))
            elif char == '/':
                if self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.SLASH_EQ, '/='))
                else:
                    tokens.append(self.make_token(TT.SLASH, '/'))
            elif char == '%':
                tokens.append(self.make_token(TT.PERCENT, '%'))
            elif char == '=':
                if self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.EQ, '=='))
                elif self.peek() == '>':
                    self.advance()
                    tokens.append(self.make_token(TT.ARROW, '=>'))
                else:
                    tokens.append(self.make_token(TT.ASSIGN, '='))
            elif char == '!':
                if self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.NE, '!='))
            elif char == '<':
                if self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.LE, '<='))
                else:
                    tokens.append(self.make_token(TT.LT, '<'))
            elif char == '>':
                if self.peek() == '=':
                    self.advance()
                    tokens.append(self.make_token(TT.GE, '>='))
                else:
                    tokens.append(self.make_token(TT.GT, '>'))
            # Delimiters
            elif char == '(':
                tokens.append(self.make_token(TT.LPAREN, '('))
            elif char == ')':
                tokens.append(self.make_token(TT.RPAREN, ')'))
            elif char == '{':
                tokens.append(self.make_token(TT.LBRACE, '{'))
            elif char == '}':
                tokens.append(self.make_token(TT.RBRACE, '}'))
            elif char == '[':
                tokens.append(self.make_token(TT.LBRACKET, '['))
            elif char == ']':
                tokens.append(self.make_token(TT.RBRACKET, ']'))
            elif char == ',':
                tokens.append(self.make_token(TT.COMMA, ','))
            elif char == ':':
                tokens.append(self.make_token(TT.COLON, ':'))
            elif char == '.':
                tokens.append(self.make_token(TT.DOT, '.'))
            else:
                raise SyntaxError(f"!! Carattere inaspettato '{char}' alla riga {self.line}:{self.col}")
        
        tokens.append(Token(TT.EOF, None, self.line, self.col))
        return tokens
    
    def make_token(self, ttype: TT, value: Any) -> Token:
        token = Token(ttype, value, self.line, self.col)
        self.advance()
        return token
    
    def current(self) -> str:
        return self.source[self.pos] if self.pos < len(self.source) else '\0'
    
    def peek(self, offset: int = 1) -> str:
        pos = self.pos + offset
        return self.source[pos] if pos < len(self.source) else '\0'
    
    def advance(self):
        if self.pos < len(self.source):
            if self.source[self.pos] == '\n':
                self.line += 1
                self.col = 1
            else:
                self.col += 1
            self.pos += 1
    
    def skip_whitespace_except_newline(self):
        while self.current() in ' \t\r':
            self.advance()
    
    def skip_line(self):
        while self.current() != '\n' and self.current() != '\0':
            self.advance()
    
    def read_number(self) -> Token:
        num_str = ""
        start_col = self.col
        has_dot = False
        
        while self.current().isdigit() or (self.current() == '.' and not has_dot):
            if self.current() == '.':
                has_dot = True
            num_str += self.current()
            self.advance()
        
        value = float(num_str) if has_dot else int(num_str)
        return Token(TT.NUMBER, value, self.line, start_col)
    
    def read_string(self) -> Token:
        quote = self.current()
        start_col = self.col
        self.advance()
        
        string = ""
        while self.current() != quote and self.current() != '\0':
            if self.current() == '\\':
                self.advance()
                if self.current() == 'n':
                    string += '\n'
                elif self.current() == 't':
                    string += '\t'
                elif self.current() == '\\':
                    string += '\\'
                elif self.current() == quote:
                    string += quote
                self.advance()
            else:
                string += self.current()
                self.advance()
        
        self.advance()  # Skip closing quote
        return Token(TT.STRING, string, self.line, start_col)
    
    def read_identifier(self) -> Token:
        ident = ""
        start_col = self.col
        
        while self.current().isalnum() or self.current() == '_':
            ident += self.current()
            self.advance()
        
        token_type = self.keywords.get(ident, TT.IDENT)
        value = ident if token_type == TT.IDENT else ident
        
        return Token(token_type, value, self.line, start_col)

# ============ AST ============

@dataclass
class Node:
    pass

@dataclass
class Program(Node):
    statements: List[Node]

@dataclass
class IncludeStmt(Node):
    path: str

@dataclass
class LetStmt(Node):
    name: str
    value: Node

@dataclass
class FnDef(Node):
    name: Optional[str]
    params: List[str]
    body: List[Node]

@dataclass
class IfStmt(Node):
    condition: Node
    then_body: List[Node]
    elif_parts: List[tuple]  # [(condition, body), ...]
    else_body: Optional[List[Node]]

@dataclass
class ForStmt(Node):
    var: str
    iterable: Node
    body: List[Node]

@dataclass
class WhileStmt(Node):
    condition: Node
    body: List[Node]

@dataclass
class ReturnStmt(Node):
    value: Optional[Node]

@dataclass
class BreakStmt(Node):
    pass

@dataclass
class ContinueStmt(Node):
    pass

@dataclass
class BinaryOp(Node):
    left: Node
    op: str
    right: Node

@dataclass
class UnaryOp(Node):
    op: str
    operand: Node

@dataclass
class Call(Node):
    func: Node
    args: List[Node]

@dataclass
class Index(Node):
    obj: Node
    index: Node

@dataclass
class Attr(Node):
    obj: Node
    attr: str

@dataclass
class Literal(Node):
    value: Any

@dataclass
class Var(Node):
    name: str

@dataclass
class ListLit(Node):
    elements: List[Node]

@dataclass
class ClassDef(Node):
    name: str
    methods: Dict[str, Node]  # {method_name: FnDef}

@dataclass
class NewInstance(Node):
    class_name: str
    args: List[Node]

@dataclass
class AttrAssign(Node):
    obj: Node
    attr: str
    value: Node

@dataclass
class CompoundAssign(Node):
    name: str
    op: str  # +=, -=, *=, /=
    value: Node

@dataclass
class IncrementDecrement(Node):
    target: Node  # Può essere Var o Attr
    op: str  # ++ or --
    prefix: bool  # True se ++x, False se x++

@dataclass
class MapLit(Node):
    pairs: List[tuple]  # [(key, value), ...]

# ============ PARSER ============

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = [t for t in tokens if t.type != TT.NEWLINE]  # Ignora newlines per semplicità
        self.pos = 0
    
    def parse(self) -> Program:
        statements = []
        while not self.is_at_end():
            statements.append(self.parse_statement())
        return Program(statements)
    
    def parse_statement(self) -> Node:
        if self.match(TT.INCLUDE):
            return self.parse_include()
        elif self.match(TT.LET):
            return self.parse_let()
        elif self.match(TT.FN):
            return self.parse_function()
        elif self.match(TT.CLASS):
            return self.parse_class()
        elif self.match(TT.IF):
            return self.parse_if()
        elif self.match(TT.FOR):
            return self.parse_for()
        elif self.match(TT.WHILE):
            return self.parse_while()
        elif self.match(TT.RETURN):
            return self.parse_return()
        elif self.match(TT.BREAK):
            return BreakStmt()
        elif self.match(TT.CONTINUE):
            return ContinueStmt()
        else:
            return self.parse_expression()
    
    def parse_include(self) -> IncludeStmt:
        # include può essere sia include "path" che include("path")
        if self.check(TT.STRING):
            path = self.consume(TT.STRING).value
        elif self.match(TT.LPAREN):
            path = self.consume(TT.STRING).value
            self.consume(TT.RPAREN)
        else:
            raise Exception("Expected file path after 'include'")
        return IncludeStmt(path)
    
    def parse_class(self) -> ClassDef:
        name = self.consume(TT.IDENT).value
        methods = {}
        
        while not self.check(TT.END):
            if self.match(TT.FN):
                func = self.parse_function()
                if func.name:
                    methods[func.name] = func
        
        self.consume(TT.END)
        return ClassDef(name, methods)
    
    def parse_let(self) -> LetStmt:
        name = self.consume(TT.IDENT).value
        self.consume(TT.ASSIGN)
        value = self.parse_expression()
        return LetStmt(name, value)
    
    def parse_function(self) -> FnDef:
        name = None
        if self.current().type == TT.IDENT:
            name = self.advance().value
        
        self.consume(TT.LPAREN)
        params = []
        while not self.check(TT.RPAREN):
            params.append(self.consume(TT.IDENT).value)
            if not self.check(TT.RPAREN):
                self.consume(TT.COMMA)
        self.consume(TT.RPAREN)
        
        # Lambda arrow syntax
        if self.match(TT.ARROW):
            expr = self.parse_expression()
            return FnDef(name, params, [ReturnStmt(expr)])
        
        # Block syntax
        body = []
        while not self.check(TT.END):
            body.append(self.parse_statement())
        self.consume(TT.END)
        
        return FnDef(name, params, body)
    
    def parse_if(self) -> IfStmt:
        condition = self.parse_expression()
        then_body = []
        while not self.check(TT.ELIF) and not self.check(TT.ELSE) and not self.check(TT.END):
            then_body.append(self.parse_statement())
        
        elif_parts = []
        while self.match(TT.ELIF):
            elif_cond = self.parse_expression()
            elif_body = []
            while not self.check(TT.ELIF) and not self.check(TT.ELSE) and not self.check(TT.END):
                elif_body.append(self.parse_statement())
            elif_parts.append((elif_cond, elif_body))
        
        else_body = None
        if self.match(TT.ELSE):
            else_body = []
            while not self.check(TT.END):
                else_body.append(self.parse_statement())
        
        self.consume(TT.END)
        return IfStmt(condition, then_body, elif_parts, else_body)
    
    def parse_for(self) -> ForStmt:
        var = self.consume(TT.IDENT).value
        self.consume(TT.IN)
        iterable = self.parse_expression()
        
        body = []
        while not self.check(TT.END):
            body.append(self.parse_statement())
        self.consume(TT.END)
        
        return ForStmt(var, iterable, body)
    
    def parse_while(self) -> WhileStmt:
        condition = self.parse_expression()
        body = []
        while not self.check(TT.END):
            body.append(self.parse_statement())
        self.consume(TT.END)
        return WhileStmt(condition, body)
    
    def parse_return(self) -> ReturnStmt:
        if self.check(TT.END) or self.is_at_end():
            return ReturnStmt(None)
        return ReturnStmt(self.parse_expression())
    
    def parse_expression(self) -> Node:
        return self.parse_assignment()
    
    def parse_assignment(self) -> Node:
        expr = self.parse_or()
        
        # Assignment ad attributo: obj.attr = value
        if isinstance(expr, Attr):
            if self.match(TT.ASSIGN):
                value = self.parse_assignment()
                return AttrAssign(expr.obj, expr.attr, value)
            elif self.current().type in [TT.PLUS_EQ, TT.MINUS_EQ, TT.STAR_EQ, TT.SLASH_EQ]:
                # obj.attr += value
                op = self.advance().value
                value = self.parse_assignment()
                # Trasforma in obj.attr = obj.attr + value
                current_value = expr
                if op == '+=':
                    new_value = BinaryOp(current_value, '+', value)
                elif op == '-=':
                    new_value = BinaryOp(current_value, '-', value)
                elif op == '*=':
                    new_value = BinaryOp(current_value, '*', value)
                elif op == '/=':
                    new_value = BinaryOp(current_value, '/', value)
                return AttrAssign(expr.obj, expr.attr, new_value)
        
        # Supporta assignment a variabile: x = x + 1
        if isinstance(expr, Var):
            # Operatori composti: +=, -=, *=, /=
            if self.current().type in [TT.PLUS_EQ, TT.MINUS_EQ, TT.STAR_EQ, TT.SLASH_EQ]:
                op = self.advance().value
                value = self.parse_assignment()
                return CompoundAssign(expr.name, op, value)
            # Assignment normale
            elif self.match(TT.ASSIGN):
                value = self.parse_assignment()
                return LetStmt(expr.name, value)
        
        return expr
    
    def parse_or(self) -> Node:
        left = self.parse_and()
        while self.match(TT.OR):
            op = 'or'
            right = self.parse_and()
            left = BinaryOp(left, op, right)
        return left
    
    def parse_and(self) -> Node:
        left = self.parse_comparison()
        while self.match(TT.AND):
            op = 'and'
            right = self.parse_comparison()
            left = BinaryOp(left, op, right)
        return left
    
    def parse_comparison(self) -> Node:
        left = self.parse_additive()
        
        while self.current().type in [TT.EQ, TT.NE, TT.LT, TT.LE, TT.GT, TT.GE]:
            op = self.advance().value
            right = self.parse_additive()
            left = BinaryOp(left, op, right)
        
        return left
    
    def parse_additive(self) -> Node:
        left = self.parse_multiplicative()
        
        while self.current().type in [TT.PLUS, TT.MINUS]:
            op = self.advance().value
            right = self.parse_multiplicative()
            left = BinaryOp(left, op, right)
        
        return left
    
    def parse_multiplicative(self) -> Node:
        left = self.parse_power()
        
        while self.current().type in [TT.STAR, TT.SLASH, TT.PERCENT]:
            op = self.advance().value
            right = self.parse_power()
            left = BinaryOp(left, op, right)
        
        return left
    
    def parse_power(self) -> Node:
        left = self.parse_unary()
        
        if self.match(TT.POWER):
            right = self.parse_power()  # Right associative
            return BinaryOp(left, '**', right)
        
        return left
    
    def parse_unary(self) -> Node:
        # Prefix increment/decrement
        if self.current().type in [TT.INCREMENT, TT.DECREMENT]:
            op = self.advance().value
            # Parsare l'operando (può essere var o attr)
            operand = self.parse_postfix()
            if isinstance(operand, (Var, Attr)):
                return IncrementDecrement(operand, op, prefix=True)
            raise SyntaxError("!! ++ e -- richiedono una variabile o attributo")
        
        if self.current().type in [TT.MINUS, TT.NOT]:
            op = self.advance().value
            if op == 'not':
                op = 'not'
            operand = self.parse_unary()
            return UnaryOp(op, operand)
        
        return self.parse_postfix()
    
    def parse_postfix(self) -> Node:
        expr = self.parse_primary()
        
        while True:
            if self.match(TT.LPAREN):
                args = []
                while not self.check(TT.RPAREN):
                    args.append(self.parse_expression())
                    if not self.check(TT.RPAREN):
                        self.consume(TT.COMMA)
                self.consume(TT.RPAREN)
                expr = Call(expr, args)
            elif self.match(TT.LBRACKET):
                index = self.parse_expression()
                self.consume(TT.RBRACKET)
                expr = Index(expr, index)
            elif self.match(TT.DOT):
                attr = self.consume(TT.IDENT).value
                expr = Attr(expr, attr)
            # Postfix increment/decrement
            elif self.current().type in [TT.INCREMENT, TT.DECREMENT]:
                if isinstance(expr, (Var, Attr)):
                    op = self.advance().value
                    expr = IncrementDecrement(expr, op, prefix=False)
                else:
                    break
            else:
                break
        
        return expr
    
    def parse_primary(self) -> Node:
        if self.match(TT.NUMBER):
            return Literal(self.previous().value)
        elif self.match(TT.STRING):
            return Literal(self.previous().value)
        elif self.match(TT.TRUE):
            return Literal(True)
        elif self.match(TT.FALSE):
            return Literal(False)
        elif self.match(TT.NIL):
            return Literal(None)
        elif self.match(TT.SELF):
            return Var('self')
        elif self.match(TT.NEW):
            class_name = self.consume(TT.IDENT).value
            self.consume(TT.LPAREN)
            args = []
            while not self.check(TT.RPAREN):
                args.append(self.parse_expression())
                if not self.check(TT.RPAREN):
                    self.consume(TT.COMMA)
            self.consume(TT.RPAREN)
            return NewInstance(class_name, args)
        elif self.match(TT.IDENT):
            return Var(self.previous().value)
        elif self.match(TT.LPAREN):
            expr = self.parse_expression()
            self.consume(TT.RPAREN)
            return expr
        elif self.match(TT.LBRACKET):
            return self.parse_list()
        elif self.match(TT.LBRACE):
            return self.parse_map()
        elif self.match(TT.FN):
            return self.parse_function()
        
        raise SyntaxError(f"!! Espressione inaspettata: {self.current().value}")
    
    def parse_list(self) -> ListLit:
        elements = []
        while not self.check(TT.RBRACKET):
            elements.append(self.parse_expression())
            if not self.check(TT.RBRACKET):
                self.consume(TT.COMMA)
        self.consume(TT.RBRACKET)
        return ListLit(elements)
    
    def parse_map(self) -> MapLit:
        pairs = []
        while not self.check(TT.RBRACE):
            if self.current().type == TT.IDENT:
                key = self.advance().value
            elif self.current().type == TT.STRING:
                key = self.advance().value
            else:
                raise SyntaxError("!! Chiave mappa deve essere identificatore o stringa")
            
            self.consume(TT.COLON)
            value = self.parse_expression()
            pairs.append((key, value))
            
            if not self.check(TT.RBRACE):
                self.consume(TT.COMMA)
        self.consume(TT.RBRACE)
        return MapLit(pairs)
    
    def current(self) -> Token:
        return self.tokens[self.pos]
    
    def previous(self) -> Token:
        return self.tokens[self.pos - 1]
    
    def advance(self) -> Token:
        if not self.is_at_end():
            self.pos += 1
        return self.previous()
    
    def match(self, *types: TT) -> bool:
        if self.current().type in types:
            self.advance()
            return True
        return False
    
    def check(self, ttype: TT) -> bool:
        return self.current().type == ttype
    
    def consume(self, ttype: TT) -> Token:
        if self.current().type != ttype:
            raise SyntaxError(f"!! Atteso {ttype}, trovato {self.current().type}")
        return self.advance()
    
    def is_at_end(self) -> bool:
        return self.current().type == TT.EOF

# ============ INTERPRETER ============

class BreakException(Exception):
    pass

class ContinueException(Exception):
    pass

class ReturnException(Exception):
    def __init__(self, value):
        self.value = value

class VerClass:
    def __init__(self, name: str, methods: Dict[str, 'VerFunction']):
        self.name = name
        self.methods = methods
    
    def instantiate(self, interpreter, args: List[Any]):
        instance = VerInstance(self)
        # Se esiste un costruttore __init__, chiamalo
        if '__init__' in self.methods:
            init_method = self.methods['__init__']
            # Crea una versione bound del metodo con self
            bound_init = self.bind_method(init_method, instance)
            interpreter.call_function(bound_init, args)
        return instance
    
    def bind_method(self, method: 'VerFunction', instance: 'VerInstance') -> 'VerFunction':
        # Crea un nuovo scope con self
        new_closure = method.closure.copy()
        new_closure['self'] = instance
        return VerFunction(method.params, method.body, new_closure)

class VerInstance:
    def __init__(self, ver_class: VerClass):
        self.ver_class = ver_class
        self.fields = {}
    
    def get(self, name: str):
        # Prima cerca nei campi dell'istanza
        if name in self.fields:
            return self.fields[name]
        # Poi cerca nei metodi della classe
        if name in self.ver_class.methods:
            return self.ver_class.bind_method(self.ver_class.methods[name], self)
        raise AttributeError(f"'{self.ver_class.name}' non ha attributo '{name}'")
    
    def set(self, name: str, value: Any):
        self.fields[name] = value
    
    def __repr__(self):
        return f"<{self.ver_class.name} instance>"

class VerFunction:
    def __init__(self, params: List[str], body: List[Node], closure: Dict):
        self.params = params
        self.body = body
        self.closure = closure  # Ora è un riferimento, non una copia!

class Interpreter:
    def __init__(self):
        self.globals = {
            'print': lambda *args: print(*args),
            'len': len,
            'range': lambda *args: list(range(*args)),
            'str': str,
            'int': int,
            'float': float,
            'type': lambda x: type(x).__name__,
            'input': input,
            'map': self.builtin_map,
            'filter': self.builtin_filter,
            'reduce': self.builtin_reduce,
            'sum': sum,
            'max': max,
            'min': min,
            'abs': abs,
        }
        self.locals_stack = [{}]
    
    def builtin_map(self, iterable, func):
        result = []
        for item in iterable:
            result.append(self.call_function(func, [item]))
        return result
    
    def builtin_filter(self, iterable, func):
        result = []
        for item in iterable:
            if self.call_function(func, [item]):
                result.append(item)
        return result
    
    def builtin_reduce(self, iterable, func, initial=None):
        it = iter(iterable)
        if initial is None:
            acc = next(it)
        else:
            acc = initial
        
        for item in it:
            acc = self.call_function(func, [acc, item])
        return acc
    
    def load_library(self, filepath: str):
        """Carica una libreria da un file .ver"""
        # Gestisci estensioni con .ver o senza
        if not filepath.endswith('.ver'):
            filepath_with_ext = filepath + '.ver'
        else:
            filepath_with_ext = filepath
        
        # Cerca il file nella cartella corrente o nelle cartelle standard
        search_paths = [
            filepath_with_ext,
            os.path.join(os.getcwd(), filepath_with_ext),
            os.path.join(os.path.dirname(__file__), 'lib', filepath_with_ext),
        ]
        
        file_path = None
        for path in search_paths:
            if os.path.exists(path):
                file_path = path
                break
        
        if not file_path:
            raise FileNotFoundError(f"!! Libreria '{filepath}' non trovata. Cercato in: {search_paths}")
        
        # Leggi e esegui il file
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                source = f.read()
            
            # Tokenizza, analizza ed esegui
            lexer = Lexer(source)
            tokens = lexer.tokenize()
            parser = Parser(tokens)
            program = parser.parse()
            
            # Esegui il programma della libreria nel contesto globale
            for stmt in program.statements:
                self.execute(stmt)
                
        except Exception as e:
            raise RuntimeError(f"!! Errore nel caricamento della libreria '{filepath}': {e}")
    
    def run(self, program: Program):
        for stmt in program.statements:
            self.execute(stmt)
    
    def execute(self, node: Node) -> Any:
        if isinstance(node, Program):
            for stmt in node.statements:
                self.execute(stmt)
        
        elif isinstance(node, IncludeStmt):
            # Carica una libreria da file
            self.load_library(node.path)
        
        elif isinstance(node, ClassDef):
            # Converti i metodi in VerFunction
            methods = {}
            for method_name, method_def in node.methods.items():
                methods[method_name] = VerFunction(
                    method_def.params, 
                    method_def.body, 
                    self.locals_stack[-1]
                )
            # Crea la classe
            ver_class = VerClass(node.name, methods)
            self.locals_stack[-1][node.name] = ver_class
        
        elif isinstance(node, NewInstance):
            # Ottieni la classe
            if node.class_name not in self.locals_stack[-1] and node.class_name not in self.globals:
                raise NameError(f"!! Classe '{node.class_name}' non definita")
            
            ver_class = None
            for scope in reversed(self.locals_stack):
                if node.class_name in scope:
                    ver_class = scope[node.class_name]
                    break
            if not ver_class:
                ver_class = self.globals.get(node.class_name)
            
            if not isinstance(ver_class, VerClass):
                raise TypeError(f"'{node.class_name}' non è una classe")
            
            # Valuta gli argomenti
            args = [self.execute(arg) for arg in node.args]
            # Crea l'istanza
            return ver_class.instantiate(self, args)
        
        elif isinstance(node, AttrAssign):
            # self.x = value
            obj = self.execute(node.obj)
            value = self.execute(node.value)
            
            if isinstance(obj, VerInstance):
                obj.set(node.attr, value)
                return value
            else:
                setattr(obj, node.attr, value)
                return value
        
        elif isinstance(node, CompoundAssign):
            # x += 5 diventa x = x + 5
            current_value = None
            for scope in reversed(self.locals_stack):
                if node.name in scope:
                    current_value = scope[node.name]
                    break
            if current_value is None and node.name in self.globals:
                current_value = self.globals[node.name]
            if current_value is None:
                raise NameError(f"!! Variabile '{node.name}' non definita")
            
            new_value = self.execute(node.value)
            
            if node.op == '+=':
                result = current_value + new_value
            elif node.op == '-=':
                result = current_value - new_value
            elif node.op == '*=':
                result = current_value * new_value
            elif node.op == '/=':
                result = current_value / new_value
            
            # Aggiorna la variabile
            for scope in reversed(self.locals_stack):
                if node.name in scope:
                    scope[node.name] = result
                    return result
            self.locals_stack[-1][node.name] = result
            return result
        
        elif isinstance(node, IncrementDecrement):
            # Ottieni il valore corrente
            if isinstance(node.target, Var):
                # Variabile normale
                current_value = None
                for scope in reversed(self.locals_stack):
                    if node.target.name in scope:
                        current_value = scope[node.target.name]
                        break
                if current_value is None and node.target.name in self.globals:
                    current_value = self.globals[node.target.name]
                if current_value is None:
                    raise NameError(f"!! Variabile '{node.target.name}' non definita")
                
                # Calcola il nuovo valore
                if node.op == '++':
                    new_value = current_value + 1
                else:  # --
                    new_value = current_value - 1
                
                # Aggiorna la variabile
                for scope in reversed(self.locals_stack):
                    if node.target.name in scope:
                        scope[node.target.name] = new_value
                        break
                else:
                    self.locals_stack[-1][node.target.name] = new_value
                
            elif isinstance(node.target, Attr):
                # Attributo: obj.field++ o obj.field--
                obj = self.execute(node.target.obj)
                
                if isinstance(obj, VerInstance):
                    current_value = obj.get(node.target.attr)
                else:
                    current_value = getattr(obj, node.target.attr)
                
                # Calcola il nuovo valore
                if node.op == '++':
                    new_value = current_value + 1
                else:  # --
                    new_value = current_value - 1
                
                # Aggiorna l'attributo
                if isinstance(obj, VerInstance):
                    obj.set(node.target.attr, new_value)
                else:
                    setattr(obj, node.target.attr, new_value)
            
            # Ritorna il valore appropriato
            return new_value if node.prefix else current_value
        
        elif isinstance(node, LetStmt):
            value = self.execute(node.value)
            # Cerca se la variabile esiste già nel closure
            for scope in reversed(self.locals_stack):
                if node.name in scope:
                    scope[node.name] = value
                    return value
            # Altrimenti crea nel scope corrente
            self.locals_stack[-1][node.name] = value
            return value  # Ritorna il valore assegnato
        
        elif isinstance(node, FnDef):
            # Passa il riferimento allo scope corrente, non una copia
            func = VerFunction(node.params, node.body, self.locals_stack[-1])
            if node.name:
                self.locals_stack[-1][node.name] = func
            return func
        
        elif isinstance(node, IfStmt):
            if self.is_truthy(self.execute(node.condition)):
                for stmt in node.then_body:
                    self.execute(stmt)
            else:
                for cond, body in node.elif_parts:
                    if self.is_truthy(self.execute(cond)):
                        for stmt in body:
                            self.execute(stmt)
                        return
                if node.else_body:
                    for stmt in node.else_body:
                        self.execute(stmt)
        
        elif isinstance(node, ForStmt):
            iterable = self.execute(node.iterable)
            for item in iterable:
                self.locals_stack[-1][node.var] = item
                try:
                    for stmt in node.body:
                        self.execute(stmt)
                except BreakException:
                    break
                except ContinueException:
                    continue
        
        elif isinstance(node, WhileStmt):
            while self.is_truthy(self.execute(node.condition)):
                try:
                    for stmt in node.body:
                        self.execute(stmt)
                except BreakException:
                    break
                except ContinueException:
                    continue
        
        elif isinstance(node, ReturnStmt):
            value = self.execute(node.value) if node.value else None
            raise ReturnException(value)
        
        elif isinstance(node, BreakStmt):
            raise BreakException()
        
        elif isinstance(node, ContinueStmt):
            raise ContinueException()
        
        elif isinstance(node, BinaryOp):
            left = self.execute(node.left)
            right = self.execute(node.right)
            
            if node.op == '+':
                return left + right
            elif node.op == '-':
                return left - right
            elif node.op == '*':
                return left * right
            elif node.op == '/':
                return left / right
            elif node.op == '%':
                return left % right
            elif node.op == '**':
                return left ** right
            elif node.op == '==':
                return left == right
            elif node.op == '!=':
                return left != right
            elif node.op == '<':
                return left < right
            elif node.op == '<=':
                return left <= right
            elif node.op == '>':
                return left > right
            elif node.op == '>=':
                return left >= right
            elif node.op == 'and':
                return self.is_truthy(left) and self.is_truthy(right)
            elif node.op == 'or':
                return left if self.is_truthy(left) else right
        
        elif isinstance(node, UnaryOp):
            operand = self.execute(node.operand)
            if node.op == '-':
                return -operand
            elif node.op == 'not':
                return not self.is_truthy(operand)
        
        elif isinstance(node, Call):
            func = self.execute(node.func)
            args = [self.execute(arg) for arg in node.args]
            return self.call_function(func, args)
        
        elif isinstance(node, Index):
            obj = self.execute(node.obj)
            index = self.execute(node.index)
            return obj[index]
        
        elif isinstance(node, Attr):
            obj = self.execute(node.obj)
            # Se è un'istanza di VerInstance, usa il metodo get
            if isinstance(obj, VerInstance):
                return obj.get(node.attr)
            return getattr(obj, node.attr)
        
        elif isinstance(node, Literal):
            return node.value
        
        elif isinstance(node, Var):
            # Cerca nelle variabili locali, poi nelle globali
            for scope in reversed(self.locals_stack):
                if node.name in scope:
                    return scope[node.name]
            if node.name in self.globals:
                return self.globals[node.name]
            raise NameError(f"Variabile '{node.name}' non definita")
        
        elif isinstance(node, ListLit):
            return [self.execute(elem) for elem in node.elements]
        
        elif isinstance(node, MapLit):
            result = {}
            for key, value in node.pairs:
                result[key] = self.execute(value)
            return result
        
        return None
    
    def call_function(self, func: Any, args: List[Any]) -> Any:
        if callable(func) and not isinstance(func, VerFunction):
            return func(*args)
        
        if isinstance(func, VerFunction):
            # Salva lo scope corrente
            prev_stack = self.locals_stack.copy()
            
            # Crea nuovo scope che eredita dal closure
            new_scope = {}
            
            # Bind parametri nel nuovo scope
            for i, param in enumerate(func.params):
                if i < len(args):
                    new_scope[param] = args[i]
            
            # Imposta lo stack: closure + nuovo scope
            self.locals_stack = [func.closure, new_scope]
            
            try:
                for stmt in func.body:
                    self.execute(stmt)
                return None
            except ReturnException as e:
                return e.value
            finally:
                # Ripristina lo stack precedente
                self.locals_stack = prev_stack
        
        raise TypeError(f"!! Oggetto non chiamabile: {type(func)}")
    
    def is_truthy(self, value: Any) -> bool:
        if value is None or value is False:
            return False
        if value == 0 or value == "" or value == []:
            return False
        return True

# ============ MAIN ============

def run_ver(source: str, filename: str = "<input>"):
    """Esegue codice Verureka"""
    try:
        lexer = Lexer(source)
        tokens = lexer.tokenize()
        
        parser = Parser(tokens)
        ast = parser.parse()
        
        interpreter = Interpreter()
        interpreter.run(ast)
        
    except Exception as e:
        print(f"!! Errore in {filename}: {e}", file=sys.stderr)
        if "--debug" in sys.argv:
            import traceback
            traceback.print_exc()
        sys.exit(1)

def run_file(filepath: str):
    """Esegue un file .ver"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            source = f.read()
        run_ver(source, filepath)
    except FileNotFoundError:
        print(f"!! File non trovato: {filepath}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"!! Errore nella lettura del file: {e}", file=sys.stderr)
        sys.exit(1)

def repl():
    """REPL interattivo"""
    print("=" * 60)
    print("Veureka REPL - Linguaggio di Programmazione Interattivo")
    print("=" * 60)
    print("Digita 'exit' o 'quit' per uscire")
    print("Digita 'help' per vedere i comandi disponibili")
    print("Versione 0.0.1")
    print()
    
    interpreter = Interpreter()
    
    while True:
        try:
            line = input("ver> ")
            
            if not line.strip():
                continue
            
            if line.strip() in ['exit', 'quit']:
                print("Arrivederci!")
                break
            
            if line.strip() == 'help':
                print("""
Comandi disponibili:
  exit, quit    - Esci dal REPL
  help          - Mostra questo messaggio
  clear         - Pulisci le variabili e il resto del codice
  vars          - Mostra tutte le variabili

Esempi:
  let x = 10
  fn quadrato(n) => n * n
  print(quadrato(5))
  
  class Persona
      fn __init__(nome)
          self.nome = nome
      end
  end
  let p = new Persona("Mario")
                """)
                continue
            
            if line.strip() == 'clear':
                interpreter.locals_stack = [{}]
                print("✓ Codice e variabili pulite.")
                continue
            
            if line.strip() == 'vars':
                print("Variabili globali:")
                for k, v in interpreter.locals_stack[-1].items():
                    print(f"  {k} = {v}")
                continue
            
            # Esegui il codice
            lexer = Lexer(line)
            tokens = lexer.tokenize()
            parser = Parser(tokens)
            ast = parser.parse()
            
            for stmt in ast.statements:
                result = interpreter.execute(stmt)
                # Stampa il risultato solo se non è None e non è uno statement
                if result is not None and not isinstance(stmt, (LetStmt, FnDef, ClassDef)):
                    print(result)
        
        except KeyboardInterrupt:
            print("\nUsa 'exit' per uscire")
        except EOFError:
            print("\nArrivederci!")
            break
        except Exception as e:
            print(f" Errore: {e}")
            if "--debug" in sys.argv:
                import traceback
                traceback.print_exc()

def run_examples():
    """Esegue gli esempi dimostrativi"""
    print("=" * 60)
    print("Veureka - Linguaggio di Programmazione")
    print("=" * 60)
    
    # Esempio 1: Base
    print("\nEsempio 1: Variabili e Funzioni")
    run_ver("""
let nome = "Mario"
let età = 25

fn saluta(persona)
    print("Ciao, " + persona + "!")
end

saluta(nome)
""")
    
    # Esempio 2: Liste e iterazione
    print("\nEsempio 2: Liste e Iterazione")
    run_ver("""
let numeri = [1, 2, 3, 4, 5]

print("Numeri originali:", numeri)

for n in numeri
    print(n * 2)
end
""")
    
    # Esempio 3: Lambda e funzioni di ordine superiore
    print("\n Esempio 3: Lambda e Higher-Order Functions")
    run_ver("""
let numeri = [1, 2, 3, 4, 5]

let doppio = fn(x) => x * 2
let quadrati = map(numeri, fn(n) => n * n)

print("Doppi:", map(numeri, doppio))
print("Quadrati:", quadrati)

let pari = filter(numeri, fn(n) => n % 2 == 0)
print("Numeri pari:", pari)
""")
    
    # Esempio 4: Mappe (dizionari)
    print("\nEsempio 4: Mappe/Dizionari")
    run_ver("""
let utente = {
    nome: "Alice",
    età: 30,
    email: "alice@example.com"
}

print("Nome:", utente["nome"])
print("Età:", utente["età"])
""")
    
    # Esempio 5: Fibonacci con ricorsione
    print("\nEsempio 5: Fibonacci Ricorsivo")
    run_ver("""
fn fibonacci(n)
    if n < 2
        return n
    end
    return fibonacci(n - 1) + fibonacci(n - 2)
end

print("Fibonacci(10) =", fibonacci(10))
""")
    
    # Esempio 6: Closures
    print("\nEsempio 6: Closures")
    run_ver("""
fn crea_contatore()
    let count = 0
    return fn() => count = count + 1
end

let contatore = crea_contatore()
print(contatore())
print(contatore())
print(contatore())
""")
    
    # Esempio 7: Factorial con while
    print("\nEsempio 7: Fattoriale")
    run_ver("""
fn fattoriale(n)
    let risultato = 1
    let i = 1
    while i <= n
        risultato = risultato * i
        i = i + 1
    end
    return risultato
end

print("5! =", fattoriale(5))
print("10! =", fattoriale(10))
""")
    
    # Esempio 8: Reduce
    print("\nEsempio 8: Reduce")
    run_ver("""
let numeri = [1, 2, 3, 4, 5]
let somma = reduce(numeri, fn(acc, n) => acc + n, 0)
print("Somma:", somma)

let prodotto = reduce(numeri, fn(acc, n) => acc * n, 1)
print("Prodotto:", prodotto)
""")
    
    print("\n" + "=" * 60)
    print("Tutti gli esempi completati!")
    print("=" * 60)
    
    # Esempio 9: Operatori compatti
    print("\nEsempio 9: Operatori Compatti (+=, -=, ++, --)")
    run_ver("""
let x = 10
print("x iniziale:", x)

x += 5
print("x += 5:", x)

x -= 3
print("x -= 3:", x)

x *= 2
print("x *= 2:", x)

x /= 4
print("x /= 4:", x)

let y = 0
print("\\nIncremento/Decremento:")
print("y++:", y++)
print("y dopo y++:", y)
print("++y:", ++y)
print("y dopo ++y:", y)
print("y--:", y--)
print("--y:", --y)
""")
    
    # Esempio 10: Classi e OOP
    print("\nEsempio 10: Classi e OOP")
    run_ver("""
class Persona
    fn __init__(nome, età)
        self.nome = nome
        self.età = età
    end
    
    fn saluta()
        print("Ciao, sono " + self.nome + " e ho " + str(self.età) + " anni")
    end
    
    fn compleanno()
        self.età += 1
        print("Buon compleanno! Ora ho " + str(self.età) + " anni")
    end
end

let mario = new Persona("Mario", 25)
mario.saluta()
mario.compleanno()
mario.saluta()

let luigi = new Persona("Luigi", 23)
luigi.saluta()
""")
    
    # Esempio 11: Classe Contatore con OOP
    print("\nEsempio 11: Classe Contatore")
    run_ver("""
class Contatore
    fn __init__(valore_iniziale)
        self.valore = valore_iniziale
    end
    
    fn incrementa()
        self.valore++
        return self.valore
    end
    
    fn decrementa()
        self.valore--
        return self.valore
    end
    
    fn get()
        return self.valore
    end
end

let c = new Contatore(0)
print("Valore iniziale:", c.get())
print("Incrementa:", c.incrementa())
print("Incrementa:", c.incrementa())
print("Incrementa:", c.incrementa())
print("Decrementa:", c.decrementa())
print("Valore finale:", c.get())
""")
    
    # Esempio 12: Classe Punto 2D
    print("\nEsempio 12: Classe Punto 2D")
    run_ver("""
class Punto
    fn __init__(x, y)
        self.x = x
        self.y = y
    end
    
    fn distanza_origine()
        return (self.x ** 2 + self.y ** 2) ** 0.5
    end
    
    fn muovi(dx, dy)
        self.x += dx
        self.y += dy
    end
    
    fn mostra()
        print("Punto(" + str(self.x) + ", " + str(self.y) + ")")
    end
end

let p = new Punto(3, 4)
p.mostra()
print("Distanza dall'origine:", p.distanza_origine())

p.muovi(1, 1)
p.mostra()
print("Nuova distanza:", p.distanza_origine())
""")
    
    
# ============ ENTRY POINT ============

def main():
    """Entry point del compilatore Verureka"""
    if len(sys.argv) == 1:
        # Nessun argomento: avvia REPL
        repl()
    elif sys.argv[1] == "--examples":
        # Esegui esempi
        run_examples()
    elif sys.argv[1] in ["-h", "--help"]:
        # Help
        print("""
Veureka - Linguaggio di Programmazione

Uso:
    python veureka.py                    # Avvia REPL interattivo
    python veureka.py script.ver        # Esegue un file
    python veureka.py --examples         # Esegue gli esempi
    python veureka.py --help             # Mostra questo messaggio
    python veureka.py --debug script.ver # Esegue con debug
Esempi di sintassi Veureka:

    # Variabili
    let x = 10
    
    # Funzioni
    fn saluta(nome)
        print("Ciao, " + nome)
    end
    
    # Lambda
    let doppio = fn(x) => x * 2
    
    # Classi
    class Persona
        fn __init__(nome, età)
            self.nome = nome
            self.età = età
        end
        
        fn saluta()
            print("Ciao, sono " + self.nome)
        end
    end
    
    let p = new Persona("Mario", 25)
    p.saluta()

Per maggiori informazioni, visita: https://github.com/vincenzofranchino/veureka-lang
        """)
    else:
        # Esegui file
        filepath = sys.argv[1]
        run_file(filepath)

if __name__ == "__main__":
    main()
