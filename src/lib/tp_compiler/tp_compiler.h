
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_H_)
#define TP_COMPILER_H_

#include "tp_compiler_pe.h"

// ----------------------------------------------------------------------------------------
// Environment

typedef struct symbol_table_{
// config section:
    // TP_CONFIG_OPTION_IS_TEST_DISASM_X64 'a'
    bool member_is_test_disasm_x64;
    // TP_CONFIG_OPTION_IS_OUTPUT_TOKEN_FILE 'b'
    bool member_is_output_token_file;
    // TP_CONFIG_OPTION_IS_OUTPUT_CURRENT_DIR 'c'
    bool member_is_output_current_dir;
    // TP_CONFIG_OPTION_IS_OUTPUT_UNICODE_CHARACTER_DATABASE_FILE 'd'
    bool member_is_output_unicode_character_database;
    // TP_CONFIG_OPTION_IS_TEST_UNICODE_CHARACTER_DATABASE 'e'
    bool member_is_test_unicode_character_database;
    // TP_CONFIG_OPTION_IS_LOAD_AND_EXEC_DLL 'f'
    bool member_is_load_and_exec_dll;
    // TP_CONFIG_OPTION_IS_IMAGE_SUBSYSTEM_EFI_APPLICATION 'g'
    uint16_t member_subsystem;
    // TP_CONFIG_OPTION_IS_HELP 'h'
//
    // TP_CONFIG_OPTION_IS_INT_CALC_COMPILER 'i'
    bool member_is_int_calc_compiler;
    // TP_CONFIG_OPTION_IS_OUTPUT_PARSE_TREE_FILE 'j'
    bool member_is_output_parse_tree_file;
    // TP_CONFIG_OPTION_IS_OUTPUT_SEMANTIC_ANALYSIS_FILE 'k'
    bool member_is_output_semantic_analysis_file;
    // TP_CONFIG_OPTION_IS_OUTPUT_LOG_FILE 'l'
    bool member_is_output_log_file;
    // TP_CONFIG_OPTION_IS_NO_OUTPUT_MESSAGES 'm'
    bool member_is_no_output_messages;
    // TP_CONFIG_OPTION_IS_NO_OUTPUT_FILES 'n'
    bool member_is_no_output_files;
    // TP_CONFIG_OPTION_IS_DISABLE_TEST_OF_CALC_FIRST_SET_AND_FOLLOW_SET 'p'
    bool member_is_need_test_first_follow_set;
    // TP_CONFIG_OPTION_IS_CALC_FIRST_SET_AND_FOLLOW_SET 'q'
    bool member_is_calc_first_follow_set;
    // TP_CONFIG_OPTION_IS_ORIGIN_WASM 'r'
    bool member_is_origin_wasm;
    // TP_CONFIG_OPTION_IS_SOURCE_CMD_PARAM 's'
    bool member_is_source_cmd_param;
    uint8_t member_source_code[TP_SOURCE_CODE_STRING_BUFFER_SIZE];
    // TP_CONFIG_OPTION_IS_TEST_CASE 'T'
    bool member_is_test_case;
    size_t member_test_case_index;
    // TP_CONFIG_OPTION_IS_TEST_MODE 't'
    bool member_is_test_mode;
    // TP_CONFIG_OPTION_IS_WCHAR_T_UTF_32 'u'
    bool member_is_wchar_t_UTF_32;
    // TP_CONFIG_OPTION_IS_OUTPUT_WASM_FILE 'w'
    bool member_is_output_wasm_file;
    // TP_CONFIG_OPTION_IS_OUTPUT_X64_FILE 'x'
    bool member_is_output_x64_file;
    // TP_CONFIG_OPTION_IS_OUTPUT_WASM_TEXT_FILE 'y'
    bool member_is_output_wasm_text_file;
    // TP_CONFIG_OPTION_IS_OUTPUT_PE_COFF_FILE 'z'
    bool member_is_output_pe_coff_file;

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
    rsize_t member_error_reason_count_prev;
    rsize_t member_error_reason_count;

// output file section:
    FILE* member_write_log_file;
    FILE* member_parse_tree_file;
    char member_write_log_file_path[_MAX_PATH];
    char member_test_src_file_path[_MAX_PATH];
    char member_read_unicode_data_path[_MAX_PATH];
    char member_unicode_data_path[_MAX_PATH];
    char member_unicode_data_index_path[_MAX_PATH];
    char member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_NUM][_MAX_PATH];
    char member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_NUM][_MAX_PATH];
    char member_token_file_path[_MAX_PATH];
    char member_pp_token_file_path[_MAX_PATH];
    char member_parse_tree_file_path[_MAX_PATH];
    char member_object_hash_file_path[_MAX_PATH];
    char member_wasm_file_path[_MAX_PATH];
    char member_wasm_text_file_path[_MAX_PATH];
    char member_x64_file_path[_MAX_PATH];
    char member_x64_text_file_path[_MAX_PATH];
    char member_coff_file_path[_MAX_PATH];
    char member_coff_text_file_path[_MAX_PATH];
    char member_coff_code_file_path[_MAX_PATH];
    char member_coff_code_text_file_path[_MAX_PATH];
    char member_pe_file_path[_MAX_PATH];
    char member_pe_text_file_path[_MAX_PATH];
    char member_pe_code_file_path[_MAX_PATH];
    char member_pe_code_text_file_path[_MAX_PATH];
    char member_load_dll_file_path[_MAX_PATH];

