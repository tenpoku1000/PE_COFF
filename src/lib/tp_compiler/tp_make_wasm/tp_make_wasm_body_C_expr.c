
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert expression of C_IR to WebAssembly(C compiler: expression).

#define TP_MAKE_WASM_NUMERIC_OPERATOR(symbol_table, index, c_expr, code, return_type) \
{ \
    int32_t arg_index = (c_expr)[i].member_c_expr_body.member_function_arg.member_arg_index; \
\
    if (TP_X64_REGISTER_ARGS < (arg_index + 1)){ \
\
        TP_PUT_LOG_MSG_ICE(symbol_table); \
        return false; \
    } \
\
    bool is_containing_function_call = \
        (c_expr)[i].member_c_expr_body.member_function_arg.member_is_containing_function_call; \
\
    if (TP_WASM_ARG_INDEX_VOID != arg_index){ \
\
        TP_WASM_CODE_ARG( \
            (symbol_table), (index), (code), \
            arg_index, (return_type), \
            is_containing_function_call \
        ); \
    }else{ \
\
        if ((c_expr)[i].member_c_expr_body.member_function_arg.member_c_return_type){ \
\
            TP_WASM_CODE_ARG( \
                (symbol_table), (index), (code), \
                TP_WASM_ARG_INDEX_EAX, (return_type), \
                is_containing_function_call \
            ); \
        }else{ \
\
            TP_WASM_CODE( \
                (symbol_table), (index), (code) \
            ); \
        } \
    } \
}

static bool set_containing_function_call_attr(
    TP_SYMBOL_TABLE* symbol_table, uint32_t function_call_depth,
    TP_C_EXPR* c_expr, rsize_t c_expr_begin,
    rsize_t c_expr_pos, rsize_t* c_expr_current
);
static bool get_var_local_index_base(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object,
    uint32_t* var_local_index_base
);
static bool get_var_local_index_i64(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, uint32_t* var_local_index_i64
);
static bool get_var_local_index_i32(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, uint32_t* var_local_index_i32
);
static bool make_wasm_C_expression_l_values(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_C_EXPR* c_expr, rsize_t c_expr_index,
    uint32_t var_local_index_base,
    uint32_t var_local_index_i32, uint32_t var_local_index_i64,
    uint8_t* section_buffer, size_t* body_size
);
static bool make_wasm_C_expression_r_values(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_C_EXPR* c_expr, rsize_t c_expr_index,
    uint32_t var_local_index_base,
    uint8_t* section_buffer, size_t* body_size
);

