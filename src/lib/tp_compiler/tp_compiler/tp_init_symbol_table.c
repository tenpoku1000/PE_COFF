
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static TP_SYMBOL_TABLE init_symbol_table_value = {
// config section:
    // TP_CONFIG_OPTION_IS_TEST_DISASM_X64 'a'
    .member_is_test_disasm_x64 = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_TOKEN_FILE 'b'
    .member_is_output_token_file = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_CURRENT_DIR 'c'
    .member_is_output_current_dir = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_UNICODE_CHARACTER_DATABASE_FILE 'd'
    .member_is_output_unicode_character_database = false,
    // TP_CONFIG_OPTION_IS_TEST_UNICODE_CHARACTER_DATABASE 'e'
    .member_is_test_unicode_character_database = false,
    // TP_CONFIG_OPTION_IS_LOAD_AND_EXEC_DLL 'f'
    .member_is_load_and_exec_dll = false,
    // TP_CONFIG_OPTION_IS_IMAGE_SUBSYSTEM_EFI_APPLICATION 'g'
    .member_subsystem = TP_IMAGE_SUBSYSTEM_WINDOWS_GUI,
    // TP_CONFIG_OPTION_IS_HELP 'h'
//
    // TP_CONFIG_OPTION_IS_INT_CALC_COMPILER 'i'
    .member_is_int_calc_compiler = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_PARSE_TREE_FILE 'j'
    .member_is_output_parse_tree_file = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_SEMANTIC_ANALYSIS_FILE 'k'
    .member_is_output_semantic_analysis_file = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_LOG_FILE 'l'
    .member_is_output_log_file = false,
    // TP_CONFIG_OPTION_IS_NO_OUTPUT_MESSAGES 'm'
    .member_is_no_output_messages = false,
    // TP_CONFIG_OPTION_IS_NO_OUTPUT_FILES 'n'
    .member_is_no_output_files = false,
    // TP_CONFIG_OPTION_IS_DISABLE_TEST_OF_CALC_FIRST_SET_AND_FOLLOW_SET 'p'
    .member_is_need_test_first_follow_set = true,
    // TP_CONFIG_OPTION_IS_CALC_FIRST_SET_AND_FOLLOW_SET 'q'
    .member_is_calc_first_follow_set = false,
    // TP_CONFIG_OPTION_IS_ORIGIN_WASM 'r'
    .member_is_origin_wasm = false,
    // TP_CONFIG_OPTION_IS_SOURCE_CMD_PARAM 's'
    .member_is_source_cmd_param = false,
    .member_source_code = { 0 },
    // TP_CONFIG_OPTION_IS_TEST_CASE 'T'
    .member_is_test_case = false,
    .member_test_case_index = 0,
    // TP_CONFIG_OPTION_IS_TEST_MODE 't'
    .member_is_test_mode = false,
    // TP_CONFIG_OPTION_IS_WCHAR_T_UTF_32 'u'
    .member_is_wchar_t_UTF_32 = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_WASM_FILE 'w'
    .member_is_output_wasm_file = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_X64_FILE 'x'
    .member_is_output_x64_file = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_WASM_TEXT_FILE 'y'
    .member_is_output_wasm_text_file = false,
    // TP_CONFIG_OPTION_IS_OUTPUT_PE_COFF_FILE 'z'
    .member_is_output_pe_coff_file = false,

// message section:
    .member_log_hide_after_disp = false,
    .member_temp_buffer = { 0 },
    .member_log_msg_buffer = { 0 },
    .member_disp_log_file = NULL,

    .member_is_error_abort = false,
    .member_error_count_prev = 0,
    .member_error_count = 0,
    .member_warning_count = 0,

    .member_error_token = NULL,
    .member_error_message = NULL,
    .member_error_reason = NULL,
    .member_error_count_prev = 0,
    .member_error_count = 0,

// output file section:
    .member_write_log_file = NULL,
    .member_parse_tree_file = NULL,
    .member_write_log_file_path = { 0 },
    .member_test_src_file_path = { 0 },
    .member_read_unicode_data_path = { 0 },
    .member_unicode_data_path = { 0 },
    .member_unicode_data_index_path = { 0 },
    .member_first_set_path = { 0 },
    .member_follow_set_path = { 0 },
    .member_token_file_path = { 0 },
    .member_pp_token_file_path = { 0 },
    .member_parse_tree_file_path = { 0 },
    .member_object_hash_file_path = { 0 },
    .member_wasm_file_path = { 0 },
    .member_wasm_text_file_path = { 0 },
    .member_x64_file_path = { 0 },
    .member_x64_text_file_path = { 0 },
    .member_coff_file_path = { 0 },
    .member_coff_text_file_path = { 0 },
    .member_coff_code_file_path = { 0 },
    .member_coff_code_text_file_path = { 0 },
    .member_pe_file_path = { 0 },
    .member_pe_text_file_path = { 0 },
    .member_pe_code_file_path = { 0 },
    .member_pe_code_text_file_path = { 0 },
    .member_load_dll_file_path = { 0 },

// input file section:
    .member_input_file.member_input_file_path = { 0 },
    .member_input_file.member_read_file = NULL,
    .member_input_file.member_is_start_of_file = true,
    .member_input_file.member_is_end_of_file = false,
    .member_input_file.member_read_lines_buffer = { 0 },
    .member_input_file.member_read_lines_length = 0,

    // from memory:
    .member_input_file.member_is_from_memory = false,
    .member_input_file.member_string_memory = NULL,
    .member_input_file.member_string_memory_length = 0,

    // physical line:
    .member_input_file.member_physical_lines = NULL,
    .member_input_file.member_physical_lines_pos = 0,
    .member_input_file.member_physical_lines_size = 0,
    .member_input_file.member_physical_lines_size_allocate_unit = TP_PHYSICAL_LINES_SIZE_ALLOCATE_UNIT,
    .member_input_file.member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_NONE,

    // logical line:
    .member_input_file.member_logical_lines = NULL,
    .member_input_file.member_logical_lines_pos = 0,
    .member_input_file.member_logical_lines_size = 0,
    .member_input_file.member_logical_lines_size_allocate_unit = TP_LOGICAL_LINES_SIZE_ALLOCATE_UNIT,

    .member_input_file.member_logical_lines_current_position = 0,

    // lexer:
    .member_input_file.member_logical_line = 0,
    .member_input_file.member_physical_line = 0,
    .member_input_file.member_physical_column = 0,

    .member_input_file.member_physical_lines_current_line_position = 0,
    .member_input_file.member_physical_lines_current_column_position = 0,

    .member_input_file.member_lexer_stack = NULL,
    .member_input_file.member_lexer_stack_pos = TP_LEXER_STACK_EMPTY,
    .member_input_file.member_lexer_stack_size = 0,
    .member_input_file.member_lexer_stack_size_allocate_unit = TP_LEXER_STACK_SIZE_ALLOCATE_UNIT,

    .member_input_file.member_string = NULL,
    .member_input_file.member_string_pos = 0,
    .member_input_file.member_string_size = 0,
    .member_input_file.member_string_size_allocate_unit = TP_STRING_SIZE_ALLOCATE_UNIT,

    // preprocessor token:
    .member_input_file.member_tp_pp_token = NULL,
    .member_input_file.member_tp_pp_token_pos = 0,
    .member_input_file.member_tp_pp_token_size = 0,
    .member_input_file.member_tp_pp_token_size_allocate_unit = TP_TOKEN_SIZE_ALLOCATE_UNIT,

// token section:
    .member_tp_token = NULL,
    .member_tp_token_pos = 0,
    .member_tp_token_size = 0,
    .member_tp_token_size_allocate_unit = TP_TOKEN_SIZE_ALLOCATE_UNIT,
    .member_tp_token_position = NULL,
    .member_nul_num = 0,

// parse tree section:
    .member_is_end_of_make_parse_tree = false,
    .member_is_error_recovery = false,
    .member_nesting_level_of_expression = 0,
    .member_tp_parse_tree = NULL,

// calc first set and follow set section:
    .member_first_set = NULL,
    .member_follow_set = NULL,

// semantic analysis section:
    .member_object_hash.member_mask = UINT8_MAX,
    .member_object_hash.member_hash_table = { 0 },

    // int_calc_compiler
    .member_var_count = 0,
    .member_last_statement = NULL,
    .member_parse_tree_type = {
        // TP_GRAMMER_TYPE_INDEX_STATEMENT_1, Grammer: Statement -> variable '=' Expression ';'
        { TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_STATEMENT_2, Grammer: Statement -> Type variable '=' Expression ';'
        { TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN },
        // TP_GRAMMER_TYPE_INDEX_EXPRESSION_1, Grammer: Expression -> Term (('+' | '-') Term)*
        { TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_EXPRESSION_2, Grammer: Expression -> Term (('+' | '-') Term)*
        { TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_TERM_1, Grammer: Term -> Factor (('*' | '/') Factor)*
        { TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_TERM_2, Grammer: Term -> Factor (('*' | '/') Factor)*
        { TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_FACTOR_1, Grammer: Factor -> '(' Expression ')'
        { TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NODE, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_FACTOR_2, Grammer: Factor -> ('+' | '-') (variable | constant)
        { TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // TP_GRAMMER_TYPE_INDEX_FACTOR_3, Grammer: Factor -> variable | constant
        { TP_PARSE_TREE_TYPE_TOKEN, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL },
        // NULL
        { TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL, TP_PARSE_TREE_TYPE_NULL }
    },
    .member_grammer_statement_1_num = 0,
    .member_grammer_statement_2_num = 0,
    .member_grammer_expression_1_num = 0,
    .member_grammer_expression_2_num = 0,
    .member_grammer_term_1_num = 0,
    .member_grammer_term_2_num = 0,
    .member_grammer_factor_1_num = 0,
    .member_grammer_factor_2_num = 0,
    .member_grammer_factor_3_num = 0,

    // C compiler
    .member_var_global_index = 0,
    .member_var_local_index = 0,

//  .member_nesting_level_of_expression = 0,
    .member_nesting_expression = 0,
    .member_is_nothing_return_statements = true,

    .member_c_object = NULL,
    .member_c_object_pos = 0,
    .member_c_object_size = 0,
    .member_c_object_size_allocate_unit = TP_C_OBJECT_SIZE_ALLOCATE_UNIT,

    .member_c_object_size_local_allocate_unit = TP_C_OBJECT_SIZE_LOCAL_ALLOCATE_UNIT,
    .member_c_expr_size_allocate_unit = TP_C_EXPR_SIZE_ALLOCATE_UNIT,

    .member_c_expr_stack = NULL,
    .member_c_expr_stack_pos = TP_C_EXPR_STACK_EMPTY,
    .member_c_expr_stack_size = 0,
    .member_c_expr_stack_size_allocate_unit = TP_C_EXPR_STACK_SIZE_ALLOCATE_UNIT,

    .member_c_scope = { 0 },
    .member_c_scope_c_object_size_allocate_unit = TP_C_SCOPE_C_OBJECT_SIZE_ALLOCATE_UNIT,
    .member_c_scope_stack_size_allocate_unit = TP_C_SCOPE_STACK_SIZE_ALLOCATE_UNIT,

// wasm section:
    .member_wasm_local_pos = 0,

    .member_wasm_data_offset_1 = 0,
    .member_wasm_data_size_1 = 0,
    .member_wasm_data_data_1 = NULL,

    .member_wasm_data_offset_2 = 0,
    .member_wasm_data_size_2 = 0,
    .member_wasm_data_data_2 = NULL,

    .member_wasm_module = { 0 },
    .member_code_count = 0,
    .member_code_index = 0,
    .member_code_body_size = 0,
    .member_code_section_buffer = NULL,

    .member_wasm_relocation = NULL,
    .member_wasm_relocation_begin = 0,
    .member_wasm_relocation_cur = 0,
    .member_wasm_relocation_pos = 0,
    .member_wasm_relocation_size = 0,
    .member_wasm_relocation_allocate_unit = TP_WASM_RELOCATION_ALLOCATE_UNIT,

    .member_func_local_types = NULL,
    .member_func_local_types_current = 0,

    // type section
    .member_wasm_type_count = 0,
    .member_wasm_types = NULL,

    // function section
    .member_wasm_function_count = 0,
    .member_wasm_functions = NULL,

    // export section
    .member_wasm_export_count = 0,
    .member_wasm_exports = NULL,

// x64 section:
    .member_wasm_code_body_buffer = NULL,
    .member_wasm_code_body_pos = 0,
    .member_wasm_code_body_size = 0,
    .member_wasm_code_body_size_current = 0,

    .member_stack = NULL,
    .member_stack_pos = TP_WASM_STACK_EMPTY,
    .member_stack_size = 0,
    .member_stack_size_allocate_unit = TP_WASM_STACK_SIZE_ALLOCATE_UNIT,

    .member_use_X86_32_register = { 0 },
    .member_use_X64_32_register = { 0 },

    .member_x64_func_local = NULL,
    .member_x64_func_index = 0,
    .member_local_variable_size_max = TP_WASM_LOCAL_VARIABLE_MAX_DEFAULT,
    .member_temporary_variable_size_max = TP_WASM_TEMPORARY_VARIABLE_MAX_DEFAULT,
    .member_x64_call_num = 0,

// PE COFF section:
    .member_is_add_data_section = true,
    .member_is_add_rdata_section = true,
    .member_is_add_text_section = true,

    // PE COFF OBJECT/IMAGE File
    .member_pe_coff_buffer = NULL,
    .member_pe_coff_size = 0,
    .member_pe_coff_current_offset = 0,

    // COFF File
    .member_coff = { 0 },
    .member_coff.member_section_number_data = TP_IMAGE_SYM_UNDEFINED,
    .member_coff.member_section_number_rdata = TP_IMAGE_SYM_UNDEFINED,
    .member_coff.member_section_number_text = TP_IMAGE_SYM_UNDEFINED,
    .member_coff.member_data_size_allocate_unit = TP_COFF_DATA_SIZE_ALLOCATE_UNIT,
    .member_coff.member_rdata_size_allocate_unit = TP_COFF_RDATA_SIZE_ALLOCATE_UNIT,
    .member_coff.member_coff_symbol_size_allocate_unit = TP_COFF_SYMBOL_SIZE_ALLOCATE_UNIT,

    // PE File header
    .member_dos_header_read = NULL,
    .member_pe_header64_read = NULL,

    // COFF File Header
    .member_coff_file_header = NULL,

    // Section Table
    .member_section_table = NULL,
    .member_section_table_size = 0,
    .member_section_table_num = 0,

    // Section Data(Relocations)
    .member_coff_relocations = NULL,
    .member_coff_relocations_size = 0,

    // COFF Symbol Table
    .member_coff_symbol_table = NULL,

    // COFF String Table
    .member_string_table_offset = 0,
    .member_string_table_size = 0,
    .member_string_table = NULL,
};

static void free_c_type(TP_SYMBOL_TABLE* symbol_table, TP_C_TYPE** c_type);
static void free_c_expr(TP_SYMBOL_TABLE* symbol_table, TP_C_TYPE** c_type);
static uint32_t calc_grammer_type_num(TP_SYMBOL_TABLE* symbol_table, size_t grammer_type_index);
static bool parse_cmd_line_param(
    int argc, char** argv, TP_SYMBOL_TABLE* symbol_table, bool* is_disp_usage,
    bool* is_test, bool* is_int_calc_compiler, size_t* test_case_index,
    char* drive, char* dir, char* fname
);

bool tp_init_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, int argc, char** argv, bool* is_disp_usage,
    bool* is_test, bool* is_int_calc_compiler, size_t* test_case_index,
    size_t test_index, time_t now, char* drive, char* dir,
    uint8_t* msg_buffer, size_t msg_buffer_size)
{
    errno_t err = 0;

    *symbol_table = init_symbol_table_value;

    symbol_table->member_disp_log_file = stderr;

    if (0 != setvbuf(symbol_table->member_disp_log_file, msg_buffer, _IOFBF, msg_buffer_size)){

        TP_PRINT_CRT_ERROR(NULL);

        return false;
    }

    symbol_table->member_grammer_statement_1_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_STATEMENT_1);
    symbol_table->member_grammer_statement_2_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_STATEMENT_2);
    symbol_table->member_grammer_expression_1_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_EXPRESSION_1);
    symbol_table->member_grammer_expression_2_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_EXPRESSION_2);
    symbol_table->member_grammer_term_1_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_TERM_1);
    symbol_table->member_grammer_term_2_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_TERM_2);
    symbol_table->member_grammer_factor_1_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_FACTOR_1);
    symbol_table->member_grammer_factor_2_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_FACTOR_2);
    symbol_table->member_grammer_factor_3_num = calc_grammer_type_num(symbol_table, TP_GRAMMER_TYPE_INDEX_FACTOR_3);

    if (NULL == is_test){

        TP_PUT_LOG_MSG_ICE(NULL);

        return false;
    }

    bool is_normal = false;

    if (false == *is_test){

        is_normal = true;
    }

    char fname[_MAX_FNAME] = { 0 };

    if ( ! parse_cmd_line_param(
        argc, argv, symbol_table, is_disp_usage,
        is_test, is_int_calc_compiler, test_case_index, drive, dir, fname)){

        return false;
    }

    if (*is_test && is_normal){

        // switch to test mode.
        return true;
    }

    if ( ! tp_make_path_log_files(
        symbol_table, drive, dir, fname,
        *is_test, *is_int_calc_compiler, test_index, now)){

        return false;
    }

    return true;
}

