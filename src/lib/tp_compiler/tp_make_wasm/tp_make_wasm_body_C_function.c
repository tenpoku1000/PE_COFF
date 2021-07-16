
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert C_IR to WebAssembly(C compiler: function definition).

static bool begin_wasm_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    uint32_t* function_body_offset, uint32_t* function_body_size,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    TP_C_GLOBAL_FUNCTION* global_function
);
static bool end_wasm_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    uint32_t* function_body_offset, uint32_t* function_body_size,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter
);

bool tp_make_wasm_C_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_FUNCTION* type_function, TP_C_DECL* c_decl)
{
    TP_C_GLOBAL_FUNCTION* global_function = &(c_decl->member_c_decl_body.member_global_function);

    c_decl->member_c_decl_kind = TP_C_DECL_KIND_GLOBAL_FUNCTION;

    uint8_t* section_buffer = symbol_table->member_code_section_buffer;

    if (section_buffer){

        bool is_external = true;

        if ( ! tp_wasm_add_function(
            symbol_table, is_external,
            c_object->member_token->member_string, // function name.
            c_object->member_token->member_string_length,
            &(global_function->member_symbol_index))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if ( ! tp_make_wasm_C_compound_statement(
        symbol_table, entry_point_symbol,
        section, c_object,
        &(type_function->member_function_body),
        &(type_function->member_function_body_offset),
        &(type_function->member_function_body_size),
        type_function->member_c_return_type,
        c_object->member_token, // function name.
        type_function->member_parameter_num,
        type_function->member_parameter,
        global_function)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_make_wasm_C_compound_statement(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    uint32_t* function_body_offset, uint32_t* function_body_size,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    TP_C_GLOBAL_FUNCTION* global_function)
{
    if (function_name){

        if ( ! begin_wasm_function(
            symbol_table, entry_point_symbol,
            section, c_object,
            compound_statement,
            function_body_offset, function_body_size,
            type_return, function_name,
            parameter_num, parameter, global_function)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    for (rsize_t i = 0;
        compound_statement->member_statement_body_pos > i; ++i){

        TP_C_OBJECT* statement_body_element =
            compound_statement->member_statement_body[i];

        if (NULL == statement_body_element){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if (TP_C_TYPE_TYPE_FUNCTION ==
            statement_body_element->member_type.member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if ( ! tp_make_wasm_C_body(
            symbol_table, entry_point_symbol,
            section, statement_body_element,
            type_return, parameter_num, parameter)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if (function_name){

        if ( ! end_wasm_function(symbol_table,
            entry_point_symbol, section, c_object,
            compound_statement,
            function_body_offset, function_body_size,
            type_return, function_name,
            parameter_num, parameter)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool begin_wasm_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    uint32_t* function_body_offset, uint32_t* function_body_size,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    TP_C_GLOBAL_FUNCTION* global_function)
{
    if ((NULL == function_body_offset) || (NULL == function_body_size)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    symbol_table->member_is_nothing_return_statements = true;

    uint8_t* section_buffer = symbol_table->member_code_section_buffer;
    size_t index = symbol_table->member_code_index;

    if (section_buffer){

        index += tp_encode_ui32leb128(section_buffer, index, *function_body_size);
    }else{

        *function_body_offset = (uint32_t)index;

        ++(symbol_table->member_code_count);
        rsize_t code_count = symbol_table->member_code_count;

        TP_WASM_FUNC_LOCAL_TYPE* tmp_func_local_types =
            (TP_WASM_FUNC_LOCAL_TYPE*)TP_REALLOC(
                symbol_table, symbol_table->member_func_local_types,
                sizeof(TP_WASM_FUNC_LOCAL_TYPE) * code_count
            );

        if (NULL == tmp_func_local_types){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        symbol_table->member_func_local_types = tmp_func_local_types;

        symbol_table->member_func_local_types[code_count - 1].member_local_types = NULL;
        symbol_table->member_func_local_types[code_count - 1].member_local_types_num = 0;
    }

    uint32_t base_local_index = 0;
    uint32_t var_count_i64 = 0;
    uint32_t var_count_i32 = 0;

    if ( ! tp_get_base_local_index(
        symbol_table, parameter_num, parameter,
        &base_local_index, &var_count_i64, &var_count_i32)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint32_t local_count = 2;
    index += tp_encode_ui32leb128(section_buffer, index, local_count);

    index += tp_encode_ui32leb128(section_buffer, index, var_count_i64);
    index += tp_encode_ui32leb128(
        section_buffer, index, TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64
    );

    index += tp_encode_ui32leb128(section_buffer, index, var_count_i32);
    index += tp_encode_ui32leb128(
        section_buffer, index, TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32
    );

    // Prologue
    TP_WASM_CODE_BEGIN;
    symbol_table->member_wasm_relocation_begin = index;
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_get_global_code(section_buffer, index, 0));
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_i64_const_code(section_buffer, index, 16));
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_i64_sub_code(section_buffer, index));
    TP_WASM_CODE_PROLOGUE(
        symbol_table, index, tp_make_tee_local_code(section_buffer, index, base_local_index)
    );
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_set_global_code(section_buffer, index, 0));
    symbol_table->member_code_index = index;
    symbol_table->member_code_body_size += (uint32_t)index;

    bool is_export = true;

    if ( ! tp_wasm_add_function_type_C(
        symbol_table, section_buffer, compound_statement, is_export,
        type_return, function_name, parameter_num, parameter, global_function)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;

fail:
    return false;
}

static bool end_wasm_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    uint32_t* function_body_offset, uint32_t* function_body_size,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter)
{
    if ((NULL == function_body_offset) || (NULL == function_body_size)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    uint8_t* section_buffer = symbol_table->member_code_section_buffer;
    size_t index = symbol_table->member_code_index;
    TP_WASM_CODE_BEGIN;

    if (symbol_table->member_is_nothing_return_statements){

        // Epilogue
        uint32_t base_local_index = 0;

        if ( ! tp_get_base_local_index(
            symbol_table, parameter_num, parameter, &base_local_index, NULL, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        TP_WASM_CODE_EPILOGUE(
            symbol_table, index,
            tp_make_get_local_code(section_buffer, index, base_local_index)
        );
        TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_i64_const_code(section_buffer, index, 16));
        TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_i64_add_code(section_buffer, index));
        TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_set_global_code(section_buffer, index, 0));
    }

    TP_WASM_CODE(symbol_table, index, tp_make_end_code(section_buffer, index));

    symbol_table->member_code_index = index;

    if (NULL == section_buffer){

        *function_body_size = (uint32_t)(index - (*function_body_offset));

        symbol_table->member_code_index += tp_encode_ui32leb128(NULL, 0, *function_body_size);

        ++(symbol_table->member_func_local_types_current);
    }

    return true;

fail:
    return false;
}

bool tp_get_base_local_index(
    TP_SYMBOL_TABLE* symbol_table, uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    uint32_t* base_local_index, uint32_t* var_count_i64, uint32_t* var_count_i32)
{
    if (NULL == base_local_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    uint32_t arg_num = parameter_num; // base_local_index

    if (1 == arg_num){

        TP_C_TYPE_FUNCTION_F_PARAM* function_parameter = parameter;

        if (function_parameter){

            TP_C_TYPE* type_parameter = function_parameter->member_type_parameter;

            if (type_parameter){

                if (TP_C_TYPE_TYPE_BASIC == type_parameter->member_type){

                    if (TP_C_TYPE_SPECIFIER_VOID ==
                        type_parameter->member_body.member_type_basic.member_type_specifier){

                        arg_num = 0;
                    }
                }else{

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }
            }else{

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
    }

    *base_local_index = arg_num; // base_local_index

    if (var_count_i64){

        *var_count_i64 = 2; // base_local_index + 1
    }

    if (var_count_i32){

        *var_count_i32 = 1; // base_local_index + 2
    }

    return true;
}