bool tp_make_wasm_C_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, uint8_t* section_buffer, size_t* code_index)
{
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
    if ((NULL == c_expr) || (0 == c_expr_pos)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    uint32_t var_local_index_base = 0;

    if ( ! get_var_local_index_base(
        symbol_table, c_object, &var_local_index_base)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint32_t var_local_index_i32 = 0;

    if ( ! get_var_local_index_i32(
        symbol_table, c_object, &var_local_index_i32)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint32_t var_local_index_i64 = 0;

    if ( ! get_var_local_index_i64(
        symbol_table, c_object, &var_local_index_i64)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    size_t index = *code_index;
    TP_WASM_CODE_BEGIN;

    c_expr_pos = (c_expr_pos ? c_expr_pos : 1);

    if ( ! set_containing_function_call_attr(
        symbol_table, 0/* function_call_depth */,
        c_expr, 0/* c_expr_begin */, c_expr_pos, NULL/* c_expr_current */)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    for (rsize_t i = 0; c_expr_pos > i; ++i){

        switch (c_expr[i].member_c_expr_kind){

        // Control flow operators

        // Call operators
        case TP_C_EXPR_KIND_CALL_INDIRECT_RIP:{

            TP_C_IDENTIFIER* c_identifier =
                &(c_expr[i].member_c_expr_body.member_identifier);

            TP_C_FUNCTION_ARG* function_arg = &(c_identifier->member_function_arg);

            TP_C_OBJECT* identifier = c_identifier->member_identifier;

            if (NULL == identifier){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            TP_TOKEN* token = identifier->member_token;

            TP_C_DECL* c_decl = &(identifier->member_type.member_decl);

            if (TP_C_DECL_KIND_GLOBAL_FUNCTION != c_decl->member_c_decl_kind){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            TP_C_DECL_BODY* c_decl_body = &(c_decl->member_c_decl_body);

            TP_WASM_CODE_CALL_RIP(
                symbol_table, index, tp_make_call_code(
                    section_buffer, index,
                    c_decl_body->member_global_function.member_function_index
                ),
                function_arg->member_arg_index,
                c_decl_body->member_global_function.member_type_return,
                c_identifier->member_function_call_depth,
                token->member_string,
                token->member_string_length,
                c_decl_body->member_global_function.member_symbol_index
            );
            break;
        }
        case TP_C_EXPR_KIND_I32_RETURN:
            break;
        case TP_C_EXPR_KIND_I64_RETURN:
            break;
        case TP_C_EXPR_KIND_ARGS_BEGIN:
            break;
        case TP_C_EXPR_KIND_ARGS_END:
            break;

        // Parametric operators
        case TP_C_EXPR_KIND_DROP:
            TP_WASM_CODE(
                symbol_table, index, tp_make_drop_code(section_buffer, index)
            );
            break;

        // Identifier
        case TP_C_EXPR_KIND_IDENTIFIER_L_VALUES:
            if ( ! make_wasm_C_expression_l_values(
                symbol_table, c_object, c_expr, i,
                var_local_index_base,
                var_local_index_i32, var_local_index_i64,
                section_buffer, &index)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
            break;
        case TP_C_EXPR_KIND_IDENTIFIER_R_VALUES:
            if ( ! make_wasm_C_expression_r_values(
                symbol_table, c_object, c_expr, i,
                var_local_index_base, section_buffer, &index)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
            break;

        // Local
        case TP_C_EXPR_KIND_I32_GET_LOCAL:
            TP_WASM_CODE(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, var_local_index_i32
                )
            );
            break;
        case TP_C_EXPR_KIND_I32_GET_LOCAL_ARG:{

            int32_t arg_index = c_expr[i].member_c_expr_body.member_function_arg.member_arg_index;

            if ((TP_X64_REGISTER_ARGS < (arg_index + 1)) || (0 > arg_index)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, arg_index
                ), arg_index, TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
            break;
        }
        case TP_C_EXPR_KIND_I64_GET_LOCAL:
            TP_WASM_CODE(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, var_local_index_i64
                )
            );
            break;
        case TP_C_EXPR_KIND_I64_GET_LOCAL_ARG:{

            int32_t arg_index = c_expr[i].member_c_expr_body.member_function_arg.member_arg_index;

            if ((TP_X64_REGISTER_ARGS < (arg_index + 1)) || (0 > arg_index)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, arg_index
                ), arg_index, TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
            break;
        }
        case TP_C_EXPR_KIND_I32_SET_LOCAL:
            TP_WASM_CODE(
                symbol_table, index, tp_make_set_local_code(
                    section_buffer, index, var_local_index_i32
                )
            );
            break;
        case TP_C_EXPR_KIND_I32_SET_LOCAL_ARG:{

            int32_t arg_index = c_expr[i].member_c_expr_body.member_function_arg.member_arg_index;

            if ((TP_X64_REGISTER_ARGS < (arg_index + 1)) || (0 > arg_index)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            TP_WASM_CODE(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, var_local_index_i32
                )
            );

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_set_local_code(
                    section_buffer, index, var_local_index_i32
                ), arg_index, TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
            break;
        }
        case TP_C_EXPR_KIND_I64_SET_LOCAL:
            TP_WASM_CODE(
                symbol_table, index, tp_make_set_local_code(
                    section_buffer, index, var_local_index_i64
                )
            );
            break;
        case TP_C_EXPR_KIND_I64_SET_LOCAL_ARG:{

            int32_t arg_index = c_expr[i].member_c_expr_body.member_function_arg.member_arg_index;

            if ((TP_X64_REGISTER_ARGS < (arg_index + 1)) || (0 > arg_index)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            TP_WASM_CODE(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, var_local_index_i64
                )
            );
            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_set_local_code(
                    section_buffer, index, var_local_index_i64
                ), arg_index, TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
            break;
        }
        case TP_C_EXPR_KIND_I32_TEE_LOCAL:
            TP_WASM_CODE(
                symbol_table, index, tp_make_tee_local_code(
                    section_buffer, index, var_local_index_i32
                )
            );
            break;
        case TP_C_EXPR_KIND_I32_TEE_LOCAL_ARG:{

            int32_t arg_index = c_expr[i].member_c_expr_body.member_function_arg.member_arg_index;

            if (TP_X64_REGISTER_ARGS < (arg_index + 1)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            TP_WASM_CODE(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, var_local_index_i32
                )
            );

            if (TP_WASM_ARG_INDEX_VOID != arg_index){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_tee_local_code(
                        section_buffer, index, var_local_index_i32
                    ), arg_index,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                    is_containing_function_call
                );
            }else{

                if (c_expr[i].member_c_expr_body.member_function_arg.member_c_return_type){

                    TP_WASM_CODE_ARG(
                        symbol_table, index, tp_make_tee_local_code(
                            section_buffer, index, var_local_index_i32
                        ), TP_WASM_ARG_INDEX_EAX,
                        TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                        is_containing_function_call
                    );
                }else{

                    TP_WASM_CODE(
                        symbol_table, index, tp_make_tee_local_code(
                            section_buffer, index, var_local_index_i32
                        )
                    );
                }
            }
            break;
        }
        case TP_C_EXPR_KIND_I64_TEE_LOCAL:
            TP_WASM_CODE(
                symbol_table, index, tp_make_tee_local_code(
                    section_buffer, index, var_local_index_i64
                )
            );
            break;
        case TP_C_EXPR_KIND_I64_TEE_LOCAL_ARG:{

            int32_t arg_index = c_expr[i].member_c_expr_body.member_function_arg.member_arg_index;

            if (TP_X64_REGISTER_ARGS < (arg_index + 1)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            TP_WASM_CODE(
                symbol_table, index, tp_make_get_local_code(
                    section_buffer, index, var_local_index_i64
                )
            );

            if (TP_WASM_ARG_INDEX_VOID != arg_index){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_tee_local_code(
                        section_buffer, index, var_local_index_i64
                    ), arg_index,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                    is_containing_function_call
                );
            }else{

                if (c_expr[i].member_c_expr_body.member_function_arg.member_c_return_type){

                    TP_WASM_CODE_ARG(
                        symbol_table, index, tp_make_tee_local_code(
                            section_buffer, index, var_local_index_i64
                        ), TP_WASM_ARG_INDEX_EAX,
                        TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                        is_containing_function_call
                    );
                }else{

                    TP_WASM_CODE(
                        symbol_table, index, tp_make_tee_local_code(
                            section_buffer, index, var_local_index_i64
                        )
                    );
                }
            }
            break;
        }

        // Global

        // Constants
        case TP_C_EXPR_KIND_I32_CONST:{

            int32_t arg_index = c_expr[i].member_c_expr_body.
                member_i32_const.member_function_arg.member_arg_index;

            if (TP_X64_REGISTER_ARGS < (arg_index + 1)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            if (TP_WASM_ARG_INDEX_VOID != arg_index){

                TP_WASM_CODE_ARG(
                    symbol_table, index,
                    tp_make_i32_const_code(
                        section_buffer, index,
                        c_expr[i].member_c_expr_body.member_i32_const.member_value
                    ),
                    arg_index, TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                    is_containing_function_call
                );
            }else{

                if ((c_expr)[i].member_c_expr_body.
                    member_i32_const.member_function_arg.member_c_return_type){

                    TP_WASM_CODE_ARG(
                        symbol_table, index,
                        tp_make_i32_const_code(
                            section_buffer, index,
                            c_expr[i].member_c_expr_body.member_i32_const.member_value
                        ),
                        TP_WASM_ARG_INDEX_EAX, TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                        is_containing_function_call
                    );
                }else{

                    TP_WASM_CODE(
                        symbol_table, index,
                        tp_make_i32_const_code(
                            section_buffer, index,
                            c_expr[i].member_c_expr_body.member_i32_const.member_value
                        )
                    );
                }
            }
            break;
        }
        case TP_C_EXPR_KIND_I64_CONST:{

            int32_t arg_index = c_expr[i].member_c_expr_body.
                member_i64_const.member_function_arg.member_arg_index;

            if (TP_X64_REGISTER_ARGS < (arg_index + 1)){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            bool is_containing_function_call =
                c_expr[i].member_c_expr_body.member_function_arg.member_is_containing_function_call;

            if (TP_WASM_ARG_INDEX_VOID != arg_index){

                TP_WASM_CODE_ARG(
                    symbol_table, index,
                    tp_make_i64_const_code(
                        section_buffer, index,
                        c_expr[i].member_c_expr_body.member_i64_const.member_value
                    ),
                    arg_index, TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                    is_containing_function_call
                );
            }else{

                if ((c_expr)[i].member_c_expr_body.
                    member_i64_const.member_function_arg.member_c_return_type){

                    TP_WASM_CODE_ARG(
                        symbol_table, index,
                        tp_make_i64_const_code(
                            section_buffer, index,
                            c_expr[i].member_c_expr_body.member_i64_const.member_value
                        ),
                        TP_WASM_ARG_INDEX_EAX, TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                        is_containing_function_call
                    );
                }else{

                    TP_WASM_CODE(
                        symbol_table, index,
                        tp_make_i64_const_code(
                            section_buffer, index,
                            c_expr[i].member_c_expr_body.member_i64_const.member_value
                        )
                    );
                }
            }
            break;
        }
        case TP_C_EXPR_KIND_STRING_LITERAL:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;

        // Comparison operators

        // Numeric operators(i32)
        case TP_C_EXPR_KIND_I32_ADD:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i32_add_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32
            );
            break;
        case TP_C_EXPR_KIND_I32_SUB:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i32_sub_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32
            );
            break;
        case TP_C_EXPR_KIND_I32_MUL:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i32_mul_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32
            );
            break;
        case TP_C_EXPR_KIND_I32_DIV:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i32_div_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32
            );
            break;
        case TP_C_EXPR_KIND_I32_XOR:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i32_xor_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32
            );
            break;

        // Numeric operators(i64)
        case TP_C_EXPR_KIND_I64_ADD:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i64_add_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
            );
            break;
        case TP_C_EXPR_KIND_I64_SUB:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i64_sub_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
            );
            break;
        case TP_C_EXPR_KIND_I64_MUL:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i64_mul_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
            );
            break;
        case TP_C_EXPR_KIND_I64_DIV:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i64_div_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
            );
            break;
        case TP_C_EXPR_KIND_I64_XOR:
            TP_MAKE_WASM_NUMERIC_OPERATOR(
                symbol_table, index, c_expr,
                tp_make_i64_xor_code(section_buffer, index),
                TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
            );
            break;

        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
    }

    *code_index = index;

    return true;

fail:
    return false;
}

