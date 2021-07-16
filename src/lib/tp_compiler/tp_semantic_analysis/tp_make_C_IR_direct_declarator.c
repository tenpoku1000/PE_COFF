
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(direct declarator).

static bool make_C_IR_direct_declarator_1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_DirectDeclaratorTmp1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_DirectDeclaratorTmp2(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_parameter_type_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_parameter_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type, int32_t* func_arg_index
);
static bool make_C_IR_parameter_declaration(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool append_function_formal_param(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* formal_param, TP_C_OBJECT* type
);

bool tp_make_C_IR_direct_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    // direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2*
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: direct-declarator -> DirectDeclaratorTmp1
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_2:{
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
        // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
        case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1:
            if ( ! make_C_IR_direct_declarator_1(
                symbol_table,
                parse_tree_child, grammer_context, c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        // Grammer: DirectDeclaratorTmp1 -> identifier
        case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1:
            if ( ! make_C_IR_DirectDeclaratorTmp1(
                symbol_table,
                parse_tree_child, grammer_context, c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
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

static bool make_C_IR_direct_declarator_1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
        // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_tmp1 =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        if ( ! make_C_IR_DirectDeclaratorTmp1(
            symbol_table,
            parse_tree_child_tmp1, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_tmp2 =
            (TP_PARSE_TREE*)(element[1].member_body.member_child);

        // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
        if (TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1 !=
            parse_tree_child_tmp2->member_grammer){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! make_C_IR_DirectDeclaratorTmp2(
            symbol_table,
            parse_tree_child_tmp2, grammer_context, c_object, type)){

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

static bool make_C_IR_DirectDeclaratorTmp1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
        // Grammer: DirectDeclaratorTmp1 -> identifier
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_ID(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        switch (grammer_context){
        case TP_GRAMMER_CONTEXT_FUNCTION_ARGS:{

            rsize_t parameter_num = type->member_function_parameter_num_attr;

            if (0 == parameter_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            if (TP_C_TYPE_TYPE_FUNCTION != type->member_type.member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            TP_C_TYPE_FUNCTION_F_PARAM* tmp_parameter =
                type->member_type.member_body.member_type_function.member_parameter;

            for (rsize_t i = 0; parameter_num > i; ++i){

                if (i == parameter_num - 1){

                    if (NULL == tmp_parameter){

                        TP_PUT_LOG_MSG_ICE(symbol_table);
                        return false;
                    }else{

                        tmp_parameter->member_parameter_name = token;
                        break;
                    }
                }

                if (NULL == tmp_parameter){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }else{

                    tmp_parameter = tmp_parameter->member_next;
                }
            }
            break;
        }
        case TP_GRAMMER_CONTEXT_FUNCTION_NAME:
//          break;
        default:
            type->member_c_namespace = TP_C_NAMESPACE_OTHER;
            type->member_token = token;

            if ( ! tp_register_defined_variable(
                symbol_table, grammer_context, token, TP_C_NAMESPACE_OTHER, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool make_C_IR_DirectDeclaratorTmp2(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1:{
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

        TP_TOKEN* left_parenthesis = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_LEFT_PAREN(left_parenthesis)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* right_parenthesis = element[2].member_body.member_tp_token;

        if ( ! IS_TOKEN_RIGHT_PAREN(right_parenthesis)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[1].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child =
            (TP_PARSE_TREE*)(element[1].member_body.member_child);

        // Grammer: parameter-type-list -> parameter-list
        if (TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_2 !=
            parse_tree_child->member_grammer){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! make_C_IR_parameter_type_list(
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

static bool make_C_IR_parameter_type_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: parameter-type-list -> parameter-list
    case TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_2:{
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
        // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)+
        case TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_1:
            break;
        // Grammer: parameter-list -> parameter-declaration
        case TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        int32_t func_arg_index = TP_WASM_ARG_INDEX_VOID;

        if ( ! make_C_IR_parameter_list(
            symbol_table, parse_tree_child, grammer_context,
            c_object, type, &func_arg_index)){

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

static bool make_C_IR_parameter_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type, int32_t* func_arg_index)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)+
    case TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_1:{
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
        // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)+
        case TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_1:
            if ( ! make_C_IR_parameter_list(
                symbol_table, parse_tree_child_left, grammer_context,
                c_object, type, func_arg_index)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        // Grammer: parameter-declaration -> declaration-specifiers declarator
        case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1:
//          break;
        // Grammer: parameter-declaration -> declaration-specifiers
        case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3:

            ++(*func_arg_index);

            if (TP_C_FUNCTION_ARGS_MAXIMUM < (*func_arg_index)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            if ( ! make_C_IR_parameter_declaration(
                symbol_table, parse_tree_child_left, grammer_context,
                c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_right =
            (TP_PARSE_TREE*)(element[2].member_body.member_child);

        switch (parse_tree_child_right->member_grammer){
        // Grammer: parameter-declaration -> declaration-specifiers declarator
        case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1:
            break;
        // Grammer: parameter-declaration -> declaration-specifiers
        case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        ++(*func_arg_index);

        if (TP_C_FUNCTION_ARGS_MAXIMUM < (*func_arg_index)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! make_C_IR_parameter_declaration(
            symbol_table, parse_tree_child_right, grammer_context,
            c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: parameter-list -> parameter-declaration
    case TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_2:{
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

        // Grammer: parameter-declaration -> declaration-specifiers
        switch (parse_tree_child->member_grammer){
            // Grammer: parameter-declaration -> declaration-specifiers declarator
        case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1:
            break;
            // Grammer: parameter-declaration -> declaration-specifiers
        case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        ++(*func_arg_index);

        if (TP_C_FUNCTION_ARGS_MAXIMUM < (*func_arg_index)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! make_C_IR_parameter_declaration(
            symbol_table, parse_tree_child, grammer_context,
            c_object, type)){

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

static bool make_C_IR_parameter_declaration(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    if (TP_C_TYPE_TYPE_FUNCTION != type->member_type.member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_OBJECT* formal_param =
        (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

    if (NULL == formal_param){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: parameter-declaration -> declaration-specifiers declarator
    case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        TP_PARSE_TREE* parse_tree_child_left =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        if ( ! tp_make_C_IR_declaration_specifiers(
            symbol_table, parse_tree_child_left,
            TP_GRAMMER_CONTEXT_FUNCTION_ARGS, c_object, formal_param)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        if ( ! tp_check_C_IR_type_specifier(
            symbol_table, TP_GRAMMER_CONTEXT_FUNCTION_ARGS, formal_param)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        if ( ! append_function_formal_param(symbol_table, formal_param, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }

        TP_PARSE_TREE* parse_tree_child_right =
            (TP_PARSE_TREE*)(element[1].member_body.member_child);

        if ( ! tp_make_C_IR_declarator(
            symbol_table, parse_tree_child_right,
            TP_GRAMMER_CONTEXT_FUNCTION_ARGS, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        break;
    }
    // Grammer: parameter-declaration -> declaration-specifiers
    case TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        TP_PARSE_TREE* parse_tree_child =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

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
            goto fail;
        }
        if ( ! tp_make_C_IR_declaration_specifiers(
            symbol_table, parse_tree_child,
            TP_GRAMMER_CONTEXT_FUNCTION_ARGS, c_object, formal_param)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        if ( ! tp_check_C_IR_type_specifier(
            symbol_table, TP_GRAMMER_CONTEXT_FUNCTION_ARGS, formal_param)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        if ( ! append_function_formal_param(symbol_table, formal_param, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    tp_free_c_object(symbol_table, &formal_param);
    return true;
fail:
    tp_free_c_object(symbol_table, &formal_param);
    return false;
}

static bool append_function_formal_param(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* formal_param, TP_C_OBJECT* type)
{
    if (TP_C_TYPE_TYPE_FUNCTION != type->member_type.member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_FUNCTION* type_function =
        &(type->member_type.member_body.member_type_function);

    if (TP_X64_REGISTER_ARGS < type_function->member_parameter_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE* c_type = &(formal_param->member_type);

    if (TP_C_TYPE_TYPE_BASIC != c_type->member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_SPECIFIER type_specifier =
        c_type->member_body.member_type_basic.member_type_specifier;

    if ((0 < type->member_function_parameter_num_attr) &&
        (type_specifier & TP_C_TYPE_SPECIFIER_VOID)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: type error(void).")
        );

        return false;
    }

    TP_C_TYPE_FUNCTION_F_PARAM* parameter = (TP_C_TYPE_FUNCTION_F_PARAM*)TP_CALLOC(
        symbol_table, 1, sizeof(TP_C_TYPE_FUNCTION_F_PARAM)
    );

    if (NULL == parameter){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    TP_C_TYPE* arg_type = (TP_C_TYPE*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_TYPE));

    if (NULL == arg_type){

        TP_PRINT_CRT_ERROR(symbol_table);

        TP_FREE(
            symbol_table, &parameter, sizeof(TP_C_TYPE_FUNCTION_F_PARAM)
        );

        return false;
    }

    arg_type->member_type = TP_C_TYPE_TYPE_BASIC;
    arg_type->member_body.\
member_type_basic.member_type_specifier = type_specifier;

    TP_C_DECL* c_decl = &(arg_type->member_decl);

    switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
    case TP_C_TYPE_SPECIFIER_VOID:
        break;
    case TP_C_TYPE_SPECIFIER_CHAR:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    case TP_C_TYPE_SPECIFIER_SHORT:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    case TP_C_TYPE_SPECIFIER_INT:
//      break;
    case TP_C_TYPE_SPECIFIER_LONG1:
        c_decl->member_c_decl_kind = TP_C_DECL_KIND_FUNCTION_ARGUMENT_I32;
        break;
    case TP_C_TYPE_SPECIFIER_LONG2:
        c_decl->member_c_decl_kind = TP_C_DECL_KIND_FUNCTION_ARGUMENT_I64;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    c_decl->member_c_decl_body.member_function_argument_index =
        symbol_table->member_var_local_index;

    ++(symbol_table->member_var_local_index);

    *parameter = (TP_C_TYPE_FUNCTION_F_PARAM){
        .member_parameter_name = NULL,
        .member_type_parameter = arg_type,
        .member_next = NULL
    };

    TP_C_TYPE_FUNCTION_F_PARAM* tmp_parameter = type_function->member_parameter;

    while (tmp_parameter){

        if (NULL == tmp_parameter->member_next){

            break;
        }

        tmp_parameter = tmp_parameter->member_next;
    }

    if (tmp_parameter && (NULL == tmp_parameter->member_next)){

        tmp_parameter->member_next = parameter;
    }else{

        type_function->member_parameter = parameter;
        type->member_function_parameter_attr = parameter;
    }

    ++(type_function->member_parameter_num);
    ++(type->member_function_parameter_num_attr);

    return true;
fail:
    return false;
}

