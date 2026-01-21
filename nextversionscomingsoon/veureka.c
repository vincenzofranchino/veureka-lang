/* Veureka - Linguaggio di Programmazione in C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

/* ============ TOKEN TYPES ============ */
typedef enum {
    /* Keywords */
    TT_LET, TT_CONST, TT_FN, TT_CLASS, TT_NEW, TT_SELF,
    TT_IF, TT_ELIF, TT_ELSE, TT_FOR, TT_IN, TT_WHILE,
    TT_RETURN, TT_BREAK, TT_CONTINUE, TT_MATCH, TT_CASE, TT_END,
    TT_TRUE, TT_FALSE, TT_NIL, TT_AND, TT_OR, TT_NOT,
    TT_INCLUDE, TT_TRY, TT_CATCH, TT_FINALLY, TT_THROW,
    
    /* Bitwise operators */
    TT_AMPERSAND, TT_PIPE, TT_CARET, TT_TILDE, TT_LSHIFT, TT_RSHIFT,
    
    /* Literals */
    TT_NUMBER, TT_STRING, TT_IDENT,
    
    /* Operators */
    TT_PLUS, TT_MINUS, TT_STAR, TT_SLASH, TT_PERCENT, TT_POWER,
    TT_EQ, TT_NE, TT_LT, TT_LE, TT_GT, TT_GE,
    TT_ASSIGN, TT_PLUS_EQ, TT_MINUS_EQ, TT_STAR_EQ, TT_SLASH_EQ,
    TT_INCREMENT, TT_DECREMENT, TT_ARROW,
    
    /* Delimiters */
    TT_LPAREN, TT_RPAREN, TT_LBRACE, TT_RBRACE, TT_LBRACKET, TT_RBRACKET,
    TT_COMMA, TT_COLON, TT_DOT, TT_NEWLINE, TT_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    double num_value;
    int line;
    int col;
} Token;

typedef struct {
    Token *tokens;
    int count;
    int capacity;
} TokenList;

typedef struct {
    const char *source;
    int pos;
    int line;
    int col;
} Lexer;

/* ============ AST NODE TYPES ============ */
typedef enum {
    AST_PROGRAM, AST_INCLUDE, AST_LET, AST_ASSIGN, AST_FN, AST_CLASS,
    AST_IF, AST_FOR, AST_WHILE, AST_RETURN, AST_BREAK, AST_CONTINUE,
    AST_BINARY_OP, AST_UNARY_OP, AST_CALL, AST_INDEX, AST_ATTR,
    AST_LITERAL, AST_VAR, AST_LIST, AST_MAP, AST_NEW, AST_ATTR_ASSIGN,
    AST_COMPOUND_ASSIGN, AST_INCREMENT, AST_TRY, AST_THROW
} NodeType;

typedef struct ASTNode {
    NodeType type;
    
    /* Generic fields */
    char *name;
    char *op;
    int is_const;
    int prefix;
    
    /* Values */
    double num_val;
    char *str_val;
    int bool_val;
    
    /* Children */
    struct ASTNode **children;
    int children_count;
    int children_capacity;
    
    /* Special fields */
    char **params;
    int params_count;
    
    /* Map pairs */
    char **map_keys;
    struct ASTNode **map_values;
    int map_count;
    
    int line;
    int col;
} ASTNode;

/* ============ VALUE TYPES ============ */

typedef enum {
    VAL_NIL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL,
    VAL_LIST,
    VAL_MAP,
    VAL_FUNCTION,
    VAL_CLASS,
    VAL_INSTANCE,
    VAL_NATIVE_FUNC
} ValueType;

typedef struct Value Value;
typedef struct VeurekFunction VeurekFunction;
typedef struct VeurekClass VeurekClass;
typedef struct VeurekInstance VeurekInstance;
typedef struct Interpreter Interpreter;
typedef struct Environment Environment;

typedef Value (*NativeFunction)(Interpreter *interp, Value *args, int arg_count);

typedef struct Value {
    ValueType type;
    
    union {
        double number;
        char *string;
        int boolean;
        
        struct {
            Value *items;
            int count;
            int capacity;
        } list;
        
        struct {
            char **keys;
            Value *values;
            int count;
            int capacity;
        } map;
        
        VeurekFunction *function;
        VeurekClass *class;
        VeurekInstance *instance;
        NativeFunction native_func;
    } data;
} Value;

typedef struct VeurekFunction {
    char **params;
    int params_count;
    ASTNode **body;
    int body_count;
    Environment *closure;
} VeurekFunction;

typedef struct VeurekClass {
    char *name;
    struct {
        char *name;
        VeurekFunction *func;
    } *methods;
    int methods_count;
} VeurekClass;

typedef struct VeurekInstance {
    VeurekClass *ver_class;
    struct {
        char *name;
        Value value;
    } *fields;
    int fields_count;
} VeurekInstance;

typedef struct Environment {
    struct {
        char *name;
        Value value;
        int is_const;
    } *vars;
    int var_count;
    int var_capacity;
    
    struct Environment *parent;
} Environment;

/* ============ EXCEPTION HANDLING ============ */

typedef enum {
    EXCEPTION_NONE,
    EXCEPTION_RETURN,
    EXCEPTION_BREAK,
    EXCEPTION_CONTINUE,
    EXCEPTION_THROW
} ExceptionType;

typedef struct {
    ExceptionType type;
    Value return_value;
    char *error_message;
} Exception;

typedef struct Interpreter {
    Environment *global_env;
    Environment *current_env;
    Exception last_exception;
} Interpreter;

/* ============ FORWARD DECLARATIONS ============ */
Value interpreter_execute(Interpreter *interp, ASTNode *node);
Value interpreter_call_function(Interpreter *interp, Value func, Value *args, int arg_count);
int is_truthy(Value v);
char* val_to_string(Value v);

/* ============ MEMORY MANAGEMENT ============ */
char* str_dup(const char *s) {
    if (!s) return NULL;
    char *dup = malloc(strlen(s) + 1);
    strcpy(dup, s);
    return dup;
}

/* ============ TOKEN FUNCTIONS ============ */
Token make_token(TokenType type, const char *value, double num_val, int line, int col) {
    Token t;
    t.type = type;
    t.value = str_dup(value);
    t.num_value = num_val;
    t.line = line;
    t.col = col;
    return t;
}

TokenList* token_list_new() {
    TokenList *tl = malloc(sizeof(TokenList));
    tl->tokens = malloc(sizeof(Token) * 100);
    tl->count = 0;
    tl->capacity = 100;
    return tl;
}

void token_list_add(TokenList *tl, Token t) {
    if (tl->count >= tl->capacity) {
        tl->capacity *= 2;
        tl->tokens = realloc(tl->tokens, sizeof(Token) * tl->capacity);
    }
    tl->tokens[tl->count++] = t;
}

/* ============ AST NODE FUNCTIONS ============ */
ASTNode* ast_node_new(NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->children = malloc(sizeof(ASTNode*) * 10);
    node->children_count = 0;
    node->children_capacity = 10;
    return node;
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
    if (parent->children_count >= parent->children_capacity) {
        parent->children_capacity *= 2;
        parent->children = realloc(parent->children, sizeof(ASTNode*) * parent->children_capacity);
    }
    parent->children[parent->children_count++] = child;
}

/* ============ LEXER ============ */
Lexer lexer_new(const char *source) {
    Lexer lex;
    lex.source = source;
    lex.pos = 0;
    lex.line = 1;
    lex.col = 1;
    return lex;
}

char lexer_current(Lexer *lex) {
    return lex->source[lex->pos];
}

char lexer_peek(Lexer *lex, int offset) {
    return lex->source[lex->pos + offset];
}

void lexer_advance(Lexer *lex) {
    if (lex->source[lex->pos] == '\n') {
        lex->line++;
        lex->col = 1;
    } else {
        lex->col++;
    }
    lex->pos++;
}

void lexer_skip_whitespace(Lexer *lex) {
    while (lexer_current(lex) == ' ' || lexer_current(lex) == '\t' || 
           lexer_current(lex) == '\r') {
        lexer_advance(lex);
    }
}

int lexer_is_at_end(Lexer *lex) {
    return lexer_current(lex) == '\0';
}

