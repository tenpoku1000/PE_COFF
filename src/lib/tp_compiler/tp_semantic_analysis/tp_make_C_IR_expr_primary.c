
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(expressions: primary expression).

static bool make_C_IR_primary_expression_identifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object,
    bool is_exec_expr_id, TP_C_OBJECT** stack_c_object,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
static bool get_stack_c_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_OBJECT* c_object, TP_C_OBJECT** stack_c_object
);
static bool make_C_IR_primary_expression_identifier_body(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* local_stack_c_object,
    TP_C_TYPE* c_type, TP_C_DECL* c_decl, TP_C_TYPE* c_return_type,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth, TP_C_EXPR* c_expr_id
);
static bool make_C_IR_primary_expression_identifier_body_return(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* local_stack_c_object,
    TP_C_TYPE* c_type, TP_C_DECL* c_decl, TP_C_TYPE* c_return_type,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
static bool make_C_IR_primary_expression_constant(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);

bool tp_make_C_IR_primary_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object,
    bool is_exec_expr_id, TP_C_OBJECT** stack_c_object,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: primary-expression -> identifier
    case TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_1:
        if ( ! make_C_IR_primary_expression_identifier(
            symbol_table, parse_tree, grammer_context,
            c_object, is_exec_expr_id, stack_c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    // Grammer: primary-expression -> constant
    case TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_2:
        if ( ! make_C_IR_primary_expression_constant(
            symbol_table, parse_tree, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    // Grammer: primary-expression -> ( expression )
    case TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_4:{
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

        TP_TOKEN* token_left = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_LEFT_PAREN(token_left)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token_right = element[2].member_body.member_tp_token;

        if ( ! IS_TOKEN_RIGHT_PAREN(token_right)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child =
        (TP_PARSE_TREE*)(element[1].member_body.member_child);

        ++(symbol_table->member_nesting_level_of_expression);

        int32_t func_arg_index_child = TP_WASM_ARG_INDEX_VOID;

        if ( ! tp_make_C_IR_expression(
            symbol_table, parse_tree_child, grammer_context,
            c_object, stack_c_object,
            &func_arg_index_child, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        --(symbol_table->member_nesting_level_of_expression);
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool make_C_IR_primary_expression_identifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object,
    bool is_exec_expr_id, TP_C_OBJECT** stack_c_object,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
        // Grammer: primary-expression -> identifier
    case TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_1:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = element[0].member_body.member_tp_token;

        // Identifier
        TP_C_EXPR c_expr_id = { 0 };

        TP_C_OBJECT* local_stack_c_object = NULL;

        if ((NULL == stack_c_object) || (stack_c_object && (NULL == *stack_c_object))){
            if ( ! get_stack_c_object(symbol_table, token, c_object, &local_stack_c_object)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            if (stack_c_object && (NULL == *stack_c_object)){

                *stack_c_object = local_stack_c_object;
            }
            c_expr_id.member_c_expr_body.member_identifier.member_identifier = local_stack_c_object;
        }
        else {
            c_expr_id.member_c_expr_body.member_identifier.member_identifier = *stack_c_object;
            local_stack_c_object = *stack_c_object;
        }

        if (false == is_exec_expr_id){

            return true;
        }

        if (NULL == func_arg_index){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_TYPE* c_type = &(local_stack_c_object->member_type);
        TP_C_DECL* c_decl = &(c_type->member_decl);
        TP_C_TYPE* c_return_type = c_object->member_c_return_type_attr;

        if ( ! make_C_IR_primary_expression_identifier_body(
            symbol_table, parse_tree, grammer_context,
            c_object, local_stack_c_object,
            c_type, c_decl, c_return_type,
            func_arg_index, is_ignore_ABI, function_call_depth, &c_expr_id)){

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

static bool get_stack_c_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_OBJECT* c_object, TP_C_OBJECT** stack_c_object)
{
    if ( ! IS_TOKEN_ID(token)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (NULL == stack_c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    REGISTER_OBJECT object = { 0 };

    if ( ! tp_register_undefined_variable(
        symbol_table, token, TP_C_NAMESPACE_OTHER, &object)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (UNDEFINED_REGISTER_OBJECT == object.member_register_object_type){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: use undefined symbol(%1)."),
            TP_LOG_PARAM_STRING(token->member_string)
        );

        return false;
    }

    TP_C_OBJECT* local_stack_c_object = object.member_stack_c_object;

    if (NULL == local_stack_c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    // Name
    if (strncmp(
        token->member_string,
        local_stack_c_object->member_token->member_string, token->member_string_length)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_C_NAMESPACE_OTHER != local_stack_c_object->member_c_namespace){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    *stack_c_object = local_stack_c_object;

    return true;
}

static bool make_C_IR_primary_expression_identifier_body(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* local_stack_c_object,
    TP_C_TYPE* c_type, TP_C_DECL* c_decl, TP_C_TYPE* c_return_type,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth, TP_C_EXPR* c_expr_id)
{
    c_expr_id->member_c_expr_body.member_identifier.member_function_arg.member_arg_index =
        (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_FUNCTION_ARGS:
        c_expr_id->member_c_expr_kind = TP_C_EXPR_KIND_IDENTIFIER_R_VALUES;
        break;
    case TP_GRAMMER_CONTEXT_FUNCTION_CALL:
        c_expr_id->member_c_expr_kind = TP_C_EXPR_KIND_CALL_INDIRECT_RIP;
        c_expr_id->member_c_expr_body.member_identifier.member_function_call_depth =
            *function_call_depth;
        break;
    default:{
        if (TP_C_TYPE_TYPE_FUNCTION_PARAM == c_type->member_type){

            TP_C_TYPE* c_type_func_param = c_type->member_body.member_type_param.member_type_param;

            if (NULL == c_type_func_param){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            if (TP_C_TYPE_TYPE_BASIC != c_type_func_param->member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            TP_C_DECL* c_decl_func_param = &(c_type_func_param->member_decl);

            if ((TP_C_DECL_KIND_FUNCTION_ARGUMENT_I32 == c_decl_func_param->member_c_decl_kind) ||
                (TP_C_DECL_KIND_FUNCTION_ARGUMENT_I64 == c_decl_func_param->member_c_decl_kind)){

                TP_EXPR_TYPE type1 = TP_EXPR_TYPE_I32;

                TP_C_TYPE_SPECIFIER type_specifier =
                    c_type_func_param->member_body.member_type_basic.member_type_specifier;

                switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
                case TP_C_TYPE_SPECIFIER_CHAR:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                case TP_C_TYPE_SPECIFIER_SHORT:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                case TP_C_TYPE_SPECIFIER_INT:
//                  break;
                case TP_C_TYPE_SPECIFIER_LONG1:
                    break;
                case TP_C_TYPE_SPECIFIER_LONG2:
                    type1 = TP_EXPR_TYPE_I64;
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
                {
                    TP_C_EXPR c_expr_arg = { 0 };
                    c_expr_arg.member_c_expr_body.member_function_arg.member_arg_index =
                        (int32_t)(c_decl_func_param->member_c_decl_body.member_function_argument_index);
                    c_expr_arg.member_c_expr_body.member_function_arg.member_c_return_type =
                        c_return_type;

                    if (TP_GRAMMER_CONTEXT_L_VALUES == grammer_context){
                        if ((TP_WASM_ARG_INDEX_VOID != (*func_arg_index)) || c_return_type){
                            c_expr_arg.member_c_expr_kind =
                                ((TP_EXPR_TYPE_I32 == type1) ? 
                                    TP_C_EXPR_KIND_I32_TEE_LOCAL_ARG :
                                    TP_C_EXPR_KIND_I64_TEE_LOCAL_ARG
                                );
                        }else{
                            c_expr_arg.member_c_expr_kind =
                                ((TP_EXPR_TYPE_I32 == type1) ?
                                    TP_C_EXPR_KIND_I32_SET_LOCAL_ARG :
                                    TP_C_EXPR_KIND_I64_SET_LOCAL_ARG
                                );
                        }
                        if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_arg)){

                            TP_PUT_LOG_MSG_TRACE(symbol_table);
                            return false;
                        }

                        c_expr_id->member_c_expr_kind = TP_C_EXPR_KIND_IDENTIFIER_L_VALUES;
                        c_expr_id->member_c_expr_body.member_identifier.member_function_arg.member_arg_index =
                            (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
                        c_expr_id->member_c_expr_body.member_identifier.member_function_arg.member_c_return_type =
                            c_return_type;
                    }else{
                        c_expr_arg.member_c_expr_kind =
                            ((TP_EXPR_TYPE_I32 == type1) ? 
                                TP_C_EXPR_KIND_I32_GET_LOCAL_ARG :
                                TP_C_EXPR_KIND_I64_GET_LOCAL_ARG
                            );
                        if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_arg)){

                            TP_PUT_LOG_MSG_TRACE(symbol_table);
                            return false;
                        }

                        c_expr_id->member_c_expr_kind =
                            ((TP_EXPR_TYPE_I32 == type1) ? 
                                TP_C_EXPR_KIND_I32_TEE_LOCAL :
                                TP_C_EXPR_KIND_I64_TEE_LOCAL
                            );
                    }
                }
            }else{

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }else{
            if (TP_GRAMMER_CONTEXT_L_VALUES == grammer_context){
                c_expr_id->member_c_expr_kind = TP_C_EXPR_KIND_IDENTIFIER_L_VALUES;
            }else{
                c_expr_id->member_c_expr_kind = TP_C_EXPR_KIND_IDENTIFIER_R_VALUES;
            }
            c_expr_id->member_c_expr_body.member_identifier.member_function_arg.member_arg_index =
                (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr_id->member_c_expr_body.member_identifier.member_function_arg.member_c_return_type =
                c_return_type;
        } }
        break;
    }

    if ( ! tp_append_c_expr(symbol_table, c_object, c_expr_id)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! make_C_IR_primary_expression_identifier_body_return(
        symbol_table, parse_tree, grammer_context,
        c_object, local_stack_c_object, c_type, c_decl, c_return_type,
        func_arg_index, is_ignore_ABI, function_call_depth)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool make_C_IR_primary_expression_identifier_body_return(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* local_stack_c_object,
    TP_C_TYPE* c_type, TP_C_DECL* c_decl, TP_C_TYPE* c_return_type,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    // Return value.
    bool is_nesting_expression =
        ((0 < symbol_table->member_nesting_level_of_expression) ||
            (0 < symbol_table->member_nesting_expression));

    bool is_return_expr = (c_return_type && (false == is_nesting_expression));

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_FUNCTION_CALL:{
        if (TP_C_TYPE_TYPE_FUNCTION != c_type->member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_TYPE_FUNCTION* type_function = &(c_type->member_body.member_type_function);

        // return type.
        TP_C_TYPE* c_return_type = type_function->member_c_return_type;

        if (TP_C_TYPE_TYPE_BASIC != c_return_type->member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_TYPE_SPECIFIER type_specifier =
            c_return_type->member_body.member_type_basic.member_type_specifier;

        // parameter.
        bool is_void = false;

        if (1 == type_function->member_parameter_num){

            TP_C_TYPE* parameter_type =
                type_function->member_parameter->member_type_parameter;

            if (TP_C_TYPE_TYPE_BASIC != parameter_type->member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            TP_C_TYPE_SPECIFIER parameter_type_specifier =
                parameter_type->member_body.member_type_basic.member_type_specifier;

            if (TP_C_TYPE_SPECIFIER_VOID == TP_MASK_C_TYPE_SPECIFIER(parameter_type_specifier)){

                is_void = true;
            }
        }
        if (false == is_void){

            int32_t formal_parameter_num = (int32_t)(type_function->member_parameter_num);

            if (formal_parameter_num){

                int32_t actual_parameter_num = (*func_arg_index) + 1;

                if ((TP_X64_REGISTER_ARGS < actual_parameter_num) || (0 > func_arg_index)){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
                if (actual_parameter_num > formal_parameter_num){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
            }
        }
        if (is_return_expr){

            if ( ! tp_c_return_type_check(symbol_table, c_object, c_type, c_return_type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }

        TP_C_EXPR c_expr = { 0 };

        switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
        case TP_C_TYPE_SPECIFIER_VOID:
            return true;
        case TP_C_TYPE_SPECIFIER_INT:
//              break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I32_RETURN;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I64_RETURN;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        break;
    }
    case TP_GRAMMER_CONTEXT_L_VALUES:

        if ((TP_C_DECL_KIND_FUNCTION_ARGUMENT_I32 != c_decl->member_c_decl_kind) &&
            (TP_C_DECL_KIND_FUNCTION_ARGUMENT_I64 != c_decl->member_c_decl_kind)){

            if (is_return_expr){

                if ( ! tp_c_return_type_check(symbol_table, c_object, c_type, c_return_type)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    return false;
                }
            }
            if (is_return_expr || is_nesting_expression){

                if (NULL == local_stack_c_object){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }

                TP_C_EXPR c_expr_id = {
                    .member_c_expr_kind = TP_C_EXPR_KIND_IDENTIFIER_R_VALUES,
                    .member_c_expr_body.member_identifier = local_stack_c_object
                };
                c_expr_id.member_c_expr_body.member_identifier.member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
                c_expr_id.member_c_expr_body.member_identifier.member_function_arg.member_c_return_type =
                    c_return_type;

                if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr_id)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    return false;
                }
            }
        }
        break;
    default:
        break;
    }

    return true;
}

static bool make_C_IR_primary_expression_constant(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    if (NULL == func_arg_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: primary-expression -> constant
    case TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        TP_TOKEN* token = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_CONSTANT(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        // Constants
        TP_C_EXPR c_expr = { 0 };

        switch (token->member_value_type){
        case TP_VALUE_TYPE_INT:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
            c_expr.member_c_expr_body.member_i32_const.member_value =
                (int32_t)(token->member_value.member_int_value);
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_c_return_type =
                    (is_ignore_ABI ? NULL : (c_object->member_c_return_type_attr));
            break;
        case TP_VALUE_TYPE_LONG_INT:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
            c_expr.member_c_expr_body.member_i32_const.member_value =
                (int32_t)(token->member_value.member_long_int_value);
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_c_return_type =
                    (is_ignore_ABI ? NULL : (c_object->member_c_return_type_attr));
            break;
        case TP_VALUE_TYPE_UNSIGNED_INT:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
            c_expr.member_c_expr_body.member_i32_const.member_value =
                (int32_t)(token->member_value.member_unsigned_int_value);
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_c_return_type =
                    (is_ignore_ABI ? NULL : (c_object->member_c_return_type_attr));
            break;
        case TP_VALUE_TYPE_UNSIGNED_LONG_INT:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
            c_expr.member_c_expr_body.member_i32_const.member_value =
                (int32_t)(token->member_value.member_unsigned_long_int_value);
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr.member_c_expr_body.member_i32_const.
                member_function_arg.member_c_return_type =
                    (is_ignore_ABI ? NULL : (c_object->member_c_return_type_attr));
            break;
        case TP_VALUE_TYPE_LONG_LONG_INT:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
            c_expr.member_c_expr_body.member_i64_const.member_value =
                (int64_t)(token->member_value.member_long_long_int_value);
            c_expr.member_c_expr_body.member_i64_const.
                member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr.member_c_expr_body.member_i64_const.
                member_function_arg.member_c_return_type =
                    (is_ignore_ABI ? NULL : (c_object->member_c_return_type_attr));
            break;
        case TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT:
            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
            c_expr.member_c_expr_body.member_i64_const.member_value =
                (int64_t)(token->member_value.member_unsigned_long_long_int_value);
            c_expr.member_c_expr_body.member_i64_const.
                member_function_arg.member_arg_index =
                    (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));
            c_expr.member_c_expr_body.member_i64_const.
                member_function_arg.member_c_return_type =
                    (is_ignore_ABI ? NULL : (c_object->member_c_return_type_attr));
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr)){

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