static bool set_containing_function_call_attr(
    TP_SYMBOL_TABLE* symbol_table, uint32_t function_call_depth,
    TP_C_EXPR* c_expr, rsize_t c_expr_begin, rsize_t c_expr_pos,
    rsize_t* c_expr_current)
{
    bool is_containing_function_call = false;

    if (c_expr_current){

        for (rsize_t i = c_expr_begin; c_expr_pos > i; ++i){

            switch (c_expr[i].member_c_expr_kind){
            case TP_C_EXPR_KIND_CALL_INDIRECT_RIP:{

                TP_C_IDENTIFIER* identifier =
                    &(c_expr[i].member_c_expr_body.member_identifier);

                if (function_call_depth == identifier->member_function_call_depth){

                    is_containing_function_call = true;
                    goto skip_1;
                }
                break;
            }
            case TP_C_EXPR_KIND_ARGS_END:{

                TP_C_FUNCTION_ARGS_BEGIN_END* function_args_begin_end =
                    &(c_expr[i].member_c_expr_body.member_function_args_begin_end);

                if (function_call_depth ==
                    function_args_begin_end->member_function_call_depth){

                    goto skip_1;
                }
                break;
            }
            default:
                break;
            }
        }
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
skip_1:
        ;
    }

    for (rsize_t i = c_expr_begin; c_expr_pos > i; ++i){

        switch (c_expr[i].member_c_expr_kind){

        // Control flow operators

        // Call operators
        case TP_C_EXPR_KIND_CALL_INDIRECT_RIP:
        case TP_C_EXPR_KIND_I32_RETURN:
        case TP_C_EXPR_KIND_I64_RETURN:
            break;
        case TP_C_EXPR_KIND_ARGS_BEGIN:{

            TP_C_FUNCTION_ARGS_BEGIN_END* function_args_begin_end =
                &(c_expr[i].member_c_expr_body.member_function_args_begin_end);

            if ( ! set_containing_function_call_attr(
                symbol_table,
                function_args_begin_end->member_function_call_depth,
                c_expr, i + 1, c_expr_pos, &i)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        }
        case TP_C_EXPR_KIND_ARGS_END:
            if (c_expr_current){

                TP_C_FUNCTION_ARGS_BEGIN_END* function_args_begin_end =
                    &(c_expr[i].member_c_expr_body.member_function_args_begin_end);

                if (function_call_depth ==
                    function_args_begin_end->member_function_call_depth){

                    *c_expr_current = i;
                    return true;
                }
            }
            break;

        // Parametric operators
        case TP_C_EXPR_KIND_DROP:
            break;

        // Identifier
        case TP_C_EXPR_KIND_IDENTIFIER_L_VALUES:
        case TP_C_EXPR_KIND_IDENTIFIER_R_VALUES:
            if (c_expr_current && is_containing_function_call){

                TP_C_IDENTIFIER* identifier =
                    &(c_expr[i].member_c_expr_body.member_identifier);

                identifier->member_function_arg.member_is_containing_function_call = true;
            }
            break;

        // Local
        case TP_C_EXPR_KIND_I32_GET_LOCAL:
        case TP_C_EXPR_KIND_I64_GET_LOCAL:
        case TP_C_EXPR_KIND_I32_SET_LOCAL:
        case TP_C_EXPR_KIND_I64_SET_LOCAL:
        case TP_C_EXPR_KIND_I32_TEE_LOCAL:
        case TP_C_EXPR_KIND_I64_TEE_LOCAL:
            break;

        // Local(ARG)
        case TP_C_EXPR_KIND_I32_GET_LOCAL_ARG:
        case TP_C_EXPR_KIND_I64_GET_LOCAL_ARG:
        case TP_C_EXPR_KIND_I32_SET_LOCAL_ARG:
        case TP_C_EXPR_KIND_I64_SET_LOCAL_ARG:
        case TP_C_EXPR_KIND_I32_TEE_LOCAL_ARG:
        case TP_C_EXPR_KIND_I64_TEE_LOCAL_ARG:
        // Numeric operators(i32)
        case TP_C_EXPR_KIND_I32_ADD:
        case TP_C_EXPR_KIND_I32_SUB:
        case TP_C_EXPR_KIND_I32_MUL:
        case TP_C_EXPR_KIND_I32_DIV:
        case TP_C_EXPR_KIND_I32_XOR:
        // Numeric operators(i64)
        case TP_C_EXPR_KIND_I64_ADD:
        case TP_C_EXPR_KIND_I64_SUB:
        case TP_C_EXPR_KIND_I64_MUL:
        case TP_C_EXPR_KIND_I64_DIV:
        case TP_C_EXPR_KIND_I64_XOR:
            if (c_expr_current && is_containing_function_call){

                TP_C_FUNCTION_ARG* function_arg =
                    &(c_expr[i].member_c_expr_body.member_function_arg);

                function_arg->member_is_containing_function_call = true;
            }
            break;

        // Comparison operators

        // Global

        // Constants
        case TP_C_EXPR_KIND_I32_CONST:
            if (c_expr_current && is_containing_function_call){

                TP_C_I32_CONST* i32_const =
                    &(c_expr[i].member_c_expr_body.member_i32_const);

                i32_const->member_function_arg.member_is_containing_function_call = true;
            }
            break;
        case TP_C_EXPR_KIND_I64_CONST:
            if (c_expr_current && is_containing_function_call){

                TP_C_I64_CONST* i64_const =
                    &(c_expr[i].member_c_expr_body.member_i64_const);

                i64_const->member_function_arg.member_is_containing_function_call = true;
            }
            break;
        case TP_C_EXPR_KIND_STRING_LITERAL:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;

        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
    }

    return true;
}

static bool get_var_local_index_base(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object,
    uint32_t* var_local_index_base)
{
    uint32_t parameter_num = c_object->member_function_parameter_num_attr;

    if (1 == parameter_num){

        TP_C_TYPE_FUNCTION_F_PARAM* function_parameter = c_object->member_function_parameter_attr;

        if (function_parameter){

            TP_C_TYPE* type_parameter = function_parameter->member_type_parameter;

            if (type_parameter){

                if (TP_C_TYPE_TYPE_BASIC == type_parameter->member_type){

                    if (TP_C_TYPE_SPECIFIER_VOID ==
                        type_parameter->member_body.member_type_basic.member_type_specifier){

                        parameter_num = 0;
                    }
                }else{

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }
            }else{

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
    }

    *var_local_index_base = parameter_num; // base_local_index

    return true;
}

static bool get_var_local_index_i64(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object,
    uint32_t* var_local_index_i64)
{
    uint32_t var_local_index_base = 0;

    if ( ! get_var_local_index_base(
        symbol_table, c_object, &var_local_index_base)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *var_local_index_i64 = var_local_index_base + 1; // base_local_index + 1

    return true;
}

static bool get_var_local_index_i32(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object,
    uint32_t* var_local_index_i32)
{
    uint32_t var_local_index_i64 = 0;

    if ( ! get_var_local_index_i64(
        symbol_table, c_object, &var_local_index_i64)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *var_local_index_i32 = var_local_index_i64 + 1; // base_local_index + 2

    return true;
}

static bool make_wasm_C_expression_l_values(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_C_EXPR* c_expr, rsize_t c_expr_index,
    uint32_t var_local_index_base,
    uint32_t var_local_index_i32, uint32_t var_local_index_i64,
    uint8_t* section_buffer, size_t* body_size)
{
    size_t index = *body_size;
    TP_WASM_CODE_BEGIN;

    TP_C_IDENTIFIER* id =
        &(c_expr[c_expr_index].member_c_expr_body.member_identifier);

    TP_C_OBJECT* identifier = id->member_identifier;

    if (NULL == identifier){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    int32_t arg_index = id->member_function_arg.member_arg_index;

    bool is_containing_function_call =
        id->member_function_arg.member_is_containing_function_call;

    TP_C_TYPE* c_return_type =
        id->member_function_arg.member_c_return_type;

    TP_TOKEN* token = identifier->member_token; 

    TP_C_DECL_BODY* c_decl_body =
        &(identifier->member_type.member_decl.member_c_decl_body);

    switch (identifier->member_type.member_decl.member_c_decl_kind){
    case TP_C_DECL_KIND_GLOBAL_VARIABLE_I32:
        TP_WASM_CODE_REL64(
            symbol_table, index, tp_make_i64_const_code(
                section_buffer, index,
                c_decl_body->member_global_variable_i32.member_wasm_offset
            ),
            c_decl_body->member_global_variable_i32.member_coff_offset,
            token->member_string,
            token->member_string_length,
            c_decl_body->member_global_variable_i32.member_symbol_index
        );

        TP_WASM_CODE(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_i32
            )
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i32_store_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_32, 0
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i32_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32, 0
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i32_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32, 0
                    )
                );
            }
        }
        break;
    case TP_C_DECL_KIND_GLOBAL_VARIABLE_I64:
        TP_WASM_CODE_REL64(
            symbol_table, index, tp_make_i64_const_code(
                section_buffer, index,
                c_decl_body->member_global_variable_i64.member_wasm_offset
            ),
            c_decl_body->member_global_variable_i64.member_coff_offset,
            token->member_string,
            token->member_string_length,
            c_decl_body->member_global_variable_i64.member_symbol_index
        );

        TP_WASM_CODE(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_i64
            )
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i64_store_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_64, 0
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i64_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64, 0
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i64_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64, 0
                    )
                );
            }
        }
        break;
    case TP_C_DECL_KIND_LOCAL_VARIABLE_I32:

        TP_WASM_CODE_BASE_MOV_LESS(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_base
            )
        );

        TP_WASM_CODE(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_i32
            )
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i32_store_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_32,
                    c_decl_body->member_local_variable_i32.member_wasm_offset
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i32_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32,
                        c_decl_body->member_local_variable_i32.member_wasm_offset
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i32_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32,
                        c_decl_body->member_local_variable_i32.member_wasm_offset
                    )
                );
            }
        }
        break;
    case TP_C_DECL_KIND_LOCAL_VARIABLE_I64:

        TP_WASM_CODE_BASE_MOV_LESS(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_base
            )
        );

        TP_WASM_CODE(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_i64
            )
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i64_store_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_64,
                    c_decl_body->member_local_variable_i64.member_wasm_offset
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i64_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64,
                        c_decl_body->member_local_variable_i64.member_wasm_offset
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i64_store_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64,
                        c_decl_body->member_local_variable_i64.member_wasm_offset
                    )
                );
            }
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    *body_size = index;

    return true;

