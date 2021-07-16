
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool test_first_set_grammer_1(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer);
static bool test_follow_set_grammer_1(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer);

// Grammer:
//
// Program -> Statement ProgramTmp1
// ProgramTmp1 -> Statement ProgramTmp1 | ε
//
// Statement -> StatementTmp1 variable '=' Expression ';'
// StatementTmp1 -> Type | ε
// Type -> int
//
// Expression -> Term ExpressionTmp1
// ExpressionTmp1 -> ExpressionTmp2 Term ExpressionTmp1 | ε
// ExpressionTmp2 -> '+' | '-'
//
// Term -> Factor TermTmp1
// TermTmp1 -> TermTmp2 Factor TermTmp1 | ε
// TermTmp2 -> '*' | '/'
//
// Factor -> '(' Expression ')' | FactorTmp1 FactorTmp2
// FactorTmp1 -> '+' | '-' | ε
// FactorTmp2 -> variable | constant
//
// Example:
//
// int32_t value1 = (1 + 2) * 3;
// int32_t value2 = 2 + (3 * value1);
// value1 = value2 + 100;
//
TP_GRAMMER** tp_make_grammer_1(TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num)
{
    // Program -> Statement ProgramTmp1
    // ProgramTmp1 -> Statement ProgramTmp1 | ε
    TP_GRAMMER* program = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_PROGRAM,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_STATEMENT),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1),
        TP_TERM_NULL
    );

    TP_GRAMMER* program_tmp1 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_STATEMENT),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    // Statement -> StatementTmp1 variable '=' Expression ';'
    // StatementTmp1 -> Type | ε
    // Type -> int
    TP_GRAMMER* statement = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_STATEMENT,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1),
        TP_TERM_SYMBOL(TP_SYMBOL_IDENTIFIER, "variable"),
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "="),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_EXPRESSION),
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, ";"),
        TP_TERM_NULL
    );

    TP_GRAMMER* statement_tmp1 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_TYPE),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    TP_GRAMMER* type = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_TYPE,
        TP_TERM_SYMBOL(TP_SYMBOL_KEYWORD, "int"),
        TP_TERM_NULL
    );

    // Expression -> Term ExpressionTmp1
    // ExpressionTmp1 -> ExpressionTmp2 Term ExpressionTmp1 | ε
    // ExpressionTmp2 -> '+' | '-'
    TP_GRAMMER* expression = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_EXPRESSION,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_TERM),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1),
        TP_TERM_NULL
    );

    TP_GRAMMER* expression_tmp1 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_TERM),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    TP_GRAMMER* expression_tmp2 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "+"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "-"),
        TP_TERM_NULL
    );

    // Term -> Factor TermTmp1
    // TermTmp1 -> TermTmp2 Factor TermTmp1 | ε
    // TermTmp2 -> '*' | '/'
    TP_GRAMMER* term = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_TERM,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_FACTOR),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_TERM_TMP1),
        TP_TERM_NULL
    );

    TP_GRAMMER* term_tmp1 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_TERM_TMP1,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_TERM_TMP2),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_FACTOR),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_TERM_TMP1),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    TP_GRAMMER* term_tmp2 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_TERM_TMP2,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "*"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "/"),
        TP_TERM_NULL
    );

    // Factor -> '(' Expression ')' | FactorTmp1 FactorTmp2
    // FactorTmp1 -> '+' | '-' | ε
    // FactorTmp2 -> variable | constant
    TP_GRAMMER* factor = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_FACTOR,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "("),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_EXPRESSION),
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, ")"),
        TP_TERM_PIPE,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_FACTOR_TMP1),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_FACTOR_TMP2),
        TP_TERM_NULL
    );

    TP_GRAMMER* factor_tmp1 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_FACTOR_TMP1,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "+"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "-"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    TP_GRAMMER* factor_tmp2 = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_FACTOR_TMP2,
        TP_TERM_SYMBOL(TP_SYMBOL_IDENTIFIER, "variable"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_CONSTANT, "constant"),
        TP_TERM_NULL
    );

    // NULL
    TP_GRAMMER* end = TP_MAKE_GRAMMER_ELEMENT(symbol_table, TP_PARSE_TREE_GRAMMER_NULL, TP_TERM_NULL);

    return TP_MAKE_GRAMMER(symbol_table, grammer_num, 
        program, program_tmp1, statement, statement_tmp1, type,
        expression, expression_tmp1, expression_tmp2,
        term, term_tmp1, term_tmp2,
        factor, factor_tmp1, factor_tmp2,
        end
    );
}

bool tp_test_grammer_1(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num)
{
    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== begin test grammer_1: ===")
    );

    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

        if ( ! test_first_set_grammer_1(symbol_table, grammer_tbl[i])){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            TP_PUT_LOG_PRINT(
                symbol_table,
                TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== first set test failed. ===")
            );

            return false;
        }
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== first set test passed. ===")
    );

    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

        if ( ! test_follow_set_grammer_1(symbol_table, grammer_tbl[i])){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            TP_PUT_LOG_PRINT(
                symbol_table,
                TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== follow set test failed. ===")
            );

            return false;
        }
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== follow set test passed. ===")
    );

    return true;
}

