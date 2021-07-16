
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(statements: jump_statement).

bool tp_make_C_IR_jump_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements: jump-statement)

    TP_C_OBJECT* jump_statement_return = NULL;

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_STATEMENTS:
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement = NULL;

    switch (c_object->member_type.member_type){
    case TP_C_TYPE_TYPE_FUNCTION:
        compound_statement =
            &(c_object->member_type.\
member_body.member_type_function.member_function_body);
        break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
        compound_statement =
            &(c_object->member_type.\
member_body.member_type_compound_statement);
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (c_object->member_type.member_type){
    case TP_C_TYPE_TYPE_FUNCTION:
//      break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:{

        jump_statement_return =
            (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

        if (NULL == jump_statement_return){

            TP_PRINT_CRT_ERROR(symbol_table);
            goto fail;
        }

        switch (parse_tree->member_grammer){
        // Grammer: jump-statement -> return expression ;
        case TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_4:{

            symbol_table->member_nesting_level_of_expression = 0;
            symbol_table->member_nesting_expression = 0;

            if (3 != parse_tree->member_element_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if ( ! IS_TOKEN_KEYWORD_RETURN(element[0].member_body.member_tp_token)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if (TP_PARSE_TREE_TYPE_TOKEN != element[2].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if ( ! IS_TOKEN_SEMICOLON(element[2].member_body.member_tp_token)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if (TP_PARSE_TREE_TYPE_NODE != element[1].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            TP_PARSE_TREE* parse_tree_child =
                (TP_PARSE_TREE*)(element[1].member_body.member_child);

            // Grammer: expression -> assignment-expression
            if (TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2 !=
                parse_tree_child->member_grammer){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            jump_statement_return->member_type.member_type =
                TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN;

            jump_statement_return->member_type.member_body.\
member_type_jump_statement_return.member_expression = parse_tree_child;

            jump_statement_return->member_type.member_body.\
member_type_jump_statement_return.member_c_return_type =
                compound_statement->member_c_return_type;

            jump_statement_return->member_c_return_type_attr =
                compound_statement->member_c_return_type;

            jump_statement_return->member_function_parameter_attr =
               compound_statement->member_function_parameter;

            jump_statement_return->member_function_parameter_num_attr =
               compound_statement->member_function_parameter_num;

            int32_t func_arg_index = TP_WASM_ARG_INDEX_VOID;
            bool is_ignore_ABI = false;
            uint32_t function_call_depth = 0;

            if ( ! tp_make_C_IR_expression(
                symbol_table, parse_tree_child, grammer_context,
                jump_statement_return, NULL,
                &func_arg_index, is_ignore_ABI, &function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
            break;
        }
       // Grammer: jump-statement -> return ;
        case TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_5:
            if (2 != parse_tree->member_element_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if ( ! IS_TOKEN_KEYWORD_RETURN(element[0].member_body.member_tp_token)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if (TP_PARSE_TREE_TYPE_TOKEN != element[1].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if ( ! IS_TOKEN_SEMICOLON(element[1].member_body.member_tp_token)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            jump_statement_return->member_type.member_type =
                TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN;
            jump_statement_return->member_type.member_body.\
member_type_jump_statement_return.member_expression = NULL;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        if ( ! tp_append_c_object_to_compound_statement(
            symbol_table, c_object, jump_statement_return)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
fail:
    tp_free_c_object(symbol_table, &jump_statement_return);
    return false;
}

