
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(declarator).

bool tp_make_C_IR_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: declarator -> direct-declarator
    case TP_PARSE_TREE_GRAMMER_DECLARATOR_2:{
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
        // Grammer: direct-declarator -> DirectDeclaratorTmp1
        case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_direct_declarator(
            symbol_table, parse_tree_child, grammer_context, c_object, type)){

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

