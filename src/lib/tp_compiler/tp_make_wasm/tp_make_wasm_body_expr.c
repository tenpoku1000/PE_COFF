
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_SECTION_NUM 6

#define TP_WASM_MODULE_SECTION_TABLE_COUNT 1
#define TP_WASM_MODULE_SECTION_TABLE_ELEMENT_TYPE_ANYFUNC 0x70
#define TP_WASM_MODULE_SECTION_TABLE_FLAGS 0
#define TP_WASM_MODULE_SECTION_TABLE_INITIAL 0

#define TP_WASM_MODULE_SECTION_MEMORY_COUNT 1
#define TP_WASM_MODULE_SECTION_MEMORY_FLAGS 0
#define TP_WASM_MODULE_SECTION_MEMORY_INITIAL 1

#define TP_WASM_MODULE_SECTION_EXPORT_COUNT 2
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_1 6
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_1 "memory"
#define TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_1 0
#define TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_2 0

#define TP_WASM_MODULE_SECTION_CODE_VAR_COUNT 2
#define TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32 0x7f

static TP_WASM_MODULE_SECTION* make_section_code_origin_wasm(TP_SYMBOL_TABLE* symbol_table);

// WebAssembly headers.
static TP_WASM_MODULE_SECTION* make_section_type(TP_SYMBOL_TABLE* symbol_table);
static TP_WASM_MODULE_SECTION* make_section_function(TP_SYMBOL_TABLE* symbol_table);
static TP_WASM_MODULE_SECTION* make_section_table(TP_SYMBOL_TABLE* symbol_table);
static TP_WASM_MODULE_SECTION* make_section_memory(TP_SYMBOL_TABLE* symbol_table);
static TP_WASM_MODULE_SECTION* make_section_export(TP_SYMBOL_TABLE* symbol_table);

// Convert parse tree to WebAssembly(int_calc_compiler).
static TP_WASM_MODULE_SECTION* make_section_code(TP_SYMBOL_TABLE* symbol_table);
static bool search_function(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context, uint8_t** func_name
);
static bool get_function(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context, uint8_t** func_name
);
static bool search_parse_tree(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section, TP_GRAMMER_CONTEXT grammer_context
);
static bool make_section_code_content(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section, TP_GRAMMER_CONTEXT grammer_context
);
static bool wasm_gen_statement_1_and_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section
);
static bool wasm_gen_expression_1_and_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section
);
static bool wasm_gen_term_1_and_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section
);
static bool wasm_gen_factor_1(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section
);
static bool wasm_gen_factor_2_and_3(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section
);

