
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(struct or union specifier).

bool tp_make_C_IR_struct_or_union_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);

    return false;
}

