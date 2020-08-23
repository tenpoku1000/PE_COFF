
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_H_)
#define TP_COMPILER_H_

#include "tp_compiler_pe.h"

// ----------------------------------------------------------------------------------------
// Environment

typedef struct symbol_table_{
// config section:
    bool member_is_output_current_dir;
    bool member_is_output_log_file;
    bool member_is_no_output_messages;
    bool member_is_no_output_files;
    bool member_is_origin_wasm;
    bool member_is_source_cmd_param;
    uint8_t member_source_code[TP_SOURCE_CODE_STRING_BUFFER_SIZE];
    bool member_is_test_mode;
    bool member_is_test_disasm_x64;
    bool member_is_output_wasm_file;
    bool member_is_output_wasm_text_file;
    bool member_is_output_x64_file;
    bool member_is_output_pe_coff_file;
    bool member_is_32;

// message section:
    bool member_log_hide_after_disp;
    uint8_t member_temp_buffer[TP_MESSAGE_BUFFER_SIZE];
    uint8_t member_log_msg_buffer[TP_MESSAGE_BUFFER_SIZE];
    FILE* member_disp_log_file;

    bool member_is_error_abort;
    size_t member_error_count_prev;
    size_t member_error_count;
    size_t member_warning_count;

    TP_TOKEN* member_error_token;
    TP_CHAR8_T* member_error_message;
    TP_CHAR8_T* member_error_reason;

// output file section:
    FILE* member_write_log_file;
    FILE* member_parse_tree_file;
    char member_write_log_file_path[_MAX_PATH];
    char member_token_file_path[_MAX_PATH];
    char member_parse_tree_file_path[_MAX_PATH];
    char member_object_hash_file_path[_MAX_PATH];
    char member_wasm_file_path[_MAX_PATH];
    char member_wasm_text_file_path[_MAX_PATH];
    char member_x64_file_path[_MAX_PATH];
    char member_x64_text_file_path[_MAX_PATH];
    char member_coff_code_text_file_path[_MAX_PATH];
    char member_pe_code_text_file_path[_MAX_PATH];
    char member_pe_uefi_path[_MAX_PATH];

// input file section:
    uint8_t member_input_file_path[_MAX_PATH];
    FILE* member_read_file;
    bool member_is_start_of_file;
    bool member_is_end_of_file;
    uint8_t member_read_lines_buffer[TP_BUFFER_SIZE];
    size_t member_read_lines_length;
    uint8_t* member_read_lines_current_position;
    uint8_t* member_read_lines_end_position;

// token section:
    rsize_t member_tp_token_pos;
    rsize_t member_tp_token_size;
    rsize_t member_tp_token_size_allocate_unit;
    TP_TOKEN* member_tp_token;
    TP_TOKEN* member_tp_token_position;
    rsize_t member_nul_num;

// parse tree section:
    uint8_t member_nesting_level_of_expression;
    TP_PARSE_TREE* member_tp_parse_tree;

// semantic analysis section:
    REGISTER_OBJECT_HASH member_object_hash;
    uint32_t member_var_count;
    TP_PARSE_TREE* member_last_statement;
    TP_PARSE_TREE_TYPE member_parse_tree_type[TP_PARSE_TREE_TYPE_MAX_NUM2][TP_PARSE_TREE_TYPE_MAX_NUM1];
    uint32_t member_grammer_statement_1_num;
    uint32_t member_grammer_statement_2_num;
    uint32_t member_grammer_expression_1_num;
    uint32_t member_grammer_expression_2_num;
    uint32_t member_grammer_term_1_num;
    uint32_t member_grammer_term_2_num;
    uint32_t member_grammer_factor_1_num;
    uint32_t member_grammer_factor_2_num;
    uint32_t member_grammer_factor_3_num;

// wasm section:
    uint32_t member_wasm_local_pos;

    uint32_t member_wasm_data_base_1;
    uint32_t member_wasm_data_offset_1;
    uint32_t member_wasm_data_size_1;
    uint8_t* member_wasm_data_data_1;

    uint32_t member_wasm_data_base_2;
    uint32_t member_wasm_data_offset_2;
    uint32_t member_wasm_data_size_2;
    uint8_t* member_wasm_data_data_2;

    TP_WASM_MODULE member_wasm_module;
    size_t member_code_index;
    uint32_t member_code_body_size;
    uint8_t* member_code_section_buffer;

    TP_WASM_RELOCATION* member_wasm_relocation;
    rsize_t member_wasm_relocation_begin;
    rsize_t member_wasm_relocation_cur;
    rsize_t member_wasm_relocation_pos;
    rsize_t member_wasm_relocation_size;
    rsize_t member_wasm_relocation_allocate_unit;

    TP_WASM_LOCAL_TYPE* member_local_types;
    uint32_t member_local_types_num;

// x64 section:
    TP_WASM_STACK_ELEMENT* member_stack;
    uint8_t* member_wasm_code_body_buffer;
    uint32_t member_wasm_code_body_pos;
    uint32_t member_wasm_code_body_size;
    int32_t member_stack_pos;
    int32_t member_stack_size;
    int32_t member_stack_size_allocate_unit;

    int32_t member_local_variable_size;
    int32_t member_local_variable_size_max;
    int32_t member_padding_local_variable_bytes;

    int32_t member_temporary_variable_size;
    int32_t member_temporary_variable_size_max;
    int32_t member_padding_temporary_variable_bytes;

    TP_WASM_STACK_ELEMENT member_use_X86_32_register[TP_X86_32_REGISTER_NUM];
    TP_WASM_STACK_ELEMENT member_use_X64_32_register[TP_X64_32_REGISTER_NUM];
    TP_X64_NV64_REGISTER member_use_nv_register[TP_X64_NV64_REGISTER_NUM];
    TP_X64_V64_REGISTER member_use_v_register[TP_X64_V64_REGISTER_NUM];

    int32_t member_register_bytes;
    int32_t member_padding_register_bytes;

    int32_t member_stack_imm32;

// PE COFF section:
    bool member_is_add_data_section;
    bool member_is_add_rdata_section;
    bool member_is_add_text_section;

    // PE COFF OBJECT/IMAGE File
    uint8_t* member_pe_coff_buffer;
    rsize_t member_pe_coff_size;
    rsize_t member_pe_coff_current_offset;

    // COFF File
    TP_COFF_WRITE member_coff;

    // PE File header
    TP_PE_DOS_HEADER* member_dos_header_read;  // NOTE: member_dos_header_read must not free memory.
    TP_PE_HEADER64_READ* member_pe_header64_read;  // NOTE: member_pe_header64_read must not free memory.

    // COFF File Header
    TP_COFF_FILE_HEADER* member_coff_file_header;  // NOTE: member_coff_file_header must not free memory.

    // Section Table
    TP_SECTION_TABLE* member_section_table;
    rsize_t member_section_table_size;
    rsize_t member_section_table_num;

    // Section Data(Relocations)
    TP_COFF_RELOCATIONS_ARRAY* member_coff_relocations;
    rsize_t member_coff_relocations_size;

    // COFF Symbol Table
    TP_COFF_SYMBOL_TABLE* member_coff_symbol_table;  // NOTE: member_coff_symbol_table must not free memory.

    // COFF String Table
    rsize_t member_string_table_offset;
    uint32_t member_string_table_size;
    uint8_t* member_string_table;  // NOTE: member_string_table must not free memory.
}TP_SYMBOL_TABLE;

