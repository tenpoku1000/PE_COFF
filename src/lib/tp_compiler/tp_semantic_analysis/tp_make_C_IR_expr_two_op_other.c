
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(expressions: two operands).

static bool make_C_IR_relational_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
static bool make_C_IR_shift_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);

bool tp_make_C_IR_equality_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: equality-expression -> (relational-expression (== | !=))+ relational-expression
    case TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_1:{
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

        // ToDo:
        if ( ! (/* IS_TOKEN_DOUBLE_EQUAL(token) || */IS_TOKEN_EXCLAMATION_MARK_EQUAL(token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left = element[0].member_body.member_child;

        if ( ! make_C_IR_relational_expression(
            symbol_table, parse_tree_child_left, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_right = element[2].member_body.member_child;

        if ( ! make_C_IR_relational_expression(
            symbol_table, parse_tree_child_right, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        // Comparison operators
        TP_EXPR_TYPE type1 = TP_EXPR_TYPE_NULL;

        if ( ! tp_get_type(symbol_table, c_object, &type1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_C_EXPR c_expr = { 0 };

        if (TP_EXPR_TYPE_I32 == type1){

            if (IS_TOKEN_EXCLAMATION_MARK_EQUAL(token)){

                c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I32_NE; // op1 != op2
            }else{

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }else if (TP_EXPR_TYPE_I64 == type1){

            if (IS_TOKEN_EXCLAMATION_MARK_EQUAL(token)){

                c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I64_NE; // op1 != op2
            }else{

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        c_expr.member_c_expr_body.member_function_arg.member_arg_index =
            (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
        c_expr.member_c_expr_body.member_function_arg.member_c_return_type =
            (is_ignore_ABI ? NULL : c_object->member_c_return_type_attr);

        if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: equality-expression -> relational-expression
    case TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        if ( ! make_C_IR_relational_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! make_C_IR_relational_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

static bool make_C_IR_relational_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: relational-expression -> shift-expression
    case TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        if ( ! make_C_IR_shift_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! make_C_IR_shift_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

static bool make_C_IR_shift_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: shift-expression -> additive-expression
    case TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        if ( ! tp_make_C_IR_additive_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! tp_make_C_IR_additive_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

