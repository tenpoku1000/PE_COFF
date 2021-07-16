
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(expressions: one operand).

static bool make_C_IR_change_of_sign(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);

bool tp_make_C_IR_unary_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: unary-expression -> postfix-expression
    case TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_1:{
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

        if ( ! tp_make_C_IR_postfix_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: unary-expression -> unary-operator cast-expression
    case TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_3:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = element[0].member_body.member_tp_token;

        if ( ! (IS_TOKEN_PLUS(token) || IS_TOKEN_MINUS(token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child =
            (TP_PARSE_TREE*)(element[1].member_body.member_child);

        if ( ! tp_make_C_IR_cast_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        if (IS_TOKEN_PLUS(token)){

            ;
        }else if (IS_TOKEN_MINUS(token)){

            if ( ! make_C_IR_change_of_sign(symbol_table, c_object,
                func_arg_index, is_ignore_ABI, function_call_depth)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! tp_make_C_IR_postfix_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

bool tp_make_C_IR_cast_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: cast-expression -> unary-expression
    case TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_1:{
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

        if ( ! tp_make_C_IR_unary_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! tp_make_C_IR_unary_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

static bool make_C_IR_change_of_sign(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    if (NULL == func_arg_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_EXPR_TYPE type1 = TP_EXPR_TYPE_NULL;

    if ( ! tp_get_type(symbol_table, c_object, &type1)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
        return false;
    }

    // const -1
    TP_C_EXPR c_expr_minus1 = { 0 };

    switch (type1){
    case TP_EXPR_TYPE_I32:
        c_expr_minus1.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
        c_expr_minus1.member_c_expr_body.member_i32_const.member_value = -1;
        break;
    case TP_EXPR_TYPE_I64:
        c_expr_minus1.member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
        c_expr_minus1.member_c_expr_body.member_i64_const.member_value = -1;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_minus1)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
        return false;
    }

    // xor
    TP_C_EXPR c_expr_xor = { 0 };

    switch (type1){
    case TP_EXPR_TYPE_I32:
        c_expr_xor.member_c_expr_kind = TP_C_EXPR_KIND_I32_XOR;
        break;
    case TP_EXPR_TYPE_I64:
        c_expr_xor.member_c_expr_kind = TP_C_EXPR_KIND_I64_XOR;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_xor)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
        return false;
    }

    // const 1
    TP_C_EXPR c_expr_1 = { 0 };

    switch (type1){
    case TP_EXPR_TYPE_I32:
        c_expr_1.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
        c_expr_1.member_c_expr_body.member_i32_const.member_value = 1;
        break;
    case TP_EXPR_TYPE_I64:
        c_expr_1.member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
        c_expr_1.member_c_expr_body.member_i64_const.member_value = 1;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_1)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
        return false;
    }

    // add
    TP_C_EXPR c_expr_add = { 0 };

    switch (type1){
    case TP_EXPR_TYPE_I32:
        c_expr_add.member_c_expr_kind = TP_C_EXPR_KIND_I32_ADD;
        break;
    case TP_EXPR_TYPE_I64:
        c_expr_add.member_c_expr_kind = TP_C_EXPR_KIND_I64_ADD;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }
    c_expr_add.member_c_expr_body.member_function_arg.member_arg_index =
        (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
    c_expr_add.member_c_expr_body.member_function_arg.member_c_return_type =
        (is_ignore_ABI ? NULL : c_object->member_c_return_type_attr);

    if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_add)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
        return false;
    }

    return true;
}