// ----------------------------------------------------------------------------------------
// :
bool tp_compiler(int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size);
bool tp_compiler_main(
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    bool* is_test_mode, size_t test_index, int32_t* return_value,
    char* drive, char* dir, time_t now
);
bool tp_test_compiler(
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    char* drive, char* dir, time_t now
);
bool tp_init_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, int argc, char** argv, bool* is_disp_usage,
    bool* is_test, size_t test_index, time_t now, char* drive, char* dir,
    uint8_t* msg_buffer, size_t msg_buffer_size
);
void tp_free_memory_and_file(TP_SYMBOL_TABLE** symbol_table);
bool tp_make_path_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir,
    bool is_test, size_t test_index, time_t now
);
bool tp_make_path(
    TP_SYMBOL_TABLE* symbol_table,
    char* drive, char* dir, char* prefix, char* fname, char* ext,
    char* path, size_t path_size
);
bool tp_get_drive_dir(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir
);

// ----------------------------------------------------------------------------------------
// token section:
bool tp_make_token(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length);
bool tp_dump_token_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_TOKEN* token, uint8_t indent_level
);

// ----------------------------------------------------------------------------------------
// parse tree section:
bool tp_make_parse_tree(TP_SYMBOL_TABLE* symbol_table);
void tp_free_parse_subtree(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE** parse_subtree);

