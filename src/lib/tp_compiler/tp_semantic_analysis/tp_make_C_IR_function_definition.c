
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(function definition).

bool tp_make_C_IR_function_definition(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(External definitions: function-definition)
    symbol_table->member_var_local_index = 0;

    TP_C_OBJECT* function_definition = NULL;
    TP_C_OBJECT* return_object = NULL;

    if (c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    // Grammer: function-definition -> 
    //     declaration-specifiers declarator compound-statement
    if (TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_1 ==
        parse_tree->member_grammer){

        if (3 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        function_definition =
            (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

        if (NULL == function_definition){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        function_definition->member_type.member_type = TP_C_TYPE_TYPE_FUNCTION;
//      function_definition->member_type.member_body.member_type_function;

        for (size_t i = 0; element_num > i; ++i){

            if (TP_PARSE_TREE_TYPE_NODE != element[i].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                goto fail;
            }

            TP_PARSE_TREE* parse_tree_child =
                (TP_PARSE_TREE*)(element[i].member_body.member_child);

            switch (i){
            case 0:{ // declaration-specifiers
                switch (parse_tree_child->member_grammer){
                    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
                    //    function-specifier | alignment-specifier) declaration-specifiers
                case TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1:
                    break;
                    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
                    //    function-specifier | alignment-specifier)
                case TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_2:
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                }

                return_object = (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

                if (NULL == return_object){

                    TP_PRINT_CRT_ERROR(symbol_table);
                    goto fail;
                }
                if ( ! tp_make_C_IR_declaration_specifiers(
                    symbol_table, parse_tree_child,
                    TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE, c_object, return_object)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }
                if ( ! tp_check_C_IR_type_specifier(
                    symbol_table, TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE, return_object)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }

                TP_C_TYPE* c_type = (TP_C_TYPE*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_TYPE));

                if (NULL == c_type){

                    TP_PRINT_CRT_ERROR(symbol_table);
                    goto fail;
                }

                *c_type = return_object->member_type;
                function_definition->member_c_return_type_attr = c_type;
                function_definition->member_type.member_body.member_type_function.member_c_return_type = c_type;
                tp_free_c_object(symbol_table, &return_object);
                break;
            }
            case 1: // declarator
                switch (parse_tree_child->member_grammer){
                // Grammer: declarator -> direct-declarator
                case TP_PARSE_TREE_GRAMMER_DECLARATOR_2:
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                }
                if ( ! tp_make_C_IR_declarator(
                    symbol_table,
                    parse_tree_child, TP_GRAMMER_CONTEXT_FUNCTION_NAME, c_object, function_definition)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }
                break;
            case 2:{ // compound-statement
                switch (parse_tree_child->member_grammer){
                // Grammer: compound-statement -> { block-item-list }
                case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1:
                    break;
                // Grammer: compound-statement -> { }
                case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2:
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                }

                TP_C_TYPE_FUNCTION* type_function =
                    &(function_definition->member_type.member_body.member_type_function);

                type_function->member_function_body.member_c_return_type =
                    type_function->member_c_return_type;

                type_function->member_function_body.member_function_parameter =
                   function_definition->member_function_parameter_attr;

                type_function->member_function_body.member_function_parameter_num =
                   function_definition->member_function_parameter_num_attr;

                if ( ! tp_make_C_IR_statements(
                    symbol_table, parse_tree_child, grammer_context, function_definition)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }
                break;
            }
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
        }

        if ( ! tp_append_c_object(symbol_table, function_definition)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        return true;
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);
fail:
    tp_free_c_object(symbol_table, &function_definition);
    tp_free_c_object(symbol_table, &return_object);
    return false;
}