bool tp_make_wasm_expr(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section, bool is_origin_wasm)
{
    TP_WASM_MODULE_SECTION** tmp_section =
        (TP_WASM_MODULE_SECTION**)TP_CALLOC(symbol_table, TP_SECTION_NUM, sizeof(TP_WASM_MODULE_SECTION*));

    if (NULL == tmp_section){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    tmp_section[0] = make_section_type(symbol_table);
    tmp_section[1] = make_section_function(symbol_table);
    tmp_section[2] = make_section_table(symbol_table);
    tmp_section[3] = make_section_memory(symbol_table);
    tmp_section[4] = make_section_export(symbol_table);

    if (is_origin_wasm){

        tmp_section[5] = make_section_code_origin_wasm(symbol_table);
    }else{

        tmp_section[5] = make_section_code(symbol_table);
    }

    *section = tmp_section;

    module->member_section_num = TP_SECTION_NUM;

    if (section[0] && section[1] && section[2] && section[3] && section[4] && section[5]){

        if (symbol_table->member_func_local_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        symbol_table->member_code_count = 1;

        TP_WASM_FUNC_LOCAL_TYPE* tmp_func_local_types =
            (TP_WASM_FUNC_LOCAL_TYPE*)TP_CALLOC(symbol_table, 1, sizeof(TP_WASM_FUNC_LOCAL_TYPE));

        if (NULL == tmp_func_local_types){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        symbol_table->member_func_local_types = tmp_func_local_types;

        return true;
    }

    return false;
}

static TP_WASM_MODULE_SECTION* make_section_code_origin_wasm(TP_SYMBOL_TABLE* symbol_table)
{
    // Example:
    // int32_t value1 = (1 + 2) * 3;
    // int32_t value2 = 2 + (3 * value1);
    // value1 = value2 + 100;
    //
    // WebAssembly:
    // (module
    //   (type (;0;) (func (result i32)))
    //   (func (;0;) (type 0) (result i32)
    //     (local i32 i32)
    //     i32.const 1
    //     i32.const 2
    //     i32.add
    //     i32.const 3
    //     i32.mul
    //     set_local 0
    //     i32.const 2
    //     i32.const 3
    //     get_local 0
    //     i32.mul
    //     i32.add
    //     set_local 1
    //     get_local 1
    //     i32.const 100
    //     i32.add
    //     tee_local 0)
    //   (table (;0;) 0 anyfunc)
    //   (memory (;0;) 1)
    //   (export "memory" (memory 0))
    //   (export "calc" (func 0)))

    uint32_t count = TP_WASM_MODULE_SECTION_CODE_COUNT_EXPR;
    uint32_t local_count = TP_WASM_MODULE_SECTION_CODE_LOCAL_COUNT_EXPR;
    uint32_t var_count = TP_WASM_MODULE_SECTION_CODE_VAR_COUNT;
    uint32_t var_type = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32;
    uint32_t body_size = tp_encode_ui32leb128(NULL, 0, local_count);
    body_size += tp_encode_ui32leb128(NULL, 0, var_count);
    body_size += tp_encode_ui32leb128(NULL, 0, var_type);
    body_size += tp_make_i32_const_code(NULL, 0, 1);
    body_size += tp_make_i32_const_code(NULL, 0, 2);
    body_size += tp_make_i32_add_code(NULL, 0);
    body_size += tp_make_i32_const_code(NULL, 0, 3);
    body_size += tp_make_i32_mul_code(NULL, 0);
    body_size += tp_make_set_local_code(NULL, 0, 0);
    body_size += tp_make_i32_const_code(NULL, 0, 2);
    body_size += tp_make_i32_const_code(NULL, 0, 3);
    body_size += tp_make_get_local_code(NULL, 0, 0);
    body_size += tp_make_i32_mul_code(NULL, 0);
    body_size += tp_make_i32_add_code(NULL, 0);
    body_size += tp_make_set_local_code(NULL, 0, 1);
    body_size += tp_make_get_local_code(NULL, 0, 1);
    body_size += tp_make_i32_const_code(NULL, 0, 100);
    body_size += tp_make_i32_add_code(NULL, 0);
    body_size += tp_make_tee_local_code(NULL, 0, 0);
    body_size += tp_make_end_code(NULL, 0);

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, body_size);
    payload_len += body_size;

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_CODE, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_CODE);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, body_size);
    index += tp_encode_ui32leb128(section_buffer, index, local_count);
    index += tp_encode_ui32leb128(section_buffer, index, var_count);
    index += tp_encode_ui32leb128(section_buffer, index, var_type);
    index += tp_make_i32_const_code(section_buffer, index, 1);
    index += tp_make_i32_const_code(section_buffer, index, 2);
    index += tp_make_i32_add_code(section_buffer, index);
    index += tp_make_i32_const_code(section_buffer, index, 3);
    index += tp_make_i32_mul_code(section_buffer, index);
    index += tp_make_set_local_code(section_buffer, index, 0);
    index += tp_make_i32_const_code(section_buffer, index, 2);
    index += tp_make_i32_const_code(section_buffer, index, 3);
    index += tp_make_get_local_code(section_buffer, index, 0);
    index += tp_make_i32_mul_code(section_buffer, index);
    index += tp_make_i32_add_code(section_buffer, index);
    index += tp_make_set_local_code(section_buffer, index, 1);
    index += tp_make_get_local_code(section_buffer, index, 1);
    index += tp_make_i32_const_code(section_buffer, index, 100);
    index += tp_make_i32_add_code(section_buffer, index);
    index += tp_make_tee_local_code(section_buffer, index, 0);
    (void)tp_make_end_code(section_buffer, index);

    return section;
}

// WebAssembly headers.