fail:
    return false;
}

static bool make_wasm_C_expression_r_values(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_C_EXPR* c_expr, rsize_t c_expr_index,
    uint32_t var_local_index_base,
    uint8_t* section_buffer, size_t* body_size)
{
    size_t index = *body_size;
    TP_WASM_CODE_BEGIN;

    TP_C_IDENTIFIER* id =
        &(c_expr[c_expr_index].member_c_expr_body.member_identifier);

    TP_C_OBJECT* identifier = id->member_identifier;

    if (NULL == identifier){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    int32_t arg_index = id->member_function_arg.member_arg_index;

    bool is_containing_function_call =
        id->member_function_arg.member_is_containing_function_call;

    TP_C_TYPE* c_return_type =
        id->member_function_arg.member_c_return_type;

    TP_TOKEN* token = identifier->member_token; 

    TP_C_DECL_BODY* c_decl_body =
        &(identifier->member_type.member_decl.member_c_decl_body);

    switch (identifier->member_type.member_decl.member_c_decl_kind){
    case TP_C_DECL_KIND_GLOBAL_VARIABLE_I32:

        TP_WASM_CODE_REL64(
            symbol_table, index, tp_make_i64_const_code(
                section_buffer, index,
                c_decl_body->member_global_variable_i32.member_wasm_offset
            ),
            c_decl_body->member_global_variable_i32.member_coff_offset,
            token->member_string,
            token->member_string_length,
            c_decl_body->member_global_variable_i32.member_symbol_index
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i32_load_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_32, 0
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i32_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32, 0
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i32_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32, 0
                    )
                );
            }
        }
        break;
    case TP_C_DECL_KIND_GLOBAL_VARIABLE_I64:

        TP_WASM_CODE_REL64(
            symbol_table, index, tp_make_i64_const_code(
                section_buffer, index,
                c_decl_body->member_global_variable_i64.member_wasm_offset
            ),
            c_decl_body->member_global_variable_i64.member_coff_offset,
            token->member_string,
            token->member_string_length,
            c_decl_body->member_global_variable_i64.member_symbol_index
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i64_load_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_64, 0
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i64_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64, 0
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i64_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64, 0
                    )
                );
            }
        }
        break;
    case TP_C_DECL_KIND_LOCAL_VARIABLE_I32:

        TP_WASM_CODE_BASE_MOV_LESS(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_base
            )
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i32_load_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_32,
                    c_decl_body->member_local_variable_i32.member_wasm_offset
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i32_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32,
                        c_decl_body->member_local_variable_i32.member_wasm_offset
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i32_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_32,
                        c_decl_body->member_local_variable_i32.member_wasm_offset
                    )
                );
            }
        }
        break;
    case TP_C_DECL_KIND_LOCAL_VARIABLE_I64:

        TP_WASM_CODE_BASE_MOV_LESS(
            symbol_table, index, tp_make_get_local_code(
                section_buffer, index, var_local_index_base
            )
        );

        if (TP_WASM_ARG_INDEX_VOID != arg_index){

            TP_WASM_CODE_ARG(
                symbol_table, index, tp_make_i64_load_code(
                    section_buffer, index,
                    TP_WASM_MEM_ALIGN_64,
                    c_decl_body->member_local_variable_i64.member_wasm_offset
                ), arg_index,
                TP_WASM_RETURN_VOID,
                is_containing_function_call
            );
        }else{

            if (c_return_type){

                TP_WASM_CODE_ARG(
                    symbol_table, index, tp_make_i64_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64,
                        c_decl_body->member_local_variable_i64.member_wasm_offset
                    ), TP_WASM_ARG_INDEX_EAX,
                    TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64,
                    is_containing_function_call
                );
            }else{

                TP_WASM_CODE(
                    symbol_table, index, tp_make_i64_load_code(
                        section_buffer, index,
                        TP_WASM_MEM_ALIGN_64,
                        c_decl_body->member_local_variable_i64.member_wasm_offset
                    )
                );
            }
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    *body_size = index;

    return true;

fail:
    return false;
}

