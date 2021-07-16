
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_PARSE_TREE_H_)
#define TP_COMPILER_PARSE_TREE_H_

#include "tp_compiler_token.h"

// ----------------------------------------------------------------------------------------
// parse tree section:

#define NESTING_LEVEL_OF_EXPRESSION_MAXIMUM 63

#define IS_TOKEN_ID(token) ((token) && (TP_SYMBOL_IDENTIFIER == (token)->member_symbol))
#define IS_TOKEN_KEYWORD(token) ((token) && (TP_SYMBOL_KEYWORD == (token)->member_symbol))
#define IS_TOKEN_KEYWORD_ALIGNAS(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Alignas", (token)->member_string)))
#define IS_TOKEN_KEYWORD_ALIGNOF(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Alignof", (token)->member_string)))
#define IS_TOKEN_KEYWORD_ATOMIC(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Atomic", (token)->member_string)))
#define IS_TOKEN_KEYWORD_AUTO(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("auto", (token)->member_string)))
#define IS_TOKEN_KEYWORD_BOOL(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Bool", (token)->member_string)))
#define IS_TOKEN_KEYWORD_BREAK(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("break", (token)->member_string)))
#define IS_TOKEN_KEYWORD_CASE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("case", (token)->member_string)))
#define IS_TOKEN_KEYWORD_CHAR(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("char", (token)->member_string)))
#define IS_TOKEN_KEYWORD_COMPLEX(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Complex", (token)->member_string)))
#define IS_TOKEN_KEYWORD_CONST(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("const", (token)->member_string)))
#define IS_TOKEN_KEYWORD_CONTINUE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("continue", (token)->member_string)))
#define IS_TOKEN_KEYWORD_DEFAULT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("default", (token)->member_string)))
#define IS_TOKEN_KEYWORD_DO(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("do", (token)->member_string)))
#define IS_TOKEN_KEYWORD_DOUBLE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("double", (token)->member_string)))
#define IS_TOKEN_KEYWORD_ELSE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("else", (token)->member_string)))
#define IS_TOKEN_KEYWORD_ENUM(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("enum", (token)->member_string)))
#define IS_TOKEN_KEYWORD_EXTERN(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("extern", (token)->member_string)))
#define IS_TOKEN_KEYWORD_FLOAT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("float", (token)->member_string)))
#define IS_TOKEN_KEYWORD_FOR(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("for", (token)->member_string)))
#define IS_TOKEN_KEYWORD_GENERIC(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Generic", (token)->member_string)))
#define IS_TOKEN_KEYWORD_GOTO(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("goto", (token)->member_string)))
#define IS_TOKEN_KEYWORD_IF(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("if", (token)->member_string)))
#define IS_TOKEN_KEYWORD_INLINE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("inline", (token)->member_string)))
#define IS_TOKEN_KEYWORD_INT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("int", (token)->member_string)))
#define IS_TOKEN_KEYWORD_LONG(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("long", (token)->member_string)))
#define IS_TOKEN_KEYWORD_NORETURN(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Noreturn", (token)->member_string)))
#define IS_TOKEN_KEYWORD_REGISTER(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("register", (token)->member_string)))
#define IS_TOKEN_KEYWORD_RESTRICT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("restrict", (token)->member_string)))
#define IS_TOKEN_KEYWORD_RETURN(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("return", (token)->member_string)))
#define IS_TOKEN_KEYWORD_SHORT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("short", (token)->member_string)))
#define IS_TOKEN_KEYWORD_SIGNED(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("signed", (token)->member_string)))
#define IS_TOKEN_KEYWORD_SIZEOF(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("sizeof", (token)->member_string)))
#define IS_TOKEN_KEYWORD_STATIC(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("static", (token)->member_string)))
#define IS_TOKEN_KEYWORD_STATIC_ASSERT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Static_assert", (token)->member_string)))
#define IS_TOKEN_KEYWORD_STRUCT(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("struct", (token)->member_string)))
#define IS_TOKEN_KEYWORD_SWITCH(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("switch", (token)->member_string)))
#define IS_TOKEN_KEYWORD_THREAD_LOCAL(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("_Thread_local", (token)->member_string)))
#define IS_TOKEN_KEYWORD_TYPEDEF(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("typedef", (token)->member_string)))
#define IS_TOKEN_KEYWORD_UNION(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("union", (token)->member_string)))
#define IS_TOKEN_KEYWORD_UNSIGNED(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("unsigned", (token)->member_string)))
#define IS_TOKEN_KEYWORD_VOID(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("void", (token)->member_string)))
#define IS_TOKEN_KEYWORD_VOLATILE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("volatile", (token)->member_string)))
#define IS_TOKEN_KEYWORD_WHILE(token) (IS_TOKEN_KEYWORD(token) && (0 == strcmp("while", (token)->member_string)))

#define IS_TOKEN_CONSTANT(token) ((token) && (TP_SYMBOL_CONSTANT == (token)->member_symbol))
#define IS_TOKEN_STRING_LITERAL(token) ((token) && (TP_SYMBOL_STRING_LITERAL == (token)->member_symbol))

