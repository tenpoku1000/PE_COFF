
// Copyright (C) 2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// WebAssembly headers(UEFI).

#define TP_SECTION_NUM 8

#define TP_WASM_MODULE_SECTION_TYPE_COUNT 2
#define TP_WASM_MODULE_SECTION_TYPE_PARAM_COUNT_1 2
#define TP_WASM_MODULE_SECTION_TYPE_PARAM_COUNT_2 4

#define TP_WASM_MODULE_SECTION_FUNCTION_COUNT 1
#define TP_WASM_MODULE_SECTION_FUNCTION_TYPES 0

#define TP_WASM_MODULE_SECTION_TABLE_COUNT 1
#define TP_WASM_MODULE_SECTION_TABLE_ELEMENT_TYPE_ANYFUNC 0x70
#define TP_WASM_MODULE_SECTION_TABLE_FLAGS 0
#define TP_WASM_MODULE_SECTION_TABLE_INITIAL 0

#define TP_WASM_MODULE_SECTION_MEMORY_COUNT 1
#define TP_WASM_MODULE_SECTION_MEMORY_FLAGS 0
#define TP_WASM_MODULE_SECTION_MEMORY_INITIAL 1

#define TP_WASM_MODULE_SECTION_GLOBAL_COUNT 3
#define TP_WASM_MODULE_SECTION_GLOBAL_TYPE_I32 0x7f
#define TP_WASM_MODULE_SECTION_GLOBAL_IMMUTABLE 0
#define TP_WASM_MODULE_SECTION_GLOBAL_MUTABLE 1

#define TP_WASM_MODULE_SECTION_EXPORT_COUNT 2
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_1 6
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_1 "memory"
#define TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_1 0
#define TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_2 0

#define TP_WASM_MODULE_SECTION_DATA_INDEX 0

TP_WASM_MODULE_SECTION* tp_make_section_type_UEFI(TP_SYMBOL_TABLE* symbol_table, bool is_64)
{
    uint32_t count = TP_WASM_MODULE_SECTION_TYPE_COUNT;
    uint32_t form = TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC;
    uint32_t param_count_1 = TP_WASM_MODULE_SECTION_TYPE_PARAM_COUNT_1;

    uint32_t param_types_1_1 = 0;
    uint32_t param_types_1_2 = 0;

    if (is_64){

        param_types_1_1 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64;
        param_types_1_2 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64;
    }else{

        param_types_1_1 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32;
        param_types_1_2 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32;
    }

    uint32_t param_count_2 = TP_WASM_MODULE_SECTION_TYPE_PARAM_COUNT_2;
    uint32_t param_types_2_1 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32;
    uint32_t param_types_2_2 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64;
    uint32_t param_types_2_3 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64;
    uint32_t param_types_2_4 = TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32;
    uint32_t return_count = TP_WASM_MODULE_SECTION_TYPE_RETURN_COUNT;
    uint32_t return_type = TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64;

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);

    payload_len += tp_encode_ui32leb128(NULL, 0, form);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_count_1);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_types_1_1);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_types_1_2);
    payload_len += tp_encode_ui32leb128(NULL, 0, return_count);
    payload_len += tp_encode_ui32leb128(NULL, 0, return_type);

    payload_len += tp_encode_ui32leb128(NULL, 0, form);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_count_2);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_types_2_1);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_types_2_2);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_types_2_3);
    payload_len += tp_encode_ui32leb128(NULL, 0, param_types_2_4);
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
    index += tp_encode_ui32leb128(section_buffer, index, param_count_1);
    index += tp_encode_ui32leb128(section_buffer, index, param_types_1_1);
    index += tp_encode_ui32leb128(section_buffer, index, param_types_1_2);
    index += tp_encode_ui32leb128(section_buffer, index, return_count);
    index += tp_encode_ui32leb128(section_buffer, index, return_type);

    index += tp_encode_ui32leb128(section_buffer, index, form);
    index += tp_encode_ui32leb128(section_buffer, index, param_count_2);
    index += tp_encode_ui32leb128(section_buffer, index, param_types_2_1);
    index += tp_encode_ui32leb128(section_buffer, index, param_types_2_2);
    index += tp_encode_ui32leb128(section_buffer, index, param_types_2_3);
    index += tp_encode_ui32leb128(section_buffer, index, param_types_2_4);
    index += tp_encode_ui32leb128(section_buffer, index, return_count);
    (void)tp_encode_ui32leb128(section_buffer, index, return_type);

    return section;
}

