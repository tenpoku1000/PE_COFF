
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_PARSE_TREE_H_)
#define TP_COMPILER_PARSE_TREE_H_

#include "tp_compiler_token.h"

// ----------------------------------------------------------------------------------------
// parse tree section:

#define IS_TOKEN_ID(token) ((token) && (TP_SYMBOL_ID == (token)->member_symbol))
#define IS_TOKEN_CONST_VALUE(token) ((token) && (TP_SYMBOL_CONST_VALUE == (token)->member_symbol))
#define IS_TOKEN_PLUS(token) ((token) && (TP_SYMBOL_PLUS == (token)->member_symbol))
#define IS_TOKEN_MINUS(token) ((token) && (TP_SYMBOL_MINUS == (token)->member_symbol))
#define IS_TOKEN_MUL(token) ((token) && (TP_SYMBOL_MUL == (token)->member_symbol))
#define IS_TOKEN_DIV(token) ((token) && (TP_SYMBOL_DIV == (token)->member_symbol))
#define IS_TOKEN_LEFT_PAREN(token) ((token) && (TP_SYMBOL_LEFT_PAREN == (token)->member_symbol))
#define IS_TOKEN_RIGHT_PAREN(token) ((token) && (TP_SYMBOL_RIGHT_PAREN == (token)->member_symbol))
#define IS_TOKEN_EQUAL(token) ((token) && (TP_SYMBOL_EQUAL == (token)->member_symbol))
#define IS_TOKEN_SEMICOLON(token) ((token) && (TP_SYMBOL_SEMICOLON == (token)->member_symbol))

#define IS_TOKEN_TYPE_UNSPECIFIED_TYPE(token) ((token) && (TP_SYMBOL_UNSPECIFIED_TYPE == (token)->member_symbol_type))
#define IS_TOKEN_TYPE_ID_INT32(token) ((token) && (TP_SYMBOL_ID_INT32 == (token)->member_symbol_type))
#define IS_TOKEN_TYPE_TYPE_INT32(token) ((token) && (TP_SYMBOL_TYPE_INT32 == (token)->member_symbol_type))
#define IS_TOKEN_TYPE_CONST_VALUE_INT32(token) ((token) && (TP_SYMBOL_CONST_VALUE_INT32 == (token)->member_symbol_type))

#define IS_TOKEN_STRING_ID_INT32(token) \
    ((token) && (TP_SYMBOL_ID == (token)->member_symbol) && (0 == strcmp("int32_t", (token)->member_string)))

typedef enum TP_PARSE_TREE_TYPE_
{
    TP_PARSE_TREE_TYPE_NULL = 0,
    TP_PARSE_TREE_TYPE_TOKEN,
    TP_PARSE_TREE_TYPE_NODE
}TP_PARSE_TREE_TYPE;

typedef union tp_parse_tree_element_union_{
    TP_TOKEN* member_tp_token;
    struct tp_parse_tree_* member_child;
}TP_PARSE_TREE_ELEMENT_UNION;

typedef struct tp_parse_tree_element_{
    TP_PARSE_TREE_TYPE member_type;
    TP_PARSE_TREE_ELEMENT_UNION member_body;
}TP_PARSE_TREE_ELEMENT;

typedef enum TP_PARSE_TREE_GRAMMER_
{
    TP_PARSE_TREE_GRAMMER_PROGRAM,
    TP_PARSE_TREE_GRAMMER_STATEMENT_1,
    TP_PARSE_TREE_GRAMMER_STATEMENT_2,
    TP_PARSE_TREE_GRAMMER_EXPRESSION_1,
    TP_PARSE_TREE_GRAMMER_EXPRESSION_2,
    TP_PARSE_TREE_GRAMMER_TERM_1,
    TP_PARSE_TREE_GRAMMER_TERM_2,
    TP_PARSE_TREE_GRAMMER_FACTOR_1,
    TP_PARSE_TREE_GRAMMER_FACTOR_2,
    TP_PARSE_TREE_GRAMMER_FACTOR_3
}TP_PARSE_TREE_GRAMMER;

typedef struct tp_parse_tree_{
    TP_PARSE_TREE_GRAMMER member_grammer;
    size_t member_element_num;
    TP_PARSE_TREE_ELEMENT* member_element;
}TP_PARSE_TREE;

#endif