static TP_WASM_MODULE_SECTION* make_section_type(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_TYPE_COUNT_EXPR;
    uint32_t form = TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC;
    uint32_t param_count = TP_WASM_MODULE_SECTION_TYPE_PARAM_COUNT_EXPR;
//  uint32_t param_types;
    uint32_t return_count = TP_WASM_MODULE_SECTION_TYPE_RETURN_COUNT;
    uint32_t return_type = TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32;

    if (symbol_table->member_wasm_types){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return NULL;
    }

    TP_WASM_SECTION_TYPE_VAR* wasm_types =
        (TP_WASM_SECTION_TYPE_VAR*)TP_CALLOC(symbol_table, 1, sizeof(TP_WASM_SECTION_TYPE_VAR));

    if (NULL == wasm_types){

        TP_PRINT_CRT_ERROR(symbol_table);

        return NULL;
    }

    symbol_table->member_wasm_types = wasm_types;
    symbol_table->member_wasm_type_count = 1;

    symbol_table->member_wasm_types[0] =
        (TP_WASM_SECTION_TYPE_VAR){
            .member_form = form,
            .member_param_count = param_count,
//          .member_param_types = param_types,
            .member_return_count = return_count,
            .member_return_type = return_type
        };

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, form);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_count);
//  payload_len += tp_encode_si64leb128(NULL, 0, param_types);
    payload_len += tp_encode_ui32leb128(NULL, 0, return_count);
    payload_len += tp_encode_ui32leb128(NULL, 0, return_type);

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_TYPE, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_TYPE);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, form);
    index += tp_encode_ui32leb128(section_buffer, index, param_count);
//  index += tp_encode_si64leb128(section_buffer, index, param_types);
    index += tp_encode_ui32leb128(section_buffer, index, return_count);
    (void)tp_encode_ui32leb128(section_buffer, index, return_type);

    return section;
}

static TP_WASM_MODULE_SECTION* make_section_function(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_FUNCTION_COUNT_EXPR;
    uint32_t types = TP_WASM_MODULE_SECTION_FUNCTION_TYPES_EXPR;

    if (symbol_table->member_wasm_functions){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return NULL;
    }

    uint32_t* wasm_functions = (uint32_t*)TP_CALLOC(symbol_table, 1, sizeof(uint32_t));

    if (NULL == wasm_functions){

        TP_PRINT_CRT_ERROR(symbol_table);

        return NULL;
    }

    symbol_table->member_wasm_functions = wasm_functions;
    symbol_table->member_wasm_function_count = 1;
    symbol_table->member_wasm_functions[0] = 0;

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, types);

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_FUNCTION, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_FUNCTION);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    (void)tp_encode_ui32leb128(section_buffer, index, types);

    return section;
}

static TP_WASM_MODULE_SECTION* make_section_table(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_TABLE_COUNT;
    uint32_t element_type = TP_WASM_MODULE_SECTION_TABLE_ELEMENT_TYPE_ANYFUNC;
    uint32_t flags = TP_WASM_MODULE_SECTION_TABLE_FLAGS;
    uint32_t initial = TP_WASM_MODULE_SECTION_TABLE_INITIAL;
//  uint32_t maximum;

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, element_type);
    payload_len += tp_encode_ui32leb128(NULL, 0, flags);
    payload_len += tp_encode_ui32leb128(NULL, 0, initial);
//  payload_len += tp_encode_ui32leb128(NULL, 0, maximum);

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_TABLE, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_TABLE);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, element_type);
    index += tp_encode_ui32leb128(section_buffer, index, flags);
    (void)tp_encode_ui32leb128(section_buffer, index, initial);
//  (void)tp_encode_ui32leb128(section_buffer, index, maximum);

    return section;
}

static TP_WASM_MODULE_SECTION* make_section_memory(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_MEMORY_COUNT;
    uint32_t flags = TP_WASM_MODULE_SECTION_MEMORY_FLAGS;
    uint32_t initial = TP_WASM_MODULE_SECTION_MEMORY_INITIAL;
//  uint32_t maximum;

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, flags);
    payload_len += tp_encode_ui32leb128(NULL, 0, initial);
//  payload_len += tp_encode_ui32leb128(NULL, 0, maximum);

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_MEMORY, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_MEMORY);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, flags);
    (void)tp_encode_ui32leb128(section_buffer, index, initial);
//  (void)tp_encode_ui32leb128(section_buffer, index, maximum);

    return section;
}

