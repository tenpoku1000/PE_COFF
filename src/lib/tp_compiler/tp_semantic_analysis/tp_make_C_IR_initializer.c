
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(initializer).

bool tp_make_C_IR_initializer(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type, TP_TOKEN* token_equal)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: initializer -> assignment-expression
    case TP_PARSE_TREE_GRAMMER_INITIALIZER_1:{
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

        switch (parse_tree_child->member_grammer){
        // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
        case TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1:
            break;
        // Grammer: assignment-expression -> conditional-expression
        case TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = type->member_token;

        if ( ! IS_TOKEN_ID(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! IS_TOKEN_EQUAL(token_equal)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        // Grammer: primary-expression -> identifier
        TP_PARSE_TREE* identifier = MAKE_PARSE_SUBTREE(
            symbol_table,
            TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_1,
            TP_TREE_TOKEN(token)
        );
        if (NULL == identifier){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
        TP_PARSE_TREE* parse_tree_assignment_expression = MAKE_PARSE_SUBTREE(
            symbol_table,
            TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1,
            TP_TREE_NODE(identifier),
            TP_TREE_TOKEN(token_equal),
            TP_TREE_NODE(parse_tree_child)
        );
        if (NULL == parse_tree_assignment_expression){

            TP_FREE(symbol_table, &(identifier->member_element),
                (identifier->member_element_num + 1) * sizeof(TP_PARSE_TREE_ELEMENT)
            );
            TP_FREE(symbol_table, &identifier, sizeof(TP_PARSE_TREE));

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        int32_t func_arg_index = TP_WASM_ARG_INDEX_VOID;
        bool is_ignore_ABI = false;
        uint32_t function_call_depth = 0;

        if ( ! tp_make_C_IR_assignment_expression(
            symbol_table, parse_tree_assignment_expression,
            grammer_context, type, NULL,
            &func_arg_index, is_ignore_ABI, &function_call_depth)){

            TP_FREE(symbol_table, &(identifier->member_element),
                (identifier->member_element_num + 1) * sizeof(TP_PARSE_TREE_ELEMENT)
            );
            TP_FREE(symbol_table, &identifier, sizeof(TP_PARSE_TREE));

            TP_FREE(symbol_table, &(parse_tree_assignment_expression->member_element),
                (parse_tree_assignment_expression->member_element_num + 1) *
                    sizeof(TP_PARSE_TREE_ELEMENT)
            );
            TP_FREE(symbol_table, &parse_tree_assignment_expression, sizeof(TP_PARSE_TREE));

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_FREE(symbol_table, &(identifier->member_element),
            (identifier->member_element_num + 1) * sizeof(TP_PARSE_TREE_ELEMENT)
        );
        TP_FREE(symbol_table, &identifier, sizeof(TP_PARSE_TREE));

        TP_FREE(symbol_table, &(parse_tree_assignment_expression->member_element),
            (parse_tree_assignment_expression->member_element_num + 1) *
                sizeof(TP_PARSE_TREE_ELEMENT)
        );
        TP_FREE(symbol_table, &parse_tree_assignment_expression, sizeof(TP_PARSE_TREE));

        if (type->member_is_static_data){

            if ( ! tp_make_C_IR_eval(
                symbol_table, TP_GRAMMER_CONTEXT_INTEGER_CONSTANT_EXPRESSION, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

