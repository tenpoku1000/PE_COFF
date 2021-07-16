
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert C_IR to WebAssembly(C compiler: declarations_struct).

bool tp_make_wasm_C_struct(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type, TP_C_TYPE_STRUCT* type_struct)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);
    return false;
}

