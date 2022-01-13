
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(translation_unit).

static bool make_C_IR_translation_unit_body(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);

bool tp_make_C_IR_translation_unit(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    switch (parse_tree->member_grammer){
    // Grammer: translation-unit -> external-declaration external-declaration+
    case TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_1:
//      break;
    // Grammer: translation-unit -> external-declaration
    case TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_2:{

        TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

        size_t element_num = parse_tree->member_element_num;

        for (size_t i = 0; element_num > i; ++i){

            if (TP_PARSE_TREE_TYPE_NULL == element[i].member_type){

                break;
            }

            if (TP_PARSE_TREE_TYPE_NODE == element[i].member_type){

                TP_PARSE_TREE* parse_tree_child = element[i].member_body.member_child;

                switch (grammer_context){
                case TP_GRAMMER_CONTEXT_TRANSLATION_UNIT:
    //              break;
                case TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS:
                    if ( ! make_C_IR_translation_unit_body(
                        symbol_table,
                        parse_tree_child, grammer_context, c_object)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);
                        return false;
                    }
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
            }
        }
        break;
    }
    default:
        if ( ! make_C_IR_translation_unit_body(
            symbol_table, parse_tree, grammer_context, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }

    return true;
}

static bool make_C_IR_translation_unit_body(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(External definitions)

    if (c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (parse_tree->member_grammer){
    // Grammer: translation-unit -> external-declaration external-declaration+
    case TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_1:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left = element[0].member_body.member_child;

        if ( ! tp_make_C_IR_translation_unit(
            symbol_table,
            parse_tree_child_left, TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        TP_PARSE_TREE* parse_tree_child_right = element[1].member_body.member_child;

        if ( ! tp_make_C_IR_translation_unit(
            symbol_table,
            parse_tree_child_right, TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    }
    // Grammer: translation-unit -> external-declaration
    case TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        if ( ! tp_make_C_IR_translation_unit(
            symbol_table,
            parse_tree_child, TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: external-declaration -> function-definition
    case TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_1:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        if ( ! tp_make_C_IR_function_definition(
            symbol_table,
            parse_tree_child, TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: external-declaration -> declaration
    case TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_2:
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ( ! tp_make_C_IR_external_declaration(
            symbol_table,
            parse_tree, TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