TokenList* lexer_tokenize(Lexer *lex) {
    TokenList *tokens = token_list_new();
    
    while (!lexer_is_at_end(lex)) {
        lexer_skip_whitespace(lex);
        if (lexer_is_at_end(lex)) break;
        
        char c = lexer_current(lex);
        int line = lex->line;
        int col = lex->col;
        
        /* Comments */
        if (c == '#') {
            while (!lexer_is_at_end(lex) && lexer_current(lex) != '\n') {
                lexer_advance(lex);
            }
            continue;
        }
        
        /* Newlines */
        if (c == '\n') {
            token_list_add(tokens, make_token(TT_NEWLINE, "\n", 0, line, col));
            lexer_advance(lex);
            continue;
        }
        
        /* Numbers */
        if (isdigit(c)) {
            char buf[50] = {0};
            int idx = 0;
            int has_dot = 0;
            
            while (!lexer_is_at_end(lex) && 
                   (isdigit(lexer_current(lex)) || 
                   (lexer_current(lex) == '.' && !has_dot))) {
                if (lexer_current(lex) == '.') has_dot = 1;
                buf[idx++] = lexer_current(lex);
                lexer_advance(lex);
            }
            buf[idx] = '\0';
            
            token_list_add(tokens, make_token(TT_NUMBER, buf, atof(buf), line, col));
            continue;
        }
        
        /* Strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            lexer_advance(lex);
            char buf[1000] = {0};
            int idx = 0;
            
            while (!lexer_is_at_end(lex) && lexer_current(lex) != quote) {
                if (lexer_current(lex) == '\\') {
                    lexer_advance(lex);
                    if (lexer_current(lex) == 'n') buf[idx++] = '\n';
                    else if (lexer_current(lex) == 't') buf[idx++] = '\t';
                    else if (lexer_current(lex) == '\\') buf[idx++] = '\\';
                    else buf[idx++] = lexer_current(lex);
                    lexer_advance(lex);
                } else {
                    buf[idx++] = lexer_current(lex);
                    lexer_advance(lex);
                }
            }
            buf[idx] = '\0';
            lexer_advance(lex);
            
            token_list_add(tokens, make_token(TT_STRING, buf, 0, line, col));
            continue;
        }
        
        /* Identifiers and keywords */
        if (isalpha(c) || c == '_') {
            char buf[100] = {0};
            int idx = 0;
            
            while (!lexer_is_at_end(lex) && 
                   (isalnum(lexer_current(lex)) || lexer_current(lex) == '_')) {
                buf[idx++] = lexer_current(lex);
                lexer_advance(lex);
            }
            buf[idx] = '\0';
            
            /* Check keywords */
            TokenType type = TT_IDENT;
            if (strcmp(buf, "let") == 0) type = TT_LET;
            else if (strcmp(buf, "const") == 0) type = TT_CONST;
            else if (strcmp(buf, "fn") == 0) type = TT_FN;
            else if (strcmp(buf, "class") == 0) type = TT_CLASS;
            else if (strcmp(buf, "new") == 0) type = TT_NEW;
            else if (strcmp(buf, "self") == 0) type = TT_SELF;
            else if (strcmp(buf, "if") == 0) type = TT_IF;
            else if (strcmp(buf, "elif") == 0) type = TT_ELIF;
            else if (strcmp(buf, "else") == 0) type = TT_ELSE;
            else if (strcmp(buf, "for") == 0) type = TT_FOR;
            else if (strcmp(buf, "in") == 0) type = TT_IN;
            else if (strcmp(buf, "while") == 0) type = TT_WHILE;
            else if (strcmp(buf, "return") == 0) type = TT_RETURN;
            else if (strcmp(buf, "break") == 0) type = TT_BREAK;
            else if (strcmp(buf, "continue") == 0) type = TT_CONTINUE;
            else if (strcmp(buf, "true") == 0) type = TT_TRUE;
            else if (strcmp(buf, "false") == 0) type = TT_FALSE;
            else if (strcmp(buf, "nil") == 0) type = TT_NIL;
            else if (strcmp(buf, "and") == 0) type = TT_AND;
            else if (strcmp(buf, "or") == 0) type = TT_OR;
            else if (strcmp(buf, "not") == 0) type = TT_NOT;
            else if (strcmp(buf, "include") == 0) type = TT_INCLUDE;
            else if (strcmp(buf, "try") == 0) type = TT_TRY;
            else if (strcmp(buf, "catch") == 0) type = TT_CATCH;
            else if (strcmp(buf, "finally") == 0) type = TT_FINALLY;
            else if (strcmp(buf, "throw") == 0) type = TT_THROW;
            else if (strcmp(buf, "end") == 0) type = TT_END;
            
            token_list_add(tokens, make_token(type, buf, 0, line, col));
            continue;
        }
        
        /* Operators */
        if (c == '+') {
            if (lexer_peek(lex, 1) == '+') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_INCREMENT, "++", 0, line, col));
            } else if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_PLUS_EQ, "+=", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_PLUS, "+", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        if (c == '-') {
            if (lexer_peek(lex, 1) == '-') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_DECREMENT, "--", 0, line, col));
            } else if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_MINUS_EQ, "-=", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_MINUS, "-", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        if (c == '*') {
            if (lexer_peek(lex, 1) == '*') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_POWER, "**", 0, line, col));
            } else if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_STAR_EQ, "*=", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_STAR, "*", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        if (c == '/') {
            if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_SLASH_EQ, "/=", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_SLASH, "/", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        if (c == '=') {
            if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_EQ, "==", 0, line, col));
            } else if (lexer_peek(lex, 1) == '>') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_ARROW, "=>", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_ASSIGN, "=", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        if (c == '!') {
            if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_NE, "!=", 0, line, col));
                lexer_advance(lex);
            } else {
                lexer_advance(lex);
            }
            continue;
        }
        
        if (c == '<') {
            if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_LE, "<=", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_LT, "<", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        if (c == '>') {
            if (lexer_peek(lex, 1) == '=') {
                lexer_advance(lex);
                token_list_add(tokens, make_token(TT_GE, ">=", 0, line, col));
            } else {
                token_list_add(tokens, make_token(TT_GT, ">", 0, line, col));
            }
            lexer_advance(lex);
            continue;
        }
        
        /* Single character tokens */
        TokenType single_type = TT_EOF;
        switch (c) {
            case '(': single_type = TT_LPAREN; break;
            case ')': single_type = TT_RPAREN; break;
            case '{': single_type = TT_LBRACE; break;
            case '}': single_type = TT_RBRACE; break;
            case '[': single_type = TT_LBRACKET; break;
            case ']': single_type = TT_RBRACKET; break;
            case ',': single_type = TT_COMMA; break;
            case ':': single_type = TT_COLON; break;
            case '.': single_type = TT_DOT; break;
            case '&': single_type = TT_AMPERSAND; break;
            case '|': single_type = TT_PIPE; break;
            case '^': single_type = TT_CARET; break;
            case '~': single_type = TT_TILDE; break;
            case '%': single_type = TT_PERCENT; break;
        }
        
        if (single_type != TT_EOF) {
            char buf[2] = {c, 0};
            token_list_add(tokens, make_token(single_type, buf, 0, line, col));
            lexer_advance(lex);
            continue;
        }
        
        fprintf(stderr, "!! Carattere inaspettato '%c' alla riga %d:%d\n", c, line, col);
        lexer_advance(lex);
    }
    
    token_list_add(tokens, make_token(TT_EOF, "", 0, lex->line, lex->col));
    return tokens;
}

/* ============ PARSER ============ */

typedef struct {
    Token *tokens;
    int count;
    int pos;
} Parser;

Parser parser_new(TokenList *token_list) {
    Parser p;
    p.tokens = token_list->tokens;
    p.count = token_list->count;
    p.pos = 0;
    
    /* Skip newlines */
    int new_count = 0;
    Token *filtered = malloc(sizeof(Token) * p.count);
    for (int i = 0; i < p.count; i++) {
        if (p.tokens[i].type != TT_NEWLINE) {
            filtered[new_count++] = p.tokens[i];
        }
    }
    p.tokens = filtered;
    p.count = new_count;
    
    return p;
}

Token parser_current(Parser *p) {
    if (p->pos >= p->count) {
        return p->tokens[p->count - 1];
    }
    return p->tokens[p->pos];
}

Token parser_previous(Parser *p) {
    return p->tokens[p->pos - 1];
}

void parser_advance(Parser *p) {
    if (p->pos < p->count) {
        p->pos++;
    }
}

int parser_check(Parser *p, TokenType type) {
    return parser_current(p).type == type;
}

int parser_match(Parser *p, TokenType type) {
    if (parser_check(p, type)) {
        parser_advance(p);
        return 1;
    }
    return 0;
}

int parser_is_at_end(Parser *p) {
    return parser_current(p).type == TT_EOF;
}

Token parser_consume(Parser *p, TokenType type, const char *msg) {
    if (!parser_check(p, type)) {
        fprintf(stderr, "!! %s (trovato %d)\n", msg, parser_current(p).type);
        exit(1);
    }
    Token t = parser_current(p);
    parser_advance(p);
    return t;
}

/* Forward declarations */
ASTNode* parser_parse_expression(Parser *p);
ASTNode* parser_parse_statement(Parser *p);
ASTNode* parser_parse_primary(Parser *p);
ASTNode* parser_parse_function(Parser *p);
ASTNode* parser_parse_class(Parser *p);
ASTNode* parser_parse_if(Parser *p);
ASTNode* parser_parse_for(Parser *p);
ASTNode* parser_parse_while(Parser *p);
ASTNode* parser_parse_try(Parser *p);
ASTNode* parser_parse_assignment(Parser *p);
ASTNode* parser_parse_or(Parser *p);
ASTNode* parser_parse_and(Parser *p);
ASTNode* parser_parse_bitwise_or(Parser *p);
ASTNode* parser_parse_bitwise_xor(Parser *p);
ASTNode* parser_parse_bitwise_and(Parser *p);
ASTNode* parser_parse_comparison(Parser *p);
ASTNode* parser_parse_additive(Parser *p);
ASTNode* parser_parse_multiplicative(Parser *p);
ASTNode* parser_parse_power(Parser *p);
ASTNode* parser_parse_unary(Parser *p);
ASTNode* parser_parse_postfix(Parser *p);

ASTNode* parser_parse_program(Parser *p) {
    ASTNode *prog = ast_node_new(AST_PROGRAM);
    
    while (!parser_is_at_end(p)) {
        ast_add_child(prog, parser_parse_statement(p));
    }
    
    return prog;
}

ASTNode* parser_parse_statement(Parser *p) {
    if (parser_match(p, TT_INCLUDE)) {
        ASTNode *node = ast_node_new(AST_INCLUDE);
        if (parser_check(p, TT_STRING)) {
            node->str_val = str_dup(parser_current(p).value);
            parser_advance(p);
        } else if (parser_match(p, TT_LPAREN)) {
            node->str_val = str_dup(parser_current(p).value);
            parser_advance(p);
            parser_consume(p, TT_RPAREN, "Expected ')' after include");
        }
        return node;
    }
    
    if (parser_match(p, TT_LET)) {
        ASTNode *node = ast_node_new(AST_LET);
        node->name = str_dup(parser_consume(p, TT_IDENT, "Expected identifier after 'let'").value);
        parser_consume(p, TT_ASSIGN, "Expected '=' after variable name");
        ast_add_child(node, parser_parse_expression(p));
        node->is_const = 0;
        return node;
    }
    
    if (parser_match(p, TT_CONST)) {
        ASTNode *node = ast_node_new(AST_LET);
        node->name = str_dup(parser_consume(p, TT_IDENT, "Expected identifier after 'const'").value);
        parser_consume(p, TT_ASSIGN, "Expected '=' after constant name");
        ast_add_child(node, parser_parse_expression(p));
        node->is_const = 1;
        return node;
    }
    
    if (parser_match(p, TT_FN)) {
        return parser_parse_function(p);
    }
    
    if (parser_match(p, TT_CLASS)) {
        return parser_parse_class(p);
    }
    
    if (parser_match(p, TT_IF)) {
        return parser_parse_if(p);
    }
    
    if (parser_match(p, TT_FOR)) {
        return parser_parse_for(p);
    }
    
    if (parser_match(p, TT_WHILE)) {
        return parser_parse_while(p);
    }
    
    if (parser_match(p, TT_TRY)) {
        return parser_parse_try(p);
    }
    
    if (parser_match(p, TT_THROW)) {
        ASTNode *node = ast_node_new(AST_THROW);
        ast_add_child(node, parser_parse_expression(p));
        return node;
    }
    
    if (parser_match(p, TT_RETURN)) {
        ASTNode *node = ast_node_new(AST_RETURN);
        if (!parser_check(p, TT_END) && !parser_is_at_end(p)) {
            ast_add_child(node, parser_parse_expression(p));
        }
        return node;
    }
    
    if (parser_match(p, TT_BREAK)) {
        return ast_node_new(AST_BREAK);
    }
    
    if (parser_match(p, TT_CONTINUE)) {
        return ast_node_new(AST_CONTINUE);
    }
    
    return parser_parse_expression(p);
}

