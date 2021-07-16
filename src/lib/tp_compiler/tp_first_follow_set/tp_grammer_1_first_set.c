
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef struct tp_grammer_first_set_{
    TP_PARSE_TREE_GRAMMER member_grammer;

    TP_GRAMMER_TERM member_first_set[
#include "../generate/first_follow_set/first_set_1_max_num.txt"
    ];
}TP_GRAMMER_FIRST_SET;

static TP_GRAMMER_FIRST_SET grammer_first_set[] = {
#include "../generate/first_follow_set/first_set_1.txt"
    { TP_PARSE_TREE_GRAMMER_NULL, { { TP_SYMBOL_NULL, NULL } } }
};

bool tp_get_grammer_1_first_set(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER_FIRST_FOLLOW_SET_INDEX index, TP_PARSE_TREE_GRAMMER grammer)
{
    rsize_t max_index = sizeof(grammer_first_set) / sizeof(TP_GRAMMER_FIRST_SET) - 1;

    if ((0 <= index) && (max_index >= index)){

        if (grammer != grammer_first_set[index].member_grammer){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        symbol_table->member_first_set = grammer_first_set[index].member_first_set;

        return true;
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);

    return false;
}

