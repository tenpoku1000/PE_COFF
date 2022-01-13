
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(declarations).

static bool normalize_C_IR_type_specifier_basic_common(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_TYPE_SPECIFIER* type_specifier
);
static bool make_C_IR_storage_class_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_type_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type
);
static bool make_C_IR_type_specifier_basic(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type,
    TP_C_TYPE_SPECIFIER* type_specifier
);
static bool make_C_IR_type_qualifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
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
            if ( ! make_C_IR_type_specifier(
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

bool tp_check_C_IR_type_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_OBJECT* type)
{
    switch (type->member_type.member_type){
    case TP_C_TYPE_TYPE_BASIC:{

        TP_C_TYPE_SPECIFIER* type_specifier = &(type->member_type.
            member_body.member_type_basic.member_type_specifier);

        if ( ! normalize_C_IR_type_specifier_basic_common(
            symbol_table, grammer_context, type_specifier)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE** type_declaration =
            &(type->member_type.member_body.
            member_type_declaration_statement.member_declaration);

        if (NULL == (*type_declaration)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        switch ((*type_declaration)->member_type){
        case TP_C_TYPE_TYPE_BASIC:{

            TP_C_TYPE_SPECIFIER* type_specifier =
                &((*type_declaration)->member_body.member_type_basic.member_type_specifier);

            if ( ! normalize_C_IR_type_specifier_basic_common(
                symbol_table, grammer_context, type_specifier)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        }
        default:
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

static bool normalize_C_IR_type_specifier_basic_common(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_TYPE_SPECIFIER* type_specifier)
{
    if (TP_C_TYPE_SPECIFIER_NONE == *type_specifier){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_SPECIFIER type_specifier_value = TP_MASK_C_TYPE_SPECIFIER(*type_specifier);

    bool is_signed = (TP_C_TYPE_SPECIFIER_SIGNED & type_specifier_value);
    bool is_unsigned = (TP_C_TYPE_SPECIFIER_UNSIGNED & type_specifier_value);

    if (is_signed && is_unsigned){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: type error.")
        );

        return false;
    }

    if ((TP_C_TYPE_SPECIFIER_VOID & (*type_specifier)) &&
        ((~TP_C_TYPE_SPECIFIER_VOID) & (*type_specifier))){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: type error.")
        );

        return false;
    }

    // int
    if (is_signed || is_unsigned){

        if ( ! type_specifier_value){

            return true;
        }
    }
    if (TP_C_TYPE_SPECIFIER_INT == type_specifier_value){

        return true;
    }

    // char
    if (TP_C_TYPE_SPECIFIER_CHAR == type_specifier_value){

        if ((false == is_signed) && (false == is_unsigned)){

            (*type_specifier) |= TP_C_TYPE_SPECIFIER_SIGNED;
        }

        return true;
    }

    // Other types.
    if (TP_C_TYPE_SPECIFIER_INT & type_specifier_value){

        (*type_specifier) ^= TP_C_TYPE_SPECIFIER_INT;
        type_specifier_value ^= TP_C_TYPE_SPECIFIER_INT;
    }

    switch (type_specifier_value){
    case TP_C_TYPE_SPECIFIER_SHORT: // short
//      break;
    case TP_C_TYPE_SPECIFIER_LONG1: // long
//      break;
    case TP_C_TYPE_SPECIFIER_LONG2: // long long
        return true;
    default:
        break;
    }

    if ((TP_GRAMMER_CONTEXT_FUNCTION_ARGS == grammer_context) ||
        (TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE == grammer_context)){

        if (TP_C_TYPE_SPECIFIER_VOID & (*type_specifier)){

            return true;
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: type error.")
    );

    return false;
}

static bool make_C_IR_storage_class_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);
    return false;
}

static bool make_C_IR_type_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type)
{
    TP_C_TYPE_SPECIFIER* type_specifier = NULL;

    // Grammer: type-specifier -> void | char | short | int | long |
    //     signed | unsigned
    if (TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1 !=
        (parse_tree->member_grammer)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (type->member_type){
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE** type_declaration = &(type->member_body.
            member_type_declaration_statement.member_declaration);

        if (*type_declaration){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_TYPE* tmp_type_declaration =
            (TP_C_TYPE*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_TYPE));

        if (NULL == tmp_type_declaration){

            TP_PRINT_CRT_ERROR(symbol_table);
            return false;
        }
        *type_declaration = tmp_type_declaration;
        type = *type_declaration;
        goto common;
    }
    case TP_C_TYPE_TYPE_FUNCTION:
        switch (grammer_context){
        case TP_GRAMMER_CONTEXT_FUNCTION_ARGS:
            break;
        case TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
//      break;
    default:
common:
        type->member_type = TP_C_TYPE_TYPE_BASIC;
        type_specifier = &(type->member_body.member_type_basic.member_type_specifier);
        break;
    }

    if ( ! make_C_IR_type_specifier_basic(
        symbol_table, parse_tree, grammer_context, c_object, type, type_specifier)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool make_C_IR_type_specifier_basic(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type,
    TP_C_TYPE_SPECIFIER* type_specifier)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    TP_TOKEN* token = NULL;

    switch (parse_tree->member_grammer){
    // Grammer: type-specifier -> void | char | short | int | long |
    //     signed | unsigned
    case TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1:
        if (0 == element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        for (size_t i = 0; element_num > i; ++i){

            if (TP_PARSE_TREE_TYPE_TOKEN != element[i].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            token = element[i].member_body.member_tp_token;

            if (IS_TOKEN_KEYWORD_VOID(token)){

                if (1 != element_num){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_VOID;
            }else if (IS_TOKEN_KEYWORD_SIGNED(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_SIGNED) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_UNSIGNED)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_SIGNED;
            }else if (IS_TOKEN_KEYWORD_UNSIGNED(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_SIGNED) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_UNSIGNED)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_UNSIGNED;
            }else if (IS_TOKEN_KEYWORD_CHAR(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_SHORT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_INT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG1) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG2)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_CHAR;
            }else if (IS_TOKEN_KEYWORD_SHORT(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_SHORT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG1) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG2)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_SHORT;
            }else if (IS_TOKEN_KEYWORD_INT(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_INT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR)){

                       goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_INT;
            }else if (IS_TOKEN_KEYWORD_LONG(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG2)){

                    goto fail;
                }else if ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG1){

                    (*type_specifier) ^= TP_C_TYPE_SPECIFIER_LONG1;
                    (*type_specifier) |= TP_C_TYPE_SPECIFIER_LONG2;
                }else{

                    (*type_specifier) |= TP_C_TYPE_SPECIFIER_LONG1;
                }
            }else{

                goto fail;
            }
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
fail:
    if (NULL == token){

        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }
    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: type error(%1)."),
        TP_LOG_PARAM_STRING(token->member_string)
    );
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

