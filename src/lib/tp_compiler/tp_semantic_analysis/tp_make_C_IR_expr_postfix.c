
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(expressions: postfix expression).

static bool make_C_IR_postfix_expressionTmp1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** c_type_func,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
static bool make_C_IR_argument_expression_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** c_type_func,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);

bool tp_make_C_IR_postfix_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    bool is_exec_expr_id = true;

    switch (parse_tree->member_grammer){
    // Grammer: postfix-expression -> primary-expression PostfixExpressionTmp1+
    case TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_1:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        is_exec_expr_id = false;

        TP_C_OBJECT* stack_c_object = NULL;

        if ( ! tp_make_C_IR_primary_expression(
            symbol_table, parse_tree_child_left, grammer_context,
            c_object, is_exec_expr_id, &stack_c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        if (NULL == stack_c_object){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_right =
            (TP_PARSE_TREE*)(element[1].member_body.member_child);

        switch (parse_tree_child_right->member_grammer){
        // Grammer: PostfixExpressionTmp1 -> ( argument-expression-list )
        case TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_2:
//          break;
        // Grammer: PostfixExpressionTmp1 -> ( )
        case TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_3:
            grammer_context = TP_GRAMMER_CONTEXT_FUNCTION_CALL;
            if ( ! make_C_IR_postfix_expressionTmp1(
                symbol_table, parse_tree_child_right, grammer_context,
                c_object, &stack_c_object,
                func_arg_index, is_ignore_ABI, function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            if (TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_2 ==
                parse_tree_child_right->member_grammer){

                if (TP_WASM_ARG_INDEX_VOID == (*func_arg_index)){

                    ++(*func_arg_index);
                }
            }
            break;
        default:
            break;
        }

        is_exec_expr_id = true;

        if ( ! tp_make_C_IR_primary_expression(
            symbol_table, parse_tree_child_left, grammer_context,
            c_object, is_exec_expr_id, &stack_c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: postfix-expression -> primary-expression
    case TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        if ( ! tp_make_C_IR_primary_expression(
            symbol_table, parse_tree_child, grammer_context,
            c_object, is_exec_expr_id, NULL,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! tp_make_C_IR_primary_expression(
            symbol_table, parse_tree, grammer_context,
            c_object, is_exec_expr_id, NULL,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

static bool make_C_IR_postfix_expressionTmp1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** c_type_func,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: PostfixExpressionTmp1 -> ( argument-expression-list )
    case TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_2:{
        if (3 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[2].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token_left = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_LEFT_PAREN(token_left)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token_right = element[2].member_body.member_tp_token;

        if ( ! IS_TOKEN_RIGHT_PAREN(token_right)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        int32_t func_arg_index_child = TP_WASM_ARG_INDEX_VOID;
        {
            TP_PARSE_TREE* parse_tree_child =
                (TP_PARSE_TREE*)(element[1].member_body.member_child);

            bool is_ignore_ABI = false;

            ++(*function_call_depth);

            TP_C_EXPR c_expr = { .member_c_expr_kind = TP_C_EXPR_KIND_ARGS_BEGIN };

            TP_C_FUNCTION_ARGS_BEGIN_END* function_args_begin_end =
                &(c_expr.member_c_expr_body.member_function_args_begin_end);

            function_args_begin_end->member_function_call_depth = *function_call_depth;

            if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }

            if ( ! make_C_IR_argument_expression_list(
                symbol_table, parse_tree_child, TP_GRAMMER_CONTEXT_FUNCTION_ARGS,
                c_object, c_type_func,
                &func_arg_index_child, is_ignore_ABI, function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }

            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_ARGS_END;

            if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }

            --(*function_call_depth);
        }

        if ((NULL == c_type_func) || (c_type_func && (NULL == (*c_type_func))) ||
            (NULL == func_arg_index)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_TYPE* c_type = &((*c_type_func)->member_type);

        if (TP_C_TYPE_TYPE_FUNCTION != c_type->member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        TP_C_TYPE_FUNCTION* type_function =
            &(c_type->member_body.member_type_function);

        uint32_t formal_parameter_num = type_function->member_parameter_num;

        TP_C_TYPE_FUNCTION_F_PARAM* parameter = type_function->member_parameter;

        if (formal_parameter_num && (NULL == parameter)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        TP_C_TYPE* type_parameter = parameter->member_type_parameter;

        if (formal_parameter_num && (NULL == type_parameter)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (formal_parameter_num && (TP_C_TYPE_TYPE_BASIC != type_parameter->member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        bool is_void_formal_parameter = ((1 == formal_parameter_num) &&
            (TP_C_TYPE_SPECIFIER_VOID ==
                type_parameter->member_body.member_type_basic.member_type_specifier
            ));

        bool is_void_actual_parameter = (TP_WASM_ARG_INDEX_VOID == func_arg_index_child);
        uint32_t actual_parameter_num =
            (uint32_t)(is_void_actual_parameter ? 1 : (func_arg_index_child + 1));

        if (formal_parameter_num != actual_parameter_num){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: formal_parameter_num(%1) != actual_parameter_num(%2)."),
                TP_LOG_PARAM_UINT64_VALUE(formal_parameter_num),
                TP_LOG_PARAM_UINT64_VALUE(actual_parameter_num)
            );
            return false;
        }
        if (is_void_formal_parameter != is_void_actual_parameter){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: Mismatch void type of formal_parameter and actual_parameter.")
            );
            return false;
        }
        break;
    }
    // Grammer: PostfixExpressionTmp1 -> ( )
    case TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_3:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token_left = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_LEFT_PAREN(token_left)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token_right = element[1].member_body.member_tp_token;

        if ( ! IS_TOKEN_RIGHT_PAREN(token_right)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool make_C_IR_argument_expression_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** c_type_func,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: argument-expression-list -> assignment-expression (, assignment-expression)+
    case TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_1:{
        if (3 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[1].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[2].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = element[1].member_body.member_tp_token;

        if ( ! IS_TOKEN_COMMA(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        switch (parse_tree_child_left->member_grammer){
        // Grammer: argument-expression-list -> assignment-expression (, assignment-expression)+
        case TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_1:
            if ( ! make_C_IR_argument_expression_list(
                symbol_table, parse_tree_child_left, grammer_context,
                c_object, c_type_func,
                func_arg_index, is_ignore_ABI, function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
        case TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1:
//          break;
        // Grammer: assignment-expression -> conditional-expression
        case TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_2:{

            ++(*func_arg_index);

            if (TP_C_FUNCTION_ARGS_MAXIMUM < (*func_arg_index)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            if ( ! tp_make_C_IR_assignment_expression(
                symbol_table, parse_tree_child_left, grammer_context,
                c_object, c_type_func,
                func_arg_index, is_ignore_ABI, function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        }
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        ++(*func_arg_index);

        if (TP_C_FUNCTION_ARGS_MAXIMUM < (*func_arg_index)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_right =
            (TP_PARSE_TREE*)(element[2].member_body.member_child);

        if ( ! tp_make_C_IR_assignment_expression(
            symbol_table, parse_tree_child_right, grammer_context,
            c_object, c_type_func,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: argument-expression-list -> assignment-expression
    case TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        ++(*func_arg_index);

        if (TP_C_FUNCTION_ARGS_MAXIMUM < (*func_arg_index)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        if ( ! tp_make_C_IR_assignment_expression(
            symbol_table, parse_tree_child, grammer_context,
            c_object, c_type_func,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

