
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Evaluate of C IR(constant expressions).

static bool eval_C_IR_const_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context,
    TP_C_TYPE_SPECIFIER type_specifier,
    TP_C_EXPR* c_expr, rsize_t c_expr_pos,
    TP_C_TYPE_DECLARATION_STATEMENT* declaration_statement
);
static bool init_c_expr_stack(TP_SYMBOL_TABLE* symbol_table);
static bool push_c_expr_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_C_EXPR* c_expr
);
static bool pop_c_expr_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_C_EXPR* c_expr
);
static bool calc_c_expr_2_operand(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_EXPR* c_expr, TP_C_EXPR* op1, TP_C_EXPR* op2
);
static bool set_const_value(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_TYPE_SPECIFIER type_specifier,
    TP_C_EXPR* c_expr,
    TP_C_TYPE_DECLARATION_STATEMENT* declaration_statement
);

bool tp_make_C_IR_eval(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_OBJECT* c_object)
{
//  if (TP_GRAMMER_CONTEXT_ARITHMETIC_CONSTANT_EXPRESSION != grammer_context){
    if (TP_GRAMMER_CONTEXT_INTEGER_CONSTANT_EXPRESSION != grammer_context){

        return true;
    }

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (false == c_object->member_is_static_data){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    // declaration;
    TP_C_TYPE* c_type_declaration = &(c_object->member_type);

    if (TP_C_TYPE_TYPE_DECLARATION_STATEMENT != c_type_declaration->member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_DECLARATION_STATEMENT* declaration_statement =
        &(c_type_declaration->member_body.member_type_declaration_statement);

    TP_C_TYPE* declaration = declaration_statement->member_declaration;

    if (NULL == declaration){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_SPECIFIER type_specifier = TP_C_TYPE_SPECIFIER_NONE;

    switch (declaration->member_type){
    case TP_C_TYPE_TYPE_BASIC:
        type_specifier =
            declaration->member_body.member_type_basic.member_type_specifier;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_C_EXPR* c_expr = declaration_statement->member_c_expr;

    if ((NULL == c_expr) || (0 == declaration_statement->member_c_expr_size)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! eval_C_IR_const_expression(
        symbol_table, grammer_context, type_specifier,
        c_expr, declaration_statement->member_c_expr_pos, declaration_statement)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool eval_C_IR_const_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context,
    TP_C_TYPE_SPECIFIER type_specifier,
    TP_C_EXPR* c_expr, rsize_t c_expr_pos,
    TP_C_TYPE_DECLARATION_STATEMENT* declaration_statement)
{
    if ( ! init_c_expr_stack(symbol_table)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    for (rsize_t i = 0; c_expr_pos > i; ++i){

        TP_C_EXPR op1 = { 0 };
        TP_C_EXPR op2 = { 0 };

        switch (c_expr[i].member_c_expr_kind){
        // Identifier
        case TP_C_EXPR_KIND_IDENTIFIER_L_VALUES:
            if (c_expr_pos != (i + 1)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            break;

        // Local
        case TP_C_EXPR_KIND_I32_SET_LOCAL:
//          break;
        case TP_C_EXPR_KIND_I64_SET_LOCAL:
            if (c_expr_pos != (i + 2)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
            break;

        // Constants
        case TP_C_EXPR_KIND_I32_CONST:
//          break;
        case TP_C_EXPR_KIND_I64_CONST:
            if ( ! push_c_expr_stack(symbol_table, &(c_expr[i]))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        case TP_C_EXPR_KIND_STRING_LITERAL:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;

        // Numeric operators(i32)
        case TP_C_EXPR_KIND_I32_ADD:
//          break;
        case TP_C_EXPR_KIND_I32_SUB:
//          break;
        case TP_C_EXPR_KIND_I32_MUL:
//          break;
        case TP_C_EXPR_KIND_I32_DIV:
//          break;
        case TP_C_EXPR_KIND_I32_XOR:
//          break;
        // Numeric operators(i64)
        case TP_C_EXPR_KIND_I64_ADD:
//          break;
        case TP_C_EXPR_KIND_I64_SUB:
//          break;
        case TP_C_EXPR_KIND_I64_MUL:
//          break;
        case TP_C_EXPR_KIND_I64_DIV:
//          break;
        case TP_C_EXPR_KIND_I64_XOR:
            if ( ! pop_c_expr_stack(symbol_table, &op2)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            if ( ! pop_c_expr_stack(symbol_table, &op1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
            if ( ! calc_c_expr_2_operand(symbol_table, &(c_expr[i]), &op1, &op2)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;

        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
    }

    TP_C_EXPR result = { 0 };

    if ( ! pop_c_expr_stack(symbol_table, &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! set_const_value(
        symbol_table, type_specifier, &result, declaration_statement)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool init_c_expr_stack(TP_SYMBOL_TABLE* symbol_table)
{
    if (symbol_table->member_c_expr_stack){

        TP_FREE(
            symbol_table, &(symbol_table->member_c_expr_stack),
            sizeof(TP_C_EXPR) * symbol_table->member_c_expr_stack_size
        );
    }

    TP_C_EXPR* tmp_c_expr = (TP_C_EXPR*)TP_CALLOC(
        symbol_table, 
        symbol_table->member_c_expr_stack_size_allocate_unit,
        sizeof(TP_C_EXPR)
    );

    if (NULL == tmp_c_expr){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    symbol_table->member_c_expr_stack = tmp_c_expr;
    symbol_table->member_c_expr_stack_pos = 0;
    symbol_table->member_c_expr_stack_size =
        symbol_table->member_c_expr_stack_size_allocate_unit;

    return true;
}

static bool push_c_expr_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_C_EXPR* c_expr)
{
    if (symbol_table->member_c_expr_stack_size ==
        (symbol_table->member_c_expr_stack_pos - 1)){

        symbol_table->member_c_expr_stack_size +=
            symbol_table->member_c_expr_stack_size_allocate_unit;

        TP_C_EXPR* tmp_c_expr = (TP_C_EXPR*)TP_REALLOC(
            symbol_table, 
            symbol_table->member_c_expr_stack,
            sizeof(TP_C_EXPR) *
                symbol_table->member_c_expr_stack_size
        );

        if (NULL == tmp_c_expr){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            tmp_c_expr + symbol_table->member_c_expr_stack_pos, 0,
            sizeof(TP_C_EXPR) *
                symbol_table->member_c_expr_stack_size_allocate_unit
        );

        symbol_table->member_c_expr_stack = tmp_c_expr;
    }

    symbol_table->member_c_expr_stack
[symbol_table->member_c_expr_stack_pos] = *c_expr;

    ++(symbol_table->member_c_expr_stack_pos);

    return true;
}

static bool pop_c_expr_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_C_EXPR* c_expr)
{
    if (0 == symbol_table->member_c_expr_stack_pos){

        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    --(symbol_table->member_c_expr_stack_pos);

    *c_expr = symbol_table->member_c_expr_stack
[symbol_table->member_c_expr_stack_pos];

    return true;
}

static bool calc_c_expr_2_operand(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_EXPR* c_expr, TP_C_EXPR* op1, TP_C_EXPR* op2)
{
    TP_C_EXPR result = { 0 };

    switch (op1->member_c_expr_kind){
    // Constants
    case TP_C_EXPR_KIND_I32_CONST:{
        if (TP_C_EXPR_KIND_I32_CONST != op2->member_c_expr_kind){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        int32_t v1 = op1->member_c_expr_body.member_i32_const.member_value;
        int32_t v2 = op2->member_c_expr_body.member_i32_const.member_value;
        result.member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
        switch (c_expr->member_c_expr_kind){
        // Numeric operators(i32)
        case TP_C_EXPR_KIND_I32_ADD:
            result.member_c_expr_body.member_i32_const.member_value = v1 + v2;
            break;
        case TP_C_EXPR_KIND_I32_SUB:
            result.member_c_expr_body.member_i32_const.member_value = v1 - v2;
            break;
        case TP_C_EXPR_KIND_I32_MUL:
            result.member_c_expr_body.member_i32_const.member_value = v1 * v2;
            break;
        case TP_C_EXPR_KIND_I32_DIV:
            result.member_c_expr_body.member_i32_const.member_value = v1 / v2;
            break;
        case TP_C_EXPR_KIND_I32_XOR:
            result.member_c_expr_body.member_i32_const.member_value = v1 ^ v2;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    case TP_C_EXPR_KIND_I64_CONST:{
        if (TP_C_EXPR_KIND_I64_CONST != op2->member_c_expr_kind){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        int64_t v1 = op1->member_c_expr_body.member_i64_const.member_value;
        int64_t v2 = op2->member_c_expr_body.member_i64_const.member_value;
        result.member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
        switch (c_expr->member_c_expr_kind){
        // Numeric operators(i64)
        case TP_C_EXPR_KIND_I64_ADD:
            result.member_c_expr_body.member_i64_const.member_value = v1 + v2;
            break;
        case TP_C_EXPR_KIND_I64_SUB:
            result.member_c_expr_body.member_i64_const.member_value = v1 - v2;
            break;
        case TP_C_EXPR_KIND_I64_MUL:
            result.member_c_expr_body.member_i64_const.member_value = v1 * v2;
            break;
        case TP_C_EXPR_KIND_I64_DIV:
            result.member_c_expr_body.member_i64_const.member_value = v1 / v2;
            break;
        case TP_C_EXPR_KIND_I64_XOR:
            result.member_c_expr_body.member_i64_const.member_value = v1 ^ v2;
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

    if ( ! push_c_expr_stack(symbol_table, &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool set_const_value(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_TYPE_SPECIFIER type_specifier,
    TP_C_EXPR* c_expr,
    TP_C_TYPE_DECLARATION_STATEMENT* declaration_statement)
{
    TP_C_TYPE_SPECIFIER type = TP_MASK_C_TYPE_SPECIFIER(type_specifier);

    bool is_unsigned = (type_specifier & TP_C_TYPE_SPECIFIER_UNSIGNED);

    switch (c_expr->member_c_expr_kind){
    // Constants
    case TP_C_EXPR_KIND_I32_CONST:{
        int32_t value = c_expr->member_c_expr_body.member_i32_const.member_value;
        int64_t value64 = 0;
        switch (type){
        case TP_C_TYPE_SPECIFIER_CHAR:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        case TP_C_TYPE_SPECIFIER_SHORT:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        case TP_C_TYPE_SPECIFIER_INT:
//          break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
            is_unsigned ? (value64 = (int64_t)(uint32_t)value) : (value = (int64_t)value);
            c_expr->member_c_expr_kind = TP_C_EXPR_KIND_I64_CONST;
            c_expr->member_c_expr_body.member_i64_const.member_value = value64;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    case TP_C_EXPR_KIND_I64_CONST:{
        int64_t value = c_expr->member_c_expr_body.member_i64_const.member_value;
        int32_t value32 = 0;
        switch (type){
        case TP_C_TYPE_SPECIFIER_CHAR:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        case TP_C_TYPE_SPECIFIER_SHORT:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        case TP_C_TYPE_SPECIFIER_INT:
//          break;
        case TP_C_TYPE_SPECIFIER_LONG1:
            is_unsigned ? (value32 = (int32_t)(uint32_t)value) : (value = (int32_t)value);
            c_expr->member_c_expr_kind = TP_C_EXPR_KIND_I32_CONST;
            c_expr->member_c_expr_body.member_i32_const.member_value = value32;
            break;
        case TP_C_TYPE_SPECIFIER_LONG2:
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

    
    TP_FREE(
        symbol_table,
        &(declaration_statement->member_c_expr),
        declaration_statement->member_c_expr_size
    );

    TP_C_EXPR* tmp_c_expr = (TP_C_EXPR*)TP_CALLOC(
        symbol_table, symbol_table->member_c_expr_size_allocate_unit, sizeof(TP_C_EXPR)
    );

    if (NULL == tmp_c_expr){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    *tmp_c_expr = *c_expr;

    declaration_statement->member_c_expr = tmp_c_expr;

    declaration_statement->member_c_expr_pos = 1;

    return true;
}