#define IS_TOKEN_PLUS(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_PLUS == (token)->member_symbol_kind))

#define IS_TOKEN_MINUS(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_MINUS == (token)->member_symbol_kind))

#define IS_TOKEN_MUL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_ASTERISK == (token)->member_symbol_kind))

#define IS_TOKEN_DIV(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_SLASH == (token)->member_symbol_kind))

#define IS_TOKEN_LEFT_PAREN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_LEFT_PARENTHESIS == (token)->member_symbol_kind))

#define IS_TOKEN_RIGHT_PAREN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_RIGHT_PARENTHESIS == (token)->member_symbol_kind))

#define IS_TOKEN_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_SEMICOLON(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_SEMICOLON == (token)->member_symbol_kind))

#define IS_TOKEN_COLON(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_COLON == (token)->member_symbol_kind))

#define IS_TOKEN_LEFT_CURLY_BRACKET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_LEFT_CURLY_BRACKET == (token)->member_symbol_kind))

#define IS_TOKEN_RIGHT_CURLY_BRACKET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_RIGHT_CURLY_BRACKET == (token)->member_symbol_kind))

#define IS_TOKEN_AMPERSAND(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_AMPERSAND == (token)->member_symbol_kind))

#define IS_TOKEN_ASTERISK(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_ASTERISK == (token)->member_symbol_kind))

#define IS_TOKEN_CARET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_CARET == (token)->member_symbol_kind))

#define IS_TOKEN_COLON(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_COLON == (token)->member_symbol_kind))

#define IS_TOKEN_COMMA(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_COMMA == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_AMPERSAND(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_AMPERSAND == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_GREATER_THAN_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_GREATER_THAN_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_LESS_THAN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_LESS_THAN == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_LESS_THAN_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_LESS_THAN_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_PLUS(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_PLUS == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_VERTICAL_BAR(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_VERTICAL_BAR == (token)->member_symbol_kind))

#define IS_TOKEN_GREATER_THAN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_GREATER_THAN == (token)->member_symbol_kind))

#define IS_TOKEN_LESS_THAN_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_LESS_THAN_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_GREATER_THAN_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_GREATER_THAN_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_GREATER_THAN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_GREATER_THAN == (token)->member_symbol_kind))

#define IS_TOKEN_PERCENT(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_PERCENT == (token)->member_symbol_kind))

#define IS_TOKEN_SLASH(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_SLASH == (token)->member_symbol_kind))

#define IS_TOKEN_ASTERISK_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_ASTERISK_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_AMPERSAND_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_AMPERSAND_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_CARET_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_CARET_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_EXCLAMATION_MARK(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_EXCLAMATION_MARK == (token)->member_symbol_kind))

#define IS_TOKEN_TILDE(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_TILDE == (token)->member_symbol_kind))

#define IS_TOKEN_DOUBLE_MINUS(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_DOUBLE_MINUS == (token)->member_symbol_kind))

#define IS_TOKEN_MINUS_GREATER_THAN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_MINUS_GREATER_THAN == (token)->member_symbol_kind))

#define IS_TOKEN_EXCLAMATION_MARK_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_EXCLAMATION_MARK_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_LEFT_CURLY_BRACKET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_LEFT_CURLY_BRACKET == (token)->member_symbol_kind))

#define IS_TOKEN_LEFT_SQUARE_BRACKET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_LEFT_SQUARE_BRACKET == (token)->member_symbol_kind))

#define IS_TOKEN_LESS_THAN(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_LESS_THAN == (token)->member_symbol_kind))

#define IS_TOKEN_MINUS_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_MINUS_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_PERCENT_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_PERCENT_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_PERIOD(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_PERIOD == (token)->member_symbol_kind))

#define IS_TOKEN_PLUS_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_PLUS_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_QUESTION_MARK(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_QUESTION_MARK == (token)->member_symbol_kind))

#define IS_TOKEN_RIGHT_CURLY_BRACKET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_RIGHT_CURLY_BRACKET == (token)->member_symbol_kind))

#define IS_TOKEN_RIGHT_SQUARE_BRACKET(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_RIGHT_SQUARE_BRACKET == (token)->member_symbol_kind))

#define IS_TOKEN_SLASH_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_SLASH_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_VERTICAL_BAR(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_VERTICAL_BAR == (token)->member_symbol_kind))

#define IS_TOKEN_VERTICAL_BAR_EQUAL(token) ((token) && \
    (TP_SYMBOL_PUNCTUATOR == (token)->member_symbol) && (TP_SYMBOL_KIND_VERTICAL_BAR_EQUAL == (token)->member_symbol_kind))