// input file section:
    TP_INPUT_FILE member_input_file;

// token section:
    TP_TOKEN* member_tp_token; // member_string is allways NULL.
    rsize_t member_tp_token_pos;
    rsize_t member_tp_token_size;
    rsize_t member_tp_token_size_allocate_unit;
    TP_TOKEN* member_tp_token_position;
    rsize_t member_nul_num;

// parse tree section:
    bool member_is_end_of_make_parse_tree;
    bool member_is_error_recovery;
    uint8_t member_nesting_level_of_expression;
    TP_PARSE_TREE* member_tp_parse_tree;

// calc first set and follow set section:
    TP_GRAMMER_TERM* member_first_set;
    TP_GRAMMER_TERM* member_follow_set;

// semantic analysis section:
    REGISTER_OBJECT_HASH member_object_hash;

    // int_calc_compiler
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

    // C compiler
    uint32_t member_var_global_index;
    uint32_t member_var_local_index;

//  uint8_t member_nesting_level_of_expression;
    uint8_t member_nesting_expression;
    bool member_is_nothing_return_statements;

    TP_C_OBJECT** member_c_object;
    rsize_t member_c_object_pos;
    rsize_t member_c_object_size;
    rsize_t member_c_object_size_allocate_unit;

    rsize_t member_c_object_size_local_allocate_unit;
    rsize_t member_c_expr_size_allocate_unit;

    TP_C_EXPR* member_c_expr_stack;
    int32_t member_c_expr_stack_pos;
    int32_t member_c_expr_stack_size;
    int32_t member_c_expr_stack_size_allocate_unit;

    TP_C_SCOPE member_c_scope;
    rsize_t member_c_scope_c_object_size_allocate_unit;
    rsize_t member_c_scope_stack_size_allocate_unit;

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
    uint32_t member_code_count;
    size_t member_code_index;
    uint32_t member_code_body_size;
    uint8_t* member_code_section_buffer;

    TP_WASM_RELOCATION* member_wasm_relocation;
    rsize_t member_wasm_relocation_begin;
    rsize_t member_wasm_relocation_cur;
    rsize_t member_wasm_relocation_pos;
    rsize_t member_wasm_relocation_size;
    rsize_t member_wasm_relocation_allocate_unit;

    // The number of elements in member_func_local_types is
    // member_code_count or member_wasm_function_count.
    TP_WASM_FUNC_LOCAL_TYPE* member_func_local_types;
    rsize_t member_func_local_types_current;

    // type section
    uint32_t member_wasm_type_count;
    TP_WASM_SECTION_TYPE_VAR* member_wasm_types;

    // function section
    uint32_t member_wasm_function_count;
    uint32_t* member_wasm_functions;

    // export section
    uint32_t member_wasm_export_count;
    TP_WASM_SECTION_EXPORT_VAR* member_wasm_exports;

