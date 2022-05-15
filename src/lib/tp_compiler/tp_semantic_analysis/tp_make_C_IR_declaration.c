
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(declaration).

static bool make_C_IR_init_declarator_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_init_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);

bool tp_make_C_IR_declaration(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Declarations: declaration)

    TP_C_OBJECT* type = NULL;
    bool is_success = false;

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: declaration -> declaration-specifiers init-declarator-list ;
    case TP_PARSE_TREE_GRAMMER_DECLARATION_1:{
        if (3 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[2].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = element[2].member_body.member_tp_token;

        if ( ! IS_TOKEN_SEMICOLON(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left = element[0].member_body.member_child;

        switch (parse_tree_child_left->member_grammer){
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

        TP_PARSE_TREE* parse_tree_child_right = element[1].member_body.member_child;

        switch (parse_tree_child_right->member_grammer){
        // Grammer: init-declarator-list -> init-declarator
        case TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS != grammer_context){

            type = (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

            if (NULL == type){

                TP_PRINT_CRT_ERROR(symbol_table);
                return false;
            }
        }else{

            type = c_object;
        }

        type->member_type.member_type = TP_C_TYPE_TYPE_DECLARATION_STATEMENT;

        if ( ! tp_make_C_IR_declaration_specifiers(
            symbol_table, parse_tree_child_left, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        if ( ! make_C_IR_init_declarator_list(
            symbol_table, parse_tree_child_right, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        break;
    }
    // Grammer: declaration -> declaration-specifiers ;
    case TP_PARSE_TREE_GRAMMER_DECLARATION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
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

        if (TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS != grammer_context){

            type = (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

            if (NULL == type){

                TP_PRINT_CRT_ERROR(symbol_table);
                return false;
            }
        }else{

            type = c_object;
        }

        type->member_type.member_type = TP_C_TYPE_TYPE_DECLARATION_STATEMENT;

        if ( ! tp_make_C_IR_declaration_specifiers(
            symbol_table, parse_tree_child, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ( ! tp_check_C_IR_type_specifier(symbol_table, grammer_context, type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    is_success = true;
fail:
    type->member_type.member_body.
        member_type_declaration_statement.member_parse_tree = parse_tree;

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS:
        if ( ! tp_append_c_object(symbol_table, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        return true;
    case TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION:
        break;
    case TP_GRAMMER_CONTEXT_STATEMENTS:
        break;
    case TP_GRAMMER_CONTEXT_DECLARATIONS:
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ( ! tp_append_c_object_to_compound_statement(
        symbol_table, c_object, type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return is_success;
}

static bool make_C_IR_init_declarator_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: init-declarator-list -> init-declarator
    case TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: init-declarator -> declarator = initializer
        case TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_1:
            break;
        // Grammer: init-declarator -> declarator
        case TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! make_C_IR_init_declarator(
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

static bool make_C_IR_init_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: init-declarator -> declarator = initializer
    case TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_1:{
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

        if ( ! IS_TOKEN_EQUAL(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left = element[0].member_body.member_child;

        if ( ! tp_make_C_IR_declarator(
            symbol_table, parse_tree_child_left, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_right = element[2].member_body.member_child;

        if ( ! tp_make_C_IR_initializer(
            symbol_table,
            parse_tree_child_right, TP_GRAMMER_CONTEXT_INITIALIZER, c_object, type, token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: init-declarator -> declarator
    case TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: declarator -> direct-declarator
        case TP_PARSE_TREE_GRAMMER_DECLARATOR_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_declarator(
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