TP_WASM_MODULE_SECTION* tp_make_section_function_UEFI(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_FUNCTION_COUNT;
    uint32_t types = TP_WASM_MODULE_SECTION_FUNCTION_TYPES;

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

TP_WASM_MODULE_SECTION* tp_make_section_table_UEFI(TP_SYMBOL_TABLE* symbol_table)
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

TP_WASM_MODULE_SECTION* tp_make_section_memory_UEFI(TP_SYMBOL_TABLE* symbol_table)
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

TP_WASM_MODULE_SECTION* tp_make_section_global_UEFI(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t heap_current, uint32_t heap_base, uint32_t data_end)
{
    uint32_t count = TP_WASM_MODULE_SECTION_GLOBAL_COUNT;
    uint32_t content_type_1 = TP_WASM_MODULE_SECTION_GLOBAL_TYPE_I32;
    uint32_t mutability_1 = TP_WASM_MODULE_SECTION_GLOBAL_MUTABLE;
    uint32_t content_type_2 = TP_WASM_MODULE_SECTION_GLOBAL_TYPE_I32;
    uint32_t mutability_2 = TP_WASM_MODULE_SECTION_GLOBAL_IMMUTABLE;
    uint32_t content_type_3 = TP_WASM_MODULE_SECTION_GLOBAL_TYPE_I32;
    uint32_t mutability_3 = TP_WASM_MODULE_SECTION_GLOBAL_IMMUTABLE;

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);

    payload_len += tp_encode_ui32leb128(NULL, 0, content_type_1);
    payload_len += tp_encode_ui32leb128(NULL, 0, mutability_1);
    payload_len += tp_make_i32_const_code(NULL, 0, (int32_t)heap_current);
    payload_len += tp_make_end_code(NULL, 0);

    payload_len += tp_encode_ui32leb128(NULL, 0, content_type_2);
    payload_len += tp_encode_ui32leb128(NULL, 0, mutability_2);
    payload_len += tp_make_i32_const_code(NULL, 0, (int32_t)heap_base);
    payload_len += tp_make_end_code(NULL, 0);

    payload_len += tp_encode_ui32leb128(NULL, 0, content_type_3);
    payload_len += tp_encode_ui32leb128(NULL, 0, mutability_3);
    payload_len += tp_make_i32_const_code(NULL, 0, (int32_t)data_end);
    payload_len += tp_make_end_code(NULL, 0);

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_GLOBAL, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_GLOBAL);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);

    index += tp_encode_ui32leb128(section_buffer, index, content_type_1);
    index += tp_encode_ui32leb128(section_buffer, index, mutability_1);
    index += tp_make_i32_const_code(section_buffer, index, (int32_t)heap_current);
    index += tp_make_end_code(section_buffer, index);

    index += tp_encode_ui32leb128(section_buffer, index, content_type_2);
    index += tp_encode_ui32leb128(section_buffer, index, mutability_2);
    index += tp_make_i32_const_code(section_buffer, index, (int32_t)heap_base);
    index += tp_make_end_code(section_buffer, index);

    index += tp_encode_ui32leb128(section_buffer, index, content_type_3);
    index += tp_encode_ui32leb128(section_buffer, index, mutability_3);
    index += tp_make_i32_const_code(section_buffer, index, (int32_t)data_end);
    (void)tp_make_end_code(section_buffer, index);

    return section;
}

TP_WASM_MODULE_SECTION* tp_make_section_export_UEFI(TP_SYMBOL_TABLE* symbol_table)
{
    uint32_t count = TP_WASM_MODULE_SECTION_EXPORT_COUNT;
    uint32_t name_length_1 = TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_1;
    uint8_t* name_1 = TP_WASM_MODULE_SECTION_EXPORT_NAME_1;
    uint8_t kind_1 = TP_WASM_SECTION_KIND_MEMORY;
    uint32_t item_index_1 = TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_1;

    uint32_t name_length_2 = TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_2_UEFI;
    uint8_t* name_2 = TP_WASM_MODULE_SECTION_EXPORT_NAME_2_UEFI;
    uint8_t kind_2 = TP_WASM_SECTION_KIND_FUNCTION;
    uint32_t item_index_2 = TP_WASM_MODULE_SECTION_EXPORT_ITEM_INDEX_2;

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

TP_WASM_MODULE_SECTION* tp_make_section_data_UEFI(
    TP_SYMBOL_TABLE* symbol_table, uint32_t offset_1, uint32_t size_1, uint8_t* data_1,
    uint32_t offset_2, uint32_t size_2, uint8_t* data_2)
{
    if ((NULL == data_1) && (NULL == data_2)){

        return NULL;
    }

    uint32_t count = 0;
    uint32_t index_1 = TP_WASM_MODULE_SECTION_DATA_INDEX;
    uint32_t index_2 = TP_WASM_MODULE_SECTION_DATA_INDEX;

    if (data_1 && data_2){

        count = 2;
    }else{

        count = 1;
        
        if (data_2){

            offset_1 = offset_2;
            size_1 = size_2;
            data_1 = data_2;
            data_2 = NULL;
        }
    }

    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);

    payload_len += tp_encode_ui32leb128(NULL, 0, index_1);
    payload_len += tp_make_i32_const_code(NULL, 0, (int32_t)offset_1);
    payload_len += tp_make_end_code(NULL, 0);
    payload_len += tp_encode_ui32leb128(NULL, 0, size_1);
    payload_len += size_1;

    if (2 == count){

        payload_len += tp_encode_ui32leb128(NULL, 0, index_2);
        payload_len += tp_make_i32_const_code(NULL, 0, (int32_t)offset_2);
        payload_len += tp_make_end_code(NULL, 0);
        payload_len += tp_encode_ui32leb128(NULL, 0, size_2);
        payload_len += size_2;
    }

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_DATA, payload_len
    );

    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_DATA);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);

    index += tp_encode_ui32leb128(section_buffer, index, index_1);
    index += tp_make_i32_const_code(section_buffer, index, (int32_t)offset_1);
    index += tp_make_end_code(section_buffer, index);
    index += tp_encode_ui32leb128(section_buffer, index, size_1);
    memcpy(section_buffer + index, data_1, size_1);
    index += size_1;

    if (2 == count){

        index += tp_encode_ui32leb128(section_buffer, index, index_2);
        index += tp_make_i32_const_code(section_buffer, index, (int32_t)offset_2);
        index += tp_make_end_code(section_buffer, index);
        index += tp_encode_ui32leb128(section_buffer, index, size_2);
        memcpy(section_buffer + index, data_2, size_2);
        index += size_2;
    }

    return section;
}

