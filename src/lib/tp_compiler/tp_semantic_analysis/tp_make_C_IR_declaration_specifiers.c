
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(declaration specifiers).

static bool make_C_IR_storage_class_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_type_qualifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);

bool tp_make_C_IR_declaration_specifiers(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: declaration-specifiers ->
    //     (storage-class-specifier | type-specifier | type-qualifier)
    case TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_2:
//      break;
    // Grammer: declaration-specifiers ->
    //     (storage-class-specifier | type-specifier | type-qualifier) declaration-specifiers
    case TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1:{

        if (TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1 ==
            parse_tree->member_grammer){

            if (2 != element_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
                (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }else{

            if (1 != element_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }

        TP_PARSE_TREE* parse_tree_child_left = element[0].member_body.member_child;

        switch (parse_tree_child_left->member_grammer){
        // Grammer: storage-class-specifier -> extern | static | auto | register
        case TP_PARSE_TREE_GRAMMER_STORAGE_CLASS_SPECIFIER_1:
            if ( ! make_C_IR_storage_class_specifier(
                symbol_table, parse_tree_child_left, grammer_context, c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        // Grammer: type-specifier -> void | char | short | int | long |
        //     signed | unsigned
        case TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1:
            if ( ! tp_make_C_IR_type_specifier(
                symbol_table, parse_tree_child_left, grammer_context, c_object, &(type->member_type))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        // Grammer: type-qualifier -> const | restrict | volatile
        case TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_1:
            if ( ! make_C_IR_type_qualifier(
                symbol_table, parse_tree_child_left, grammer_context, c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1 ==
            parse_tree->member_grammer){

            TP_PARSE_TREE* parse_tree_child_right = element[1].member_body.member_child;

            switch (parse_tree_child_right->member_grammer){
            // Grammer: declaration-specifiers ->
            //     (storage-class-specifier | type-specifier | type-qualifier) declaration-specifiers
            case TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1:
                break;
            // Grammer: declaration-specifiers ->
            //     (storage-class-specifier | type-specifier | type-qualifier)
            case TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_2:
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            if ( ! tp_make_C_IR_declaration_specifiers(
                symbol_table, parse_tree_child_right, grammer_context, c_object, type)){

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

static bool make_C_IR_storage_class_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);
    return false;
}

static bool make_C_IR_type_qualifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);
    return false;
}

