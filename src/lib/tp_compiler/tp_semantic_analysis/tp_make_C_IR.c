
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR.

bool tp_make_C_IR(TP_SYMBOL_TABLE* symbol_table)
{
    symbol_table->member_nesting_level_of_expression = 0;
    symbol_table->member_nesting_expression = 0;

    if ( ! tp_push_c_namespace_stack(
        symbol_table, TP_GRAMMER_CONTEXT_TRANSLATION_UNIT)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_C_IR_translation_unit(
        symbol_table, symbol_table->member_tp_parse_tree,
        TP_GRAMMER_CONTEXT_TRANSLATION_UNIT, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_pop_c_namespace_stack(
        symbol_table, TP_GRAMMER_CONTEXT_TRANSLATION_UNIT)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_push_c_namespace_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context)
{
    if (symbol_table->member_is_int_calc_compiler){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_TRANSLATION_UNIT:
        break;
    case TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION:
        break;
    case TP_GRAMMER_CONTEXT_STATEMENTS:
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    // variable, function, typedef, enum.
    TP_C_NAMESPACE_STACK* c_namespace_other = &(symbol_table->\
member_c_scope.member_c_namespace[TP_C_NAMESPACE_OTHER]);

    if (c_namespace_other->member_stack_pos ==
        (c_namespace_other->member_stack_size / sizeof(TP_C_NAMESPACE_STACK_ELEMENT))){

        rsize_t c_scope_stack_size_allocate_unit = 
            symbol_table->member_c_scope_stack_size_allocate_unit *
                sizeof(TP_C_NAMESPACE_STACK_ELEMENT);

        rsize_t stack_size =
            c_namespace_other->member_stack_size + c_scope_stack_size_allocate_unit;

        TP_C_NAMESPACE_STACK_ELEMENT* tmp_stack_other =
            (TP_C_NAMESPACE_STACK_ELEMENT*)TP_REALLOC(
                symbol_table, c_namespace_other->member_stack, stack_size
            );

        if (NULL == tmp_stack_other){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tmp_stack_other) + c_namespace_other->member_stack_size,
            0, c_scope_stack_size_allocate_unit
        );

        c_namespace_other->member_stack = tmp_stack_other;

        c_namespace_other->member_stack_size = stack_size;
    }

    ++(c_namespace_other->member_stack_pos);

    return true;
}

bool tp_pop_c_namespace_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context)
{
    if (symbol_table->member_is_int_calc_compiler){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_TRANSLATION_UNIT:
        break;
    case TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION:
        break;
    case TP_GRAMMER_CONTEXT_STATEMENTS:
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    // variable, function, typedef, enum.
    TP_C_NAMESPACE_STACK* c_namespace_other = &(symbol_table->\
member_c_scope.member_c_namespace[TP_C_NAMESPACE_OTHER]);

    if (c_namespace_other->member_stack){

        TP_C_NAMESPACE_STACK_ELEMENT* stack_element =
            &(c_namespace_other->member_stack[c_namespace_other->member_stack_pos - 1]);

        // Current namespace.
        for (rsize_t i = 0; stack_element->member_c_object_pos > i; ++i){

            if (NULL == stack_element->member_c_object){

                continue;
            }

            TP_C_OBJECT* c_object = stack_element->member_c_object[i];
            TP_C_OBJECT* c_object_original = c_object;
            bool is_same_name_object = false;

            if (NULL == c_object_original){

                continue;
            }

            // Find object at outside of stack.
            while (c_object && c_object->member_outside_of_stack){

                if (0 == strcmp(
                    c_object_original->member_token->member_string,
                    c_object->member_token->member_string)){

                    is_same_name_object = true;
                    break;
                }

                c_object = c_object->member_outside_of_stack;
            }

            SAME_HASH_DATA* sama_hash_data = c_object_original->member_sama_hash_data;

            if (NULL == sama_hash_data){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            if (is_same_name_object){

                sama_hash_data->member_register_object.member_stack_c_object = c_object;

                --(sama_hash_data->member_register_object.member_ref_count);

                if (0 == sama_hash_data->member_register_object.member_ref_count){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                c_object->member_sama_hash_data = c_object_original->member_sama_hash_data;
            }else{

                --(sama_hash_data->member_register_object.member_ref_count);

                if (0 == sama_hash_data->member_register_object.member_ref_count){

                    memset(sama_hash_data, 0, sizeof(SAME_HASH_DATA));
                }else{

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }
            }
        }

        TP_FREE2(
            symbol_table, &(stack_element->member_c_object),
            stack_element->member_c_object_size
        );

        if (0 == c_namespace_other->member_stack_pos){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }else{

            memset(((TP_CHAR8_T*)stack_element), 0, sizeof(TP_C_NAMESPACE_STACK_ELEMENT));

            --(c_namespace_other->member_stack_pos);
        }
    }

    return true;
}

bool tp_append_c_object(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object)
{
    if ((symbol_table->member_c_object) &&
        (TP_C_NAMESPACE_OTHER == c_object->member_c_namespace)){

        rsize_t c_object_num = symbol_table->member_c_object_pos;

        for (rsize_t i = 0; c_object_num >= i; ++i){

            TP_C_OBJECT* c_object_element = symbol_table->member_c_object[i];

            if (NULL == c_object_element){

                continue;
            }

            if (TP_C_NAMESPACE_OTHER != c_object_element->member_c_namespace){

                continue;
            }

            if (NULL == c_object_element->member_token){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            if (NULL == c_object->member_token){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            if (0 == strcmp(
                c_object->member_token->member_string,
                c_object_element->member_token->member_string)){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: Duplicate OBJECT name(%1)."),
                    TP_LOG_PARAM_STRING(c_object->member_token->member_string)
                );

                return false;
            }
        }
    }

    if ((NULL == symbol_table->member_c_object) ||
        (symbol_table->member_c_object_pos == (symbol_table->member_c_object_size / sizeof(TP_C_OBJECT*)))){

        rsize_t c_object_size_allocate_unit = 
            symbol_table->member_c_object_size_allocate_unit * sizeof(TP_C_OBJECT*);

        rsize_t c_object_size =  symbol_table->member_c_object_size + c_object_size_allocate_unit;

        TP_C_OBJECT** tmp_c_object = (TP_C_OBJECT**)TP_REALLOC(
            symbol_table, symbol_table->member_c_object, c_object_size
        );

        if (NULL == tmp_c_object){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tmp_c_object) + symbol_table->member_c_object_size, 0,
            c_object_size_allocate_unit
        );

        symbol_table->member_c_object = tmp_c_object;
        symbol_table->member_c_object_size = c_object_size;
    }

    symbol_table->member_c_object[symbol_table->member_c_object_pos] = c_object;

    ++(symbol_table->member_c_object_pos);

    return true;
}