#define IS_TOKEN_TYPE_UNSPECIFIED_TYPE(token) ((token) && (TP_SYMBOL_UNSPECIFIED_TYPE == (token)->member_symbol_type))
#define IS_TOKEN_TYPE_ID_INT32(token) (IS_TOKEN_ID(token) && (TP_SYMBOL_ID_INT32 == (token)->member_symbol_type))
#define IS_TOKEN_TYPE_INT32(token) (IS_TOKEN_KEYWORD(token) && (TP_SYMBOL_TYPE_INT32 == (token)->member_symbol_type))
#define IS_TOKEN_TYPE_CONST_VALUE_INT32(token) (IS_TOKEN_CONSTANT(token) && \
    (TP_VALUE_TYPE_INT == (token)->member_value_type))

#define TP_POS(symbol_table) ((symbol_table)->member_tp_token_position)

#define IS_END_OF_TOKEN(token) ((NULL == (token)) || ((token) && (TP_SYMBOL_NULL == (token)->member_symbol)))

#define MAKE_PARSE_SUBTREE(symbol_table, grammer, ...) \
  tp_make_parse_subtree( \
    (symbol_table), (grammer), \
    (TP_PARSE_TREE_ELEMENT[]){ __VA_ARGS__ }, \
    sizeof((TP_PARSE_TREE_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_PARSE_TREE_ELEMENT) \
  )
#define TP_TREE_TOKEN(token) (TP_PARSE_TREE_ELEMENT){ \
    .member_type = TP_PARSE_TREE_TYPE_TOKEN, \
    .member_body.member_tp_token = (token) \
}
#define TP_TREE_TOKEN_NULL (TP_PARSE_TREE_ELEMENT){ \
    .member_type = TP_PARSE_TREE_TYPE_NULL, \
    .member_body.member_tp_token = NULL \
}
#define TP_TREE_NODE(child) (TP_PARSE_TREE_ELEMENT){ \
    .member_type = TP_PARSE_TREE_TYPE_NODE, \
    .member_body.member_child = (child) \
}

typedef enum TP_PARSE_TREE_TYPE_
{
    TP_PARSE_TREE_TYPE_NULL = 0,
    TP_PARSE_TREE_TYPE_TOKEN,
    TP_PARSE_TREE_TYPE_NODE
}TP_PARSE_TREE_TYPE;

typedef union tp_parse_tree_element_union_{
    TP_TOKEN* member_tp_token;  // NOTE: member_token must not free memory.
    struct tp_parse_tree_* member_child;
}TP_PARSE_TREE_ELEMENT_UNION;

typedef struct tp_parse_tree_element_{
    TP_PARSE_TREE_TYPE member_type;
    TP_PARSE_TREE_ELEMENT_UNION member_body;
}TP_PARSE_TREE_ELEMENT;

