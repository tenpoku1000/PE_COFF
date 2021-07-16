
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert C_IR to WebAssembly(C compiler: declarations).

bool tp_make_wasm_C_declaration_statement(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type)
{
    TP_C_TYPE* declaration = NULL;

    switch (kind_of_type){
    case TP_C_TYPE_TYPE_FUNCTION_PARAM:{

        TP_C_TYPE_FUNCTION_FORMAL_PARAM* type_param =
            &(c_object->member_type.member_body.member_type_param);
        declaration = type_param->member_type_param;
        break;
    }
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE_DECLARATION_STATEMENT* type_declaration_statement =
            &(c_object->member_type.member_body.member_type_declaration_statement);
        declaration = type_declaration_statement->member_declaration;
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if (NULL == declaration){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch(declaration->member_type){
    case TP_C_TYPE_TYPE_BASIC:
        if ( ! tp_make_wasm_C_basic(
            symbol_table, entry_point_symbol,
            section, c_object, kind_of_type,
            &(declaration->member_body.member_type_basic))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    case TP_C_TYPE_TYPE_STRUCT:
        if ( ! tp_make_wasm_C_struct(
            symbol_table, entry_point_symbol,
            section, c_object, kind_of_type,
            &(declaration->member_body.member_type_struct))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
//  case TP_C_TYPE_TYPE_UNION:
//  case TP_C_TYPE_TYPE_ENUM:
    case TP_C_TYPE_TYPE_POINTER:
        if ( ! tp_make_wasm_C_pointer(
            symbol_table, entry_point_symbol,
            section, c_object, kind_of_type,
            &(declaration->member_body.member_type_pointer))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
//  case TP_C_TYPE_TYPE_ARRAY:
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

