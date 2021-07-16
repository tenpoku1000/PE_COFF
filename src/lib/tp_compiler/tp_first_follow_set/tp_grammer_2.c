
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool test_first_set_grammer_2(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer);
static bool test_follow_set_grammer_2(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer);

// Grammer:
//
// E -> TE'
// E' -> +TE' | -TE' | ε
// T -> FT'
// T' -> *FT' | /FT' | ε
// F -> (E) | i | num
//
// Example:
//
// (1 + 2) * 3 + id
//
TP_GRAMMER** tp_make_grammer_2(TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num)
{
    // E -> TE'
    TP_GRAMMER* E = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_E,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_T),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_E_TMP),
        TP_TERM_NULL
    );

    // E' -> +TE' | -TE' | ε
    TP_GRAMMER* E_TMP = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_E_TMP,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "+"),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_T),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_E_TMP),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "-"),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_T),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_E_TMP),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    // T -> FT'
    TP_GRAMMER* T = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_T,
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_F),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_T_TMP),
        TP_TERM_NULL
    );

    // T' -> *FT' | /FT' | ε
    TP_GRAMMER* T_TMP = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_T_TMP,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "*"),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_F),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_T_TMP),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "/"),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_F),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_T_TMP),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL_EMPTY,
        TP_TERM_NULL
    );

    // F -> (E) | i | num
    TP_GRAMMER* F = TP_MAKE_GRAMMER_ELEMENT(
        symbol_table, TP_PARSE_TREE_GRAMMER_F,
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, "("),
        TP_NONTERM_SYMBOL_REFERENCE(TP_PARSE_TREE_GRAMMER_E),
        TP_TERM_SYMBOL(TP_SYMBOL_PUNCTUATOR, ")"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_IDENTIFIER, "i"),
        TP_TERM_PIPE,
        TP_TERM_SYMBOL(TP_SYMBOL_CONSTANT, "num"),
        TP_TERM_NULL
    );

    // NULL
    TP_GRAMMER* end = TP_MAKE_GRAMMER_ELEMENT(symbol_table, TP_PARSE_TREE_GRAMMER_NULL, TP_TERM_NULL);

    return TP_MAKE_GRAMMER(symbol_table, grammer_num, E, E_TMP, T, T_TMP, F, end);
}

bool tp_test_grammer_2(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num)
{
    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== begin test grammer_2: ===")
    );

    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

        if ( ! test_first_set_grammer_2(symbol_table, grammer_tbl[i])){

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

        if ( ! test_follow_set_grammer_2(symbol_table, grammer_tbl[i])){

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

static bool test_first_set_grammer_2(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer)
{
    // E -> TE'
    static TP_GRAMMER_TERM first_set_GRAMMER_E[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "i" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "num" },
    };

    // E' -> +TE' | -TE' | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_E_TMP[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // T -> FT'
    static TP_GRAMMER_TERM first_set_GRAMMER_T[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "i" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "num" },
    };

    // T' -> *FT' | /FT' | ε
    static TP_GRAMMER_TERM first_set_GRAMMER_T_TMP[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "*" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "/" },
        { .member_symbol = TP_SYMBOL_EMPTY, .member_string = "EMPTY" },
    };

    // F -> (E) | i | num
    static TP_GRAMMER_TERM first_set_GRAMMER_F[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "(" },
        { .member_symbol = TP_SYMBOL_IDENTIFIER, .member_string = "i" },
        { .member_symbol = TP_SYMBOL_CONSTANT, .member_string = "num" },
    };

    static TP_GRAMMER_TERM* first_set[] = {
        first_set_GRAMMER_E,
        first_set_GRAMMER_E_TMP,
        first_set_GRAMMER_T,
        first_set_GRAMMER_T_TMP,
        first_set_GRAMMER_F,
        NULL
    };

    static rsize_t first_set_num[] = {
        sizeof(first_set_GRAMMER_E) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_E_TMP) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_T) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_T_TMP) / sizeof(TP_GRAMMER_TERM),
        sizeof(first_set_GRAMMER_F) / sizeof(TP_GRAMMER_TERM),
        0
    };

    static TP_PARSE_TREE_GRAMMER grammer_index[] = {
        TP_PARSE_TREE_GRAMMER_E,
        TP_PARSE_TREE_GRAMMER_E_TMP,
        TP_PARSE_TREE_GRAMMER_T,
        TP_PARSE_TREE_GRAMMER_T_TMP,
        TP_PARSE_TREE_GRAMMER_F,
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

static bool test_follow_set_grammer_2(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer)
{
    // E -> TE'
    static TP_GRAMMER_TERM follow_set_GRAMMER_E[] = {
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // E' -> +TE' | -TE' | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_E_TMP[] = {
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // T -> FT'
    static TP_GRAMMER_TERM follow_set_GRAMMER_T[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // T' -> *FT' | /FT' | ε
    static TP_GRAMMER_TERM follow_set_GRAMMER_T_TMP[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    // F -> (E) | i | num
    static TP_GRAMMER_TERM follow_set_GRAMMER_F[] = {
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "*" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "/" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "+" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = "-" },
        { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" },
        { .member_symbol = TP_SYMBOL_PUNCTUATOR, .member_string = ")" },
    };

    static TP_GRAMMER_TERM* follow_set[] = {
        follow_set_GRAMMER_E,
        follow_set_GRAMMER_E_TMP,
        follow_set_GRAMMER_T,
        follow_set_GRAMMER_T_TMP,
        follow_set_GRAMMER_F,
        NULL
    };

    static rsize_t follow_set_num[] = {
        sizeof(follow_set_GRAMMER_E) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_E_TMP) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_T) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_T_TMP) / sizeof(TP_GRAMMER_TERM),
        sizeof(follow_set_GRAMMER_F) / sizeof(TP_GRAMMER_TERM),
        0
    };

    static TP_PARSE_TREE_GRAMMER grammer_index[] = {
        TP_PARSE_TREE_GRAMMER_E,
        TP_PARSE_TREE_GRAMMER_E_TMP,
        TP_PARSE_TREE_GRAMMER_T,
        TP_PARSE_TREE_GRAMMER_T_TMP,
        TP_PARSE_TREE_GRAMMER_F,
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