// ----------------------------------------------------------------------------------------
// semantic analysis section:
bool tp_semantic_analysis(TP_SYMBOL_TABLE* symbol_table);
bool tp_search_object(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, REGISTER_OBJECT* register_object);
void tp_free_object_hash(
    TP_SYMBOL_TABLE* symbol_table,
    REGISTER_OBJECT_HASH* object_hash, REGISTER_OBJECT_HASH_ELEMENT* hash_element
);

// ----------------------------------------------------------------------------------------
// wasm section:
bool tp_make_wasm(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, bool is_origin_wasm, bool is_pe_coff, bool is_64
);
bool tp_get_wasm_export_code_section(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, TP_WASM_MODULE_SECTION** code_section, uint32_t* return_type
);
bool tp_disasm_wasm(TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol);
bool tp_make_wasm_expr(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section, bool is_origin_wasm
);
bool tp_make_wasm_coff(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section, bool is_origin_wasm, bool is_64
);
bool tp_wasm_add_global_variable_i32(
    TP_SYMBOL_TABLE* symbol_table, uint32_t var_id, uint8_t* symbol, rsize_t symbol_length,
    uint32_t init_value, uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index
);
bool tp_wasm_add_global_variable_i64(
    TP_SYMBOL_TABLE* symbol_table, uint32_t var_id, uint8_t* symbol, rsize_t symbol_length,
    uint64_t init_value, uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index
);
bool tp_wasm_add_local_variable_i32(
    TP_SYMBOL_TABLE* symbol_table, uint32_t local_index, uint32_t* wasm_offset
);
bool tp_wasm_add_local_variable_i64(
    TP_SYMBOL_TABLE* symbol_table, uint32_t local_index, uint32_t* wasm_offset
);
bool tp_wasm_add_string_literal(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t* string_literal_index, uint8_t* symbol, rsize_t symbol_length,
    uint16_t* string, rsize_t string_length,
    uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index
);
bool tp_wasm_add_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* symbol, rsize_t symbol_length
);
bool tp_wasm_init_relocation(TP_SYMBOL_TABLE* symbol_table);
bool tp_wasm_add_relocation(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_RELOCATION* wasm_relocation
);
bool tp_wasm_get_relocation(
    TP_SYMBOL_TABLE* symbol_table, rsize_t begin_index, TP_WASM_RELOCATION** wasm_relocation
);
// UEFI
TP_WASM_MODULE_SECTION* tp_make_section_type_UEFI(TP_SYMBOL_TABLE* symbol_table, bool is_64);
TP_WASM_MODULE_SECTION* tp_make_section_function_UEFI(TP_SYMBOL_TABLE* symbol_table);
TP_WASM_MODULE_SECTION* tp_make_section_table_UEFI(TP_SYMBOL_TABLE* symbol_table);
TP_WASM_MODULE_SECTION* tp_make_section_memory_UEFI(TP_SYMBOL_TABLE* symbol_table);
TP_WASM_MODULE_SECTION* tp_make_section_global_UEFI(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t heap_current, uint32_t heap_base, uint32_t data_end
);
TP_WASM_MODULE_SECTION* tp_make_section_export_UEFI(TP_SYMBOL_TABLE* symbol_table);
TP_WASM_MODULE_SECTION* tp_make_section_data_UEFI(
    TP_SYMBOL_TABLE* symbol_table, uint32_t offset_1, uint32_t size_1, uint8_t* data_1,
    uint32_t offset_2, uint32_t size_2, uint8_t* data_2
);
// UEFI: WASM32
TP_WASM_MODULE_SECTION* tp_make_section_code_origin_wasm_UEFI32(TP_SYMBOL_TABLE* symbol_table);
// UEFI: original WASM64
TP_WASM_MODULE_SECTION* tp_make_section_code_origin_wasm_UEFI64(TP_SYMBOL_TABLE* symbol_table);

