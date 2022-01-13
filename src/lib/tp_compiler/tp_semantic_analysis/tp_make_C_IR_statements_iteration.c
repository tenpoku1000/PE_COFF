
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(statements: iteration_statement).

static bool make_C_IR_iteration_statement_do(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    TP_C_OBJECT* iteration_statement_do
);

bool tp_make_C_IR_iteration_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements: iteration_statement)
    TP_C_OBJECT* iteration_statement = NULL;

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
            &(c_object->member_type.
member_body.member_type_function.member_function_body);
        break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
        compound_statement =
            &(c_object->member_type.
member_body.member_type_compound_statement);
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    switch (c_object->member_type.member_type){
    case TP_C_TYPE_TYPE_FUNCTION:
//      break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:{

        iteration_statement =
            (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

        if (NULL == iteration_statement){

            TP_PRINT_CRT_ERROR(symbol_table);
            goto fail;
        }

        switch (parse_tree->member_grammer){
        // Grammer: iteration-statement -> do statement while ( expression ) ;
        case TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_DO_1:
            if ( ! make_C_IR_iteration_statement_do(
                symbol_table, parse_tree, grammer_context,
                c_object, compound_statement, iteration_statement)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
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

    if ( ! tp_append_c_object_to_compound_statement(
        symbol_table, c_object, iteration_statement)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
        return false;
    }

    return true;

fail:
    if ( ! tp_append_c_object_to_compound_statement(
        symbol_table, c_object, iteration_statement)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
    }

    return false;
}

static bool make_C_IR_iteration_statement_do(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    TP_C_OBJECT* iteration_statement_do)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: iteration-statement -> do statement while ( expression ) ;
    case TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_DO_1:{

        symbol_table->member_nesting_level_of_expression = 0;
        symbol_table->member_nesting_expression = 0;

        if (7 != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            ( ! IS_TOKEN_KEYWORD_DO(element[0].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[1].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[2].member_type) ||
            ( ! IS_TOKEN_KEYWORD_WHILE(element[2].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[3].member_type) ||
            ( ! IS_TOKEN_LEFT_PAREN(element[3].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[4].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        TP_PARSE_TREE* parse_tree_child_expression = element[4].member_body.member_child;

        // Grammer: expression -> assignment-expression
        if (TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2 !=
            parse_tree_child_expression->member_grammer){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[5].member_type) ||
            ( ! IS_TOKEN_RIGHT_PAREN(element[5].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[6].member_type) ||
            ( ! IS_TOKEN_SEMICOLON(element[6].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        iteration_statement_do->member_type.member_type =
            TP_C_TYPE_TYPE_ITERATION_STATEMENT_DO;

        iteration_statement_do->member_type.member_body.
member_type_iteration_statement_do.member_expression = parse_tree_child_expression;

        TP_C_INHERIT_ATTR_TO_C_OBJECT_FROM_COMPOUND_STATEMENT(
            iteration_statement_do, compound_statement
        );

        TP_PARSE_TREE* parse_tree_child_statement = element[1].member_body.member_child;

        switch (parse_tree_child_statement->member_grammer){
        // Grammer: statement -> compound-statement
        case TP_PARSE_TREE_GRAMMER_C_STATEMENT_2:
            if ( ! tp_make_C_IR_statements(
                symbol_table, parse_tree_child_statement,
                grammer_context, iteration_statement_do)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        int32_t func_arg_index = TP_WASM_ARG_INDEX_VOID;
        bool is_ignore_ABI = true;
        uint32_t function_call_depth = 0;

        if ( ! tp_make_C_IR_expression(
            symbol_table, parse_tree_child_expression, grammer_context,
            iteration_statement_do, NULL,
            &func_arg_index, is_ignore_ABI, &function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }

        TP_EXPR_TYPE type1 = TP_EXPR_TYPE_NULL;

        if ( ! tp_get_type(symbol_table, iteration_statement_do, &type1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_C_EXPR c_expr_op2 = { 0 };
        TP_C_EXPR c_expr_operator = { 0 };

        switch (type1){
        case TP_EXPR_TYPE_I32:
            c_expr_op2.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
            c_expr_op2.member_c_expr_body.
member_i32_const.member_function_arg.member_arg_index = TP_WASM_ARG_INDEX_VOID;
            if ( ! tp_append_c_expr(symbol_table, iteration_statement_do, &c_expr_op2)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            c_expr_operator.member_c_expr_kind = TP_C_EXPR_KIND_I32_NE;  // op1 != op2
            break;
        case TP_EXPR_TYPE_I64:
            c_expr_op2.member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
            c_expr_op2.member_c_expr_body.
member_i64_const.member_function_arg.member_arg_index = TP_WASM_ARG_INDEX_VOID;
            if ( ! tp_append_c_expr(symbol_table, iteration_statement_do, &c_expr_op2)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            c_expr_operator.member_c_expr_kind = TP_C_EXPR_KIND_I64_NE;  // op1 != op2
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_append_c_expr(symbol_table, iteration_statement_do, &c_expr_operator)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    return true;
fail:
    return false;
}