typedef enum TP_PARSE_TREE_GRAMMER_
{
    TP_PARSE_TREE_GRAMMER_NULL = 0,

// int_calc_compiler

    // Grammer:
    TP_PARSE_TREE_GRAMMER_PROGRAM,
    TP_PARSE_TREE_GRAMMER_STATEMENT_1,
    TP_PARSE_TREE_GRAMMER_STATEMENT_2,
    TP_PARSE_TREE_GRAMMER_EXPRESSION_1,
    TP_PARSE_TREE_GRAMMER_EXPRESSION_2,
    TP_PARSE_TREE_GRAMMER_TERM_1,
    TP_PARSE_TREE_GRAMMER_TERM_2,
    TP_PARSE_TREE_GRAMMER_FACTOR_1,
    TP_PARSE_TREE_GRAMMER_FACTOR_2,
    TP_PARSE_TREE_GRAMMER_FACTOR_3,

    // grammer_1: calc first set and follow set.
    TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1,
    TP_PARSE_TREE_GRAMMER_STATEMENT,
    TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1,
    TP_PARSE_TREE_GRAMMER_TYPE,
    TP_PARSE_TREE_GRAMMER_EXPRESSION,
    TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1,
    TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2,
    TP_PARSE_TREE_GRAMMER_TERM,
    TP_PARSE_TREE_GRAMMER_TERM_TMP1,
    TP_PARSE_TREE_GRAMMER_TERM_TMP2,
    TP_PARSE_TREE_GRAMMER_FACTOR,
    TP_PARSE_TREE_GRAMMER_FACTOR_TMP1,
    TP_PARSE_TREE_GRAMMER_FACTOR_TMP2,

    // grammer_2, 3: calc first set and follow set.
    TP_PARSE_TREE_GRAMMER_E,
    TP_PARSE_TREE_GRAMMER_E_TMP,
    TP_PARSE_TREE_GRAMMER_T,
    TP_PARSE_TREE_GRAMMER_T_TMP,
    TP_PARSE_TREE_GRAMMER_F,

// C compiler(Error recovery)

    // grammer_C: calc first set and follow set.
    // ToDo:

// C compiler(External definitions)

    // Grammer: translation-unit -> external-declaration external-declaration+
    TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_1,
    // Grammer: translation-unit -> external-declaration
    TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_2,

    // Grammer: external-declaration -> function-definition
    TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_1,
    // Grammer: external-declaration -> declaration
    TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_2,

    // Grammer: function-definition -> 
    //     declaration-specifiers declarator compound-statement
    TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_1,
    // Grammer: function-definition -> 
    //     declaration-specifiers declarator declaration-list compound-statement
    TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_2,

    // Grammer: declaration-list -> declaration declaration+
    TP_PARSE_TREE_GRAMMER_DECLARATION_LIST_1,
    // Grammer: declaration-list -> declaration
    TP_PARSE_TREE_GRAMMER_DECLARATION_LIST_2,

// C compiler(Statements)

    // Grammer: statement -> labeled-statement
    TP_PARSE_TREE_GRAMMER_C_STATEMENT_1,
    // Grammer: statement -> compound-statement
    TP_PARSE_TREE_GRAMMER_C_STATEMENT_2,
    // Grammer: statement -> expression-statement
    TP_PARSE_TREE_GRAMMER_C_STATEMENT_3,
    // Grammer: statement -> selection-statement
    TP_PARSE_TREE_GRAMMER_C_STATEMENT_4,
    // Grammer: statement -> iteration-statement
    TP_PARSE_TREE_GRAMMER_C_STATEMENT_5,
    // Grammer: statement -> jump-statement
    TP_PARSE_TREE_GRAMMER_C_STATEMENT_6,

    // Grammer: compound-statement -> { block-item-list }
    TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1,
    // Grammer: compound-statement -> { }
    TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2,

    // Grammer: labeled-statement -> identifier : statement
    TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_1,
    // Grammer: labeled-statement -> case constant-expression : statement
    TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_2,
    // Grammer: labeled-statement -> default : statement
    TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_3,

    // Grammer: block-item-list -> block-item block-item+
    TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_1,
    // Grammer: block-item-list -> block-item
    TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_2,

    // Grammer: block-item -> declaration
    TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_1,
    // Grammer: block-item -> statement
    TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_2,

    // Grammer: expression-statement -> expression ;
    TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_1,
    // Grammer: expression-statement -> ;
    TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_2,

    // Grammer: if ( expression ) statement else statement
    TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_IF_1,
    // Grammer: if ( expression ) statement
    TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_IF_2,

    // Grammer: selection-statement -> switch ( expression ) statement
    TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_SWITCH_1,

    // Grammer: iteration-statement -> while ( expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_WHILE_1,

    // Grammer: iteration-statement -> do statement while ( expression ) ;
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_DO_1,

    // Grammer: iteration-statement -> for ( declaration expression ; expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_1,
    // Grammer: iteration-statement -> for ( expression ; expression ; expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_2,
    // Grammer: iteration-statement -> for ( ; expression ; expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_3,
    // Grammer: iteration-statement -> for ( declaration ; expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_4,
    // Grammer: iteration-statement -> for ( expression ; ; expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_5,
    // Grammer: iteration-statement -> for ( declaration expression ; ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_6,
    // Grammer: iteration-statement -> for ( expression ; expression ; ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_7,
    // Grammer: iteration-statement -> for ( ; ; expression ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_8,
    // Grammer: iteration-statement -> for ( ; expression ; ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_9,
    // Grammer: iteration-statement -> for ( declaration ; ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_10,
    // Grammer: iteration-statement -> for ( expression ; ; ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_11,
    // Grammer: iteration-statement -> for ( ; ; ) statement
    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_12,

    // Grammer: jump-statement -> goto identifier ;
    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_1,
    // Grammer: jump-statement -> continue ;
    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_2,
    // Grammer: jump-statement -> break ;
    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_3,
    // Grammer: jump-statement -> return expression ;
    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_4,
    // Grammer: jump-statement -> return ;
    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_5,

// C compiler(Expressions)

    // Grammer: expression -> (assignment-expression ,)+ assignment-expression
    TP_PARSE_TREE_GRAMMER_C_EXPRESSION_1,
    // Grammer: expression -> assignment-expression
    TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2,

    // Grammer: constant-expression -> conditional-expression
    TP_PARSE_TREE_GRAMMER_CONSTANT_EXPRESSION_1,

    // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
    TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1,
    // Grammer: assignment-expression -> conditional-expression
    TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_2,

    // Grammer: conditional-expression -> logical-OR-expression (? expression : conditional-expression)?
    TP_PARSE_TREE_GRAMMER_CONDITIONAL_EXPRESSION_1,

    // Grammer: primary-expression -> identifier
    TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_1,
    // Grammer: primary-expression -> constant
    TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_2,
    // Grammer: primary-expression -> string-literal
    TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_3,
    // Grammer: primary-expression -> ( expression )
    TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_4,
    // Grammer: primary-expression -> generic-selection
    TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_5,

    // Grammer: generic-selection -> _Generic ( assignment-expression , generic-assoc-list )
    TP_PARSE_TREE_GRAMMER_GENERIC_SELECTION_1,
    // Grammer: generic-assoc-list -> (generic-association ,)+ generic-association
    TP_PARSE_TREE_GRAMMER_GENERIC_ASSOC_LIST_1,
    // Grammer: generic-assoc-list -> generic-association
    TP_PARSE_TREE_GRAMMER_GENERIC_ASSOC_LIST_2,
    // Grammer: generic-association -> type-name : assignment-expression
    TP_PARSE_TREE_GRAMMER_GENERIC_ASSOCIATION_1,
    // Grammer: generic-association -> default : assignment-expression
    TP_PARSE_TREE_GRAMMER_GENERIC_ASSOCIATION_2,

    // Grammer: postfix-expression -> primary-expression PostfixExpressionTmp1+
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_1,
    // Grammer: postfix-expression -> primary-expression
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_2,
    // Grammer: postfix-expression -> ( type-name ) { initializer-list , }
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_3,
    // Grammer: postfix-expression -> ( type-name ) { initializer-list }
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_4,

    // Grammer: PostfixExpressionTmp1 -> [ expression ]
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_1,
    // Grammer: PostfixExpressionTmp1 -> ( argument-expression-list )
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_2,
    // Grammer: PostfixExpressionTmp1 -> ( )
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_3,
    // Grammer: PostfixExpressionTmp1 -> (. | ->) identifier
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_4,
    // Grammer: PostfixExpressionTmp1 -> ++ | --
    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_5,

    // Grammer: argument-expression-list -> (assignment-expression ,)+ assignment-expression
    TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_1,
    // Grammer: argument-expression-list -> assignment-expression
    TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_2,

    // Grammer: unary-expression -> postfix-expression
    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_1,
    // Grammer: unary-expression -> (++ | --) unary-expression
    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_2,
    // Grammer: unary-expression -> unary-operator cast-expression
    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_3,
    // Grammer: unary-expression -> sizeof unary-expression
    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_4,
    // Grammer: unary-expression -> sizeof ( type-name )
    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_5,
    // Grammer: unary-expression -> _Alignof ( type-name )
    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_6,

    // Grammer: cast-expression -> unary-expression
    TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_1,
    // Grammer: cast-expression -> ( type-name ) cast-expression
    TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_2,

    // Grammer: multiplicative-expression -> (cast-expression ((* | / | %))+ cast-expression
    TP_PARSE_TREE_GRAMMER_MULTIPLICATIVE_EXPRESSION_1,
    // Grammer: multiplicative-expression -> cast-expression
    TP_PARSE_TREE_GRAMMER_MULTIPLICATIVE_EXPRESSION_2,

    // Grammer: additive-expression -> (multiplicative-expression ((+ | -))+ multiplicative-expression
    TP_PARSE_TREE_GRAMMER_ADDITIVE_EXPRESSION_1,
    // Grammer: additive-expression -> multiplicative-expression
    TP_PARSE_TREE_GRAMMER_ADDITIVE_EXPRESSION_2,

    // Grammer: shift-expression -> (additive-expression (<< | >>))+ additive-expression
    TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_1,
    // Grammer: shift-expression -> additive-expression
    TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_2,

    // Grammer: relational-expression -> (shift-expression (< | > | <= | >=))+ shift-expression
    TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_1,
    // Grammer: relational-expression -> shift-expression
    TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_2,

    // Grammer: equality-expression -> (relational-expression (== | !=))+ relational-expression
    TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_1,
    // Grammer: equality-expression -> relational-expression
    TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_2,

    // Grammer: AND-expression -> (equality-expression &)+ equality-expression
    TP_PARSE_TREE_GRAMMER_AND_EXPRESSION_1,
    // Grammer: AND-expression -> equality-expression
    TP_PARSE_TREE_GRAMMER_AND_EXPRESSION_2,

    // Grammer: exclusive-OR-expression -> (AND-expression ^)+ AND-expression
    TP_PARSE_TREE_GRAMMER_EXCLUSIVE_OR_EXPRESSION_1,
    // Grammer: exclusive-OR-expression -> AND-expression
    TP_PARSE_TREE_GRAMMER_EXCLUSIVE_OR_EXPRESSION_2,

    // Grammer: inclusive-OR-expression -> (exclusive-OR-expression |)+ exclusive-OR-expression
    TP_PARSE_TREE_GRAMMER_INCLUSIVE_OR_EXPRESSION_1,
    // Grammer: inclusive-OR-expression -> exclusive-OR-expression
    TP_PARSE_TREE_GRAMMER_INCLUSIVE_OR_EXPRESSION_2,

    // Grammer: logical-AND-expression -> (inclusive-OR-expression &&)+ inclusive-OR-expression
    TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_1,
    // Grammer: logical-AND-expression -> inclusive-OR-expression
    TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_2,

    // Grammer: logical-OR-expression -> (logical-AND-expression ||)+ logical-AND-expression
    TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_1,
    // Grammer: logical-OR-expression -> logical-AND-expression
    TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_2,

// C compiler(Declarations: declaration)

    // Grammer: declaration -> declaration-specifiers init-declarator-list ;
    TP_PARSE_TREE_GRAMMER_DECLARATION_1,
    // Grammer: declaration -> declaration-specifiers ;
    TP_PARSE_TREE_GRAMMER_DECLARATION_2,
    // Grammer: declaration -> static_assert-declaration
    TP_PARSE_TREE_GRAMMER_DECLARATION_3,

    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
    //    function-specifier | alignment-specifier) declaration-specifiers
    TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1,
    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
    //    function-specifier | alignment-specifier)
    TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_2,

    // Grammer: storage-class-specifier -> typedef | extern | static | _Thread_local | auto | register
    TP_PARSE_TREE_GRAMMER_STORAGE_CLASS_SPECIFIER_1,

    // Grammer: function-specifier -> inline | _Noreturn
    TP_PARSE_TREE_GRAMMER_FUNCTION_SPECIFIER_1,

    // Grammer: alignment-specifier -> _Alignas ( type-name | constant-expression )
    TP_PARSE_TREE_GRAMMER_ALIGNMENT_SPECIFIER_1,

    // Grammer: init-declarator-list -> init-declarator (, init-declarator)+
    TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_1,
    // Grammer: init-declarator-list -> init-declarator
    TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_2,

    // Grammer: init-declarator -> declarator = initializer
    TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_1,
    // Grammer: init-declarator -> declarator
    TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_2,

    // Grammer: declarator -> pointer direct-declarator
    TP_PARSE_TREE_GRAMMER_DECLARATOR_1,
    // Grammer: declarator -> direct-declarator
    TP_PARSE_TREE_GRAMMER_DECLARATOR_2,

    // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1,
    // Grammer: direct-declarator -> DirectDeclaratorTmp1
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_2,

    // Grammer: DirectDeclaratorTmp1 -> identifier
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1,
    // Grammer: DirectDeclaratorTmp1 -> ( declarator )
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_2,

    // Grammer: DirectDeclaratorTmp2 ->
    //     [ static type-qualifier-list assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_STATIC_1,
    // Grammer: DirectDeclaratorTmp2 ->
    //     [ static assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_STATIC_2,

    // Grammer: DirectDeclaratorTmp2 -> [ * ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_1,
    // Grammer: DirectDeclaratorTmp2 -> [ assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_2,
    // Grammer: DirectDeclaratorTmp2 -> [ ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_3,

    // Grammer: DirectDeclaratorTmp2 ->
    //     [ type-qualifier-list static assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_1,
    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list * ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_2,
    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_3,
    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list ]
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_4,

    // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1,
    // Grammer: DirectDeclaratorTmp2 -> ( identifier-list )
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_2,
    // Grammer: DirectDeclaratorTmp2 -> ( )
    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_3,

    // Grammer: identifier-list -> identifier
    TP_PARSE_TREE_GRAMMER_IDENTIFIER_LIST_1,
    // Grammer: identifier-list -> identifier (, identifier)+
    TP_PARSE_TREE_GRAMMER_IDENTIFIER_LIST_2,

    // Grammer: initializer -> assignment-expression
    TP_PARSE_TREE_GRAMMER_INITIALIZER_1,
    // Grammer: initializer -> { initializer-list , }
    TP_PARSE_TREE_GRAMMER_INITIALIZER_2,
    // Grammer: initializer -> { initializer-list }
    TP_PARSE_TREE_GRAMMER_INITIALIZER_3,

    // Grammer: initializer-list -> initializerListTmp1 (, initializerListTmp1)+
    TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_1,
    // Grammer: initializer-list -> initializerListTmp1
    TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_2,

    // Grammer: initializerListTmp1 -> designation initializer
    TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_TMP1_1,
    // Grammer: initializerListTmp1 -> initializer
    TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_TMP1_2,

    // Grammer: designation -> designator-list =
    TP_PARSE_TREE_GRAMMER_DESIGNATION_1,

    // Grammer: designator-list -> designator+
    TP_PARSE_TREE_GRAMMER_DESIGNATOR_LIST_1,

    // Grammer: designator -> [ constant-expression ]
    TP_PARSE_TREE_GRAMMER_DESIGNATOR_1,
    // Grammer: designator -> . identifier
    TP_PARSE_TREE_GRAMMER_DESIGNATOR_2,