void tp_free_memory_and_file(TP_SYMBOL_TABLE** symbol_table)
{
    if (NULL == symbol_table){

        TP_PUT_LOG_MSG_ICE(NULL);

        return;
    }

// config section:

// message section:

// input file section:

    TP_INPUT_FILE* input_file = &((*symbol_table)->member_input_file);

    // physical line:

    TP_CHAR8_T** physical_lines = input_file->member_physical_lines;
    rsize_t physical_lines_pos = input_file->member_physical_lines_pos;

    for (rsize_t i = 0; physical_lines_pos > i; ++i){

        TP_CHAR8_T* line_buffer = physical_lines[i];
        size_t length = strlen(line_buffer);

        TP_FREE(*symbol_table, &line_buffer, length);
        input_file->member_physical_lines[i] = NULL;
    }
    TP_FREE2(
        *symbol_table, &(input_file->member_physical_lines),
        input_file->member_physical_lines_pos * sizeof(TP_CHAR8_T*)
    );

    // logical line:

    // NOTE: member_physical_lines release memory by double pointer only.
    if (input_file->member_logical_lines){

        rsize_t logical_lines_pos = input_file->member_logical_lines_pos;

        for (rsize_t i = 0; logical_lines_pos > i; ++i){

            TP_FREE2(
                *symbol_table, &(input_file->member_logical_lines[i].member_physical_lines),
                input_file->member_logical_lines[i].member_physical_lines_size
            );
        }

        TP_FREE(
            *symbol_table, &(input_file->member_logical_lines),
            input_file->member_logical_lines_size
        );
    }

    // lexer:

    TP_FREE(
        *symbol_table, &(input_file->member_lexer_stack),
        input_file->member_lexer_stack_size
    );

    TP_FREE(
        *symbol_table, &(input_file->member_string),
        input_file->member_string_pos
    );

    TP_TOKEN* pp_token = input_file->member_tp_pp_token;

    if (pp_token){

        rsize_t pp_token_pos = input_file->member_tp_pp_token_pos;

        for (rsize_t i = 0; pp_token_pos > i; ++i){

            if (pp_token[i].member_string){

                TP_FREE(*symbol_table, &(pp_token[i].member_string), pp_token[i].member_string_length);
            }
        }

        TP_FREE(
            *symbol_table, &(input_file->member_tp_pp_token),
            input_file->member_tp_pp_token_size
        );
    }

// token section:

    TP_TOKEN* token = (*symbol_table)->member_tp_token;
    
    if (token){

        rsize_t token_pos = (*symbol_table)->member_tp_token_pos;

        for (rsize_t i = 0; token_pos > i; ++i){

            if (token[i].member_string){

                TP_FREE(*symbol_table, &(token[i].member_string), token[i].member_string_length);
            }

            switch (token[i].member_value_type){
            case TP_VALUE_TYPE_UTF_8_STRING:
                TP_FREE(
                    *symbol_table, &(token[i].member_value.member_utf_8_string.member_string),
                    token[i].member_value.member_utf_8_string.member_string_length
                );
                break;
            case TP_VALUE_TYPE_UTF_16_STRING:
                TP_FREE(
                    *symbol_table, &(token[i].member_value.member_utf_16_string.member_string),
                    token[i].member_value.member_utf_16_string.member_string_length
                );
                break;
            case TP_VALUE_TYPE_UTF_32_STRING:
                TP_FREE(
                    *symbol_table, &(token[i].member_value.member_utf_32_string.member_string),
                    token[i].member_value.member_utf_32_string.member_string_length
                );
                break;
            default:
                break;
            }
        }

        TP_FREE(*symbol_table, &((*symbol_table)->member_tp_token), (*symbol_table)->member_tp_token_pos);
    }

// parse tree section:

    tp_free_parse_subtree(*symbol_table, &((*symbol_table)->member_tp_parse_tree));

// calc first set and follow set section:

// semantic analysis section:

    // int_calc_compiler

    // C compiler: namespace
    {
        TP_C_NAMESPACE_STACK* c_namespace = (*symbol_table)->member_c_scope.member_c_namespace;
        rsize_t c_namespace_num = TP_C_NAMESPACE_NUM;

        for (rsize_t i = 0; c_namespace_num > i; ++i){

            if (c_namespace[i].member_stack){

                rsize_t stack_pos = c_namespace[i].member_stack_pos;

                for (rsize_t j = 0; stack_pos > j; ++j){

                    // NOTE: member_c_object[element] must not free memory.
                    TP_FREE2(
                        *symbol_table, &(c_namespace[i].member_stack[j].member_c_object),
                        c_namespace[i].member_stack[j].member_c_object_size
                    );
                }

                TP_FREE(
                    *symbol_table, &(c_namespace[i].member_stack), c_namespace[i].member_stack_size
                );
            }
        }
    }

    // C compiler: hash table
    tp_free_object_hash(*symbol_table, &((*symbol_table)->member_object_hash), (*symbol_table)->member_object_hash.member_hash_table);

    // C compiler: c_object
    TP_C_OBJECT** c_object = (*symbol_table)->member_c_object;

    if (c_object){

        rsize_t c_object_pos = (*symbol_table)->member_c_object_pos;

        for (rsize_t i = 0; c_object_pos > i; ++i){

            if (c_object[i]){

                tp_free_c_object(*symbol_table, &(c_object[i]));
            }
        }

        TP_FREE2(*symbol_table, &c_object, (*symbol_table)->member_c_object_size);
    }

    // C compiler: c_expr_stack
    TP_FREE(*symbol_table, &((*symbol_table)->member_c_expr_stack), (*symbol_table)->member_c_expr_stack_size);

// wasm section:

    TP_FREE(*symbol_table, &((*symbol_table)->member_wasm_data_data_1), (*symbol_table)->member_wasm_data_size_1);
    TP_FREE(*symbol_table, &((*symbol_table)->member_wasm_data_data_2), (*symbol_table)->member_wasm_data_size_2);

    TP_WASM_MODULE* wasm_module = &((*symbol_table)->member_wasm_module);

    TP_WASM_MODULE_SECTION** section = wasm_module->member_section;

    if (section){

        for (size_t i = 0; wasm_module->member_section_num > i; ++i){

            if (section[i]){

                TP_FREE(*symbol_table, &(section[i]->member_name_len_name_payload_data), section[i]->member_section_size);

                TP_FREE(*symbol_table, &(section[i]), sizeof(TP_WASM_MODULE_SECTION));
            }
        }

        TP_FREE2(*symbol_table, &section, wasm_module->member_section_num * sizeof(TP_WASM_MODULE_SECTION*));
    }

    if (wasm_module->member_module_content){

        TP_FREE(*symbol_table, &(wasm_module->member_module_content),
            sizeof(TP_WASM_MODULE_CONTENT) + wasm_module->member_content_size);
    }

    TP_FREE(*symbol_table, &((*symbol_table)->member_wasm_relocation), (*symbol_table)->member_wasm_relocation_size);
    (*symbol_table)->member_wasm_relocation_begin = 0;
    (*symbol_table)->member_wasm_relocation_cur = 0;
    (*symbol_table)->member_wasm_relocation_pos = 0;
    (*symbol_table)->member_wasm_relocation_size = 0;

    for (rsize_t i = 0; (*symbol_table)->member_code_count > i; ++i){

        TP_FREE(
            *symbol_table, &((*symbol_table)->member_func_local_types[i].member_local_types),
            (*symbol_table)->member_func_local_types[i].member_local_types_num * sizeof(TP_WASM_LOCAL_TYPE)
        );
    }

    TP_FREE(
        *symbol_table, &((*symbol_table)->member_func_local_types),
        (*symbol_table)->member_code_count * sizeof(TP_WASM_FUNC_LOCAL_TYPE)
    );

    // type section
    if ((*symbol_table)->member_wasm_types){

        rsize_t type_count = (*symbol_table)->member_wasm_type_count;

        for (rsize_t i = 0; type_count > i; ++i){

            TP_FREE(
                *symbol_table, &((*symbol_table)->member_wasm_types[i].member_param_types),
                (*symbol_table)->member_wasm_types[i].member_param_count * sizeof(uint32_t)
            );
        }

        TP_FREE(
            *symbol_table, &((*symbol_table)->member_wasm_types),
            (*symbol_table)->member_wasm_type_count * sizeof(TP_WASM_SECTION_TYPE_VAR)
        );
    }

    // function section
    TP_FREE(
        *symbol_table, &((*symbol_table)->member_wasm_functions),
        (*symbol_table)->member_wasm_function_count * sizeof(uint32_t)
    );

    // export section
    TP_FREE(
        *symbol_table, &((*symbol_table)->member_wasm_exports),
        (*symbol_table)->member_wasm_export_count * sizeof(TP_WASM_SECTION_EXPORT_VAR)
    );

// x64 section:

    TP_FREE(*symbol_table, &((*symbol_table)->member_stack), (*symbol_table)->member_stack_size);

    TP_FREE(
        *symbol_table, &((*symbol_table)->member_x64_func_local),
        (*symbol_table)->member_wasm_function_count
    );

// PE COFF section:

    // PE COFF OBJECT/IMAGE File
    TP_FREE(
        *symbol_table, &((*symbol_table)->member_pe_coff_buffer),
        (*symbol_table)->member_pe_coff_size
    );

    TP_COFF_WRITE* coff = &((*symbol_table)->member_coff);

    // .data
    TP_FREE(*symbol_table, &(coff->member_data), coff->member_data_size);

    // .rdata
    TP_FREE(*symbol_table, &(coff->member_rdata), coff->member_rdata_size);

    // Section Data(Relocations: .data)
    TP_FREE(
        *symbol_table, &(coff->member_data_coff_relocations.member_relocations),
        coff->member_data_coff_relocations.member_size
    );

    // Section Data(Relocations: .rdata)
    TP_FREE(
        *symbol_table, &(coff->member_rdata_coff_relocations.member_relocations),
        coff->member_rdata_coff_relocations.member_size
    );

    // Section Data(Relocations: .text)
    TP_FREE(
        *symbol_table, &(coff->member_text_coff_relocations.member_relocations),
        coff->member_text_coff_relocations.member_size
    );

    // COFF Symbol Table
    TP_FREE(*symbol_table, &(coff->member_coff_symbol), coff->member_coff_symbol_size);

    // COFF String Table
    TP_FREE(*symbol_table, &(coff->member_string_table), coff->member_string_table_size);

    // Section Table
    TP_FREE(*symbol_table, &((*symbol_table)->member_section_table), (*symbol_table)->member_section_table_size);

    // Section Data(Relocations)
    if ((*symbol_table)->member_coff_relocations){

        for (rsize_t i = 0; (*symbol_table)->member_section_table_num > i; ++i){

            if ((*symbol_table)->member_coff_relocations[i].member_relocations){

                TP_FREE(
                    *symbol_table, &((*symbol_table)->member_coff_relocations[i].member_relocations),
                    (*symbol_table)->member_coff_relocations[i].member_size
                );
            }
        }

        TP_FREE(*symbol_table, &((*symbol_table)->member_coff_relocations), (*symbol_table)->member_coff_relocations_size);
    }

// input file section:

    if ( ! tp_close_file(*symbol_table, &((*symbol_table)->member_input_file.member_read_file))){

        TP_PUT_LOG_MSG_TRACE(*symbol_table);
    }

// output file section:

    if ( ! tp_close_file(*symbol_table, &((*symbol_table)->member_parse_tree_file))){

        TP_PUT_LOG_MSG_TRACE(*symbol_table);
    }

    (void)tp_close_file(NULL, &((*symbol_table)->member_write_log_file));

    TP_FREE(NULL, symbol_table, sizeof(TP_SYMBOL_TABLE));
}

