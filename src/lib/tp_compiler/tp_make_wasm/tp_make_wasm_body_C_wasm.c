
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// WebAssembly headers(C compiler).

#define TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_1 6
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_1 "memory"
#define TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_1 0

static bool wasm_add_function_section_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer,
    uint32_t index_of_type, bool is_export, TP_TOKEN* function_name,
    uint32_t* function_index
);
static bool wasm_add_export_section_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer,
    uint32_t index_of_function, bool is_export, TP_TOKEN* function_name
);

bool tp_wasm_add_function_type_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement, bool is_export,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    TP_C_GLOBAL_FUNCTION* global_function)
{
    // Type of return value.
    switch (type_return->member_type){
    case TP_C_TYPE_TYPE_BASIC:
        switch (TP_MASK_C_TYPE_SPECIFIER(
            type_return->member_body.member_type_basic.member_type_specifier)){
        case TP_C_TYPE_SPECIFIER_VOID:
            global_function->member_type_return = TP_WASM_RETURN_VOID;
            break;
        case TP_C_TYPE_SPECIFIER_INT:
//          break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            global_function->member_type_return = TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            global_function->member_type_return = TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    // type section
    uint32_t type_count = symbol_table->member_wasm_type_count;

    if (section_buffer && (0 == type_count)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    uint32_t* param_types = NULL;

    if (parameter_num){

        param_types = (uint32_t*)TP_CALLOC(
            symbol_table, parameter_num, sizeof(uint32_t)
        );

        if (NULL == param_types){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        TP_C_TYPE_FUNCTION_F_PARAM* current = parameter;

        for (uint32_t i = 0; parameter_num > i; ++i){

            if (NULL == current){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                goto fail;
            }

            switch(current->member_type_parameter->member_type){
            case TP_C_TYPE_TYPE_BASIC:{

                TP_C_TYPE_BASIC* type_basic =
                    &(current->member_type_parameter->member_body.member_type_basic);

                switch (TP_MASK_C_TYPE_SPECIFIER(type_basic->member_type_specifier)){
                case TP_C_TYPE_SPECIFIER_VOID:
                    if (1 == parameter_num){

                        parameter_num = 0;
                        TP_FREE(symbol_table, &param_types, sizeof(uint32_t));
                        goto type_skip;
                    }
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                case TP_C_TYPE_SPECIFIER_CHAR:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                case TP_C_TYPE_SPECIFIER_SHORT:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                case TP_C_TYPE_SPECIFIER_INT:
//                  break;
                case TP_C_TYPE_SPECIFIER_LONG1:
                    param_types[i] = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32;
                    break;
                case TP_C_TYPE_SPECIFIER_LONG2:
                    param_types[i] = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64;
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                }
                break;
            }
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            current = parameter->member_next;
        }
    }

type_skip:
        ;

    uint32_t return_count = 0;
    uint32_t return_type = TP_WASM_RETURN_VOID;

    switch(type_return->member_type){
    case TP_C_TYPE_TYPE_BASIC:{

        TP_C_TYPE_BASIC* type_basic = &(type_return->member_body.member_type_basic);

        switch (TP_MASK_C_TYPE_SPECIFIER(type_basic->member_type_specifier)){
        case TP_C_TYPE_SPECIFIER_VOID:
            break;
        case TP_C_TYPE_SPECIFIER_CHAR:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        case TP_C_TYPE_SPECIFIER_SHORT:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        case TP_C_TYPE_SPECIFIER_INT:
//          break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            return_count = 1;
            return_type = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            return_count = 1;
            return_type = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    bool is_match_type = false;
    uint32_t index_of_type = 0;

    if (symbol_table->member_wasm_types){

        for (uint32_t i = 0; type_count > i; ++i){

            if ((TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC !=
                    symbol_table->member_wasm_types[i].member_form) ||
                (parameter_num !=
                    symbol_table->member_wasm_types[i].member_param_count) ||
                (return_count !=
                    symbol_table->member_wasm_types[i].member_return_count) ||
                (return_type !=
                    symbol_table->member_wasm_types[i].member_return_type)){

                continue;
            }

            if (0 == parameter_num){

                is_match_type = true;
                index_of_type = i;
                break;
            }

            for (uint32_t j = 0; parameter_num > j; ++j){

                if (param_types[j] !=
                    symbol_table->member_wasm_types[i].member_param_types[j]){

                    goto type_next;
                }
            }

            is_match_type = true;
            index_of_type = i;
            break;
type_next:
            ;
        }
    }

    if (NULL == section_buffer){

        if (UINT32_MAX == type_count){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: overflow of wasm type section.")
            );

            goto fail;
        }

        if (is_match_type){

            if (parameter_num){

                TP_FREE(symbol_table, &param_types, parameter_num * sizeof(uint32_t));
            }
        }else{

            ++(symbol_table->member_wasm_type_count);

            TP_WASM_SECTION_TYPE_VAR* wasm_types = (TP_WASM_SECTION_TYPE_VAR*)TP_REALLOC(
                symbol_table, symbol_table->member_wasm_types,
                sizeof(TP_WASM_SECTION_TYPE_VAR) * symbol_table->member_wasm_type_count
            );

            if (NULL == wasm_types){

                TP_PRINT_CRT_ERROR(symbol_table);

                goto fail;
            }

            symbol_table->member_wasm_types = wasm_types;

            index_of_type = symbol_table->member_wasm_type_count - 1;

            symbol_table->member_wasm_types[index_of_type] = (TP_WASM_SECTION_TYPE_VAR){
                .member_form = TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC,
                .member_param_count = parameter_num,
                .member_param_types = param_types,
                .member_return_count = return_count,
                .member_return_type = return_type
            };
        }
    }else{

        switch (symbol_table->member_wasm_type_count){
        case 0:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        case 1:
            index_of_type = 0;
            break;
        default:
            if (false == is_match_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            break;
        }

        if (parameter_num){

            TP_FREE(symbol_table, &param_types, parameter_num * sizeof(uint32_t));
        }
    }

    if ( ! wasm_add_function_section_C(
        symbol_table, section_buffer, index_of_type, is_export, function_name,
        &(global_function->member_function_index))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
fail:
    if (parameter_num){

        TP_FREE(symbol_table, &param_types, parameter_num * sizeof(uint32_t));
    }
    return false;
}

static bool wasm_add_function_section_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer,
    uint32_t index_of_type, bool is_export, TP_TOKEN* function_name,
    uint32_t* function_index)
{
    uint32_t index_of_function = 0;

    if (NULL == section_buffer){

        if (UINT32_MAX == symbol_table->member_wasm_function_count){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: overflow of wasm function section.")
            );

            return false;
        }

        ++(symbol_table->member_wasm_function_count);

        uint32_t* wasm_functions = (uint32_t*)TP_REALLOC(
            symbol_table, symbol_table->member_wasm_functions,
            sizeof(uint32_t) * symbol_table->member_wasm_function_count
        );

        if (NULL == wasm_functions){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        symbol_table->member_wasm_functions = wasm_functions;

        index_of_function = symbol_table->member_wasm_function_count - 1;

        symbol_table->member_wasm_functions[index_of_function] = index_of_type;

        *function_index = index_of_function;
    }else{

        index_of_function = symbol_table->member_wasm_function_count - 1;
    }

    if ( ! wasm_add_export_section_C(symbol_table, section_buffer, index_of_function, is_export, function_name)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool wasm_add_export_section_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer,
    uint32_t index_of_function, bool is_export, TP_TOKEN* function_name)
{
    if (false == is_export){

        return true;
    }

    if (NULL == section_buffer){

        if (UINT32_MAX == symbol_table->member_wasm_export_count){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: overflow of wasm export section.")
            );

            return false;
        }

        ++(symbol_table->member_wasm_export_count);

        TP_WASM_SECTION_EXPORT_VAR* wasm_exports = (TP_WASM_SECTION_EXPORT_VAR*)TP_REALLOC(
            symbol_table, symbol_table->member_wasm_exports,
            sizeof(TP_WASM_SECTION_EXPORT_VAR) * symbol_table->member_wasm_export_count
        );

        if (NULL == wasm_exports){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        symbol_table->member_wasm_exports = wasm_exports;

        symbol_table->member_wasm_exports[symbol_table->member_wasm_export_count - 1] =
            (TP_WASM_SECTION_EXPORT_VAR){
                .member_name_length = (uint32_t)(function_name->member_string_length),
                .member_name = function_name->member_string,
                .member_kind = TP_WASM_SECTION_KIND_FUNCTION,
                .member_item_index = index_of_function
            };
    }

    return true;
}

TP_WASM_MODULE_SECTION* tp_make_section_type_C(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = symbol_table->member_wasm_type_count;
    uint32_t payload_len = 0;

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    for (uint32_t i = 0; 2 > i; ++i){

        if (0 == i){

            payload_len = tp_encode_ui32leb128(NULL, 0, count);

            for (uint32_t j = 0; count > j; ++j){

                payload_len += tp_encode_ui32leb128(
                    NULL, 0, symbol_table->member_wasm_types[j].member_form
                );

                uint32_t param_count = symbol_table->member_wasm_types[j].member_param_count;

                payload_len += tp_encode_ui32leb128(NULL, 0, param_count);

                for (uint32_t k = 0; param_count > k; ++k){

                    payload_len += tp_encode_ui32leb128(
                        NULL, 0, symbol_table->member_wasm_types[j].member_param_types[k]
                    );
                }

                payload_len += tp_encode_ui32leb128(
                    NULL, 0, symbol_table->member_wasm_types[j].member_return_count
                );
                payload_len += tp_encode_ui32leb128(
                    NULL, 0, symbol_table->member_wasm_types[j].member_return_type
                );
            }
        }else{

            TP_MAKE_WASM_SECTION_BUFFER(
                symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_TYPE, payload_len
            );

            size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_TYPE);
            index += tp_encode_ui32leb128(section_buffer, index, payload_len);
            index += tp_encode_ui32leb128(section_buffer, index, count);

            for (uint32_t j = 0; count > j; ++j){

                index += tp_encode_ui32leb128(
                    section_buffer, index, symbol_table->member_wasm_types[j].member_form
                );

                uint32_t param_count = symbol_table->member_wasm_types[j].member_param_count;

                index += tp_encode_ui32leb128(section_buffer, index, param_count);

                for (uint32_t k = 0; param_count > k; ++k){

                    index += tp_encode_ui32leb128(
                        section_buffer, index, symbol_table->member_wasm_types[j].member_param_types[k]
                    );
                }

                index += tp_encode_ui32leb128(
                    section_buffer, index, symbol_table->member_wasm_types[j].member_return_count
                );
                index += tp_encode_ui32leb128(
                    section_buffer, index, symbol_table->member_wasm_types[j].member_return_type
                );
            }
        }
    }

    return section;
}

TP_WASM_MODULE_SECTION* tp_make_section_function_C(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = symbol_table->member_wasm_function_count;
    uint32_t payload_len = 0;

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    for (uint32_t i = 0; 2 > i; ++i){

        if (0 == i){

            payload_len = tp_encode_ui32leb128(NULL, 0, count);

            for (uint32_t j = 0; count > j; ++j){

                payload_len += tp_encode_ui32leb128(
                    section_buffer, 0, symbol_table->member_wasm_functions[j]
                );
            }
        }else{

            TP_MAKE_WASM_SECTION_BUFFER(
                symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_FUNCTION, payload_len
            );

            size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_FUNCTION);
            index += tp_encode_ui32leb128(section_buffer, index, payload_len);
            index += tp_encode_ui32leb128(section_buffer, index, count);

            for (uint32_t j = 0; count > j; ++j){

                index += tp_encode_ui32leb128(
                    section_buffer, index, symbol_table->member_wasm_functions[j]
                );
            }
        }
    }

    return section;
}

TP_WASM_MODULE_SECTION* tp_make_section_export_C(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = symbol_table->member_wasm_export_count;
    uint32_t payload_len = 0;

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    // memory
    uint32_t name_length_1 = TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_1;
    uint8_t* name_1 = TP_WASM_MODULE_SECTION_EXPORT_NAME_1;
    uint8_t kind_1 = TP_WASM_SECTION_KIND_MEMORY;
    uint32_t item_index_1 = TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_1;

    for (uint32_t i = 0; 2 > i; ++i){

        if (0 == i){

            payload_len = tp_encode_ui32leb128(NULL, 0, count + 1);

            // function
            for (uint32_t j = 0; count > j; ++j){

                payload_len += tp_encode_ui32leb128(
                    NULL, 0, symbol_table->member_wasm_exports[j].member_name_length
                );
                payload_len +=
                    symbol_table->member_wasm_exports[j].member_name_length;
                payload_len += sizeof(uint8_t);
                payload_len += tp_encode_ui32leb128(
                    NULL, 0, symbol_table->member_wasm_exports[j].member_item_index
                );
            }

            // memory
            payload_len += tp_encode_ui32leb128(NULL, 0, name_length_1);
            payload_len += name_length_1;
            payload_len += sizeof(uint8_t);
            payload_len += tp_encode_ui32leb128(NULL, 0, item_index_1);
        }else{

            TP_MAKE_WASM_SECTION_BUFFER(
                symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_EXPORT, payload_len
            );

            size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_EXPORT);
            index += tp_encode_ui32leb128(section_buffer, index, payload_len);
            index += tp_encode_ui32leb128(section_buffer, index, count + 1);

            // function
            for (uint32_t j = 0; count > j; ++j){

                index += tp_encode_ui32leb128(
                    section_buffer, index, symbol_table->member_wasm_exports[j].member_name_length
                );
                memcpy(
                    section_buffer + index,
                    symbol_table->member_wasm_exports[j].member_name,
                    symbol_table->member_wasm_exports[j].member_name_length
                );
                index +=
                    symbol_table->member_wasm_exports[j].member_name_length;
                section_buffer[index] =
                    symbol_table->member_wasm_exports[j].member_kind;
                index += sizeof(uint8_t);
                index += tp_encode_ui32leb128(
                    section_buffer, index, symbol_table->member_wasm_exports[j].member_item_index
                );
            }

            // memory
            index += tp_encode_ui32leb128(section_buffer, index, name_length_1);
            memcpy(section_buffer + index, name_1, name_length_1);
            index += name_length_1;
            section_buffer[index] = kind_1;
            index += sizeof(uint8_t);
            index += tp_encode_ui32leb128(section_buffer, index, item_index_1);
        }
    }

    return section;
}

