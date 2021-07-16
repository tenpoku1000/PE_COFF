
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool calc_first_follow_set(TP_SYMBOL_TABLE* symbol_table);
static bool calc_first_follow(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num, bool is_generate
);

bool tp_calc_first_follow_set_entry_point(TP_SYMBOL_TABLE* symbol_table)
{
    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(" ")
    );

    if ( ! calc_first_follow_set(symbol_table)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\ncalc_first_follow error.\n")
        );

        return false;
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\ncalc_first_follow success.\n")
    );

    return true;
}

static bool calc_first_follow_set(TP_SYMBOL_TABLE* symbol_table)
{
    typedef struct func_table_{
        rsize_t grammer_num;
        TP_GRAMMER** grammer_tbl;
        TP_GRAMMER** (*tp_make_grammer)(TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num);
        bool (*tp_test_grammer)(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num);
    }FUNC_TABLE;

    FUNC_TABLE func_table[] = {
        { 0, NULL, tp_make_grammer_1, tp_test_grammer_1 },
        { 0, NULL, tp_make_grammer_2, tp_test_grammer_2 },
        { 0, NULL, tp_make_grammer_3, tp_test_grammer_3 },
        { 0, NULL, NULL, NULL }
    };

    bool status = false;

    for (rsize_t i = 0; NULL != func_table[i].tp_make_grammer; ++i){

        func_table[i].grammer_tbl = func_table[i].tp_make_grammer(
            symbol_table, &(func_table[i].grammer_num)
        );

        if ( ! calc_first_follow(
            symbol_table, func_table[i].grammer_tbl, func_table[i].grammer_num, 0 == i/* is_generate */)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto out;
        }
    }

    if ( ! symbol_table->member_is_need_test_first_follow_set){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== test is disable. ===")
        );

        status = true;

        goto out;
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== begin test: ===")
    );

    bool is_test_failed = false;

    for (rsize_t i = 0; NULL != func_table[i].tp_make_grammer; ++i){

        if ( ! func_table[i].tp_test_grammer(
            symbol_table, func_table[i].grammer_tbl, func_table[i].grammer_num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            TP_PUT_LOG_PRINT(
                symbol_table,
                TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== test failed. ===")
            );

            is_test_failed = true;
        }
    }

    if (is_test_failed){

        goto out;
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== all test passed. ===")
    );

    status = true;

out:

    for (rsize_t i = 0; NULL != func_table[i].tp_make_grammer; ++i){

        tp_free_grammer(symbol_table, &(func_table[i].grammer_tbl), func_table[i].grammer_num);
    }

    return status;
}

static bool calc_first_follow(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num, bool is_generate)
{
    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== calc_empty ===")
    );

    if ( ! tp_calc_empty(symbol_table, grammer_tbl, grammer_num)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        tp_free_grammer(symbol_table, &grammer_tbl, grammer_num);

        return false;
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== calc_first_set ===")
    );

    if ( ! tp_calc_first_set(symbol_table, grammer_tbl, grammer_num, is_generate)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        tp_free_grammer(symbol_table, &grammer_tbl, grammer_num);

        return false;
    }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== calc_follow_set ===")
    );

    if ( ! tp_calc_follow_set(symbol_table, grammer_tbl, grammer_num, is_generate)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        tp_free_grammer(symbol_table, &grammer_tbl, grammer_num);

        return false;
    }

    return true;
}