void tp_free_c_object(TP_SYMBOL_TABLE* symbol_table, TP_C_OBJECT** c_object)
{
    if ((NULL == symbol_table) || (NULL == c_object) || (c_object && (NULL == *c_object))){

        return;
    }

    // C compiler: type
    TP_C_TYPE* c_type = &((*c_object)->member_type);
    free_c_type(symbol_table, &c_type);

    // C compiler: object
    TP_FREE(symbol_table, c_object, sizeof(TP_C_OBJECT));
}

static void free_c_type(TP_SYMBOL_TABLE* symbol_table, TP_C_TYPE** c_type)
{
    if ((NULL == symbol_table) || (NULL == c_type) || (NULL == *c_type)){

        return;
    }

    // C compiler: type
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement = NULL;

    switch ((*c_type)->member_type){
    case TP_C_TYPE_TYPE_BASIC:
        break;
    case TP_C_TYPE_TYPE_FUNCTION:{

        TP_C_TYPE_FUNCTION* function = &((*c_type)->member_body.member_type_function);

        if (0 < function->member_parameter_num){

            for (TP_C_TYPE_FUNCTION_F_PARAM* next_parameter = function->member_parameter; ; ){

//              free_c_type(symbol_table, &(next_parameter->member_type_parameter));

                TP_FREE(symbol_table, &(next_parameter->member_type_parameter), sizeof(TP_C_TYPE));

                TP_C_TYPE_FUNCTION_F_PARAM* cur_parameter = next_parameter;

                next_parameter = next_parameter->member_next;

                TP_FREE(
                    symbol_table, &cur_parameter, sizeof(TP_C_TYPE_FUNCTION_F_PARAM)
                );

                if (NULL == next_parameter){

                    break;
                }
            }

            function->member_parameter = NULL;
        }

        TP_FREE(symbol_table, &(function->member_c_return_type), sizeof(TP_C_TYPE));

        compound_statement = &(function->member_function_body);
        goto compound_statement;
    }
    case TP_C_TYPE_TYPE_FUNCTION_PARAM:{

        TP_C_TYPE** type_param =
            &((*c_type)->member_body.member_type_param.member_type_param);
        free_c_type(symbol_table, type_param);
        TP_FREE(symbol_table, type_param, sizeof(TP_C_TYPE));
        break;
    }
    case TP_C_TYPE_TYPE_NULL_STATEMENT:
        break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:{
        compound_statement = &((*c_type)->member_body.member_type_compound_statement);
compound_statement:
        ;
        TP_C_OBJECT** c_object_local = compound_statement->member_statement_body;

        if (c_object_local){

            rsize_t c_object_local_pos = compound_statement->member_statement_body_pos;

            for (rsize_t i = 0; c_object_local_pos > i; ++i){

                if (c_object_local[i]){

                    tp_free_c_object(symbol_table, &(c_object_local[i]));
                }
            }

            TP_FREE2(symbol_table, &c_object_local, compound_statement->member_statement_body_size);
        }
        break;
    }
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE** type_declaration =
            &((*c_type)->member_body.member_type_declaration_statement.member_declaration);
        free_c_type(symbol_table, type_declaration);
        TP_FREE(symbol_table, type_declaration, sizeof(TP_C_TYPE));
//      break;
    }
    case TP_C_TYPE_TYPE_EXPRESSION_STATEMENT:
//      break;
    case TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN:
        free_c_expr(symbol_table, c_type);
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return;
    }
}