// x64 section:
    uint8_t* member_wasm_code_body_buffer;
    uint32_t member_wasm_code_body_pos;
    uint32_t member_wasm_code_body_size;
    uint32_t member_wasm_code_body_size_current;

    TP_WASM_STACK_ELEMENT* member_stack;
    int32_t member_stack_pos;
    int32_t member_stack_size;
    int32_t member_stack_size_allocate_unit;

    TP_WASM_STACK_ELEMENT member_use_X86_32_register[TP_X86_32_REGISTER_NUM];
    TP_WASM_STACK_ELEMENT member_use_X64_32_register[TP_X64_32_REGISTER_NUM];

    // The number of elements in member_x64_func_local is
    // member_wasm_function_count.
    TP_X64_FUNC_LOCAL* member_x64_func_local;
    rsize_t member_x64_func_index;
    int32_t member_local_variable_size_max;
    int32_t member_temporary_variable_size_max;
    uint32_t member_x64_call_num;

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

const TP_TOKEN global_pp_token_template;

// ----------------------------------------------------------------------------------------
// Main section:
bool tp_compiler(int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size);
bool tp_compiler_main(
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    bool* is_test_mode, bool* is_int_calc_compiler, size_t* test_case_index,
    size_t test_index, int32_t* return_value, char* drive, char* dir, time_t now
);
bool tp_test_compiler(
    bool is_int_calc_compiler, size_t test_case_index,
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    char* drive, char* dir, time_t now
);
bool tp_init_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, int argc, char** argv, bool* is_disp_usage,
    bool* is_test, bool* is_int_calc_compiler, size_t* test_case_index,
    size_t test_index, time_t now, char* drive, char* dir,
    uint8_t* msg_buffer, size_t msg_buffer_size
);
void tp_free_memory_and_file(TP_SYMBOL_TABLE** symbol_table);
void tp_free_c_object(TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT** c_object);
bool tp_make_path_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param, char* fname,
    bool is_test, bool is_int_calc_compiler, size_t test_index, time_t now
);
bool tp_make_path(
    TP_SYMBOL_TABLE* symbol_table,
    char* drive, char* dir, char* prefix, char* fname, char* ext,
    char* path, size_t path_size
);
bool tp_get_drive_dir(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir
);
bool tp_get_current_drive_dir(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir
);

// ----------------------------------------------------------------------------------------
// token section:
bool tp_make_token(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length);
bool tp_dump_token_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_TOKEN* token, uint8_t indent_level
);

bool tp_lexer(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
bool tp_append_pp_token(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_TOKEN* pp_token);
bool tp_getchar_pos_of_physical_line(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_CHAR8_T** char_pos, bool* is_end_of_logical_line
);
TP_LOGICAL_LINES* tp_get_logical_line(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
void tp_check_end_of_source_code(
    TP_INPUT_FILE* input_file, bool is_end_logical_line, bool* is_end_of_source_code
);
bool tp_make_pp_token_white_space(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);
bool tp_make_pp_token_character_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);
bool tp_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match
);
bool tp_universal_character_name(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    bool is_start_char, TP_CHAR8_T current_char, bool* is_match
);
bool tp_make_pp_token_string_literal(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);
bool tp_make_pp_token_identifier(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);
bool tp_identifier_nondigit(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    bool is_start_char, TP_CHAR8_T current_char, bool* is_match
);
bool tp_make_pp_token_pp_number(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);
bool tp_make_pp_token_punctuator(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);

