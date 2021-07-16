
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(expressions: two operands: logical).

static bool make_C_IR_logical_AND_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);

bool tp_make_C_IR_logical_OR_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: logical-OR-expression -> logical-AND-expression
    case TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_2:{
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

        if ( ! make_C_IR_logical_AND_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! make_C_IR_logical_AND_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

static bool make_C_IR_logical_AND_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: logical-AND-expression -> inclusive-OR-expression
    case TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_2:{
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

        if ( ! tp_make_C_IR_inclusive_OR_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        if ( ! tp_make_C_IR_inclusive_OR_expression(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