static void free_c_expr(TP_SYMBOL_TABLE* symbol_table, TP_C_TYPE** c_type)
{
    // C compiler: expression
    TP_C_TYPE_BODY* c_type_body = &((*c_type)->member_body);

    switch ((*c_type)->member_type){
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE_DECLARATION_STATEMENT* type_declaration_statement =
            &(c_type_body->member_type_declaration_statement);

        TP_FREE(
            symbol_table, &(type_declaration_statement->member_c_expr),
            type_declaration_statement->member_c_expr_size
        );
        break;
    }
    case TP_C_TYPE_TYPE_EXPRESSION_STATEMENT:{

        TP_C_TYPE_EXPRESSION_STATEMENT* type_expression_statement =
            &(c_type_body->member_type_expression_statement);

        TP_FREE(
            symbol_table, &(type_expression_statement->member_c_expr),
            type_expression_statement->member_c_expr_size
        );
        break;
    }
    case TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN:{

        TP_C_TYPE_JUMP_STATEMENT_RETURN* type_jump_statement_return =
            &(c_type_body->member_type_jump_statement_return);

        TP_FREE(
            symbol_table, &(type_jump_statement_return->member_c_expr),
            type_jump_statement_return->member_c_expr_size
        );
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return;
    }
}

static uint32_t calc_grammer_type_num(TP_SYMBOL_TABLE* symbol_table, size_t grammer_type_index)
{
    uint32_t grammer_type_num = 0;

    for (size_t i = 0; TP_PARSE_TREE_TYPE_MAX_NUM1 > i; ++i){

        if (TP_PARSE_TREE_TYPE_NULL == symbol_table->member_parse_tree_type[grammer_type_index][i]){

            break;
        }

        ++grammer_type_num;
    }

    return grammer_type_num;
}

