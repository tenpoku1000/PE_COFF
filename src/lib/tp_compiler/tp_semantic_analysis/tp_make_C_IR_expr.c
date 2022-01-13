
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(expressions).

static bool c_arg_type_check(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_EXPR_TYPE expr_type,
    TP_C_OBJECT* c_type_func, int32_t func_arg_index
);
static bool get_type_common(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_EXPR* c_expr, TP_EXPR_TYPE* type
);

bool tp_make_C_IR_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** stack_c_object,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    // C compiler(Expressions)

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: expression -> assignment-expression
    case TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        if ( ! tp_make_C_IR_assignment_expression(
            symbol_table, parse_tree_child, grammer_context,
            c_object, stack_c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

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

bool tp_make_C_IR_assignment_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** c_type_func,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth)
{
    if ((NESTING_LEVEL_OF_EXPRESSION_MAXIMUM <=
        symbol_table->member_nesting_level_of_expression) ||
        (NESTING_LEVEL_OF_EXPRESSION_MAXIMUM <=
        symbol_table->member_nesting_expression)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ((TP_GRAMMER_CONTEXT_FUNCTION_CALL == grammer_context) &&
        ((NULL == c_type_func) || (c_type_func && (NULL == *c_type_func)))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
    case TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1:{

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

        TP_PARSE_TREE* parse_tree_child_right = element[2].member_body.member_child;

        if ( ! tp_make_C_IR_assignment_expression(
            symbol_table, parse_tree_child_right, TP_GRAMMER_CONTEXT_R_VALUES,
            c_object, c_type_func,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        // set_local
        TP_EXPR_TYPE type1 = TP_EXPR_TYPE_NULL;

        if ( ! tp_get_type(symbol_table, c_object, &type1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        switch (type1){
        case TP_EXPR_TYPE_I32:
            break;
        case TP_EXPR_TYPE_I64:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_EXPR c_expr = { 0 };

        if (TP_GRAMMER_CONTEXT_FUNCTION_CALL == grammer_context){

            c_expr.member_c_expr_kind =
                ((TP_EXPR_TYPE_I32 == type1) ? 
                    TP_C_EXPR_KIND_I32_SET_LOCAL_ARG :
                    TP_C_EXPR_KIND_I64_SET_LOCAL_ARG
                );
            c_expr.member_c_expr_body.member_function_arg.member_arg_index =
                (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));

            if ( ! c_arg_type_check(symbol_table, c_object, type1, *c_type_func, *func_arg_index)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }else{
            c_expr.member_c_expr_kind =
                ((TP_EXPR_TYPE_I32 == type1) ? 
                    TP_C_EXPR_KIND_I32_SET_LOCAL :
                    TP_C_EXPR_KIND_I64_SET_LOCAL
                );
        }

        if ( ! tp_append_c_expr(symbol_table, c_object, &c_expr)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_left = element[0].member_body.member_child;

        bool is_nesting_expression = 
            ((0 < symbol_table->member_nesting_level_of_expression) ||
            (0 < symbol_table->member_nesting_expression));

        if (false == is_nesting_expression){

            is_ignore_ABI = false;
        }

        if ( ! tp_make_C_IR_unary_expression(
            symbol_table, parse_tree_child_left, TP_GRAMMER_CONTEXT_L_VALUES,
            c_object, func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: assignment-expression -> conditional-expression
    case TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        ++(symbol_table->member_nesting_expression);

        if ( ! tp_make_C_IR_conditional_expression(
            symbol_table, parse_tree_child, grammer_context, c_object,
            func_arg_index, is_ignore_ABI, function_call_depth)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        --(symbol_table->member_nesting_expression);

        TP_C_TYPE* c_return_type = c_object->member_c_return_type_attr;

        bool is_nesting_expression = 
            ((0 < symbol_table->member_nesting_level_of_expression) ||
            (0 < symbol_table->member_nesting_expression));

        bool is_return_expr = (c_return_type && (false == is_nesting_expression));

        TP_EXPR_TYPE type1 = TP_EXPR_TYPE_NULL;

        if ( ! tp_get_type(symbol_table, c_object, &type1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_C_TYPE c_type = { .member_type = TP_C_TYPE_TYPE_BASIC };

        switch (type1){
        case TP_EXPR_TYPE_I32:
            c_type.member_body.member_type_basic.member_type_specifier
                = TP_C_TYPE_SPECIFIER_INT;
            break;
        case TP_EXPR_TYPE_I64:
            c_type.member_body.member_type_basic.member_type_specifier
                = TP_C_TYPE_SPECIFIER_LONG2;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_EXPR c_expr = { 0 };

        if (TP_GRAMMER_CONTEXT_FUNCTION_CALL == grammer_context){

            c_expr.member_c_expr_kind =
                ((TP_EXPR_TYPE_I32 == type1) ?
                    TP_C_EXPR_KIND_I32_SET_LOCAL_ARG :
                    TP_C_EXPR_KIND_I64_SET_LOCAL_ARG
                );
            c_expr.member_c_expr_body.member_function_arg.member_arg_index =
                (is_ignore_ABI ? TP_WASM_ARG_INDEX_VOID : (*func_arg_index));

            if ( ! c_arg_type_check(symbol_table, c_object, type1, *c_type_func, *func_arg_index)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
        }else if (is_return_expr){

            if ( ! tp_c_return_type_check(symbol_table, c_object, &c_type, c_return_type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }

            break;
        }else if (is_nesting_expression ||
            (TP_GRAMMER_CONTEXT_INITIALIZER == grammer_context) ||
            (TP_GRAMMER_CONTEXT_R_VALUES == grammer_context)){

            break;
        }else{

            c_expr.member_c_expr_kind = TP_C_EXPR_KIND_DROP;
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

static bool c_arg_type_check(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_EXPR_TYPE expr_type,
    TP_C_OBJECT* c_type_func, int32_t func_arg_index)
{
    TP_C_TYPE_SPECIFIER type_specifier = TP_C_TYPE_TYPE_NONE;

    switch (expr_type){
    case TP_EXPR_TYPE_I32:
        type_specifier = TP_C_TYPE_SPECIFIER_INT;
        break;
    case TP_EXPR_TYPE_I64:
        type_specifier = TP_C_TYPE_SPECIFIER_LONG2;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_C_TYPE* type = &(c_type_func->member_type);

    if (TP_C_TYPE_TYPE_FUNCTION != type->member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_FUNCTION* type_func = &(type->member_body.member_type_function);

    TP_C_TYPE_FUNCTION_F_PARAM* parameter = type_func->member_parameter;

    for (uint32_t i = 0; type_func->member_parameter_num; ++i){

        if (func_arg_index == (int32_t)i){

            TP_C_TYPE_SPECIFIER type_specifier_func = TP_C_TYPE_TYPE_NONE;

            TP_C_TYPE* type_parameter = parameter->member_type_parameter;

            switch (type_parameter->member_type){
            case TP_C_TYPE_TYPE_BASIC:
                type_specifier_func =
                    type_parameter->member_body.member_type_basic.member_type_specifier;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            if (type_specifier != type_specifier_func){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                    TP_LOG_PARAM_STRING(
                        "ERROR: The expression type of the function argument is incompatible."
                    )
                );

                return false;
            }

            return true;
        }

        parameter = parameter->member_next;
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);

    return false;
}

bool tp_c_return_type_check(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_C_TYPE* c_type, TP_C_TYPE* c_return_type)
{
    TP_C_TYPE_SPECIFIER type_specifier_return = TP_C_TYPE_TYPE_NONE;
    TP_C_TYPE_SPECIFIER type_specifier_expr = TP_C_TYPE_TYPE_NONE;

    switch (c_return_type->member_type){
    case TP_C_TYPE_TYPE_BASIC:{

        TP_C_TYPE_SPECIFIER type_specifier =
            c_return_type->member_body.member_type_basic.member_type_specifier;

        switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
        case TP_C_TYPE_SPECIFIER_INT:
//          break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            type_specifier_return = TP_C_TYPE_SPECIFIER_INT;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            type_specifier_return = TP_C_TYPE_SPECIFIER_LONG2;
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

    switch (c_type->member_type){
    case TP_C_TYPE_TYPE_BASIC:{

        TP_C_TYPE_SPECIFIER type_specifier =
            c_type->member_body.member_type_basic.member_type_specifier;

        switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
        case TP_C_TYPE_SPECIFIER_INT:
//          break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            type_specifier_expr = TP_C_TYPE_SPECIFIER_INT;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            type_specifier_expr = TP_C_TYPE_SPECIFIER_LONG2;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    case TP_C_TYPE_TYPE_FUNCTION:{

        TP_C_TYPE* c_return_type = 
            c_type->member_body.member_type_function.member_c_return_type;

        switch (c_return_type->member_type){
        case TP_C_TYPE_TYPE_BASIC:{

            TP_C_TYPE_SPECIFIER type_specifier =
                c_return_type->member_body.member_type_basic.member_type_specifier;

            switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
            case TP_C_TYPE_SPECIFIER_INT:
//              break;
            case TP_C_TYPE_SPECIFIER_LONG1:
                type_specifier_expr = TP_C_TYPE_SPECIFIER_INT;
                break;
            case TP_C_TYPE_SPECIFIER_LONG2:
                type_specifier_expr = TP_C_TYPE_SPECIFIER_LONG2;
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
        break;
    }
    case TP_C_TYPE_TYPE_FUNCTION_PARAM:{

        TP_C_TYPE* type_param =
            c_type->member_body.member_type_param.member_type_param;

        switch (type_param->member_type){
        case TP_C_TYPE_TYPE_BASIC:{

            TP_C_TYPE_SPECIFIER type_specifier =
                type_param->member_body.member_type_basic.member_type_specifier;

            switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
            case TP_C_TYPE_SPECIFIER_INT:
//              break;
            case TP_C_TYPE_SPECIFIER_LONG1:
                type_specifier_expr = TP_C_TYPE_SPECIFIER_INT;
                break;
            case TP_C_TYPE_SPECIFIER_LONG2:
                type_specifier_expr = TP_C_TYPE_SPECIFIER_LONG2;
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
        break;
    }
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE* type_declaration =
            c_type->member_body.member_type_declaration_statement.member_declaration;

        switch (type_declaration->member_type){
        case TP_C_TYPE_TYPE_BASIC:{

            TP_C_TYPE_SPECIFIER type_specifier =
                type_declaration->member_body.member_type_basic.member_type_specifier;

            switch (TP_MASK_C_TYPE_SPECIFIER(type_specifier)){
            case TP_C_TYPE_SPECIFIER_INT:
//              break;
            case TP_C_TYPE_SPECIFIER_LONG1:
                type_specifier_expr = TP_C_TYPE_SPECIFIER_INT;
                break;
            case TP_C_TYPE_SPECIFIER_LONG2:
                type_specifier_expr = TP_C_TYPE_SPECIFIER_LONG2;
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
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if (type_specifier_return != type_specifier_expr){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING(
                "ERROR: The expression type of the return statement is incompatible."
            )
        );

        return false;
    }

    return true;
}

bool tp_get_type(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object, TP_EXPR_TYPE* type)
{
    if ((NULL == c_object) || (NULL == type)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_EXPR* c_expr = NULL;
    rsize_t c_expr_pos = 0;

    TP_C_TYPE* c_type = &(c_object->member_type);
    TP_C_TYPE_BODY* c_type_body = &(c_type->member_body);

    switch (c_type->member_type){
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE_DECLARATION_STATEMENT* type_declaration_statement =
            &(c_type_body->member_type_declaration_statement);
        c_expr = type_declaration_statement->member_c_expr;
        c_expr_pos = type_declaration_statement->member_c_expr_pos;
        break;
    }
    case TP_C_TYPE_TYPE_EXPRESSION_STATEMENT:{

        TP_C_TYPE_EXPRESSION_STATEMENT* type_expression_statement =
            &(c_type_body->member_type_expression_statement);
        c_expr = type_expression_statement->member_c_expr;
        c_expr_pos = type_expression_statement->member_c_expr_pos;
        break;
    }
    case TP_C_TYPE_TYPE_ITERATION_STATEMENT_DO:{

        TP_C_TYPE_ITERATION_STATEMENT_DO* type_iteration_statement_do =
            &(c_type_body->member_type_iteration_statement_do);
        c_expr = type_iteration_statement_do->member_c_expr;
        c_expr_pos = type_iteration_statement_do->member_c_expr_pos;
        break;
    }
    case TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN:{

        TP_C_TYPE_JUMP_STATEMENT_RETURN* type_jump_statement_return =
            &(c_type_body->member_type_jump_statement_return);
        c_expr = type_jump_statement_return->member_c_expr;
        c_expr_pos = type_jump_statement_return->member_c_expr_pos;
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_EXPR_TYPE tmp_type = TP_EXPR_TYPE_NULL;

    if (0 == c_expr_pos){

        *type = tmp_type;

        return true;
    }

    if (NULL == c_expr){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_EXPR* c_expr_element = &(c_expr[c_expr_pos - 1]);

    if ( ! get_type_common(symbol_table, c_expr_element, &tmp_type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *type = tmp_type;

    return true;
}

static bool get_type_common(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_EXPR* c_expr, TP_EXPR_TYPE* type)
{
    if ((NULL == c_expr) || (NULL == type)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (c_expr->member_c_expr_kind){
    // Identifier
    case TP_C_EXPR_KIND_IDENTIFIER_L_VALUES:
//      break;
    case TP_C_EXPR_KIND_IDENTIFIER_R_VALUES:{

        TP_C_OBJECT* identifier = c_expr->member_c_expr_body.member_identifier.member_identifier;

        if (NULL == identifier){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        TP_C_TYPE_SPECIFIER c_type_specifier = TP_C_TYPE_SPECIFIER_NONE;

        switch (identifier->member_type.member_type){
        case TP_C_TYPE_TYPE_FUNCTION_PARAM:{

            TP_C_TYPE* type_param =
                identifier->member_type.member_body.
                    member_type_param.member_type_param;

            if (NULL == type_param){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            switch (type_param->member_type){
            case TP_C_TYPE_TYPE_BASIC:
                c_type_specifier =
                    type_param->member_body.member_type_basic.member_type_specifier;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            break;
        }
        case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

            TP_C_TYPE* type_declaration =
                identifier->member_type.member_body.
                    member_type_declaration_statement.member_declaration;

            if (NULL == type_declaration){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            switch (type_declaration->member_type){
            case TP_C_TYPE_TYPE_BASIC:
                c_type_specifier =
                    type_declaration->member_body.member_type_basic.member_type_specifier;
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

        if (TP_C_TYPE_SPECIFIER_UNSIGNED < c_type_specifier){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        switch (TP_MASK_C_TYPE_SPECIFIER(c_type_specifier)){
        case TP_C_TYPE_SPECIFIER_INT:
//              break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            *type = TP_EXPR_TYPE_I32;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            *type = TP_EXPR_TYPE_I64;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }

    // I32
    case TP_C_EXPR_KIND_I32_RETURN:
//      break;
    case TP_C_EXPR_KIND_I32_GET_LOCAL:
//      break;
    case TP_C_EXPR_KIND_I32_GET_LOCAL_ARG:
//      break;
    case TP_C_EXPR_KIND_I32_TEE_LOCAL:
//      break;
    case TP_C_EXPR_KIND_I32_TEE_LOCAL_ARG:
//      break;
    case TP_C_EXPR_KIND_I32_CONST:
//      break;
    case TP_C_EXPR_KIND_I32_NE:  // op1 != op2
//      break;
    case TP_C_EXPR_KIND_I32_ADD:
//      break;
    case TP_C_EXPR_KIND_I32_SUB:
//      break;
    case TP_C_EXPR_KIND_I32_MUL:
//      break;
    case TP_C_EXPR_KIND_I32_DIV:
//      break;
    case TP_C_EXPR_KIND_I32_XOR:
        *type = TP_EXPR_TYPE_I32;
        break;

    // I64
    case TP_C_EXPR_KIND_I64_RETURN:
//      break;
    case TP_C_EXPR_KIND_I64_GET_LOCAL:
//      break;
    case TP_C_EXPR_KIND_I64_GET_LOCAL_ARG:
//      break;
    case TP_C_EXPR_KIND_I64_TEE_LOCAL:
//      break;
    case TP_C_EXPR_KIND_I64_TEE_LOCAL_ARG:
//      break;
    case TP_C_EXPR_KIND_I64_CONST:
//      break;
    case TP_C_EXPR_KIND_I64_NE:  // op1 != op2
//      break;
    case TP_C_EXPR_KIND_I64_ADD:
//      break;
    case TP_C_EXPR_KIND_I64_SUB:
//      break;
    case TP_C_EXPR_KIND_I64_MUL:
//      break;
    case TP_C_EXPR_KIND_I64_DIV:
//      break;
    case TP_C_EXPR_KIND_I64_XOR:
        *type = TP_EXPR_TYPE_I64;
        break;

    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

bool tp_append_c_expr(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object, TP_C_EXPR* c_expr_param)
{
    TP_C_EXPR** c_expr = NULL;
    rsize_t* c_expr_pos = NULL;
    rsize_t* c_expr_size = NULL;

    TP_C_TYPE* c_type = &(c_object->member_type);
    TP_C_TYPE_BODY* c_type_body = &(c_type->member_body);

    switch (c_type->member_type){
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE_DECLARATION_STATEMENT* type_declaration_statement =
            &(c_type_body->member_type_declaration_statement);
        c_expr = &(type_declaration_statement->member_c_expr);
        c_expr_pos = &(type_declaration_statement->member_c_expr_pos);
        c_expr_size = &(type_declaration_statement->member_c_expr_size);
        break;
    }
    case TP_C_TYPE_TYPE_EXPRESSION_STATEMENT:{

        TP_C_TYPE_EXPRESSION_STATEMENT* type_expression_statement =
            &(c_type_body->member_type_expression_statement);
        c_expr = &(type_expression_statement->member_c_expr);
        c_expr_pos = &(type_expression_statement->member_c_expr_pos);
        c_expr_size = &(type_expression_statement->member_c_expr_size);
        break;
    }
    case TP_C_TYPE_TYPE_ITERATION_STATEMENT_DO:{

        TP_C_TYPE_ITERATION_STATEMENT_DO* type_iteration_statement_do =
            &(c_type_body->member_type_iteration_statement_do);
        c_expr = &(type_iteration_statement_do->member_c_expr);
        c_expr_pos = &(type_iteration_statement_do->member_c_expr_pos);
        c_expr_size = &(type_iteration_statement_do->member_c_expr_size);
        break;
    }
    case TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN:{

        TP_C_TYPE_JUMP_STATEMENT_RETURN* type_jump_statement_return =
            &(c_type_body->member_type_jump_statement_return);
        c_expr = &(type_jump_statement_return->member_c_expr);
        c_expr_pos = &(type_jump_statement_return->member_c_expr_pos);
        c_expr_size = &(type_jump_statement_return->member_c_expr_size);
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if (*c_expr_pos == ((*c_expr_size) / sizeof(TP_C_EXPR))){

        rsize_t c_expr_size_allocate_unit = 
            symbol_table->member_c_expr_size_allocate_unit * sizeof(TP_C_EXPR);

        rsize_t tmp_c_expr_size =  (*c_expr_size) + c_expr_size_allocate_unit;

        TP_C_EXPR* tmp_c_expr = (TP_C_EXPR*)TP_REALLOC(
            symbol_table, *c_expr, tmp_c_expr_size
        );

        if (NULL == tmp_c_expr){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tmp_c_expr) + (*c_expr_size), 0,
            c_expr_size_allocate_unit
        );

        *c_expr = tmp_c_expr;
        *c_expr_size = tmp_c_expr_size;
    }

    (*c_expr)[*c_expr_pos] = *c_expr_param;

    ++(*c_expr_pos);

    return true;
}