static TP_WASM_MODULE_SECTION* make_section_export(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_EXPORT_COUNT;

    uint32_t name_length_1 = TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_1;
    uint8_t* name_1 = TP_WASM_MODULE_SECTION_EXPORT_NAME_1;
    uint8_t kind_1 = TP_WASM_SECTION_KIND_MEMORY;
    uint32_t item_index_1 = TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_1;

    uint32_t name_length_2 = TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_2_EXPR;
    uint8_t* name_2 = TP_WASM_MODULE_SECTION_EXPORT_NAME_2_EXPR;
    uint8_t kind_2 = TP_WASM_SECTION_KIND_FUNCTION;
    uint32_t item_index_2 = TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_2;

    if (symbol_table->member_wasm_exports){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return NULL;
    }

    TP_WASM_SECTION_EXPORT_VAR* wasm_exports =
        (TP_WASM_SECTION_EXPORT_VAR*)TP_CALLOC(
            symbol_table, 2, sizeof(TP_WASM_SECTION_EXPORT_VAR)
        );

    if (NULL == wasm_exports){

        TP_PRINT_CRT_ERROR(symbol_table);

        return NULL;
    }

    symbol_table->member_wasm_exports = wasm_exports;
    symbol_table->member_wasm_export_count = 2;

    symbol_table->member_wasm_exports[0] =
        (TP_WASM_SECTION_EXPORT_VAR){
            .member_name_length = name_length_1,
            .member_name = name_1,
            .member_kind = kind_1,
            .member_item_index = item_index_1
        };

    symbol_table->member_wasm_exports[1] =
        (TP_WASM_SECTION_EXPORT_VAR){
            .member_name_length = name_length_2,
            .member_name = name_2,
            .member_kind = kind_2,
            .member_item_index = item_index_2
        };

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, name_length_1);
    payload_len += name_length_1;
    payload_len += sizeof(uint8_t);
    payload_len += tp_encode_ui32leb128(NULL, 0, item_index_1);
    payload_len += tp_encode_ui32leb128(NULL, 0, name_length_2);
    payload_len += name_length_2;
    payload_len += sizeof(uint8_t);
    payload_len += tp_encode_ui32leb128(NULL, 0, item_index_2);

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_EXPORT, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_EXPORT);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, name_length_1);
    memcpy(section_buffer + index, name_1, name_length_1);
    index += name_length_1;
    section_buffer[index] = kind_1;
    index += sizeof(uint8_t);
    index += tp_encode_ui32leb128(section_buffer, index, item_index_1);
    index += tp_encode_ui32leb128(section_buffer, index, name_length_2);
    memcpy(section_buffer + index, name_2, name_length_2);
    index += name_length_2;
    section_buffer[index] = kind_2;
    index += sizeof(uint8_t);
    (void)tp_encode_ui32leb128(section_buffer, index, item_index_2);

    return section;
}

static TP_WASM_MODULE_SECTION* make_section_code(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_CODE_COUNT_EXPR;
    uint32_t local_count = TP_WASM_MODULE_SECTION_CODE_LOCAL_COUNT_EXPR;
    uint32_t var_count = symbol_table->member_var_count; // Calculated by semantic analysis.
    uint32_t var_type = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32;

    uint32_t body_size = tp_encode_ui32leb128(NULL, 0, local_count);
    body_size += tp_encode_ui32leb128(NULL, 0, var_count);
    body_size += tp_encode_ui32leb128(NULL, 0, var_type);
    symbol_table->member_code_body_size = body_size;

    if ( ! search_parse_tree(
        symbol_table, symbol_table->member_tp_parse_tree, NULL, TP_GRAMMER_CONTEXT_NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return NULL;
    }

    symbol_table->member_code_body_size += tp_make_end_code(NULL, 0);

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, symbol_table->member_code_body_size);
    payload_len += symbol_table->member_code_body_size;

    TP_WASM_MODULE_SECTION* section = NULL;
    symbol_table->member_code_section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, symbol_table->member_code_section_buffer,
        TP_WASM_SECTION_TYPE_CODE, payload_len
    );

    uint8_t* section_buffer = symbol_table->member_code_section_buffer;
    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_CODE);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, symbol_table->member_code_body_size);
    index += tp_encode_ui32leb128(section_buffer, index, local_count);
    index += tp_encode_ui32leb128(section_buffer, index, var_count);
    index += tp_encode_ui32leb128(section_buffer, index, var_type);
    symbol_table->member_code_index = index;

    if ( ! search_parse_tree(
        symbol_table, symbol_table->member_tp_parse_tree, section, TP_GRAMMER_CONTEXT_NULL)){

        if (symbol_table->member_code_section_buffer){

            TP_FREE(symbol_table, &(symbol_table->member_code_section_buffer), payload_len);
        }

        if (section){

            TP_FREE(symbol_table, &section, sizeof(TP_WASM_MODULE_SECTION));
        }

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return NULL;
    }

    (void)tp_make_end_code(symbol_table->member_code_section_buffer, symbol_table->member_code_index);

    return section;
}

