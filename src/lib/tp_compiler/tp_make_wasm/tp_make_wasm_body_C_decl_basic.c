
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert C_IR to WebAssembly(C compiler: declarations_basic).

bool tp_make_wasm_C_basic(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type, TP_C_TYPE_BASIC* type_basic)
{
    TP_C_TYPE_FUNCTION_FORMAL_PARAM* type_param = NULL;
    TP_C_TYPE_DECLARATION_STATEMENT* type_declaration_statement = NULL;

    switch (kind_of_type){
    case TP_C_TYPE_TYPE_FUNCTION_PARAM:
        type_param = &(c_object->member_type.member_body.member_type_param);
        break;
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:
        type_declaration_statement =
            &(c_object->member_type.member_body.member_type_declaration_statement);
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    bool is_signed =
        (TP_C_TYPE_SPECIFIER_SIGNED & type_basic->member_type_specifier);

    bool is_unsigned =
        (TP_C_TYPE_SPECIFIER_UNSIGNED & type_basic->member_type_specifier);

    if (is_signed && is_unsigned){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    // C compiler: declaration
    TP_C_DECL* c_decl = &(c_object->member_type.member_decl);
    bool is_data_segment = c_object->member_is_static_data;

    if (is_data_segment){

        if ((NULL == type_declaration_statement) ||
            (1 != type_declaration_statement->member_c_expr_pos)){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
    }

    switch (TP_MASK_C_TYPE_SPECIFIER(type_basic->member_type_specifier)){
    case TP_C_TYPE_SPECIFIER_VOID:
        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(TP_MSG_FMT("ERROR: Bad void type."))
        );
        return false;
    case TP_C_TYPE_SPECIFIER_CHAR:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    case TP_C_TYPE_SPECIFIER_SHORT:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    case TP_C_TYPE_SPECIFIER_INT:
//      break;
    case TP_C_TYPE_SPECIFIER_LONG1:
        if (is_data_segment){

            c_decl->member_c_decl_kind = TP_C_DECL_KIND_GLOBAL_VARIABLE_I32;

            TP_C_GLOBAL_VARIABLE_I32* global_variable_i32 =
                &(c_decl->member_c_decl_body.member_global_variable_i32);

            global_variable_i32->member_var_id = symbol_table->member_var_global_index;

            ++(symbol_table->member_var_global_index);

            global_variable_i32->member_init_value =
                type_declaration_statement->member_c_expr[0].
                    member_c_expr_body.member_i32_const.member_value;

            bool is_external = true;

            if ( ! tp_wasm_add_global_variable_i32(
                symbol_table, is_external,
                global_variable_i32->member_var_id,
                c_object->member_token->member_string,
                c_object->member_token->member_string_length,
                global_variable_i32->member_init_value,
                &(global_variable_i32->member_wasm_offset),
                &(global_variable_i32->member_coff_offset),
                &(global_variable_i32->member_symbol_index))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }else{

            c_decl->member_c_decl_kind = TP_C_DECL_KIND_LOCAL_VARIABLE_I32;

            TP_C_LOCAL_VARIABLE_I32* local_variable_i32 =
                &(c_decl->member_c_decl_body.member_local_variable_i32);

            local_variable_i32->member_local_index = symbol_table->member_var_local_index;

            ++(symbol_table->member_var_local_index);

            if ( ! tp_wasm_add_local_variable_i32(
                symbol_table,
                local_variable_i32->member_local_index,
                &(local_variable_i32->member_wasm_offset))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }
        break;
    case TP_C_TYPE_SPECIFIER_LONG2:
        if (is_data_segment){

            c_decl->member_c_decl_kind = TP_C_DECL_KIND_GLOBAL_VARIABLE_I64;

            TP_C_GLOBAL_VARIABLE_I64* global_variable_i64 =
                &(c_decl->member_c_decl_body.member_global_variable_i64);

            global_variable_i64->member_var_id = symbol_table->member_var_global_index;

            ++(symbol_table->member_var_global_index);

            global_variable_i64->member_init_value =
                type_declaration_statement->member_c_expr[0].
                    member_c_expr_body.member_i64_const.member_value;

            bool is_external = true;

            if ( ! tp_wasm_add_global_variable_i64(
                symbol_table, is_external,
                global_variable_i64->member_var_id,
                c_object->member_token->member_string,
                c_object->member_token->member_string_length,
                global_variable_i64->member_init_value,
                &(global_variable_i64->member_wasm_offset),
                &(global_variable_i64->member_coff_offset),
                &(global_variable_i64->member_symbol_index))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }else{

            c_decl->member_c_decl_kind = TP_C_DECL_KIND_LOCAL_VARIABLE_I64;

            TP_C_LOCAL_VARIABLE_I64* local_variable_i64 =
                &(c_decl->member_c_decl_body.member_local_variable_i64);

            local_variable_i64->member_local_index = symbol_table->member_var_local_index;

            ++(symbol_table->member_var_local_index);

            if ( ! tp_wasm_add_local_variable_i64(
                symbol_table,
                local_variable_i64->member_local_index,
                &(local_variable_i64->member_wasm_offset))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ((false == is_data_segment) && (NULL == type_param)){

        if ( ! tp_make_wasm_C_expression(
            symbol_table, c_object,
            symbol_table->member_code_section_buffer,
            &(symbol_table->member_code_index))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

