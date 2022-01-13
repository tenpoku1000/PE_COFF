
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(statements: expression_statement).

bool tp_make_C_IR_expression_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements: expression-statement)
    TP_C_OBJECT* expression_statement = NULL;

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

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (c_object->member_type.member_type){
    case TP_C_TYPE_TYPE_FUNCTION:
//      break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:{

        expression_statement =
            (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

        if (NULL == expression_statement){

            TP_PRINT_CRT_ERROR(symbol_table);
            goto fail;
        }

        switch (parse_tree->member_grammer){
            // Grammer: expression-statement -> expression ;
        case TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_1:{

            symbol_table->member_nesting_level_of_expression = 0;
            symbol_table->member_nesting_expression = 0;

            if (2 != parse_tree->member_element_num){

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
            if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

            // Grammer: expression -> assignment-expression
            if (TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2 !=
                parse_tree_child->member_grammer){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            expression_statement->member_type.member_type =
                TP_C_TYPE_TYPE_EXPRESSION_STATEMENT;
            expression_statement->member_type.member_body.
member_type_expression_statement.member_expression = parse_tree_child;

            TP_C_INHERIT_ATTR_TO_C_OBJECT_FROM_C_OBJECT(
                expression_statement, c_object
            );

            int32_t func_arg_index = TP_WASM_ARG_INDEX_VOID;
            bool is_ignore_ABI = false;
            uint32_t function_call_depth = 0;

            if ( ! tp_make_C_IR_expression(
                symbol_table, parse_tree_child, grammer_context,
                expression_statement, NULL,
                &func_arg_index, is_ignore_ABI, &function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
            break;
        }
        // Grammer: expression-statement -> ;
        case TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_2:
            if (1 != parse_tree->member_element_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            if ( ! IS_TOKEN_SEMICOLON(element[0].member_body.member_tp_token)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
            expression_statement->member_type.member_type =
                TP_C_TYPE_TYPE_NULL_STATEMENT;
            // TP_C_TYPE_NULL_STATEMENT is not defined.
            // expression_statement->member_type.member_body
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        if ( ! tp_append_c_object_to_compound_statement(
            symbol_table, c_object, expression_statement)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    return true;
fail:
    tp_free_c_object(symbol_table, &expression_statement);
    return false;
}