// Control flow operators
uint32_t tp_make_loop_code(uint8_t* buffer, size_t offset, int32_t sig);
uint32_t tp_make_end_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_br_if_code(uint8_t* buffer, size_t offset, uint32_t relative_depth);
uint32_t tp_make_ret_code(uint8_t* buffer, size_t offset);

// Call operators
uint32_t tp_make_call_indirect_code(
    uint8_t* buffer, size_t offset, uint32_t type_index, uint32_t reserved
);

// Parametric operators
uint32_t tp_make_drop_code(uint8_t* buffer, size_t offset);

// Variable access
uint32_t tp_make_get_local_code(uint8_t* buffer, size_t offset, uint32_t local_index);
uint32_t tp_make_set_local_code(uint8_t* buffer, size_t offset, uint32_t local_index);
uint32_t tp_make_tee_local_code(uint8_t* buffer, size_t offset, uint32_t local_index);
uint32_t tp_make_get_global_code(uint8_t* buffer, size_t offset, uint32_t global_index);
uint32_t tp_make_set_global_code(uint8_t* buffer, size_t offset, uint32_t global_index);

// Memory-related operators
uint32_t tp_make_i32_load_code(
    uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset
);
uint32_t tp_make_i32_store_code(
    uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset
);
uint32_t tp_make_i64_load_code(
    uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset
);
uint32_t tp_make_i64_store_code(
    uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset
);

// Constants
uint32_t tp_make_i32_const_code(uint8_t* buffer, size_t offset, int32_t value);
uint32_t tp_make_i64_const_code(uint8_t* buffer, size_t offset, int64_t value);

// Comparison operators
uint32_t tp_make_i64_ne_code(uint8_t* buffer, size_t offset);

// Numeric operators(i32)
uint32_t tp_make_i32_add_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i32_sub_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i32_mul_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i32_div_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i32_xor_code(uint8_t* buffer, size_t offset);

// Numeric operators(i64)
uint32_t tp_make_i64_add_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i64_sub_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i64_mul_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i64_div_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_i64_xor_code(uint8_t* buffer, size_t offset);

// ----------------------------------------------------------------------------------------
// x64 section:
bool tp_make_x64_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol, int32_t* return_value,
    bool is_pe_coff, char* fname, char* ext
);
bool tp_wasm_stack_push(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_ELEMENT* value);
bool tp_get_local_variable_offset(
    TP_SYMBOL_TABLE* symbol_table, uint32_t local_index, int32_t* local_variable_offset
);
bool tp_allocate_temporary_variable(
    TP_SYMBOL_TABLE* symbol_table, TP_X64_ALLOCATE_MODE allocate_mode,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_WASM_STACK_ELEMENT* wasm_stack_element
);
bool tp_free_register(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_ELEMENT* stack_element);
uint32_t tp_encode_allocate_stack(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    uint32_t local_variable_size
);
uint32_t tp_encode_x64_push_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_X64_64_REGISTER reg64
);
uint32_t tp_encode_relocation_type_base(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset, 
    TP_WASM_RELOCATION* wasm_relocation
);

// Control flow operators

bool tp_encode_loop_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, int32_t sig
);
uint32_t tp_encode_br_if_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t relative_depth, TP_WASM_STACK_ELEMENT* label,
    TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);
