
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert C_IR to WebAssembly(C compiler: statement).

// do statement while ( expression ) ;
bool tp_make_wasm_C_iteration_statement_do(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_ITERATION_STATEMENT_DO* type_iteration_statement_do,
    TP_C_TYPE* type_return, uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter)
{
    TP_PARSE_TREE* expression = type_iteration_statement_do->member_expression;
    TP_C_EXPR* c_expr = type_iteration_statement_do->member_c_expr;
    TP_C_TYPE* statement = &(type_iteration_statement_do->member_statement->member_type);

    if ((NULL == expression) || (NULL == c_expr) || (NULL == statement)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    {
        uint8_t* section_buffer = symbol_table->member_code_section_buffer;
        size_t index = symbol_table->member_code_index;
        TP_WASM_CODE_BEGIN;

    // do{
        // loop  ;; label = @1
        TP_WASM_CODE(symbol_table, index, tp_make_loop_code(section_buffer, index, TP_WASM_BLOCK_TYPE_VOID));
        symbol_table->member_code_index = index;
    }

    switch(statement->member_type){
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
        if ( ! tp_make_wasm_C_compound_statement(
            symbol_table, entry_point_symbol,
            section, c_object,
            &(type_iteration_statement_do->member_statement->member_type.
                member_body.member_type_compound_statement),
            NULL, NULL, type_return, NULL, parameter_num, parameter, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    {
        uint8_t* section_buffer = symbol_table->member_code_section_buffer;
        size_t index = symbol_table->member_code_index;
        TP_WASM_CODE_BEGIN;

        if ( ! tp_make_wasm_C_expression(
            symbol_table, c_object, section_buffer, &index)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

    // }while (expression);
        // br_if 0 (;@1;)
        TP_WASM_CODE(symbol_table, index, tp_make_br_if_code(section_buffer, index, 0/* relative_depth */));
        // end
        TP_WASM_CODE(symbol_table, index, tp_make_end_code(section_buffer, index));
        symbol_table->member_code_index = index;
    }

    return true;
fail:
    return false;
}

// Grammer: jump-statement -> return expression ;
// Grammer: jump-statement -> return ;
bool tp_make_wasm_C_jump_statement_return(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_JUMP_STATEMENT_RETURN* type_jump_statement_return,
    TP_C_TYPE* type_return, uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter)
{
    uint8_t* section_buffer = symbol_table->member_code_section_buffer;
    size_t index = symbol_table->member_code_index;
    TP_WASM_CODE_BEGIN;

    TP_PARSE_TREE* expression = type_jump_statement_return->member_expression;

    if (type_return && (NULL == expression)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ((NULL == type_return) && expression){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! tp_make_wasm_C_expression(
        symbol_table, c_object, section_buffer, &index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // Epilogue
    uint32_t base_local_index = 0;

    if ( ! tp_get_base_local_index(symbol_table, parameter_num, parameter, &base_local_index, NULL, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    TP_WASM_CODE_EPILOGUE(
        symbol_table, index,
        tp_make_get_local_code(section_buffer, index, base_local_index)
    );
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_i64_const_code(section_buffer, index, 16));
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_i64_add_code(section_buffer, index));
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_set_global_code(section_buffer, index, 0));

    // return
    TP_WASM_CODE(symbol_table, index, tp_make_ret_code(section_buffer, index));

    symbol_table->member_code_index = index;

    symbol_table->member_is_nothing_return_statements = false;

    return true;

fail:
    return false;
}

