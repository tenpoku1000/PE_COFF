
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert C_IR to WebAssembly(C compiler).

static bool make_wasm_C_main(
    TP_SYMBOL_TABLE* symbol_table,  uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section
);

TP_WASM_MODULE_SECTION* tp_make_wasm_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol)
{
    TP_WASM_MODULE_SECTION* section = NULL;

    symbol_table->member_code_body_size = 0;
    symbol_table->member_code_index = 0;
    symbol_table->member_code_count = 0;

    uint8_t* section_buffer = symbol_table->member_code_section_buffer;
    size_t index = symbol_table->member_code_index;
    TP_WASM_CODE_BEGIN;

    if ( ! make_wasm_C_main(symbol_table, entry_point_symbol, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    index = symbol_table->member_code_index;

    if (0 == symbol_table->member_code_count){

        TP_WASM_CODE(symbol_table, index, tp_make_end_code(section_buffer, index));
        symbol_table->member_code_index = index;
        symbol_table->member_code_body_size = (uint32_t)index;
    }else{

        symbol_table->member_code_body_size = (uint32_t)index;
    }

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, symbol_table->member_code_count);
    payload_len += symbol_table->member_code_body_size;

    symbol_table->member_code_section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, symbol_table->member_code_section_buffer,
        TP_WASM_SECTION_TYPE_CODE, payload_len
    );

    section_buffer = symbol_table->member_code_section_buffer;
    index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_CODE);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, symbol_table->member_code_count);
    symbol_table->member_code_index = index;

    TP_FREE(symbol_table, &(symbol_table->member_wasm_relocation), symbol_table->member_wasm_relocation_size);
    symbol_table->member_wasm_relocation_begin = 0;
    symbol_table->member_wasm_relocation_cur = 0;
    symbol_table->member_wasm_relocation_pos = 0;
    symbol_table->member_wasm_relocation_size = 0;

    if ( ! make_wasm_C_main(symbol_table, entry_point_symbol, section)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    index = symbol_table->member_code_index;

    if (0 == symbol_table->member_code_count){

        TP_WASM_CODE(symbol_table, index, tp_make_end_code(section_buffer, index));
    }

    return section;

fail:
    if (section){

        TP_FREE(symbol_table, &(section->member_name_len_name_payload_data), section->member_section_size);

        TP_FREE(symbol_table, &section, sizeof(TP_WASM_MODULE_SECTION));
    }

    return NULL;
}

static bool make_wasm_C_main(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, TP_WASM_MODULE_SECTION* section)
{
    symbol_table->member_wasm_local_pos = 0;

    if (NULL == symbol_table->member_c_object){

        TP_PUT_LOG_PRINT(
            symbol_table, TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("There is no TP_C_OBJECT."),
        );

        return true;
    }

    rsize_t c_object_pos = (symbol_table->member_c_object_pos ? symbol_table->member_c_object_pos : 1);

    for (rsize_t j = 0; c_object_pos > j; ++j){

        TP_C_OBJECT* c_object = symbol_table->member_c_object[j];

        if (NULL == c_object){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if ( ! tp_make_wasm_C_body(
            symbol_table, entry_point_symbol,
            section, c_object, NULL, 0, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

bool tp_make_wasm_C_body(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE* type_return,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter)
{
    switch(c_object->member_type.member_type){
    case TP_C_TYPE_TYPE_FUNCTION:
        if ( ! tp_make_wasm_C_function(
            symbol_table, entry_point_symbol,
            section, c_object,
            &(c_object->member_type.member_body.member_type_function),
            &(c_object->member_type.member_decl))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    case TP_C_TYPE_TYPE_FUNCTION_PARAM:
        if ( ! tp_make_wasm_C_declaration_statement(
            symbol_table, entry_point_symbol,
            section, c_object, TP_C_TYPE_TYPE_FUNCTION_PARAM)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
//  case TP_C_TYPE_TYPE_TYPEDEF:
    case TP_C_TYPE_TYPE_NULL_STATEMENT:
        break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
        if ( ! tp_make_wasm_C_compound_statement(
            symbol_table, entry_point_symbol,
            section, c_object,
            &(c_object->member_type.member_body.member_type_compound_statement),
            NULL, NULL, type_return, NULL, parameter_num, parameter, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
//  case TP_C_TYPE_TYPE_LABELED_STATEMENT_LABEL:
//  case TP_C_TYPE_TYPE_LABELED_STATEMENT_CASE:
//  case TP_C_TYPE_TYPE_LABELED_STATEMENT_DEFAULT:
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:
        if ( ! tp_make_wasm_C_declaration_statement(
            symbol_table, entry_point_symbol,
            section, c_object, TP_C_TYPE_TYPE_DECLARATION_STATEMENT)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    case TP_C_TYPE_TYPE_EXPRESSION_STATEMENT:
        if ( ! tp_make_wasm_C_expression(
            symbol_table, c_object,
            symbol_table->member_code_section_buffer,
            &(symbol_table->member_code_index))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
//  case TP_C_TYPE_TYPE_SELECTION_STATEMENT_IF:
//  case TP_C_TYPE_TYPE_SELECTION_STATEMENT_SWITCH:
//  case TP_C_TYPE_TYPE_ITERATION_STATEMENT_WHILE:
    case TP_C_TYPE_TYPE_ITERATION_STATEMENT_DO:
        if ( ! tp_make_wasm_C_iteration_statement_do(
            symbol_table, entry_point_symbol,
            section, c_object,
            &(c_object->member_type.member_body.
                member_type_iteration_statement_do),
            type_return, parameter_num, parameter)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
//  case TP_C_TYPE_TYPE_ITERATION_STATEMENT_FOR:
//  case TP_C_TYPE_TYPE_ITERATION_STATEMENT_FOR_DECL:
//  case TP_C_TYPE_TYPE_JUMP_STATEMENT_GOTO:
//  case TP_C_TYPE_TYPE_JUMP_STATEMENT_CONTINUE:
//  case TP_C_TYPE_TYPE_JUMP_STATEMENT_BREAK:
    case TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN:
        if ( ! tp_make_wasm_C_jump_statement_return(
            symbol_table, entry_point_symbol,
            section, c_object,
            &(c_object->
                member_type.member_body.member_type_jump_statement_return),
            type_return, parameter_num, parameter)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