uint32_t tp_encode_return_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation
);

// Call operators

uint32_t tp_encode_call_indirect_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t type_index, uint32_t reserved, TP_WASM_STACK_ELEMENT* op1
);

// Variable access

uint32_t tp_encode_get_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index, bool* status
);
uint32_t tp_encode_set_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index, TP_WASM_STACK_ELEMENT* op1
);
uint32_t tp_encode_tee_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index,
    TP_WASM_STACK_ELEMENT* op1, bool* status
);
uint32_t tp_encode_get_global_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t global_index
);
uint32_t tp_encode_set_global_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t global_index, TP_WASM_STACK_ELEMENT* op1
);

// Memory-related operators

uint32_t tp_encode_i32_load_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1
);
uint32_t tp_encode_i32_store_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);
uint32_t tp_encode_i64_load_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1
);
uint32_t tp_encode_i64_store_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);

// Constants

uint32_t tp_encode_i32_const_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, int32_t value, bool* status
);
uint32_t tp_encode_i64_const_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, int64_t value, bool* status
);

// Comparison operators

uint32_t tp_encode_i64_ne_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);

// Numeric operators(i32)
// Numeric operators(i64)

uint32_t tp_encode_add_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);
uint32_t tp_encode_sub_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);
uint32_t tp_encode_mul_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);
uint32_t tp_encode_div_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);
uint32_t tp_encode_xor_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);

// x64 Assembly

uint32_t tp_encode_x64_2_operand(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);

uint32_t tp_encode_x64_mov_imm(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    int64_t imm, TP_X64_MOV_IMM_MODE x64_mov_imm_mode, TP_WASM_STACK_ELEMENT* result
);
uint32_t tp_encode_x64_add_sub_imm(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_X64_64_REGISTER reg64,
    int32_t imm, TP_X64_ADD_SUB_IMM_MODE x64_add_sub_imm_mode
);
bool tp_convert_to_x64_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    TP_X64_64_REGISTER* reg64_dst_reg, TP_WASM_STACK_ELEMENT* op1
);
uint32_t tp_encode_x64_lea_rel_mem(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64_64_REGISTER reg64_dst_reg, TP_WASM_STACK_ELEMENT* src
);
uint32_t tp_encode_x64_lea(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64_64_REGISTER reg64_dst_reg,
    TP_X64_64_REGISTER reg64_src_index, TP_X64_64_REGISTER reg64_src_base, int32_t offset
);
uint32_t tp_encode_x64_push_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_X64_64_REGISTER reg64
);
uint32_t tp_encode_x64_pop_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_X64_64_REGISTER reg64
);
uint32_t tp_encode_x64_jne(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_WASM_STACK_ELEMENT* label
);
uint32_t tp_encode_x64_call(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_WASM_STACK_ELEMENT* op1
);
uint32_t tp_encode_x64_1_opcode(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint8_t opcode
);

// ----------------------------------------------------------------------------------------
// PE COFF section:
bool tp_make_COFF(
    TP_SYMBOL_TABLE* symbol_table, char* fname, char* ext,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint8_t* coff_buffer, rsize_t coff_file_size, rsize_t code_offset
);
bool tp_get_coff_size_and_calc_offset(
    TP_SYMBOL_TABLE* symbol_table, bool is_pe_coff,
    uint32_t x64_code_buffer_size, rsize_t* coff_file_size, rsize_t* code_offset
);
bool tp_get_entry_point_and_calc_code_relocations(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* coff_buffer, rsize_t coff_file_size,
    rsize_t code_offset, uint8_t* entry_point_symbol, uint8_t** entry_point
);
bool tp_add_coff_section(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind, TP_SECTION_ALIGN section_align
);
bool tp_append_coff_data_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* data, rsize_t size, TP_SECTION_ALIGN section_align
);
bool tp_append_coff_rdata_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* rdata, rsize_t size, TP_SECTION_ALIGN section_align
);
bool tp_append_coff_relocation(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind,
    uint32_t virtual_address, uint32_t symbol_table_index
);
bool tp_append_coff_symbol(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind,
    uint8_t* symbol, rsize_t symbol_length,
    uint32_t offset_value, uint32_t* symbol_index
);