ASTNode* parser_parse_function(Parser *p) {
    ASTNode *node = ast_node_new(AST_FN);
    
    /* Optional name */
    if (parser_check(p, TT_IDENT)) {
        node->name = str_dup(parser_current(p).value);
        parser_advance(p);
    }
    
    /* Parameters */
    parser_consume(p, TT_LPAREN, "Expected '(' after function");
    node->params = malloc(sizeof(char*) * 20);
    node->params_count = 0;
    
    while (!parser_check(p, TT_RPAREN)) {
        Token t = parser_consume(p, TT_IDENT, "Expected parameter name");
        node->params[node->params_count++] = str_dup(t.value);
        
        if (!parser_check(p, TT_RPAREN)) {
            parser_consume(p, TT_COMMA, "Expected ',' between parameters");
        }
    }
    parser_consume(p, TT_RPAREN, "Expected ')' after parameters");
    
    /* Lambda syntax => or block */
    if (parser_match(p, TT_ARROW)) {
        ASTNode *ret = ast_node_new(AST_RETURN);
        ast_add_child(ret, parser_parse_expression(p));
        ast_add_child(node, ret);
    } else {
        /* Block body */
        while (!parser_check(p, TT_END)) {
            ast_add_child(node, parser_parse_statement(p));
        }
        parser_consume(p, TT_END, "Expected 'end' to close function");
    }
    
    return node;
}

ASTNode* parser_parse_class(Parser *p) {
    ASTNode *node = ast_node_new(AST_CLASS);
    node->name = str_dup(parser_consume(p, TT_IDENT, "Expected class name").value);
    
    while (!parser_check(p, TT_END)) {
        if (parser_match(p, TT_FN)) {
            ast_add_child(node, parser_parse_function(p));
        } else {
            parser_advance(p);
        }
    }
    
    parser_consume(p, TT_END, "Expected 'end' to close class");
    return node;
}

ASTNode* parser_parse_if(Parser *p) {
    ASTNode *node = ast_node_new(AST_IF);
    
    /* Condition */
    ast_add_child(node, parser_parse_expression(p));
    
    /* Then body */
    ASTNode *then_body = ast_node_new(AST_PROGRAM);
    while (!parser_check(p, TT_ELIF) && !parser_check(p, TT_ELSE) && !parser_check(p, TT_END)) {
        ast_add_child(then_body, parser_parse_statement(p));
    }
    ast_add_child(node, then_body);
    
    /* Elif/Else chain */
    ASTNode *current_if = node;
    
    while (parser_match(p, TT_ELIF)) {
        ASTNode *elif_node = ast_node_new(AST_IF);
        ast_add_child(elif_node, parser_parse_expression(p));
        
        ASTNode *elif_then = ast_node_new(AST_PROGRAM);
        while (!parser_check(p, TT_ELIF) && !parser_check(p, TT_ELSE) && !parser_check(p, TT_END)) {
            ast_add_child(elif_then, parser_parse_statement(p));
        }
        ast_add_child(elif_node, elif_then);
        
        ast_add_child(current_if, elif_node);
        current_if = elif_node;
    }
    
    if (parser_match(p, TT_ELSE)) {
        ASTNode *else_body = ast_node_new(AST_PROGRAM);
        while (!parser_check(p, TT_END)) {
            ast_add_child(else_body, parser_parse_statement(p));
        }
        ast_add_child(current_if, else_body);
    }
    
    parser_consume(p, TT_END, "Expected 'end' to close if");
    return node;
}

ASTNode* parser_parse_for(Parser *p) {
    ASTNode *node = ast_node_new(AST_FOR);
    node->name = str_dup(parser_consume(p, TT_IDENT, "Expected variable name in for loop").value);
    parser_consume(p, TT_IN, "Expected 'in' in for loop");
    ast_add_child(node, parser_parse_expression(p));
    
    while (!parser_check(p, TT_END)) {
        ast_add_child(node, parser_parse_statement(p));
    }
    parser_consume(p, TT_END, "Expected 'end' to close for loop");
    
    return node;
}

ASTNode* parser_parse_while(Parser *p) {
    ASTNode *node = ast_node_new(AST_WHILE);
    ast_add_child(node, parser_parse_expression(p));
    
    while (!parser_check(p, TT_END)) {
        ast_add_child(node, parser_parse_statement(p));
    }
    parser_consume(p, TT_END, "Expected 'end' to close while loop");
    
    return node;
}

ASTNode* parser_parse_try(Parser *p) {
    ASTNode *node = ast_node_new(AST_TRY);
    
    /* Try body */
    while (!parser_check(p, TT_CATCH) && !parser_check(p, TT_FINALLY) && !parser_is_at_end(p)) {
        ast_add_child(node, parser_parse_statement(p));
    }
    
    /* Catch */
    if (parser_match(p, TT_CATCH)) {
        if (parser_match(p, TT_LPAREN)) {
            node->name = str_dup(parser_consume(p, TT_IDENT, "Expected identifier in catch").value);
            parser_consume(p, TT_RPAREN, "Expected ')' after catch variable");
        } else if (parser_check(p, TT_IDENT)) {
            node->name = str_dup(parser_current(p).value);
            parser_advance(p);
        }
        
        while (!parser_check(p, TT_FINALLY) && !parser_check(p, TT_END)) {
            ast_add_child(node, parser_parse_statement(p));
        }
    }
    
    /* Finally */
    if (parser_match(p, TT_FINALLY)) {
        while (!parser_check(p, TT_END)) {
            ast_add_child(node, parser_parse_statement(p));
        }
    }
    
    parser_consume(p, TT_END, "Expected 'end' to close try");
    return node;
}

ASTNode* parser_parse_expression(Parser *p) {
    return parser_parse_assignment(p);
}

ASTNode* parser_parse_assignment(Parser *p) {
    ASTNode *expr = parser_parse_or(p);
    
    /* Assignment to attribute */
    if (expr->type == AST_ATTR && parser_check(p, TT_ASSIGN)) {
        parser_advance(p);
        ASTNode *node = ast_node_new(AST_ATTR_ASSIGN);
        ast_add_child(node, expr->children[0]); /* object */
        node->name = str_dup(expr->name); /* attribute name */
        ast_add_child(node, parser_parse_assignment(p)); /* value */
        return node;
    }
    
    /* Compound assignment to attribute */
    if (expr->type == AST_ATTR && (parser_check(p, TT_PLUS_EQ) || parser_check(p, TT_MINUS_EQ) || 
                                    parser_check(p, TT_STAR_EQ) || parser_check(p, TT_SLASH_EQ))) {
        char *op = str_dup(parser_current(p).value);
        parser_advance(p);
        
        ASTNode *node = ast_node_new(AST_ATTR_ASSIGN);
        ast_add_child(node, expr->children[0]);
        node->name = str_dup(expr->name);
        
        /* Create binary op: attr op value */
        ASTNode *binop = ast_node_new(AST_BINARY_OP);
        if (strcmp(op, "+=") == 0) binop->op = str_dup("+");
        else if (strcmp(op, "-=") == 0) binop->op = str_dup("-");
        else if (strcmp(op, "*=") == 0) binop->op = str_dup("*");
        else if (strcmp(op, "/=") == 0) binop->op = str_dup("/");
        
        ast_add_child(binop, expr);
        ast_add_child(binop, parser_parse_assignment(p));
        ast_add_child(node, binop);
        
        return node;
    }
    
    /* Assignment to variable */
    if (expr->type == AST_VAR) {
        /* Compound assignments */
        if (parser_check(p, TT_PLUS_EQ) || parser_check(p, TT_MINUS_EQ) || 
            parser_check(p, TT_STAR_EQ) || parser_check(p, TT_SLASH_EQ)) {
            ASTNode *node = ast_node_new(AST_COMPOUND_ASSIGN);
            node->name = str_dup(expr->name);
            node->op = str_dup(parser_current(p).value);
            parser_advance(p);
            ast_add_child(node, parser_parse_assignment(p));
            return node;
        }
        /* Normal assignment */
        else if (parser_match(p, TT_ASSIGN)) {
            ASTNode *node = ast_node_new(AST_ASSIGN);
            node->name = str_dup(expr->name);
            ast_add_child(node, parser_parse_assignment(p));
            return node;
        }
    }
    
    return expr;
}