// C compiler(Declarations: type-name)

    // Grammer: type-name -> specifier-qualifier-list abstract-declarator
    TP_PARSE_TREE_GRAMMER_TYPE_NAME_1,
    // Grammer: type-name -> specifier-qualifier-list
    TP_PARSE_TREE_GRAMMER_TYPE_NAME_2,

    // Grammer: specifier-qualifier-list -> type-specifier specifier-qualifier-list
    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_1,
    // Grammer: specifier-qualifier-list -> type-specifier
    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_2,
    // Grammer: specifier-qualifier-list -> type-qualifier specifier-qualifier-list
    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_3,
    // Grammer: specifier-qualifier-list -> type-qualifier
    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_4,

    // Grammer: type-specifier -> void | char | short | int | long |
    //     float | double | signed | unsigned | _Bool | _Complex
    TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1,
    // Grammer: type-specifier -> atomic-type-specifier
    TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_2,
    // Grammer: type-specifier -> struct-or-union-specifier
    TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_3,
    // Grammer: type-specifier -> enum-specifier
    TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_4,
    // Grammer: type-specifier -> typedef-name
    TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_5,

    // Grammer: atomic-type-specifier -> _Atomic ( type-name )
    TP_PARSE_TREE_GRAMMER_ATOMIC_TYPE_SPECIFIER_1,

    // Grammer: enum-specifier -> enum identifier { enumerator-list , }
    TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_1,
    // Grammer: enum-specifier -> enum identifier { enumerator-list }
    TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_2,
    // Grammer: enum-specifier -> enum { enumerator-list , }
    TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_3,
    // Grammer: enum-specifier -> enum { enumerator-list }
    TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_4,
    // Grammer: enum-specifier -> enum identifier
    TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_5,

    // Grammer: enumerator-list -> enumerator (, enumerator)+
    TP_PARSE_TREE_GRAMMER_ENUMERATOR_LIST_1,
    // Grammer: enumerator-list -> enumerator
    TP_PARSE_TREE_GRAMMER_ENUMERATOR_LIST_2,

    // Grammer: enumerator -> identifier = constant-expression
    TP_PARSE_TREE_GRAMMER_ENUMERATOR_1,
    // Grammer: enumerator -> identifier
    TP_PARSE_TREE_GRAMMER_ENUMERATOR_2,

    // Grammer: typedef-name -> identifier
    TP_PARSE_TREE_GRAMMER_TYPEDEF_NAME_1,

    // Grammer: struct-or-union-specifier -> struct-or-union identifier { struct-declaration-list }
    TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_1,
    // Grammer: struct-or-union-specifier -> struct-or-union { struct-declaration-list }
    TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_2,
    // Grammer: struct-or-union-specifier -> struct-or-union identifier
    TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_3,

    // Grammer: struct-or-union -> struct | union
    TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_1,

    // Grammer: struct-declaration-list -> struct-declaration+
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_LIST_1,

    // Grammer: struct-declaration -> specifier-qualifier-list struct-declarator-list ;
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_1,
    // Grammer: struct-declaration -> specifier-qualifier-list ;
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_2,
    // Grammer: struct-declaration -> static_assert-declaration
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_3,

    // Grammer: struct-declarator-list -> struct-declarator (, struct-declarator)+
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_LIST_1,
    // Grammer: struct-declarator-list -> struct-declarator
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_LIST_2,

    // Grammer: struct-declarator -> declarator : constant-expression
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_1,
    // Grammer: struct-declarator -> : constant-expression
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_2,
    // Grammer: struct-declarator -> declarator
    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_3,

    // Grammer: abstract-declarator -> pointer direct-abstract-declarator
    TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_1,
    // Grammer: abstract-declarator -> direct-abstract-declarator
    TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_2,
    // Grammer: abstract-declarator -> pointer
    TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_3,

    // Grammer: pointer -> * type-qualifier-list pointer
    TP_PARSE_TREE_GRAMMER_POINTER_1,
    // Grammer: pointer -> * type-qualifier-list
    TP_PARSE_TREE_GRAMMER_POINTER_2,
    // Grammer: pointer -> * pointer
    TP_PARSE_TREE_GRAMMER_POINTER_3,
    // Grammer: pointer -> *
    TP_PARSE_TREE_GRAMMER_POINTER_4,

    // Grammer: type-qualifier-list -> type-qualifier+
    TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_LIST_1,

    // Grammer: type-qualifier -> const | restrict | volatile | _Atomic
    TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_1,

    // Grammer: static_assert-declaration -> _Static_assert ( constant-expression , string-literal ) ;
    TP_PARSE_TREE_GRAMMER_STATIC_ASSERT_DECLARATION_1,

    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1 DirectAbstractDeclaratorTmp2+
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_1,
    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_2,
    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp2
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_3,

    // Grammer: DirectAbstractDeclaratorTmp1 -> ( abstract-declarator )
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP1_1,

    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ static type-qualifier-list assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_STATIC_1,
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ static assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_STATIC_2,

    // Grammer: DirectAbstractDeclaratorTmp2 -> [*]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_1,
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_2,
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_3,

    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list static assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_1,
    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list assignment-expression ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_2,
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ type-qualifier-list ]
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_3,

    // Grammer: DirectAbstractDeclaratorTmp2 -> ( parameter-type-list )
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_PAREN_1,
    // Grammer: DirectAbstractDeclaratorTmp2 -> ( )
    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_PAREN_2,

    // Grammer: parameter-type-list -> parameter-list ParameterTypeListVararg
    TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_1,
    // Grammer: parameter-type-list -> parameter-list
    TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_2,

    // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)+
    TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_1,
    // Grammer: parameter-list -> parameter-declaration
    TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_2,

    // Grammer: ParameterTypeListVararg -> , ...
    TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_VARARG_1,

    // Grammer: parameter-declaration -> declaration-specifiers declarator
    TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1,
    // Grammer: parameter-declaration -> declaration-specifiers abstract-declarator
    TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_2,
    // Grammer: parameter-declaration -> declaration-specifiers
    TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3,

    TP_PARSE_TREE_GRAMMER_NUM
}TP_PARSE_TREE_GRAMMER;