bool tp_make_PE_file(TP_SYMBOL_TABLE* symbol_table, char* fname, char* ext, uint8_t* entry_point_symbol);

// Convert from COFF Object to PE Image. 
bool tp_make_PE_file_buffer(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* entry_point_symbol);

// PE File header
bool tp_make_PE_file_PE_HEADER64(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

// PE File Data Directory
bool tp_make_PE_file_PE_DATA_DIRECTORY(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_PE_OPTIONAL_HEADER64* optional_header
);
bool tp_make_PE_file_PE_BASE_RELOCATION(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* raw_data, uint32_t data_size
);

// Section Table
bool tp_make_PE_file_SECTION_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

// COFF Symbol Table
bool tp_make_PE_file_COFF_SYMBOL_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);
bool tp_make_PE_file_COFF_SYMBOL_TABLE_content(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables, rsize_t index
);

// COFF String Table
bool tp_make_PE_file_COFF_STRING_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

// Utilities
bool tp_make_PE_file_raw_data(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* raw_data, rsize_t size
);
bool tp_seek_PE_COFF_file(TP_SYMBOL_TABLE* symbol_table, long seek_position, long line_bytes);

// ----------------------------------------------------------------------------------------
// :
bool tp_test_disasm_x64(TP_SYMBOL_TABLE* symbol_table);
bool tp_disasm_x64(
    TP_SYMBOL_TABLE* symbol_table, char* path,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint8_t* disasm_string, uint8_t* disasm_additional_string
);

// ----------------------------------------------------------------------------------------
// :

bool tp_open_read_file(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_open_read_file_text(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_open_write_file(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_open_write_file_text(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_ftell(TP_SYMBOL_TABLE* symbol_table, FILE* file_stream, long* seek_position);
bool tp_seek(TP_SYMBOL_TABLE* symbol_table, FILE* file_stream, long seek_position, long line_bytes);
bool tp_close_file(TP_SYMBOL_TABLE* symbol_table, FILE** file_stream);
bool tp_write_file(TP_SYMBOL_TABLE* symbol_table, char* path, void* content, uint32_t content_size);

// ----------------------------------------------------------------------------------------
// :

uint32_t tp_encode_si64leb128(uint8_t* buffer, size_t offset, int64_t value);
uint32_t tp_encode_ui32leb128(uint8_t* buffer, size_t offset, uint32_t value);
int64_t tp_decode_si64leb128(uint8_t* buffer, uint32_t* size);
int32_t tp_decode_si32leb128(uint8_t* buffer, uint32_t* size);
uint32_t tp_decode_ui32leb128(uint8_t* buffer, uint32_t* size);

// ----------------------------------------------------------------------------------------
// :

void tp_free(TP_SYMBOL_TABLE* symbol_table, void** ptr, size_t size, uint8_t* file, uint8_t* func, size_t line_num);
void tp_free2(TP_SYMBOL_TABLE* symbol_table, void*** ptr, size_t size, uint8_t* file, uint8_t* func, size_t line_num);
void tp_get_last_error(TP_SYMBOL_TABLE* symbol_table, TP_ERROR_TYPE error_type, uint8_t* file, uint8_t* func, size_t line_num);
void tp_print_crt_error(TP_SYMBOL_TABLE* symbol_table, TP_ERROR_TYPE error_type, uint8_t* file, uint8_t* func, size_t line_num);
bool tp_put_log_msg(
    TP_SYMBOL_TABLE* symbol_table, TP_LOG_TYPE log_type, TP_ERROR_TYPE error_type,
    uint8_t* format_string, uint8_t* file, uint8_t* func, size_t line_num,
    TP_LOG_PARAM_ELEMENT* log_param_element, size_t log_param_element_num
);

#endif