bool tp_preprocessor(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
bool tp_do_preprocess(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
bool tp_convert_escape_sequence(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
bool tp_convert_pp_number(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);

// ----------------------------------------------------------------------------------------
// parse tree section:
bool tp_make_parse_tree(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_make_parse_subtree(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER grammer,
    TP_PARSE_TREE_ELEMENT* parse_tree_element, size_t parse_tree_element_num
);
void tp_free_parse_subtree(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE** parse_subtree);
bool tp_print_parse_tree_grammer(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, FILE* write_file, uint8_t* indent_string
);

TP_PARSE_TREE* tp_parse_translation_unit(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_statement(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_compound_statement(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_expression(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_constant_expression(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_assignment_expression(TP_SYMBOL_TABLE* symbol_table);

// Grammer(declaration):
TP_PARSE_TREE* tp_parse_declaration(TP_SYMBOL_TABLE* symbol_table);
// Grammer(type-name):
TP_PARSE_TREE* tp_parse_type_name(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_type_specifier(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_pointer(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_type_qualifier_list(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_type_qualifier(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_static_assert_declaration(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_parameter_type_list(TP_SYMBOL_TABLE* symbol_table);

TP_PARSE_TREE* tp_parse_initializer_list(TP_SYMBOL_TABLE* symbol_table);
TP_PARSE_TREE* tp_parse_declaration_specifiers(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* prev_node);
TP_PARSE_TREE* tp_parse_declarator(TP_SYMBOL_TABLE* symbol_table);

bool tp_set_error_reason(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* error_message, TP_CHAR8_T* error_reason
);
bool tp_put_error_message(TP_SYMBOL_TABLE* symbol_table);
bool tp_parse_error_recovery(TP_SYMBOL_TABLE* symbol_table);

bool tp_calc_first_follow_set_entry_point(TP_SYMBOL_TABLE* symbol_table);

bool tp_calc_empty(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num);
bool tp_calc_first_set(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num, bool is_generate);
bool tp_is_contained_empty_first_set(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer);
bool tp_calc_follow_set(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num, bool is_generate);

TP_GRAMMER** tp_make_grammer_1(TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num);
bool tp_test_grammer_1(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num);

TP_GRAMMER** tp_make_grammer_2(TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num);
bool tp_test_grammer_2(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num);

TP_GRAMMER** tp_make_grammer_3(TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num);
bool tp_test_grammer_3(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num);

TP_GRAMMER** tp_make_grammer(
    TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num_out, TP_GRAMMER** grammer, rsize_t grammer_num
);
TP_GRAMMER* tp_make_grammer_element(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER grammer, TP_GRAMMER_ELEMENT* grammer_element, rsize_t grammer_element_num
);
void tp_free_grammer(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER*** grammer, rsize_t grammer_element_num);
bool tp_print_grammer(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_PARSE_TREE_GRAMMER grammer);
bool tp_print_symbol(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SYMBOL symbol);
bool tp_compare_first_or_follow_set(
    TP_GRAMMER_TERM* test_first_or_follow_set, rsize_t test_first_or_follow_set_num,
    TP_GRAMMER_TERM* first_or_follow_set
);
bool tp_get_grammer_1_first_set(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER_FIRST_FOLLOW_SET_INDEX index, TP_PARSE_TREE_GRAMMER grammer
);
bool tp_get_grammer_1_follow_set(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER_FIRST_FOLLOW_SET_INDEX index, TP_PARSE_TREE_GRAMMER grammer
);

// ----------------------------------------------------------------------------------------
// semantic analysis section:
bool tp_semantic_analysis(TP_SYMBOL_TABLE* symbol_table);
bool tp_register_defined_variable(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace, TP_C_OBJECT* c_object
);
bool tp_register_undefined_variable(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace, REGISTER_OBJECT* object
);
void tp_free_object_hash(
    TP_SYMBOL_TABLE* symbol_table,
    REGISTER_OBJECT_HASH* object_hash, REGISTER_OBJECT_HASH_ELEMENT* hash_element
);
// Convert parse tree to C IR.
bool tp_make_C_IR(TP_SYMBOL_TABLE* symbol_table);
bool tp_push_c_namespace_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context
);
bool tp_pop_c_namespace_stack(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context
);
bool tp_append_c_object(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object
);
// Convert parse tree to C IR(translation_unit).
bool tp_make_C_IR_translation_unit(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
// Convert parse tree to C IR(external declarations).
bool tp_make_C_IR_external_declaration(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
// Convert parse tree to C IR(function definition).
bool tp_make_C_IR_function_definition(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
// Convert parse tree to C IR(statements).
bool tp_make_C_IR_statements(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
bool tp_append_c_object_to_compound_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* parent_c_object, TP_C_OBJECT* c_object
);
bool tp_make_C_IR_expression_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
bool tp_make_C_IR_jump_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
// Convert parse tree to C IR(declarations).
bool tp_make_C_IR_declaration(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
bool tp_make_C_IR_declaration_specifiers(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
bool tp_check_C_IR_type_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_OBJECT* type
);
bool tp_make_C_IR_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
// Convert parse tree to C IR(direct declarator).
bool tp_make_C_IR_direct_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
// Convert parse tree to C IR(initializer).
bool tp_make_C_IR_initializer(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type, TP_TOKEN* token_equal
);
// Evaluate of C IR(expressions).
bool tp_make_C_IR_eval(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_OBJECT* c_object
);
// C compiler: expression
bool tp_make_C_IR_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** stack_c_object,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
bool tp_make_C_IR_assignment_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT** c_type_func,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
bool tp_c_return_type_check(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* c_object, TP_C_TYPE* c_type, TP_C_TYPE* c_return_type
);
bool tp_get_type(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object, TP_EXPR_TYPE* type
);
bool tp_append_c_expr(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object, TP_C_EXPR* c_expr_param
);
// C compiler: expression(primary expression)
bool tp_make_C_IR_primary_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object,
    bool is_exec_expr_id, TP_C_OBJECT** stack_c_object,
    int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(postfix expression)
bool tp_make_C_IR_postfix_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(one operand)
bool tp_make_C_IR_unary_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
bool tp_make_C_IR_cast_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(conditional expression)
bool tp_make_C_IR_conditional_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(two operands: logical)
bool tp_make_C_IR_logical_OR_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(two operands: bit)
bool tp_make_C_IR_inclusive_OR_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(two operands: arithmetic)
bool tp_make_C_IR_additive_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);
// C compiler: expression(two operands)
bool tp_make_C_IR_equality_expression(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, int32_t* func_arg_index, bool is_ignore_ABI,
    uint32_t* function_call_depth
);

// int_calc_compiler
bool tp_get_var_value(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, size_t index, uint32_t* var_value
);
bool tp_variable_reference_check_expr(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context
);

// ----------------------------------------------------------------------------------------
// wasm section:
bool tp_make_wasm(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, bool is_origin_wasm
);
// C compiler
TP_WASM_MODULE_SECTION* tp_make_wasm_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol
);
bool tp_make_wasm_C_body(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE* type_return,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter
);
// C compiler: declarations
bool tp_make_wasm_C_declaration_statement(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type
);
bool tp_make_wasm_C_basic(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type, TP_C_TYPE_BASIC* type_basic
);
bool tp_make_wasm_C_struct(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type, TP_C_TYPE_STRUCT* type_struct
);
bool tp_make_wasm_C_pointer(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_TYPE kind_of_type, TP_C_TYPE_POINTER* type_pointer
);
// C compiler: expression
bool tp_make_wasm_C_expression(
    TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT* c_object,
    uint8_t* section_buffer, size_t* code_index
);
// C compiler: statement
bool tp_make_wasm_C_iteration_statement_do(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_ITERATION_STATEMENT_DO* type_iteration_statement_do
);
bool tp_make_wasm_C_jump_statement_return(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_JUMP_STATEMENT_RETURN* type_jump_statement_return,
    TP_C_TYPE* type_return, uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter
);
// C compiler: function definition
bool tp_make_wasm_C_function(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_FUNCTION* type_function, TP_C_DECL* c_decl
);
bool tp_make_wasm_C_compound_statement(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION* section, TP_C_OBJECT* c_object,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement,
    uint32_t* function_body_offset, uint32_t* function_body_size,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    TP_C_GLOBAL_FUNCTION* global_function
);
bool tp_get_base_local_index(
    TP_SYMBOL_TABLE* symbol_table, uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    uint32_t* base_local_index, uint32_t* var_count_i64, uint32_t* var_count_i32
);
// wasm/coff
bool tp_wasm_add_function_type_C(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer,
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement, bool is_export,
    TP_C_TYPE* type_return, TP_TOKEN* function_name,
    uint32_t parameter_num, TP_C_TYPE_FUNCTION_F_PARAM* parameter,
    TP_C_GLOBAL_FUNCTION* global_function
);
TP_WASM_MODULE_SECTION* tp_make_section_type_C(TP_SYMBOL_TABLE* symbol_table);
TP_WASM_MODULE_SECTION* tp_make_section_function_C(TP_SYMBOL_TABLE* symbol_table);
TP_WASM_MODULE_SECTION* tp_make_section_export_C(TP_SYMBOL_TABLE* symbol_table);
bool tp_get_wasm_export_code_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION** code_section, uint32_t* return_type
);
bool tp_disasm_wasm(TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol);
bool tp_make_wasm_expr(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section, bool is_origin_wasm
);
bool tp_make_wasm_coff(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section,
    uint8_t* entry_point_symbol, bool is_origin_wasm
);
bool tp_wasm_add_global_variable_i32(
    TP_SYMBOL_TABLE* symbol_table, bool is_external,
    uint32_t var_id, uint8_t* symbol, rsize_t symbol_length,
    uint32_t init_value, uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index
);
bool tp_wasm_add_global_variable_i64(
    TP_SYMBOL_TABLE* symbol_table, bool is_external,
    uint32_t var_id, uint8_t* symbol, rsize_t symbol_length,
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
    TP_SYMBOL_TABLE* symbol_table, bool is_external,
    uint8_t* symbol, rsize_t symbol_length, uint32_t* symbol_index
);
bool tp_wasm_add_relocation(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_RELOCATION* wasm_relocation
);
bool tp_wasm_get_relocation(
    TP_SYMBOL_TABLE* symbol_table, rsize_t begin_index, TP_WASM_RELOCATION** wasm_relocation
);
// UEFI
TP_WASM_MODULE_SECTION* tp_make_section_type_UEFI(TP_SYMBOL_TABLE* symbol_table);
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
// UEFI: original wasm64
TP_WASM_MODULE_SECTION* tp_make_section_code_origin_wasm_UEFI64(TP_SYMBOL_TABLE* symbol_table);

// Control flow operators
uint32_t tp_make_loop_code(uint8_t* buffer, size_t offset, int32_t sig);
uint32_t tp_make_end_code(uint8_t* buffer, size_t offset);
uint32_t tp_make_br_if_code(uint8_t* buffer, size_t offset, uint32_t relative_depth);
uint32_t tp_make_ret_code(uint8_t* buffer, size_t offset);

// Call operators
uint32_t tp_make_call_code(uint8_t* buffer, size_t offset, uint32_t function_index);
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
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, int32_t* return_value
);
bool tp_wasm_stack_push(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_ELEMENT* value);
TP_WASM_STACK_ELEMENT tp_wasm_stack_pop(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_POP_MODE pop_mode,
    uint32_t* offset
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
    TP_WASM_RELOCATION* wasm_relocation, bool* status
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
    uint32_t function_or_type_index, uint32_t reserved, TP_WASM_STACK_ELEMENT* op1
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
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1
);
uint32_t tp_encode_x64_1_opcode(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint8_t opcode
);

// ----------------------------------------------------------------------------------------
// PE COFF section:
bool tp_make_COFF(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size, bool is_output
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
    TP_SYMBOL_TABLE* symbol_table,
    bool is_external, TP_SECTION_KIND section_kind,
    uint8_t* symbol, rsize_t symbol_length,
    uint32_t offset_value, uint32_t* symbol_index
);
bool tp_set_function_offset_to_symbol(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t func_index, uint32_t text_size, uint32_t offset
);

bool tp_make_PE_file(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol, bool* is_read
);

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

// disasm_x64
bool tp_test_disasm_x64(TP_SYMBOL_TABLE* symbol_table);
bool tp_disasm_x64(
    TP_SYMBOL_TABLE* symbol_table, char* path,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint8_t* disasm_string, uint8_t* disasm_additional_string
);
// PE loader
bool tp_load_and_exec_dll(
    TP_SYMBOL_TABLE* symbol_table, char* func_name, int* return_value
);
bool tp_load_dll(
    TP_SYMBOL_TABLE* symbol_table, uint8_t** pe_buffer, rsize_t* pe_buffer_size
);

// ----------------------------------------------------------------------------------------
// Utilities section:

// File
bool tp_open_read_file(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_open_read_file_text(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_open_write_file(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_open_write_file_text(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream);
bool tp_ftell(TP_SYMBOL_TABLE* symbol_table, FILE* file_stream, long* seek_position);
bool tp_seek(TP_SYMBOL_TABLE* symbol_table, FILE* file_stream, long seek_position, long line_bytes);
bool tp_close_file(TP_SYMBOL_TABLE* symbol_table, FILE** file_stream);
bool tp_write_file(TP_SYMBOL_TABLE* symbol_table, char* path, void* content, uint32_t content_size);

// Unicode
bool tp_is_valid_utf_8(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* lines_buffer, size_t read_lines_length,
    long* utf_8_restart_pos
);
bool tp_encode_utf_8(
    TP_SYMBOL_TABLE* symbol_table, uint32_t value, TP_CHAR8_T* string_buffer, rsize_t* total_bytes
);
bool tp_decode_utf_8(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_32_STRING* utf_32_string, TP_CHAR8_T* string_buffer, rsize_t string_length
);
bool tp_decode_utf_8_char(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T* value32, TP_CHAR8_T* string_buffer, rsize_t string_length
);
bool tp_encode_utf_16(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_16_STRING* utf_16_string, TP_CHAR8_T* string_buffer, rsize_t string_length
);
bool tp_encode_ucs_2_char(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR16_T* value16, TP_CHAR8_T* string_buffer, rsize_t string_length
);
bool tp_decode_utf_16(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_16_STRING* utf_16_string, TP_TOKEN_UTF_8_STRING* utf_8_string
);
bool tp_decode_ucs_2_char(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR16_T value16, TP_TOKEN_UTF_8_STRING* utf_8_string
);
bool tp_decode_utf_32(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_32_STRING* utf_32_string, TP_TOKEN_UTF_8_STRING* utf_8_string
);
bool tp_decode_utf_32_char(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T value32, TP_TOKEN_UTF_8_STRING* utf_8_string
);
bool tp_unicode_data_gen(TP_SYMBOL_TABLE* symbol_table, TP_UNICODE_DATA_GEN_KIND kind);
bool tp_test_unicode_data_index(TP_SYMBOL_TABLE* symbol_table);
bool tp_is_numeric_code_point(TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T code_point, bool* is_numeric);

// LEB128
uint32_t tp_encode_si64leb128(uint8_t* buffer, size_t offset, int64_t value);
uint32_t tp_encode_ui32leb128(uint8_t* buffer, size_t offset, uint32_t value);
int64_t tp_decode_si64leb128(uint8_t* buffer, uint32_t* size);
int32_t tp_decode_si32leb128(uint8_t* buffer, uint32_t* size);
uint32_t tp_decode_ui32leb128(uint8_t* buffer, uint32_t* size);

// Utilities
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