ASTNode* parser_parse_or(Parser *p) {
    ASTNode *left = parser_parse_and(p);
    
    while (parser_match(p, TT_OR)) {
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup("or");
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_and(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_and(Parser *p) {
    ASTNode *left = parser_parse_bitwise_or(p);
    
    while (parser_match(p, TT_AND)) {
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup("and");
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_bitwise_or(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_bitwise_or(Parser *p) {
    ASTNode *left = parser_parse_bitwise_xor(p);
    
    while (parser_match(p, TT_PIPE)) {
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup("|");
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_bitwise_xor(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_bitwise_xor(Parser *p) {
    ASTNode *left = parser_parse_bitwise_and(p);
    
    while (parser_match(p, TT_CARET)) {
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup("^");
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_bitwise_and(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_bitwise_and(Parser *p) {
    ASTNode *left = parser_parse_comparison(p);
    
    while (parser_match(p, TT_AMPERSAND)) {
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup("&");
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_comparison(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_comparison(Parser *p) {
    ASTNode *left = parser_parse_additive(p);
    
    while (parser_check(p, TT_EQ) || parser_check(p, TT_NE) || 
           parser_check(p, TT_LT) || parser_check(p, TT_LE) || 
           parser_check(p, TT_GT) || parser_check(p, TT_GE)) {
        Token op = parser_current(p);
        parser_advance(p);
        
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup(op.value);
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_additive(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_additive(Parser *p) {
    ASTNode *left = parser_parse_multiplicative(p);
    
    while (parser_check(p, TT_PLUS) || parser_check(p, TT_MINUS)) {
        Token op = parser_current(p);
        parser_advance(p);
        
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup(op.value);
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_multiplicative(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_multiplicative(Parser *p) {
    ASTNode *left = parser_parse_power(p);
    
    while (parser_check(p, TT_STAR) || parser_check(p, TT_SLASH) || parser_check(p, TT_PERCENT)) {
        Token op = parser_current(p);
        parser_advance(p);
        
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup(op.value);
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_power(p));
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_power(Parser *p) {
    ASTNode *left = parser_parse_unary(p);
    
    if (parser_match(p, TT_POWER)) {
        ASTNode *node = ast_node_new(AST_BINARY_OP);
        node->op = str_dup("**");
        ast_add_child(node, left);
        ast_add_child(node, parser_parse_power(p));
        return node;
    }
    
    return left;
}

ASTNode* parser_parse_unary(Parser *p) {
    /* Prefix increment/decrement */
    if (parser_check(p, TT_INCREMENT) || parser_check(p, TT_DECREMENT)) {
        Token op = parser_current(p);
        parser_advance(p);
        
        ASTNode *operand = parser_parse_postfix(p);
        if (operand->type == AST_VAR || operand->type == AST_ATTR) {
            ASTNode *node = ast_node_new(AST_INCREMENT);
            node->op = str_dup(op.value);
            node->prefix = 1;
            ast_add_child(node, operand);
            return node;
        }
        fprintf(stderr, "!! ++ e -- richiedono una variabile o attributo\n");
        exit(1);
    }
    
    if (parser_check(p, TT_MINUS) || parser_check(p, TT_NOT) || parser_check(p, TT_TILDE)) {
        Token op = parser_current(p);
        parser_advance(p);
        
        ASTNode *node = ast_node_new(AST_UNARY_OP);
        node->op = str_dup(op.value);
        ast_add_child(node, parser_parse_unary(p));
        return node;
    }
    
    return parser_parse_postfix(p);
}

ASTNode* parser_parse_postfix(Parser *p) {
    ASTNode *expr = parser_parse_primary(p);
    
    while (1) {
        if (parser_match(p, TT_LPAREN)) {
            /* Function call */
            ASTNode *node = ast_node_new(AST_CALL);
            ast_add_child(node, expr);
            
            while (!parser_check(p, TT_RPAREN)) {
                ast_add_child(node, parser_parse_expression(p));
                if (!parser_check(p, TT_RPAREN)) {
                    parser_consume(p, TT_COMMA, "Expected ',' between arguments");
                }
            }
            parser_consume(p, TT_RPAREN, "Expected ')' after arguments");
            expr = node;
        }
        else if (parser_match(p, TT_LBRACKET)) {
            /* Indexing */
            ASTNode *node = ast_node_new(AST_INDEX);
            ast_add_child(node, expr);
            ast_add_child(node, parser_parse_expression(p));
            parser_consume(p, TT_RBRACKET, "Expected ']' after index");
            expr = node;
        }
        else if (parser_match(p, TT_DOT)) {
            /* Attribute access */
            Token attr = parser_consume(p, TT_IDENT, "Expected attribute name");
            ASTNode *node = ast_node_new(AST_ATTR);
            ast_add_child(node, expr);
            node->name = str_dup(attr.value);
            expr = node;
        }
        /* Postfix increment/decrement */
        else if (parser_check(p, TT_INCREMENT) || parser_check(p, TT_DECREMENT)) {
            if (expr->type == AST_VAR || expr->type == AST_ATTR) {
                Token op = parser_current(p);
                parser_advance(p);
                ASTNode *node = ast_node_new(AST_INCREMENT);
                node->op = str_dup(op.value);
                node->prefix = 0;
                ast_add_child(node, expr);
                expr = node;
            } else {
                break;
            }
        }
        else {
            break;
        }
    }
    
    return expr;
}

ASTNode* parser_parse_primary(Parser *p) {
    if (parser_match(p, TT_NUMBER)) {
        ASTNode *node = ast_node_new(AST_LITERAL);
        node->num_val = parser_previous(p).num_value;
        return node;
    }
    
    if (parser_match(p, TT_STRING)) {
        ASTNode *node = ast_node_new(AST_LITERAL);
        node->str_val = str_dup(parser_previous(p).value);
        return node;
    }
    
    if (parser_match(p, TT_TRUE)) {
        ASTNode *node = ast_node_new(AST_LITERAL);
        node->bool_val = 1;
        return node;
    }
    
    if (parser_match(p, TT_FALSE)) {
        ASTNode *node = ast_node_new(AST_LITERAL);
        node->bool_val = 0;
        return node;
    }
    
    if (parser_match(p, TT_NIL)) {
        return ast_node_new(AST_LITERAL);
    }
    
    if (parser_match(p, TT_SELF)) {
        ASTNode *node = ast_node_new(AST_VAR);
        node->name = str_dup("self");
        return node;
    }
    
    if (parser_match(p, TT_IDENT)) {
        ASTNode *node = ast_node_new(AST_VAR);
        node->name = str_dup(parser_previous(p).value);
        return node;
    }
    
    if (parser_match(p, TT_LPAREN)) {
        ASTNode *expr = parser_parse_expression(p);
        parser_consume(p, TT_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    if (parser_match(p, TT_LBRACKET)) {
        ASTNode *node = ast_node_new(AST_LIST);
        while (!parser_check(p, TT_RBRACKET)) {
            ast_add_child(node, parser_parse_expression(p));
            if (!parser_check(p, TT_RBRACKET)) {
                parser_consume(p, TT_COMMA, "Expected ',' between list elements");
            }
        }
        parser_consume(p, TT_RBRACKET, "Expected ']' after list");
        return node;
    }
    
    if (parser_match(p, TT_LBRACE)) {
        ASTNode *node = ast_node_new(AST_MAP);
        node->map_keys = malloc(sizeof(char*) * 50);
        node->map_values = malloc(sizeof(ASTNode*) * 50);
        node->map_count = 0;
        
        while (!parser_check(p, TT_RBRACE)) {
            Token key_token = parser_current(p);
            if (parser_check(p, TT_IDENT) || parser_check(p, TT_STRING) || parser_check(p, TT_NUMBER)) {
                parser_advance(p);
                node->map_keys[node->map_count] = str_dup(key_token.value);
            }
            
            parser_consume(p, TT_COLON, "Expected ':' in map");
            node->map_values[node->map_count] = parser_parse_expression(p);
            node->map_count++;
            
            if (!parser_check(p, TT_RBRACE)) {
                parser_consume(p, TT_COMMA, "Expected ',' between map pairs");
            }
        }
        parser_consume(p, TT_RBRACE, "Expected '}' after map");
        return node;
    }
    
    if (parser_match(p, TT_FN)) {
        return parser_parse_function(p);
    }
    
    if (parser_match(p, TT_NEW)) {
        ASTNode *node = ast_node_new(AST_NEW);
        node->name = str_dup(parser_consume(p, TT_IDENT, "Expected class name").value);
        parser_consume(p, TT_LPAREN, "Expected '(' after class name");
        
        while (!parser_check(p, TT_RPAREN)) {
            ast_add_child(node, parser_parse_expression(p));
            if (!parser_check(p, TT_RPAREN)) {
                parser_consume(p, TT_COMMA, "Expected ',' between arguments");
            }
        }
        parser_consume(p, TT_RPAREN, "Expected ')' after constructor arguments");
        return node;
    }
    
    fprintf(stderr, "!! Espressione inaspettata\n");
    exit(1);
}

/* ============ VALUE FUNCTIONS ============ */

Value val_nil() {
    Value v;
    v.type = VAL_NIL;
    return v;
}

Value val_number(double n) {
    Value v;
    v.type = VAL_NUMBER;
    v.data.number = n;
    return v;
}

Value val_string(const char *s) {
    Value v;
    v.type = VAL_STRING;
    v.data.string = str_dup(s);
    return v;
}

Value val_bool(int b) {
    Value v;
    v.type = VAL_BOOL;
    v.data.boolean = b ? 1 : 0;
    return v;
}

Value val_list() {
    Value v;
    v.type = VAL_LIST;
    v.data.list.items = malloc(sizeof(Value) * 10);
    v.data.list.count = 0;
    v.data.list.capacity = 10;
    return v;
}

Value val_map() {
    Value v;
    v.type = VAL_MAP;
    v.data.map.keys = malloc(sizeof(char*) * 10);
    v.data.map.values = malloc(sizeof(Value) * 10);
    v.data.map.count = 0;
    v.data.map.capacity = 10;
    return v;
}

Value val_function(VeurekFunction *func) {
    Value v;
    v.type = VAL_FUNCTION;
    v.data.function = func;
    return v;
}

Value val_class(VeurekClass *cls) {
    Value v;
    v.type = VAL_CLASS;
    v.data.class = cls;
    return v;
}

Value val_instance(VeurekInstance *inst) {
    Value v;
    v.type = VAL_INSTANCE;
    v.data.instance = inst;
    return v;
}

Value val_native_func(NativeFunction func) {
    Value v;
    v.type = VAL_NATIVE_FUNC;
    v.data.native_func = func;
    return v;
}

void val_list_add(Value *list, Value item) {
    if (list->data.list.count >= list->data.list.capacity) {
        list->data.list.capacity *= 2;
        list->data.list.items = realloc(list->data.list.items, 
                                        sizeof(Value) * list->data.list.capacity);
    }
    list->data.list.items[list->data.list.count++] = item;
}

void val_map_set(Value *map, const char *key, Value value) {
    for (int i = 0; i < map->data.map.count; i++) {
        if (strcmp(map->data.map.keys[i], key) == 0) {
            map->data.map.values[i] = value;
            return;
        }
    }
    
    if (map->data.map.count >= map->data.map.capacity) {
        map->data.map.capacity *= 2;
        map->data.map.keys = realloc(map->data.map.keys, sizeof(char*) * map->data.map.capacity);
        map->data.map.values = realloc(map->data.map.values, sizeof(Value) * map->data.map.capacity);
    }
    
    map->data.map.keys[map->data.map.count] = str_dup(key);
    map->data.map.values[map->data.map.count++] = value;
}

Value val_map_get(Value *map, const char *key) {
    for (int i = 0; i < map->data.map.count; i++) {
        if (strcmp(map->data.map.keys[i], key) == 0) {
            return map->data.map.values[i];
        }
    }
    return val_nil();
}

int is_truthy(Value v) {
    switch (v.type) {
        case VAL_NIL:
            return 0;
        case VAL_BOOL:
            return v.data.boolean;
        case VAL_NUMBER:
            return v.data.number != 0;
        case VAL_STRING:
            return strlen(v.data.string) > 0;
        case VAL_LIST:
            return v.data.list.count > 0;
        default:
            return 1;
    }
}

char* val_to_string(Value v) {
    static char buffer[4096];
    
    switch (v.type) {
        case VAL_NIL:
            strcpy(buffer, "nil");
            break;
        case VAL_NUMBER:
            if (v.data.number == (int)v.data.number) {
                snprintf(buffer, sizeof(buffer), "%d", (int)v.data.number);
            } else {
                snprintf(buffer, sizeof(buffer), "%g", v.data.number);
            }
            break;
        case VAL_STRING:
            return v.data.string;
        case VAL_BOOL:
            strcpy(buffer, v.data.boolean ? "true" : "false");
            break;
        case VAL_LIST: {
            /* Use a separate static buffer for lists to avoid recursion issues */
            static char list_buffer[4096];
            int pos = 0;
            list_buffer[pos++] = '[';
            list_buffer[pos] = '\0';
            
            for (int i = 0; i < v.data.list.count; i++) {
                /* Get item string and copy it immediately before buffer gets overwritten */
                char *item_str = val_to_string(v.data.list.items[i]);
                char temp[512];
                
                if (v.data.list.items[i].type == VAL_STRING) {
                    snprintf(temp, sizeof(temp), "\"%s\"", item_str);
                } else {
                    strncpy(temp, item_str, sizeof(temp) - 1);
                    temp[sizeof(temp) - 1] = '\0';
                }
                
                int temp_len = strlen(temp);
                if (pos + temp_len + 10 < sizeof(list_buffer)) {
                    strcpy(list_buffer + pos, temp);
                    pos += temp_len;
                    
                    if (i < v.data.list.count - 1) {
                        strcpy(list_buffer + pos, ", ");
                        pos += 2;
                    }
                }
            }
            list_buffer[pos++] = ']';
            list_buffer[pos] = '\0';
            strcpy(buffer, list_buffer);
            break;
        }
        case VAL_MAP:
            strcpy(buffer, "{map}");
            break;
        case VAL_FUNCTION:
            strcpy(buffer, "<function>");
            break;
        case VAL_CLASS:
            snprintf(buffer, sizeof(buffer), "<class %s>", v.data.class->name);
            break;
        case VAL_INSTANCE:
            snprintf(buffer, sizeof(buffer), "<%s instance>", v.data.instance->ver_class->name);
            break;
        case VAL_NATIVE_FUNC:
            strcpy(buffer, "<native function>");
            break;
    }
    return buffer;
}

/* ============ ENVIRONMENT ============ */

Environment* env_new() {
    Environment *env = malloc(sizeof(Environment));
    env->vars = malloc(sizeof(*env->vars) * 50);
    env->var_count = 0;
    env->var_capacity = 50;
    env->parent = NULL;
    return env;
}

Environment* env_child(Environment *parent) {
    Environment *env = env_new();
    env->parent = parent;
    return env;
}

void env_define(Environment *env, const char *name, Value value, int is_const) {
    if (env->var_count >= env->var_capacity) {
        env->var_capacity *= 2;
        env->vars = realloc(env->vars, sizeof(*env->vars) * env->var_capacity);
    }
    
    env->vars[env->var_count].name = str_dup(name);
    env->vars[env->var_count].value = value;
    env->vars[env->var_count].is_const = is_const;
    env->var_count++;
}

int env_get_var(Environment *env, const char *name, Value *out) {
    Environment *e = env;
    while (e) {
        for (int i = 0; i < e->var_count; i++) {
            if (strcmp(e->vars[i].name, name) == 0) {
                *out = e->vars[i].value;
                return 1;
            }
        }
        e = e->parent;
    }
    return 0;
}

int env_set_var(Environment *env, const char *name, Value value) {
    Environment *e = env;
    while (e) {
        for (int i = 0; i < e->var_count; i++) {
            if (strcmp(e->vars[i].name, name) == 0) {
                if (e->vars[i].is_const) {
                    fprintf(stderr, "!! Errore: La costante '%s' non può essere modificata\n", name);
                    return 0;
                }
                e->vars[i].value = value;
                return 1;
            }
        }
        e = e->parent;
    }
    
    /* If not found, create in current scope */
    env_define(env, name, value, 0);
    return 1;
}

/* ============ EXCEPTION FUNCTIONS ============ */

Exception exc_none() {
    Exception e;
    e.type = EXCEPTION_NONE;
    e.return_value = val_nil();
    e.error_message = NULL;
    return e;
}

Exception exc_return(Value v) {
    Exception e;
    e.type = EXCEPTION_RETURN;
    e.return_value = v;
    e.error_message = NULL;
    return e;
}

Exception exc_break() {
    Exception e;
    e.type = EXCEPTION_BREAK;
    e.return_value = val_nil();
    e.error_message = NULL;
    return e;
}

Exception exc_continue() {
    Exception e;
    e.type = EXCEPTION_CONTINUE;
    e.return_value = val_nil();
    e.error_message = NULL;
    return e;
}

Exception exc_throw(const char *msg) {
    Exception e;
    e.type = EXCEPTION_THROW;
    e.return_value = val_nil();
    e.error_message = str_dup(msg);
    return e;
}

/* ============ BUILT-IN FUNCTIONS ============ */

Value builtin_print(Interpreter *interp, Value *args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        printf("%s", val_to_string(args[i]));
        if (i < arg_count - 1) printf(" ");
    }
    printf("\n");
    return val_nil();
}

Value builtin_len(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1) {
        fprintf(stderr, "!! len() richiede 1 argomento\n");
        return val_number(0);
    }
    
    if (args[0].type == VAL_STRING) {
        return val_number(strlen(args[0].data.string));
    } else if (args[0].type == VAL_LIST) {
        return val_number(args[0].data.list.count);
    } else if (args[0].type == VAL_MAP) {
        return val_number(args[0].data.map.count);
    }
    return val_number(0);
}

Value builtin_range(Interpreter *interp, Value *args, int arg_count) {
    int start = 0, stop = 0, step = 1;
    int has_step = 0;  /* Track if step was explicitly provided */
    
    if (arg_count == 1) {
        stop = (int)args[0].data.number;
    } else if (arg_count == 2) {
        start = (int)args[0].data.number;
        stop = (int)args[1].data.number;
    } else if (arg_count == 3) {
        start = (int)args[0].data.number;
        stop = (int)args[1].data.number;
        step = (int)args[2].data.number;
        has_step = 1;
    }
    
    Value list = val_list();
    
    /* Auto-detect step direction if not explicitly provided */
    if (!has_step) {
        if (start >= stop) {
            step = -1;
        } else {
            step = 1;
        }
    }
    
    /* Support both increasing and decreasing ranges */
    if (step > 0) {
        for (int i = start; i < stop; i += step) {
            val_list_add(&list, val_number(i));
        }
    } else {
        for (int i = start; i > stop; i += step) {
            val_list_add(&list, val_number(i));
        }
    }
    
    return list;
}

Value builtin_str(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1) {
        fprintf(stderr, "!! str() richiede 1 argomento\n");
        return val_string("");
    }
    return val_string(val_to_string(args[0]));
}

Value builtin_int(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1) {
        fprintf(stderr, "!! int() richiede 1 argomento\n");
        return val_number(0);
    }
    
    if (args[0].type == VAL_NUMBER) {
        return val_number((int)args[0].data.number);
    } else if (args[0].type == VAL_STRING) {
        return val_number(atoi(args[0].data.string));
    }
    return val_number(0);
}

Value builtin_float(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1) {
        fprintf(stderr, "!! float() richiede 1 argomento\n");
        return val_number(0);
    }
    
    if (args[0].type == VAL_NUMBER) {
        return args[0];
    } else if (args[0].type == VAL_STRING) {
        return val_number(atof(args[0].data.string));
    }
    return val_number(0);
}

Value builtin_type(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1) {
        fprintf(stderr, "!! type() richiede 1 argomento\n");
        return val_string("unknown");
    }
    
    switch (args[0].type) {
        case VAL_NIL: return val_string("nil");
        case VAL_NUMBER: return val_string("number");
        case VAL_STRING: return val_string("string");
        case VAL_BOOL: return val_string("bool");
        case VAL_LIST: return val_string("list");
        case VAL_MAP: return val_string("map");
        case VAL_FUNCTION: return val_string("function");
        case VAL_CLASS: return val_string("class");
        case VAL_INSTANCE: return val_string("instance");
        case VAL_NATIVE_FUNC: return val_string("native_function");
        default: return val_string("unknown");
    }
}

Value builtin_input(Interpreter *interp, Value *args, int arg_count) {
    char buffer[1024];
    if (arg_count > 0) {
        printf("%s", val_to_string(args[0]));
        fflush(stdout);
    }
    if (fgets(buffer, sizeof(buffer), stdin)) {
        int len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        return val_string(buffer);
    }
    return val_string("");
}

Value builtin_map(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 2) {
        fprintf(stderr, "!! map() richiede 2 argomenti\n");
        return val_list();
    }
    
    if (args[0].type != VAL_LIST) {
        fprintf(stderr, "!! map() richiede una lista come primo argomento\n");
        return val_list();
    }
    
    Value result = val_list();
    for (int i = 0; i < args[0].data.list.count; i++) {
        Value item_args[1] = {args[0].data.list.items[i]};
        Value mapped = interpreter_call_function(interp, args[1], item_args, 1);
        val_list_add(&result, mapped);
    }
    return result;
}

Value builtin_filter(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 2) {
        fprintf(stderr, "!! filter() richiede 2 argomenti\n");
        return val_list();
    }
    
    if (args[0].type != VAL_LIST) {
        fprintf(stderr, "!! filter() richiede una lista come primo argomento\n");
        return val_list();
    }
    
    Value result = val_list();
    for (int i = 0; i < args[0].data.list.count; i++) {
        Value item_args[1] = {args[0].data.list.items[i]};
        Value keep = interpreter_call_function(interp, args[1], item_args, 1);
        if (is_truthy(keep)) {
            val_list_add(&result, args[0].data.list.items[i]);
        }
    }
    return result;
}

Value builtin_reduce(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count < 2) {
        fprintf(stderr, "!! reduce() richiede almeno 2 argomenti\n");
        return val_nil();
    }
    
    if (args[0].type != VAL_LIST) {
        fprintf(stderr, "!! reduce() richiede una lista come primo argomento\n");
        return val_nil();
    }
    
    Value acc;
    int start_idx = 0;
    
    if (arg_count == 3) {
        acc = args[2];
    } else {
        if (args[0].data.list.count == 0) return val_nil();
        acc = args[0].data.list.items[0];
        start_idx = 1;
    }
    
    for (int i = start_idx; i < args[0].data.list.count; i++) {
        Value reduce_args[2] = {acc, args[0].data.list.items[i]};
        acc = interpreter_call_function(interp, args[1], reduce_args, 2);
    }
    return acc;
}

Value builtin_sum(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1 || args[0].type != VAL_LIST) {
        return val_number(0);
    }
    
    double sum = 0;
    for (int i = 0; i < args[0].data.list.count; i++) {
        if (args[0].data.list.items[i].type == VAL_NUMBER) {
            sum += args[0].data.list.items[i].data.number;
        }
    }
    return val_number(sum);
}

Value builtin_max(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1 || args[0].type != VAL_LIST || args[0].data.list.count == 0) {
        return val_nil();
    }
    
    double max = args[0].data.list.items[0].data.number;
    for (int i = 1; i < args[0].data.list.count; i++) {
        if (args[0].data.list.items[i].type == VAL_NUMBER) {
            if (args[0].data.list.items[i].data.number > max) {
                max = args[0].data.list.items[i].data.number;
            }
        }
    }
    return val_number(max);
}

Value builtin_min(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1 || args[0].type != VAL_LIST || args[0].data.list.count == 0) {
        return val_nil();
    }
    
    double min = args[0].data.list.items[0].data.number;
    for (int i = 1; i < args[0].data.list.count; i++) {
        if (args[0].data.list.items[i].type == VAL_NUMBER) {
            if (args[0].data.list.items[i].data.number < min) {
                min = args[0].data.list.items[i].data.number;
            }
        }
    }
    return val_number(min);
}

Value builtin_abs(Interpreter *interp, Value *args, int arg_count) {
    if (arg_count != 1 || args[0].type != VAL_NUMBER) {
        return val_number(0);
    }
    return val_number(fabs(args[0].data.number));
}

/* ============ INTERPRETER ============ */

Interpreter* interpreter_new() {
    Interpreter *interp = malloc(sizeof(Interpreter));
    interp->global_env = env_new();
    interp->current_env = interp->global_env;
    interp->last_exception = exc_none();
    
    /* Register built-in functions */
    env_define(interp->global_env, "print", val_native_func(builtin_print), 1);
    env_define(interp->global_env, "len", val_native_func(builtin_len), 1);
    env_define(interp->global_env, "range", val_native_func(builtin_range), 1);
    env_define(interp->global_env, "str", val_native_func(builtin_str), 1);
    env_define(interp->global_env, "int", val_native_func(builtin_int), 1);
    env_define(interp->global_env, "float", val_native_func(builtin_float), 1);
    env_define(interp->global_env, "type", val_native_func(builtin_type), 1);
    env_define(interp->global_env, "input", val_native_func(builtin_input), 1);
    env_define(interp->global_env, "map", val_native_func(builtin_map), 1);
    env_define(interp->global_env, "filter", val_native_func(builtin_filter), 1);
    env_define(interp->global_env, "reduce", val_native_func(builtin_reduce), 1);
    env_define(interp->global_env, "sum", val_native_func(builtin_sum), 1);
    env_define(interp->global_env, "max", val_native_func(builtin_max), 1);
    env_define(interp->global_env, "min", val_native_func(builtin_min), 1);
    env_define(interp->global_env, "abs", val_native_func(builtin_abs), 1);
    
    return interp;
}

Value interpreter_execute(Interpreter *interp, ASTNode *node) {
    if (!node) return val_nil();
    
    switch (node->type) {
        case AST_PROGRAM: {
            Value result = val_nil();
            for (int i = 0; i < node->children_count; i++) {
                result = interpreter_execute(interp, node->children[i]);
                if (interp->last_exception.type != EXCEPTION_NONE) {
                    break;
                }
            }
            return result;
        }
        
        case AST_INCLUDE: {
            if (node->str_val) {
                char filepath[1024];
                if (strstr(node->str_val, ".ver") == NULL) {
                    snprintf(filepath, sizeof(filepath), "%s.ver", node->str_val);
                } else {
                    snprintf(filepath, sizeof(filepath), "%s", node->str_val);
                }
                
                FILE *f = fopen(filepath, "r");
                if (!f) {
                    /* Try lib/ directory */
                    snprintf(filepath, sizeof(filepath), "lib/%s.ver", node->str_val);
                    f = fopen(filepath, "r");
                }
                
                if (!f) {
                    fprintf(stderr, "!! File non trovato: %s\n", node->str_val);
                    return val_nil();
                }
                
                fseek(f, 0, SEEK_END);
                long size = ftell(f);
                fseek(f, 0, SEEK_SET);
                char *lib_source = malloc(size + 1);
                fread(lib_source, 1, size, f);
                lib_source[size] = '\0';
                fclose(f);
                
                Lexer lib_lexer = lexer_new(lib_source);
                TokenList *lib_tokens = lexer_tokenize(&lib_lexer);
                Parser lib_parser = parser_new(lib_tokens);
                ASTNode *lib_program = parser_parse_program(&lib_parser);
                
                Environment *saved_env = interp->current_env;
                interp->current_env = interp->global_env;
                
                interpreter_execute(interp, lib_program);
                
                interp->current_env = saved_env;
                free(lib_source);
            }
            return val_nil();
        }
        
        case AST_LET: {
            Value value = interpreter_execute(interp, node->children[0]);
            env_define(interp->current_env, node->name, value, node->is_const);
            return value;
        }
        
        case AST_ASSIGN: {
            Value value = interpreter_execute(interp, node->children[0]);
            env_set_var(interp->current_env, node->name, value);
            return value;
        }
        
        case AST_COMPOUND_ASSIGN: {
            Value current_value;
            if (!env_get_var(interp->current_env, node->name, &current_value)) {
                fprintf(stderr, "!! Variabile '%s' non definita\n", node->name);
                return val_nil();
            }
            
            Value new_value = interpreter_execute(interp, node->children[0]);
            Value result;
            
            if (strcmp(node->op, "+=") == 0) {
                if (current_value.type == VAL_STRING || new_value.type == VAL_STRING) {
                    char buffer[2048];
                    snprintf(buffer, sizeof(buffer), "%s%s", 
                            val_to_string(current_value), val_to_string(new_value));
                    result = val_string(buffer);
                } else {
                    result = val_number(current_value.data.number + new_value.data.number);
                }
            }
            else if (strcmp(node->op, "-=") == 0) {
                result = val_number(current_value.data.number - new_value.data.number);
            }
            else if (strcmp(node->op, "*=") == 0) {
                result = val_number(current_value.data.number * new_value.data.number);
            }
            else if (strcmp(node->op, "/=") == 0) {
                result = val_number(current_value.data.number / new_value.data.number);
            }
            
            env_set_var(interp->current_env, node->name, result);
            return result;
        }
        
        case AST_INCREMENT: {
            ASTNode *target = node->children[0];
            Value current_value;
            
            if (target->type == AST_VAR) {
                if (!env_get_var(interp->current_env, target->name, &current_value)) {
                    fprintf(stderr, "!! Variabile '%s' non definita\n", target->name);
                    return val_nil();
                }
                
                double new_val = current_value.data.number;
                if (strcmp(node->op, "++") == 0) {
                    new_val += 1;
                } else {
                    new_val -= 1;
                }
                
                env_set_var(interp->current_env, target->name, val_number(new_val));
                return node->prefix ? val_number(new_val) : current_value;
            }
            else if (target->type == AST_ATTR) {
                Value obj = interpreter_execute(interp, target->children[0]);
                if (obj.type == VAL_INSTANCE) {
                    for (int i = 0; i < obj.data.instance->fields_count; i++) {
                        if (strcmp(obj.data.instance->fields[i].name, target->name) == 0) {
                            current_value = obj.data.instance->fields[i].value;
                            double new_val = current_value.data.number;
                            if (strcmp(node->op, "++") == 0) {
                                new_val += 1;
                            } else {
                                new_val -= 1;
                            }
                            obj.data.instance->fields[i].value = val_number(new_val);
                            return node->prefix ? val_number(new_val) : current_value;
                        }
                    }
                }
            }
            return val_nil();
        }
        
        case AST_VAR: {
            Value result;
            if (env_get_var(interp->current_env, node->name, &result)) {
                return result;
            }
            fprintf(stderr, "!! Variabile '%s' non definita\n", node->name);
            return val_nil();
        }
        
        case AST_LITERAL: {
            if (node->str_val) {
                return val_string(node->str_val);
            } else if (node->bool_val == 1) {
                /* Explicitly true */
                return val_bool(1);
            } else if (node->bool_val == 0 && node->num_val == 0 && !node->str_val) {
                /* Check if it's false (bool_val=0, num_val=0, no string) */
                /* But we need to distinguish from number 0 */
                /* Check if this was parsed as a boolean token (false) or number 0 */
                /* Since we can't distinguish, check if there's a string - if no string and
                   both are 0, it could be either. We prioritize bool for backward compat */
                return val_bool(0);
            } else {
                return val_number(node->num_val);
            }
        }
        
        case AST_LIST: {
            Value list = val_list();
            for (int i = 0; i < node->children_count; i++) {
                Value elem = interpreter_execute(interp, node->children[i]);
                val_list_add(&list, elem);
            }
            return list;
        }
        
        case AST_MAP: {
            Value map = val_map();
            for (int i = 0; i < node->map_count; i++) {
                Value value = interpreter_execute(interp, node->map_values[i]);
                val_map_set(&map, node->map_keys[i], value);
            }
            return map;
        }
        
        case AST_FN: {
            VeurekFunction *func = malloc(sizeof(VeurekFunction));
            func->params = node->params;
            func->params_count = node->params_count;
            func->body = node->children;
            func->body_count = node->children_count;
            func->closure = interp->current_env;
            
            Value func_val = val_function(func);
            if (node->name) {
                env_define(interp->current_env, node->name, func_val, 0);
            }
            return func_val;
        }
        
        case AST_CALL: {
            Value func = interpreter_execute(interp, node->children[0]);
            
            Value *args = malloc(sizeof(Value) * (node->children_count - 1));
            for (int i = 1; i < node->children_count; i++) {
                args[i - 1] = interpreter_execute(interp, node->children[i]);
            }
            
            Value result = interpreter_call_function(interp, func, args, node->children_count - 1);
            free(args);
            return result;
        }
        
        case AST_BINARY_OP: {
            Value left = interpreter_execute(interp, node->children[0]);
            Value right = interpreter_execute(interp, node->children[1]);
            
            if (strcmp(node->op, "+") == 0) {
                if (left.type == VAL_STRING || right.type == VAL_STRING) {
                    char result[2048];
                    snprintf(result, sizeof(result), "%s%s", 
                            val_to_string(left), val_to_string(right));
                    return val_string(result);
                } else if (left.type == VAL_LIST && right.type == VAL_LIST) {
                    /* List concatenation */
                    Value result = val_list();
                    for (int i = 0; i < left.data.list.count; i++) {
                        val_list_add(&result, left.data.list.items[i]);
                    }
                    for (int i = 0; i < right.data.list.count; i++) {
                        val_list_add(&result, right.data.list.items[i]);
                    }
                    return result;
                } else if (left.type == VAL_LIST && right.type != VAL_LIST) {
                    /* List + single value */
                    Value result = val_list();
                    for (int i = 0; i < left.data.list.count; i++) {
                        val_list_add(&result, left.data.list.items[i]);
                    }
                    val_list_add(&result, right);
                    return result;
                } else if (left.type != VAL_LIST && right.type == VAL_LIST) {
                    /* Single value + List */
                    Value result = val_list();
                    val_list_add(&result, left);
                    for (int i = 0; i < right.data.list.count; i++) {
                        val_list_add(&result, right.data.list.items[i]);
                    }
                    return result;
                } else {
                    return val_number(left.data.number + right.data.number);
                }
            }
            else if (strcmp(node->op, "-") == 0) {
                return val_number(left.data.number - right.data.number);
            }
            else if (strcmp(node->op, "*") == 0) {
                return val_number(left.data.number * right.data.number);
            }
            else if (strcmp(node->op, "/") == 0) {
                if (right.data.number == 0) {
                    fprintf(stderr, "!! Errore: divisione per zero\n");
                    return val_number(0);
                }
                return val_number(left.data.number / right.data.number);
            }
            else if (strcmp(node->op, "%") == 0) {
                return val_number((int)left.data.number % (int)right.data.number);
            }
            else if (strcmp(node->op, "**") == 0) {
                return val_number(pow(left.data.number, right.data.number));
            }
            else if (strcmp(node->op, "==") == 0) {
                if (left.type != right.type) return val_bool(0);
                switch (left.type) {
                    case VAL_NUMBER:
                        return val_bool(left.data.number == right.data.number);
                    case VAL_STRING:
                        return val_bool(strcmp(left.data.string, right.data.string) == 0);
                    case VAL_BOOL:
                        return val_bool(left.data.boolean == right.data.boolean);
                    default:
                        return val_bool(0);
                }
            }
            else if (strcmp(node->op, "!=") == 0) {
                if (left.type != right.type) return val_bool(1);
                switch (left.type) {
                    case VAL_NUMBER:
                        return val_bool(left.data.number != right.data.number);
                    case VAL_STRING:
                        return val_bool(strcmp(left.data.string, right.data.string) != 0);
                    case VAL_BOOL:
                        return val_bool(left.data.boolean != right.data.boolean);
                    default:
                        return val_bool(1);
                }
            }
            else if (strcmp(node->op, "<") == 0) {
                return val_bool(left.data.number < right.data.number);
            }
            else if (strcmp(node->op, "<=") == 0) {
                return val_bool(left.data.number <= right.data.number);
            }
            else if (strcmp(node->op, ">") == 0) {
                return val_bool(left.data.number > right.data.number);
            }
            else if (strcmp(node->op, ">=") == 0) {
                return val_bool(left.data.number >= right.data.number);
            }
            else if (strcmp(node->op, "and") == 0) {
                return val_bool(is_truthy(left) && is_truthy(right));
            }
            else if (strcmp(node->op, "or") == 0) {
                return val_bool(is_truthy(left) || is_truthy(right));
            }
            else if (strcmp(node->op, "&") == 0) {
                return val_number((int)left.data.number & (int)right.data.number);
            }
            else if (strcmp(node->op, "|") == 0) {
                return val_number((int)left.data.number | (int)right.data.number);
            }
            else if (strcmp(node->op, "^") == 0) {
                return val_number((int)left.data.number ^ (int)right.data.number);
            }
            
            return val_nil();
        }
        
        case AST_UNARY_OP: {
            Value operand = interpreter_execute(interp, node->children[0]);
            
            if (strcmp(node->op, "-") == 0) {
                return val_number(-operand.data.number);
            }
            else if (strcmp(node->op, "not") == 0) {
                return val_bool(!is_truthy(operand));
            }
            else if (strcmp(node->op, "~") == 0) {
                return val_number(~(int)operand.data.number);
            }
            
            return val_nil();
        }
        
        case AST_IF: {
            Value cond = interpreter_execute(interp, node->children[0]);
            
            if (is_truthy(cond)) {
                if (node->children_count > 1 && node->children[1]->type == AST_PROGRAM) {
                    for (int i = 0; i < node->children[1]->children_count; i++) {
                        interpreter_execute(interp, node->children[1]->children[i]);
                        if (interp->last_exception.type != EXCEPTION_NONE) break;
                    }
                }
            } else {
                if (node->children_count > 2) {
                    if (node->children[2]->type == AST_PROGRAM) {
                        for (int i = 0; i < node->children[2]->children_count; i++) {
                            interpreter_execute(interp, node->children[2]->children[i]);
                            if (interp->last_exception.type != EXCEPTION_NONE) break;
                        }
                    } else if (node->children[2]->type == AST_IF) {
                        interpreter_execute(interp, node->children[2]);
                    }
                }
            }
            return val_nil();
        }
        
        case AST_FOR: {
            Value iterable = interpreter_execute(interp, node->children[0]);
            
            if (iterable.type != VAL_LIST) {
                fprintf(stderr, "!! for loop richiede una lista\n");
                return val_nil();
            }
            
            for (int i = 0; i < iterable.data.list.count; i++) {
                /* Create a new environment for each iteration to support proper scoping */
                Environment *loop_env = env_child(interp->current_env);
                
                env_define(loop_env, node->name, iterable.data.list.items[i], 0);
                
                Environment *prev_env = interp->current_env;
                interp->current_env = loop_env;
                
                for (int j = 1; j < node->children_count; j++) {
                    interpreter_execute(interp, node->children[j]);
                    
                    if (interp->last_exception.type == EXCEPTION_BREAK) {
                        interp->last_exception = exc_none();
                        interp->current_env = prev_env;
                        return val_nil();
                    }
                    if (interp->last_exception.type == EXCEPTION_CONTINUE) {
                        interp->last_exception = exc_none();
                        break;
                    }
                }
                
                interp->current_env = prev_env;
            }
            return val_nil();
        }
        
        case AST_WHILE: {
            while (is_truthy(interpreter_execute(interp, node->children[0]))) {
                for (int i = 1; i < node->children_count; i++) {
                    interpreter_execute(interp, node->children[i]);
                    
                    if (interp->last_exception.type == EXCEPTION_BREAK) {
                        interp->last_exception = exc_none();
                        return val_nil();
                    }
                    if (interp->last_exception.type == EXCEPTION_CONTINUE) {
                        interp->last_exception = exc_none();
                        break;
                    }
                }
            }
            return val_nil();
        }
        
        case AST_RETURN: {
            Value ret_val = val_nil();
            if (node->children_count > 0) {
                ret_val = interpreter_execute(interp, node->children[0]);
            }
            interp->last_exception = exc_return(ret_val);
            return ret_val;
        }
        
        case AST_BREAK: {
            interp->last_exception = exc_break();
            return val_nil();
        }
        
        case AST_CONTINUE: {
            interp->last_exception = exc_continue();
            return val_nil();
        }
        
        case AST_THROW: {
            Value error = interpreter_execute(interp, node->children[0]);
            interp->last_exception = exc_throw(val_to_string(error));
            return val_nil();
        }
        
        case AST_TRY: {
            /* Try block - save exception state */
            Exception saved_exc = interp->last_exception;
            interp->last_exception = exc_none();
            
            for (int i = 0; i < node->children_count; i++) {
                interpreter_execute(interp, node->children[i]);
                
                if (interp->last_exception.type == EXCEPTION_THROW) {
                    if (node->name) {
                        Environment *catch_env = env_child(interp->current_env);
                        env_define(catch_env, node->name, 
                                 val_string(interp->last_exception.error_message), 0);
                        
                        Environment *prev_env = interp->current_env;
                        interp->current_env = catch_env;
                        interp->last_exception = exc_none();
                        
                        /* Execute catch block */
                        for (int j = i + 1; j < node->children_count; j++) {
                            interpreter_execute(interp, node->children[j]);
                        }
                        
                        interp->current_env = prev_env;
                    }
                    break;
                }
            }
            
            /* Restore exception if not handled */
            if (interp->last_exception.type == EXCEPTION_NONE) {
                interp->last_exception = saved_exc;
            }
            return val_nil();
        }
        
        case AST_INDEX: {
            Value obj = interpreter_execute(interp, node->children[0]);
            Value index = interpreter_execute(interp, node->children[1]);
            
            if (obj.type == VAL_LIST) {
                int idx = (int)index.data.number;
                if (idx >= 0 && idx < obj.data.list.count) {
                    return obj.data.list.items[idx];
                }
            } else if (obj.type == VAL_MAP) {
                return val_map_get(&obj, val_to_string(index));
            }
            return val_nil();
        }
        
        case AST_ATTR: {
            Value obj = interpreter_execute(interp, node->children[0]);
            
            if (obj.type == VAL_INSTANCE) {
                /* Check fields */
                for (int i = 0; i < obj.data.instance->fields_count; i++) {
                    if (strcmp(obj.data.instance->fields[i].name, node->name) == 0) {
                        return obj.data.instance->fields[i].value;
                    }
                }
                
                /* Check methods */
                for (int i = 0; i < obj.data.instance->ver_class->methods_count; i++) {
                    if (strcmp(obj.data.instance->ver_class->methods[i].name, node->name) == 0) {
                        VeurekFunction *method = obj.data.instance->ver_class->methods[i].func;
                        
                        /* Bind self */
                        Environment *method_env = env_child(method->closure);
                        env_define(method_env, "self", obj, 0);
                        
                        VeurekFunction *bound = malloc(sizeof(VeurekFunction));
                        bound->params = method->params;
                        bound->params_count = method->params_count;
                        bound->body = method->body;
                        bound->body_count = method->body_count;
                        bound->closure = method_env;
                        
                        return val_function(bound);
                    }
                }
            }
            return val_nil();
        }
        
        case AST_ATTR_ASSIGN: {
            Value obj = interpreter_execute(interp, node->children[0]);
            Value value = interpreter_execute(interp, node->children[1]);
            
            if (obj.type == VAL_INSTANCE) {
                /* Check if field exists */
                for (int i = 0; i < obj.data.instance->fields_count; i++) {
                    if (strcmp(obj.data.instance->fields[i].name, node->name) == 0) {
                        obj.data.instance->fields[i].value = value;
                        return value;
                    }
                }
                
                /* Create new field */
                obj.data.instance->fields = realloc(obj.data.instance->fields,
                    sizeof(*obj.data.instance->fields) * (obj.data.instance->fields_count + 1));
                obj.data.instance->fields[obj.data.instance->fields_count].name = str_dup(node->name);
                obj.data.instance->fields[obj.data.instance->fields_count].value = value;
                obj.data.instance->fields_count++;
            }
            return value;
        }
        
        case AST_NEW: {
            Value class_val;
            if (!env_get_var(interp->current_env, node->name, &class_val)) {
                fprintf(stderr, "!! Classe '%s' non definita\n", node->name);
                return val_nil();
            }
            
            if (class_val.type != VAL_CLASS) {
                fprintf(stderr, "!! '%s' non è una classe\n", node->name);
                return val_nil();
            }
            
            VeurekInstance *inst = malloc(sizeof(VeurekInstance));
            inst->ver_class = class_val.data.class;
            inst->fields = NULL;
            inst->fields_count = 0;
            
            Value inst_val = val_instance(inst);
            
            /* Call __init__ if exists */
            for (int i = 0; i < class_val.data.class->methods_count; i++) {
                if (strcmp(class_val.data.class->methods[i].name, "__init__") == 0) {
                    VeurekFunction *init = class_val.data.class->methods[i].func;
                    
                    /* Bind self */
                    Environment *init_env = env_child(init->closure);
                    env_define(init_env, "self", inst_val, 0);
                    
                    VeurekFunction *bound_init = malloc(sizeof(VeurekFunction));
                    bound_init->params = init->params;
                    bound_init->params_count = init->params_count;
                    bound_init->body = init->body;
                    bound_init->body_count = init->body_count;
                    bound_init->closure = init_env;
                    
                    /* Evaluate constructor args */
                    Value *args = malloc(sizeof(Value) * node->children_count);
                    for (int j = 0; j < node->children_count; j++) {
                        args[j] = interpreter_execute(interp, node->children[j]);
                    }
                    
                    interpreter_call_function(interp, val_function(bound_init), args, node->children_count);
                    free(args);
                    break;
                }
            }
            
            return inst_val;
        }
        
        case AST_CLASS: {
            VeurekClass *cls = malloc(sizeof(VeurekClass));
            cls->name = str_dup(node->name);
            cls->methods = malloc(sizeof(*cls->methods) * 10);
            cls->methods_count = 0;
            
            for (int i = 0; i < node->children_count; i++) {
                if (node->children[i]->type == AST_FN) {
                    VeurekFunction *func = malloc(sizeof(VeurekFunction));
                    func->params = node->children[i]->params;
                    func->params_count = node->children[i]->params_count;
                    func->body = node->children[i]->children;
                    func->body_count = node->children[i]->children_count;
                    func->closure = interp->current_env;
                    
                    cls->methods[cls->methods_count].name = str_dup(node->children[i]->name);
                    cls->methods[cls->methods_count].func = func;
                    cls->methods_count++;
                }
            }
            
            Value class_val = val_class(cls);
            env_define(interp->current_env, node->name, class_val, 0);
            return class_val;
        }
        
        default:
            return val_nil();
    }
}

Value interpreter_call_function(Interpreter *interp, Value func, Value *args, int arg_count) {
    if (func.type == VAL_NATIVE_FUNC) {
        return func.data.native_func(interp, args, arg_count);
    }
    
    if (func.type == VAL_FUNCTION) {
        VeurekFunction *vfunc = func.data.function;
        
        /* Create new environment */
        Environment *func_env = env_child(vfunc->closure);
        
        /* Bind parameters */
        for (int i = 0; i < vfunc->params_count && i < arg_count; i++) {
            env_define(func_env, vfunc->params[i], args[i], 0);
        }
        
        /* Execute function body */
        Environment *prev_env = interp->current_env;
        interp->current_env = func_env;
        
        Value result = val_nil();
        for (int i = 0; i < vfunc->body_count; i++) {
            result = interpreter_execute(interp, vfunc->body[i]);
            
            if (interp->last_exception.type == EXCEPTION_RETURN) {
                result = interp->last_exception.return_value;
                interp->last_exception = exc_none();
                break;
            }
        }
        
        interp->current_env = prev_env;
        return result;
    }
    
    fprintf(stderr, "!! Oggetto non chiamabile\n");
    return val_nil();
}

/* ============ MAIN EXECUTION ============ */

void run_ver(const char *source, const char *filename, Interpreter *existing_interp) {
    Lexer lexer = lexer_new(source);
    TokenList *tokens = lexer_tokenize(&lexer);
    
    Parser parser = parser_new(tokens);
    ASTNode *program = parser_parse_program(&parser);
    
    Interpreter *interp;
    
    if (existing_interp != NULL) {
        interp = existing_interp;
    } else {
        interp = interpreter_new();
    }
    
    interpreter_execute(interp, program);
}

void run_file(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "!! File non trovato: %s\n", filepath);
        exit(1);
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    
    run_ver(source, filepath, NULL);
    free(source);
}

void repl() {
    printf("============================================================\n");
    printf("Veureka REPL - Linguaggio di Programmazione Interattivo\n");
    printf("============================================================\n");
    printf("Digita 'exit' o 'quit' per uscire\n");
    printf("Digita 'help' per vedere i comandi disponibili\n");
    printf("Versione 0.0.1\n\n");
    
    char line[1024];
    Interpreter *interp = interpreter_new();
    
    while (1) {
        printf("ver> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\nArrivederci!\n");
            break;
        }
        
        int len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        if (strlen(line) == 0) continue;
        
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            printf("Arrivederci!\n");
            break;
        }
        
        if (strcmp(line, "help") == 0) {
            printf("\nComandi disponibili:\n");
            printf("  exit, quit    - Esci dal REPL\n");
            printf("  help          - Mostra questo messaggio\n");
            printf("  vars          - Mostra tutte le variabili\n\n");
            printf("Esempi:\n");
            printf("  let x = 10\n");
            printf("  fn quadrato(n) => n * n\n");
            printf("  print(quadrato(5))\n");
            continue;
        }
        
        if (strcmp(line, "vars") == 0) {
            printf("Variabili globali:\n");
            for (int i = 0; i < interp->current_env->var_count; i++) {
                printf("  %s = %s\n", interp->current_env->vars[i].name, 
                       val_to_string(interp->current_env->vars[i].value));
            }
            continue;
        }
        
        Lexer lexer = lexer_new(line);
        TokenList *tokens = lexer_tokenize(&lexer);
        Parser parser = parser_new(tokens);
        ASTNode *program = parser_parse_program(&parser);
        
        for (int i = 0; i < program->children_count; i++) {
            Value result = interpreter_execute(interp, program->children[i]);
            if (program->children[i]->type != AST_LET && 
                program->children[i]->type != AST_FN &&
                program->children[i]->type != AST_CLASS &&
                result.type != VAL_NIL) {
                printf("%s\n", val_to_string(result));
            }
        }
    }
}

void run_examples() {
    printf("============================================================\n");
    printf("Veureka - Linguaggio di Programmazione\n");
    printf("============================================================\n\n");
    
    printf("Esempio 1: Variabili e Funzioni\n");
    run_ver(
        "let nome = \"Mario\"\n"
        "let eta = 25\n"
        "fn saluta(persona)\n"
        "    print(\"Ciao, \" + persona + \"!\")\n"
        "end\n"
        "saluta(nome)\n",
        "<example1>", NULL
    );
    
    printf("\nEsempio 2: Liste e Iterazione\n");
    run_ver(
        "let numeri = [1, 2, 3, 4, 5]\n"
        "print(\"Numeri:\", numeri)\n"
        "for n in numeri\n"
        "    print(n * 2)\n"
        "end\n",
        "<example2>", NULL
    );
    
    printf("\nEsempio 3: Lambda e Higher-Order Functions\n");
    run_ver(
        "let numeri = [1, 2, 3, 4, 5]\n"
        "let doppio = fn(x) => x * 2\n"
        "let quadrati = map(numeri, fn(n) => n * n)\n"
        "print(\"Doppi:\", map(numeri, doppio))\n"
        "print(\"Quadrati:\", quadrati)\n"
        "let pari = filter(numeri, fn(n) => n % 2 == 0)\n"
        "print(\"Numeri pari:\", pari)\n",
        "<example3>", NULL
    );
    
    printf("\nEsempio 4: Fibonacci Ricorsivo\n");
    run_ver(
        "fn fibonacci(n)\n"
        "    if n < 2\n"
        "        return n\n"
        "    end\n"
        "    return fibonacci(n - 1) + fibonacci(n - 2)\n"
        "end\n"
        "print(\"fibonacci(10) =\", fibonacci(10))\n",
        "<example4>", NULL
    );
    
    printf("\nEsempio 5: Classi e OOP\n");
    run_ver(
        "class Persona\n"
        "    fn __init__(nome, eta)\n"
        "        self.nome = nome\n"
        "        self.eta = eta\n"
        "    end\n"
        "    fn saluta()\n"
        "        print(\"Ciao, sono \" + self.nome + \" e ho \" + str(self.eta) + \" anni\")\n"
        "    end\n"
        "    fn compleanno()\n"
        "        self.eta += 1\n"
        "        print(\"Buon compleanno! Ora ho \" + str(self.eta) + \" anni\")\n"
        "    end\n"
        "end\n"
        "let mario = new Persona(\"Mario\", 25)\n"
        "mario.saluta()\n"
        "mario.compleanno()\n",
        "<example5>", NULL
    );
    
    printf("\n============================================================\n");
    printf("Tutti gli esempi completati!\n");
    printf("============================================================\n");
}

void show_help() {
    printf("Veureka - Linguaggio di Programmazione\n\n");
    printf("Uso:\n");
    printf("    veureka                    # Avvia REPL interattivo\n");
    printf("    veureka script.ver        # Esegue un file\n");
    printf("    veureka --examples         # Esegue gli esempi\n");
    printf("    veureka --help             # Mostra questo messaggio\n\n");
    printf("Per maggiori informazioni, visita: https://github.com/vincenzofranchino/veureka-lang\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            show_help();
        } else if (strcmp(argv[1], "--examples") == 0) {
            run_examples();
        } else {
            run_file(argv[1]);
        }
    } else {
        fprintf(stderr, "!! Uso: veureka [file.ver] [--help] [--examples]\n");
        exit(1);
    }
    
    return 0;
}