typedef struct tp_c_object_ TP_C_OBJECT;

typedef struct tp_parse_tree_{
    TP_PARSE_TREE_GRAMMER member_grammer;
    size_t member_element_num;
    TP_PARSE_TREE_ELEMENT* member_element;
}TP_PARSE_TREE;

// calc first set and follow set section:

typedef enum TP_GRAMMER_TYPE_
{
    TP_GRAMMER_TYPE_NULL = 0,
    TP_GRAMMER_TYPE_TERM,
    TP_GRAMMER_TYPE_NONTERM
}TP_GRAMMER_TYPE;

typedef struct tp_grammer_term_{
    TP_SYMBOL member_symbol;
    TP_CHAR8_T* member_string;
}TP_GRAMMER_TERM;

typedef enum TP_PARSE_TREE_GRAMMER_FIRST_FOLLOW_SET_INDEX_{
    TP_GRAMMER_1_PROGRAM_FIRST_FOLLOW_SET_INDEX = 0,
    TP_GRAMMER_1_PROGRAM_TMP1_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_STATEMENT_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_STATEMENT_TMP1_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_TYPE_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_EXPRESSION_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_EXPRESSION_TMP1_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_EXPRESSION_TMP2_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_TERM_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_TERM_TMP1_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_TERM_TMP2_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_FACTOR_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_FACTOR_TMP1_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_FACTOR_TMP2_FIRST_FOLLOW_SET_INDEX,
    TP_GRAMMER_1_NULL_FIRST_FOLLOW_SET_INDEX
}TP_PARSE_TREE_GRAMMER_FIRST_FOLLOW_SET_INDEX;