static bool test_first_set_grammer_1(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer)
{
    // Program -> Statement ProgramTmp1
    static TP_GRAMMER_TERM first_set_GRAMMER_PROGRAM[] = {
        { .member_symbol = TP_SYMBOL_KEYWORD, .member_string = "int" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
    };

    // ProgramTmp1 -> Statement ProgramTmp1 | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_PROGRAM_TMP1[] = {
        { .member_symbol = TP_SYMBOL_KEYWORD, .member_string = "int" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // Statement -> StatementTmp1 variable '=' Expression ';'
    static TP_GRAMMER_TERM first_set_GRAMMER_STATEMENT[] = {
        { .member_symbol = TP_SYMBOL_KEYWORD, .member_string = "int" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
    };

    // StatementTmp1 -> Type | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_STATEMENT_TMP1[] = {
        { .member_symbol = TP_SYMBOL_KEYWORD, .member_string = "int" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // Type -> int
    static TP_GRAMMER_TERM first_set_GRAMMER_TYPE[] = {
        { .member_symbol = TP_SYMBOL_KEYWORD, .member_string = "int" },
    };

    // Expression -> Term ExpressionTmp1
    static TP_GRAMMER_TERM first_set_GRAMMER_EXPRESSION[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    // ExpressionTmp1 -> ExpressionTmp2 Term ExpressionTmp1 | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_EXPRESSION_TMP1[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // ExpressionTmp2 -> '+' | '-'
    static TP_GRAMMER_TERM first_set_GRAMMER_EXPRESSION_TMP2[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
    };

    // Term -> Factor TermTmp1
    static TP_GRAMMER_TERM first_set_GRAMMER_TERM[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    // TermTmp1 -> TermTmp2 Factor TermTmp1 | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_TERM_TMP1[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "*" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "/" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // TermTmp2 -> '*' | '/'
    static TP_GRAMMER_TERM first_set_GRAMMER_TERM_TMP2[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "*" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "/" },
    };

    // Factor -> '(' Expression ')' | FactorTmp1 FactorTmp2
    static TP_GRAMMER_TERM first_set_GRAMMER_FACTOR[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    // FactorTmp1 -> '+' | '-' | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_FACTOR_TMP1[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // FactorTmp2 -> variable | constant
    static TP_GRAMMER_TERM first_set_GRAMMER_FACTOR_TMP2[] = {
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    static TP_GRAMMER_TERM* first_set[] = {
        first_set_GRAMMER_PROGRAM,
        first_set_GRAMMER_PROGRAM_TMP1,
        first_set_GRAMMER_STATEMENT,
        first_set_GRAMMER_STATEMENT_TMP1,
        first_set_GRAMMER_TYPE,
        first_set_GRAMMER_EXPRESSION,
        first_set_GRAMMER_EXPRESSION_TMP1,
        first_set_GRAMMER_EXPRESSION_TMP2,
        first_set_GRAMMER_TERM,
        first_set_GRAMMER_TERM_TMP1,
        first_set_GRAMMER_TERM_TMP2,
        first_set_GRAMMER_FACTOR,
        first_set_GRAMMER_FACTOR_TMP1,
        first_set_GRAMMER_FACTOR_TMP2,
        NULL
    };

    static rsize_t first_set_num[] = {
        sizeof(first_set_GRAMMER_PROGRAM) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_PROGRAM_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_STATEMENT) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_STATEMENT_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_TYPE) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_EXPRESSION) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_EXPRESSION_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_EXPRESSION_TMP2) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_TERM) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_TERM_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_TERM_TMP2) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_FACTOR) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_FACTOR_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_FACTOR_TMP2) / sizeof(TP_GRAMMER_TERM),
        0
    };

    static TP_PARSE_TREE_GRAMMER grammer_index[] = {
        TP_PARSE_TREE_GRAMMER_PROGRAM,
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
        TP_PARSE_TREE_GRAMMER_NULL
    };

    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_index[i]; ++i){

        if (grammer_index[i] == grammer->member_grammer){

            if (first_set_num[i] != grammer->member_first_set_num){

                TP_PUT_LOG_PRINT(
                    symbol_table, TP_MSG_FMT("\n=== first set num(index = %1: %2, %3) ==="),
                    TP_LOG_PARAM_UINT64_VALUE(i),
                    TP_LOG_PARAM_UINT64_VALUE(first_set_num[i]),
                    TP_LOG_PARAM_UINT64_VALUE(grammer->member_first_set_num)
                );

                return false;
            }

            if ( ! tp_compare_first_or_follow_set(first_set[i], first_set_num[i], grammer->member_first_set)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                TP_PUT_LOG_PRINT(
                    symbol_table,
                    TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== tp_compare_first_or_follow_set failed. ===")
                );

                return false;
            }

            return true;
        }
    }

    TP_PUT_LOG_PRINT(
        symbol_table, TP_MSG_FMT("\n=== unmatch grammer(%1). ==="),
        TP_LOG_PARAM_INT32_VALUE(grammer->member_grammer)
    );

    return false;
}