static bool search_parse_tree(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section, TP_GRAMMER_CONTEXT grammer_context)
{
    bool is_make_section_code_success = true;

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    for (size_t i = 0; element_num > i; ++i){

        if (TP_PARSE_TREE_TYPE_NULL == element[i].member_type){

            break;
        }

        if (TP_PARSE_TREE_TYPE_NODE == element[i].member_type){

            if ( ! search_parse_tree(
                symbol_table, (TP_PARSE_TREE*)(element[i].member_body.member_child), section, grammer_context)){

                is_make_section_code_success = false;
            }
        }
    }

    if ( ! make_section_code_content(symbol_table, parse_tree, section, grammer_context)){

        is_make_section_code_success = false;
    }

    return is_make_section_code_success;
}

static bool make_section_code_content(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section, TP_GRAMMER_CONTEXT grammer_context)
{
    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_PROGRAM:
        break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT_1:
//      break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT_2:
        return wasm_gen_statement_1_and_2(symbol_table, parse_tree, section);
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_1:
//      break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_2:
        return wasm_gen_expression_1_and_2(symbol_table, parse_tree, section);
    case TP_PARSE_TREE_GRAMMER_TERM_1:
//      break;
    case TP_PARSE_TREE_GRAMMER_TERM_2:
        return wasm_gen_term_1_and_2(symbol_table, parse_tree, section);
    case TP_PARSE_TREE_GRAMMER_FACTOR_1:
        return wasm_gen_factor_1(symbol_table, parse_tree, section);
    case TP_PARSE_TREE_GRAMMER_FACTOR_2:
//      break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_3:
        return wasm_gen_factor_2_and_3(symbol_table, parse_tree, section);
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool wasm_gen_statement_1_and_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section)
{
    // Grammer: Statement -> Type? variable '=' Expression ';'

    uint32_t var_value = 0;

    switch (parse_tree->member_grammer){
    // Grammer: Statement -> variable '=' Expression ';'
    case TP_PARSE_TREE_GRAMMER_STATEMENT_1:
        if (symbol_table->member_grammer_statement_1_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if ( ! tp_get_var_value(symbol_table, parse_tree, 0, &var_value)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    // Grammer: Statement -> Type variable '=' Expression ';'
    case TP_PARSE_TREE_GRAMMER_STATEMENT_2:
        if (symbol_table->member_grammer_statement_2_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if ( ! tp_get_var_value(symbol_table, parse_tree, 1, &var_value)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    default:

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (NULL == symbol_table->member_last_statement){ // Setup by semantic analysis.

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (symbol_table->member_last_statement == parse_tree){

        if (section){

            symbol_table->member_code_index += tp_make_tee_local_code(
                symbol_table->member_code_section_buffer, symbol_table->member_code_index, var_value
            );
        }else{

            symbol_table->member_code_body_size += tp_make_tee_local_code(NULL, 0, var_value);
        }
    }else{

        if (section){

            symbol_table->member_code_index += tp_make_set_local_code(
                symbol_table->member_code_section_buffer, symbol_table->member_code_index, var_value
            );
        }else{

            symbol_table->member_code_body_size += tp_make_set_local_code(NULL, 0, var_value);
        }
    }

    return true;
}

static bool wasm_gen_expression_1_and_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section)
{
    // Grammer: Expression -> Term (('+' | '-') Term)*

    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_1:
        if (symbol_table->member_grammer_expression_1_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_EXPRESSION_1][1]){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_2:
        if (symbol_table->member_grammer_expression_2_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_EXPRESSION_2][1]){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    default:

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    bool is_add = IS_TOKEN_PLUS(parse_tree->member_element[1].member_body.member_tp_token);

    if (section){

        if (is_add){

            symbol_table->member_code_index += tp_make_i32_add_code(
                symbol_table->member_code_section_buffer, symbol_table->member_code_index
            );
        }else{

            symbol_table->member_code_index += tp_make_i32_sub_code(
                symbol_table->member_code_section_buffer, symbol_table->member_code_index
            );
        }
    }else{

        if (is_add){

            symbol_table->member_code_body_size += tp_make_i32_add_code(NULL, 0);
        }else{

            symbol_table->member_code_body_size += tp_make_i32_sub_code(NULL, 0);
        }
    }

    return true;
}

static bool wasm_gen_term_1_and_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section)
{
    // Grammer: Term -> Factor (('*' | '/') Factor)*

    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_TERM_1:
        if (symbol_table->member_grammer_term_1_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_TERM_1][1]){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_2:
        if (symbol_table->member_grammer_term_2_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_TERM_2][1]){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    default:

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    bool is_mul = IS_TOKEN_MUL(parse_tree->member_element[1].member_body.member_tp_token);

    if (section){

        if (is_mul){

            symbol_table->member_code_index += tp_make_i32_mul_code(
                symbol_table->member_code_section_buffer, symbol_table->member_code_index
            );
        }else{

            symbol_table->member_code_index += tp_make_i32_div_code(
                symbol_table->member_code_section_buffer, symbol_table->member_code_index
            );
        }
    }else{

        if (is_mul){

            symbol_table->member_code_body_size += tp_make_i32_mul_code(NULL, 0);
        }else{

            symbol_table->member_code_body_size += tp_make_i32_div_code(NULL, 0);
        }
    }

    return true;
}

static bool wasm_gen_factor_1(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section)
{
    // Factor -> '(' Expression ')'

    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_FACTOR_1:
        if (symbol_table->member_grammer_factor_1_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        // NOTE: Skip left paren and right_paren.
        break;
    default:

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    return true;
}

static bool wasm_gen_factor_2_and_3(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_WASM_MODULE_SECTION* section)
{
    // Factor -> ('+' | '-')? (variable | constant)

    bool is_minus = false;
    bool is_const = false;
    int32_t const_value = 0;
    uint32_t var_value = 0;

    switch (parse_tree->member_grammer){
    // Factor -> ('+' | '-') (variable | constant)
    case TP_PARSE_TREE_GRAMMER_FACTOR_2:
        if (symbol_table->member_grammer_factor_2_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_2][0]) ||
            (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_2][1])){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        is_minus = IS_TOKEN_MINUS(parse_tree->member_element[0].member_body.member_tp_token);
        is_const = IS_TOKEN_TYPE_CONST_VALUE_INT32(parse_tree->member_element[1].member_body.member_tp_token);

        if (is_const){

            const_value = parse_tree->member_element[1].member_body.member_tp_token->member_value.member_int_value;
        }else{

            if ( ! tp_get_var_value(symbol_table, parse_tree, 1, &var_value)){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }
        }
        break;
    // Factor -> variable | constant
    case TP_PARSE_TREE_GRAMMER_FACTOR_3:
        if (symbol_table->member_grammer_factor_3_num != parse_tree->member_element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_3][0]){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        is_const = IS_TOKEN_TYPE_CONST_VALUE_INT32(parse_tree->member_element[0].member_body.member_tp_token);

        if (is_const){

            const_value = parse_tree->member_element[0].member_body.member_tp_token->member_value.member_int_value;
        }else{

            if ( ! tp_get_var_value(symbol_table, parse_tree, 0, &var_value)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
        break;
    default:

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    uint8_t* p = symbol_table->member_code_section_buffer;
    size_t code_index = symbol_table->member_code_index;
    uint32_t code_body_size = symbol_table->member_code_body_size;

    if (is_const){

        if (section){

            code_index += tp_make_i32_const_code(p, code_index, (is_minus ? -const_value : const_value));
        }else{

            code_body_size += tp_make_i32_const_code(NULL, 0, (is_minus ? -const_value : const_value));
        }
    }else{

        if (is_minus){

            // Change of sign.
            if (section){

                code_index += tp_make_i32_const_code(p, code_index, -1);
                code_index += tp_make_get_local_code(p, code_index, var_value);
                code_index += tp_make_i32_xor_code(p, code_index);
                code_index += tp_make_i32_const_code(p, code_index, 1);
                code_index += tp_make_i32_add_code(p, code_index);
            }else{

                code_body_size += tp_make_i32_const_code(NULL, 0, -1);
                code_body_size += tp_make_get_local_code(NULL, 0, var_value);
                code_body_size += tp_make_i32_xor_code(NULL, 0);
                code_body_size += tp_make_i32_const_code(NULL, 0, 1);
                code_body_size += tp_make_i32_add_code(NULL, 0);
            }
        }else{

            if (section){

                code_index += tp_make_get_local_code(p, code_index, var_value);
            }else{

                code_body_size += tp_make_get_local_code(NULL, 0, var_value);
            }
        }
    }

    symbol_table->member_code_index = code_index;
    symbol_table->member_code_body_size = code_body_size;

    return true;
}