typedef union tp_grammer_element_union_{
    TP_GRAMMER_TERM member_term;
    TP_PARSE_TREE_GRAMMER member_nonterm;
}TP_GRAMMER_ELEMENT_UNION;

typedef struct tp_grammer_element_{
    TP_GRAMMER_TYPE member_type;
    TP_GRAMMER_ELEMENT_UNION member_body;
    bool member_is_empty;
}TP_GRAMMER_ELEMENT;

typedef struct tp_grammer_{
    TP_PARSE_TREE_GRAMMER member_grammer;

    bool member_is_visit;
    bool member_is_empty;

    TP_GRAMMER_TERM* member_first_set;
    rsize_t member_first_set_num;

    TP_GRAMMER_TERM* member_follow_set;
    rsize_t member_follow_set_num;

    rsize_t member_element_num;
    TP_GRAMMER_ELEMENT* member_element;
}TP_GRAMMER;

#define TP_TERM_ELEMENT(symbol, string) (TP_GRAMMER_TERM){ \
    .member_symbol = (symbol), \
    .member_string = (string) \
}
#define TP_TERM_SYMBOL(symbol, string) (TP_GRAMMER_ELEMENT){ \
    .member_type = TP_GRAMMER_TYPE_TERM, \
    .member_body.member_term = TP_TERM_ELEMENT((symbol), (string)), \
    .member_is_empty = false \
}
// ε
#define TP_TERM_SYMBOL_EMPTY (TP_GRAMMER_ELEMENT){ \
    .member_type = TP_GRAMMER_TYPE_TERM, \
    .member_body.member_term = TP_TERM_ELEMENT(TP_SYMBOL_EMPTY, ""), \
    .member_is_empty = true \
}
// |
#define TP_TERM_PIPE (TP_GRAMMER_ELEMENT){ \
    .member_type = TP_GRAMMER_TYPE_TERM, \
    .member_body.member_term = TP_TERM_ELEMENT(TP_SYMBOL_PIPE, "|"), \
    .member_is_empty = false \
}
#define TP_TERM_NULL (TP_GRAMMER_ELEMENT){ \
    .member_type = TP_GRAMMER_TYPE_NULL, \
    .member_body.member_term = TP_TERM_ELEMENT(TP_SYMBOL_NULL, ""), \
    .member_is_empty = false \
}

#define TP_NONTERM_SYMBOL_REFERENCE(grammer) (TP_GRAMMER_ELEMENT){ \
    .member_type = TP_GRAMMER_TYPE_NONTERM, \
    .member_body.member_nonterm = (grammer), \
    .member_is_empty = false \
}

#define TP_MAKE_GRAMMER(symbol_table, grammer_num, ...) \
    tp_make_grammer( \
      (symbol_table), (grammer_num), \
      (TP_GRAMMER*[]){ __VA_ARGS__ }, \
      sizeof((TP_GRAMMER*[]){ __VA_ARGS__ }) / sizeof(TP_GRAMMER*) \
    )

#define TP_MAKE_GRAMMER_ELEMENT(symbol_table, grammer, ...) \
    tp_make_grammer_element( \
      (symbol_table), (grammer), \
      (TP_GRAMMER_ELEMENT[]){ __VA_ARGS__ }, \
      sizeof((TP_GRAMMER_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_GRAMMER_ELEMENT) \
    )

#endif