static bool test_follow_set_grammer_1(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer)
{
    // Program -> Statement ProgramTmp1
    static TP_GRAMMER_TERM follow_set_GRAMMER_PROGRAM[] = {
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
    };

    // ProgramTmp1 -> Statement ProgramTmp1 | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_PROGRAM_TMP1[] = {
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
    };

    // Statement -> StatementTmp1 variable '=' Expression ';'
    static TP_GRAMMER_TERM follow_set_GRAMMER_STATEMENT[] = {
        { .member_symbol = TP_SYMBOL_KEYWORD, .member_string = "int" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
    };

    // StatementTmp1 -> Type | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_STATEMENT_TMP1[] = {
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
    };

    // Type -> int
    static TP_GRAMMER_TERM follow_set_GRAMMER_TYPE[] = {
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
    };

    // Expression -> Term ExpressionTmp1
    static TP_GRAMMER_TERM follow_set_GRAMMER_EXPRESSION[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ";" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // ExpressionTmp1 -> ExpressionTmp2 Term ExpressionTmp1 | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_EXPRESSION_TMP1[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ";" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // ExpressionTmp2 -> '+' | '-'
    static TP_GRAMMER_TERM follow_set_GRAMMER_EXPRESSION_TMP2[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    // Term -> Factor TermTmp1
    static TP_GRAMMER_TERM follow_set_GRAMMER_TERM[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ";" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // TermTmp1 -> TermTmp2 Factor TermTmp1 | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_TERM_TMP1[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ";" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // TermTmp2 -> '*' | '/'
    static TP_GRAMMER_TERM follow_set_GRAMMER_TERM_TMP2[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    // Factor -> '(' Expression ')' | FactorTmp1 FactorTmp2
    static TP_GRAMMER_TERM follow_set_GRAMMER_FACTOR[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "*" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "/" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ";" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // FactorTmp1 -> '+' | '-' | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_FACTOR_TMP1[] = {
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "variable" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "constant" },
    };

    // FactorTmp2 -> variable | constant
    static TP_GRAMMER_TERM follow_set_GRAMMER_FACTOR_TMP2[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "*" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "/" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ";" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    static TP_GRAMMER_TERM* follow_set[] = {
        follow_set_GRAMMER_PROGRAM,
        follow_set_GRAMMER_PROGRAM_TMP1,
        follow_set_GRAMMER_STATEMENT,
        follow_set_GRAMMER_STATEMENT_TMP1,
        follow_set_GRAMMER_TYPE,
        follow_set_GRAMMER_EXPRESSION,
        follow_set_GRAMMER_EXPRESSION_TMP1,
        follow_set_GRAMMER_EXPRESSION_TMP2,
        follow_set_GRAMMER_TERM,
        follow_set_GRAMMER_TERM_TMP1,
        follow_set_GRAMMER_TERM_TMP2,
        follow_set_GRAMMER_FACTOR,
        follow_set_GRAMMER_FACTOR_TMP1,
        follow_set_GRAMMER_FACTOR_TMP2,
        NULL
    };

    static rsize_t follow_set_num[] = {
        sizeof(follow_set_GRAMMER_PROGRAM) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_PROGRAM_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_STATEMENT) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_STATEMENT_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_TYPE) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_EXPRESSION) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_EXPRESSION_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_EXPRESSION_TMP2) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_TERM) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_TERM_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_TERM_TMP2) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_FACTOR) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_FACTOR_TMP1) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_FACTOR_TMP2) / sizeof(TP_GRAMMER_TERM),
        0
    };

    static TP_PARSE_TREE_GRAMMER grammer_index[] = {
        TP_PARSE_TREE_GRAMMER_PROGRAM,
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
        TP_PARSE_TREE_GRAMMER_NULL
    };

    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_index[i]; ++i){

        if (grammer_index[i] == grammer->member_grammer){

            if (follow_set_num[i] != grammer->member_follow_set_num){

                TP_PUT_LOG_PRINT(
                    symbol_table, TP_MSG_FMT("\n=== follow set num(index = %1: %2, %3) ==="),
                    TP_LOG_PARAM_UINT64_VALUE(i),
                    TP_LOG_PARAM_UINT64_VALUE(follow_set_num[i]),
                    TP_LOG_PARAM_UINT64_VALUE(grammer->member_follow_set_num)
                );

                return false;
            }

            if ( ! tp_compare_first_or_follow_set(follow_set[i], follow_set_num[i], grammer->member_follow_set)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                TP_PUT_LOG_PRINT(
                    symbol_table,
                    TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== tp_compare_first_or_follow_set failed. ===")
                );

                return false;
            }

            return true;
        }
    }

    TP_PUT_LOG_PRINT(
        symbol_table, TP_MSG_FMT("\n=== unmatch grammer(%1). ==="),
        TP_LOG_PARAM_INT32_VALUE(grammer->member_grammer)
    );

    return false;
}