static bool parse_cmd_line_param(
    int argc, char** argv, TP_SYMBOL_TABLE* symbol_table, bool* is_disp_usage,
    bool* is_test, bool* is_int_calc_compiler, size_t* test_case_index,
    char* drive, char* dir, char* fname)
{
    if (symbol_table->member_is_output_current_dir){

        if ( ! tp_get_current_drive_dir(NULL, drive, dir)){

            return false;
        }
    }else{

        if ( ! tp_get_drive_dir(NULL, drive, dir)){

            return false;
        }
    }

    char* command_line_param = NULL;

    for (int i = 0; argc > i; ++i){

        if (0 == i){

            continue;
        }

        char* param = argv[i];

        if (('/' != param[0]) && ('-' != param[0])){

            if (command_line_param){

                goto fail;
            }else{

                command_line_param = param;
            }
        }else{

            rsize_t length = strlen(param);

            for (int j = 1; length > j; ++j){

                switch (param[j]){
                case TP_CONFIG_OPTION_IS_TEST_DISASM_X64: // -a
                    symbol_table->member_is_test_disasm_x64 = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_TOKEN_FILE: // -b
                    symbol_table->member_is_output_token_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_CURRENT_DIR: // -c
                    symbol_table->member_is_output_current_dir = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_UNICODE_CHARACTER_DATABASE_FILE: // -d
                    symbol_table->member_is_output_unicode_character_database = true;
                    break;
                case TP_CONFIG_OPTION_IS_TEST_UNICODE_CHARACTER_DATABASE: // -e
                    symbol_table->member_is_test_unicode_character_database = true;
                    break;
                case TP_CONFIG_OPTION_IS_LOAD_AND_EXEC_DLL: // -f
                    symbol_table->member_is_load_and_exec_dll = true;
                    break;
                case TP_CONFIG_OPTION_IS_IMAGE_SUBSYSTEM_EFI_APPLICATION: // -g
                    symbol_table->member_subsystem = TP_IMAGE_SUBSYSTEM_EFI_APPLICATION;
                    break;
                case TP_CONFIG_OPTION_IS_HELP: // -h
                    goto fail;
                case TP_CONFIG_OPTION_IS_INT_CALC_COMPILER: // -i
                    *is_int_calc_compiler = true;
                    symbol_table->member_is_int_calc_compiler = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_PARSE_TREE_FILE: // -j
                    symbol_table->member_is_output_parse_tree_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_SEMANTIC_ANALYSIS_FILE: // -k
                    symbol_table->member_is_output_semantic_analysis_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_LOG_FILE: // -l
                    symbol_table->member_is_output_log_file = true;
                    // Include -b -j -k -w -x -y options.
                    symbol_table->member_is_output_token_file = true; // -b
                    symbol_table->member_is_output_parse_tree_file = true; // -j
                    symbol_table->member_is_output_semantic_analysis_file = true; // -k
                    symbol_table->member_is_output_wasm_file = true; // -w
                    symbol_table->member_is_output_x64_file = true; // -x
                    symbol_table->member_is_output_wasm_text_file = true; // -y
                    break;
                case TP_CONFIG_OPTION_IS_NO_OUTPUT_MESSAGES: // -m
                    symbol_table->member_is_no_output_messages = true;
                    break;
                case TP_CONFIG_OPTION_IS_NO_OUTPUT_FILES: // -n
                    symbol_table->member_is_no_output_files = true;
                    break;
                case TP_CONFIG_OPTION_IS_DISABLE_TEST_OF_CALC_FIRST_SET_AND_FOLLOW_SET: // -p
                    symbol_table->member_is_need_test_first_follow_set = false;
                    break;
                case TP_CONFIG_OPTION_IS_CALC_FIRST_SET_AND_FOLLOW_SET: // -q
                    symbol_table->member_is_calc_first_follow_set = true;
                    break;
                case TP_CONFIG_OPTION_IS_ORIGIN_WASM: // -r
                    symbol_table->member_is_origin_wasm = true;
                    break;
                case TP_CONFIG_OPTION_IS_SOURCE_CMD_PARAM: // -s
                    symbol_table->member_is_source_cmd_param = true;
                    break;
                case TP_CONFIG_OPTION_IS_TEST_CASE: // -T
                    symbol_table->member_is_test_case = true;
//                  break;
                case TP_CONFIG_OPTION_IS_TEST_MODE: // -t
                    *is_test = true;
                    symbol_table->member_is_test_mode = true;
                    symbol_table->member_is_output_log_file = true; // -l
                    // Include -b -j -k -w -x -y -z options.
                    symbol_table->member_is_output_token_file = true; // -b
                    symbol_table->member_is_output_parse_tree_file = true; // -j
                    symbol_table->member_is_output_semantic_analysis_file = true; // -k
                    symbol_table->member_is_output_wasm_file = true; // -w
                    symbol_table->member_is_output_x64_file = true; // -x
                    symbol_table->member_is_output_wasm_text_file = true; // -y
                    symbol_table->member_is_output_pe_coff_file = true; // -z
                    break;
                case TP_CONFIG_OPTION_IS_WCHAR_T_UTF_32: // -u
                    symbol_table->member_is_wchar_t_UTF_32 = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_WASM_FILE: // -w
                    symbol_table->member_is_output_wasm_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_X64_FILE: // -x
                    symbol_table->member_is_output_x64_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_WASM_TEXT_FILE: // -y
                    symbol_table->member_is_output_wasm_text_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_PE_COFF_FILE: // -z
                    symbol_table->member_is_output_pe_coff_file = true;
                    break;
                default:
                    goto fail;
                }
            }
        }
    }

    if ((NULL == command_line_param) && (1 == argc)){

        goto fail;
    }

    if (command_line_param &&
        ((symbol_table->member_is_origin_wasm) ||
        ((symbol_table->member_is_test_mode) &&
        (false == symbol_table->member_is_test_case)))){

        goto fail;
    }

    if (command_line_param && (symbol_table->member_is_test_case)){

        if ( ! strlen(command_line_param)){

            goto fail;
        }

        char* error_first_char = NULL;

        size_t value = (size_t)strtoull(command_line_param, &error_first_char, 10);

        *test_case_index = value;
        symbol_table->member_test_case_index = value;

        if (command_line_param == error_first_char){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: strtoull(%1) convert failed."),
                TP_LOG_PARAM_STRING(command_line_param)
            );

            return false;
        }

        if (ERANGE == errno){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }
    }

    if (symbol_table->member_is_source_cmd_param){

        size_t length = (command_line_param ? strlen(command_line_param) : 0);

        if (TP_SOURCE_CODE_STRING_LENGTH_MAX < length){

            goto fail;
        }

        sprintf_s(
            symbol_table->member_source_code,
            sizeof(symbol_table->member_source_code),
            "%s", command_line_param
        );
    }

    if (command_line_param &&
        ((false == symbol_table->member_is_origin_wasm) &&
        (false == symbol_table->member_is_source_cmd_param))){

        char ext[_MAX_EXT] = { 0 };

        errno_t err = _splitpath_s(command_line_param, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

        if (err){

            TP_PRINT_CRT_ERROR(NULL);

            return false;
        }

        if ( ! tp_make_path(
            NULL, drive, dir, NULL, fname, ext,
            symbol_table->member_input_file.member_input_file_path,
            sizeof(symbol_table->member_input_file.member_input_file_path))){

            return false;
        }
    }else if (symbol_table->member_is_origin_wasm){

        sprintf_s(fname, _MAX_FNAME, "%s", TP_UEFI_DEFAULT_FILE_NAME);
    }

    errno_t err = _set_errno(0);

    return true;

fail:

    *is_disp_usage = true;

    fprintf_s(stderr, "usage: tenpoku-dev [-/][abcdefghijklmnpqrsTtuwxyz] [input file] [test case] [source code string]\n");
    fprintf_s(stderr, "  -a : set test of disassembler of x64.\n");
    fprintf_s(stderr, "  -b : set output token file.\n");
    fprintf_s(stderr, "  -c : set input/output current directory(other than DLL and EFI).\n");
    fprintf_s(stderr, "  -d : set output UNICODE CHARACTER DATABASE file.\n");
    fprintf_s(stderr, "  -e : set test UNICODE CHARACTER DATABASE.\n");
    fprintf_s(stderr, "  -f : set execute of dll.\n");
    fprintf_s(stderr, "  -g : set TP_IMAGE_SUBSYSTEM_EFI_APPLICATION to subsystem.\n");
    fprintf_s(stderr, "  -h : display of usage message.\n");
    fprintf_s(stderr, "  -i : set use int_calc_compiler.\n");
    fprintf_s(stderr, "  -j : set output parse tree file.\n");
    fprintf_s(stderr, "  -k : set output semantic analysis file.\n");
    fprintf_s(stderr, "  -l : set output log file(include -b -j -k -w -x -y -z options).\n");
    fprintf_s(stderr, "  -m : set no output messages.\n");
    fprintf_s(stderr, "  -n : set no output files.\n");
    fprintf_s(stderr, "  -p : set disable test of calc first set and follow set.\n");
    fprintf_s(stderr, "  -q : set calc first set and follow set.\n");
    fprintf_s(stderr, "  -r : set origin wasm. [input file] is not necessary.\n");
    fprintf_s(stderr, "  -s : set source code command line parameter mode.\n");
    fprintf_s(
        stderr,
        "       need [source code string] up to %d characters. [input file] is not necessary.\n",
        TP_SOURCE_CODE_STRING_LENGTH_MAX
    );
    fprintf_s(stderr, "  -T : set [test case] of test mode. [input file] is not necessary.\n");
    fprintf_s(stderr, "  -t : set test mode(include -l option). [input file] is not necessary.\n");
    fprintf_s(stderr, "  -u : set wchar_t to UTF-32.\n");
    fprintf_s(stderr, "  -w : set output wasm file.\n");
    fprintf_s(stderr, "  -x : set output x64 file.\n");
    fprintf_s(stderr, "  -y : set output wasm text file.\n");
    fprintf_s(stderr, "  -z : set output PE/COFF file.\n");

    err = _set_errno(0);

    return false;
}

