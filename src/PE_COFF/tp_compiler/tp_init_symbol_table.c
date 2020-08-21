
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static TP_SYMBOL_TABLE init_symbol_table_value = {
// config section:
    .member_is_output_current_dir = false,
    .member_is_output_log_file = false,
    .member_is_no_output_messages = false,
    .member_is_no_output_files = false,
    .member_is_origin_wasm = false,
    .member_is_source_cmd_param = false,
    .member_source_code = { 0 },
    .member_is_test_mode = false,
    .member_is_test_disasm_x64 = false,
    .member_is_output_wasm_file = false,
    .member_is_output_wasm_text_file = false,
    .member_is_output_x64_file = false,
    .member_is_output_pe_coff_file = false,
    .member_is_32 = false,

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

// output file section:
    .member_write_log_file = NULL,
    .member_parse_tree_file = NULL,
    .member_write_log_file_path = { 0 },
    .member_token_file_path = { 0 },
    .member_parse_tree_file_path = { 0 },
    .member_object_hash_file_path = { 0 },
    .member_wasm_file_path = { 0 },
    .member_wasm_text_file_path = { 0 },
    .member_x64_file_path = { 0 },
    .member_x64_text_file_path = { 0 },
    .member_coff_code_text_file_path = { 0 },
    .member_pe_code_text_file_path = { 0 },

// input file section:
    .member_input_file_path = { 0 },
    .member_read_file = NULL,
    .member_is_start_of_file = true,
    .member_is_end_of_file = false,
    .member_read_lines_buffer = { 0 },
    .member_read_lines_length = 0,
    .member_read_lines_current_position = 0,
    .member_read_lines_end_position = 0,

// token section:
    .member_tp_token = NULL,
    .member_tp_token_pos = 0,
    .member_tp_token_size = 0,
    .member_tp_token_size_allocate_unit = TP_TOKEN_SIZE_ALLOCATE_UNIT,
    .member_tp_token_position = NULL,
    .member_nul_num = 0,

// parse tree section:
    .member_nesting_level_of_expression = 0,
    .member_tp_parse_tree = NULL,

// semantic analysis section:
    .member_object_hash.member_mask = UINT8_MAX,
    .member_object_hash.member_hash_table = { 0 },
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

// wasm section:
    .member_wasm_local_pos = 0,

    .member_wasm_data_offset_1 = 0,
    .member_wasm_data_size_1 = 0,
    .member_wasm_data_data_1 = NULL,

    .member_wasm_data_offset_2 = 0,
    .member_wasm_data_size_2 = 0,
    .member_wasm_data_data_2 = NULL,

    .member_wasm_module = { 0 },
    .member_code_index = 0,
    .member_code_body_size = 0,
    .member_code_section_buffer = NULL,

    .member_wasm_relocation = NULL,
    .member_wasm_relocation_begin = 0,
    .member_wasm_relocation_cur = 0,
    .member_wasm_relocation_pos = 0,
    .member_wasm_relocation_size = 0,
    .member_wasm_relocation_allocate_unit = TP_WASM_RELOCATION_ALLOCATE_UNIT,

    .member_local_types = NULL,
    .member_local_types_num = 0,

// x64 section:
    .member_stack = NULL,
    .member_wasm_code_body_buffer = NULL,
    .member_wasm_code_body_pos = 0,
    .member_wasm_code_body_size = 0,
    .member_stack_pos = TP_WASM_STACK_EMPTY,
    .member_stack_size = 0,
    .member_stack_size_allocate_unit = TP_WASM_STACK_SIZE_ALLOCATE_UNIT,

    .member_local_variable_size = 0,
    .member_local_variable_size_max = TP_WASM_LOCAL_VARIABLE_MAX_DEFAULT,
    .member_padding_local_variable_bytes = 0,

    .member_temporary_variable_size = 0,
    .member_temporary_variable_size_max = TP_WASM_TEMPORARY_VARIABLE_MAX_DEFAULT,
    .member_padding_temporary_variable_bytes = 0,

    .member_use_X86_32_register = { 0 },
    .member_use_X64_32_register = { 0 },
    .member_use_nv_register = { TP_X64_NV64_REGISTER_NULL },
    .member_use_v_register = { TP_X64_V64_REGISTER_NULL },

    .member_register_bytes = 0,
    .member_padding_register_bytes = 0,

    .member_stack_imm32 = 0,

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

static uint32_t calc_grammer_type_num(TP_SYMBOL_TABLE* symbol_table, size_t grammer_type_index);
static bool parse_cmd_line_param(
    int argc, char** argv, TP_SYMBOL_TABLE* symbol_table, bool* is_disp_usage, bool* is_test
);

bool tp_init_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, int argc, char** argv, bool* is_disp_usage,
    bool* is_test, size_t test_index, time_t now, char* drive, char* dir,
    uint8_t* msg_buffer, size_t msg_buffer_size)
{
    errno_t err = 0;

    *symbol_table = init_symbol_table_value;

    symbol_table->member_disp_log_file = stderr;

    if (0 != setvbuf(symbol_table->member_disp_log_file, msg_buffer, _IOFBF, msg_buffer_size)){

        goto fail;
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

    bool is_normal = false;

    if (is_test && (false == *is_test)){

        is_normal = true;
    }

    if ( ! parse_cmd_line_param(argc, argv, symbol_table, is_disp_usage, is_test)){

        return false;
    }

    if (is_test && *is_test && is_normal){

        // switch to test mode.
        return true;
    }

    char base_dir[_MAX_PATH];
    memset(base_dir, 0, sizeof(base_dir));

    if (symbol_table->member_is_output_current_dir){

        DWORD status = GetCurrentDirectoryA(sizeof(base_dir), base_dir);

        if (0 == status){

            goto fail;
        }
    }else{

        HMODULE handle = GetModuleHandleA(NULL);

        if (0 == handle){

            goto fail;
        }

        DWORD status = GetModuleFileNameA(handle, base_dir, sizeof(base_dir));

        if (0 == status){

            goto fail;
        }
    }

    err = _splitpath_s(base_dir, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (err){

        goto fail;
    }

    if ( ! tp_make_path_log_files(symbol_table, drive, dir, *is_test, test_index, now)){

        return false;
    }

    return true;

fail:

    if (err){

        TP_PRINT_CRT_ERROR(NULL);
    }else{

        TP_GET_LAST_ERROR(NULL);
    }

    return false;
}

void tp_free_memory_and_file(TP_SYMBOL_TABLE** symbol_table)
{
    if (NULL == symbol_table){

        return;
    }

// config section:

// message section:

// token section:

    if ((*symbol_table)->member_tp_token){

        TP_FREE(*symbol_table, &((*symbol_table)->member_tp_token), (*symbol_table)->member_tp_token_size);
    }

// parse tree section:

    if ((*symbol_table)->member_tp_parse_tree){

        tp_free_parse_subtree(*symbol_table, &((*symbol_table)->member_tp_parse_tree));
    }

// semantic analysis section:

    tp_free_object_hash(*symbol_table, &((*symbol_table)->member_object_hash), (*symbol_table)->member_object_hash.member_hash_table);

// wasm section:

    TP_FREE(*symbol_table, &((*symbol_table)->member_wasm_data_data_1), (*symbol_table)->member_wasm_data_size_1);
    TP_FREE(*symbol_table, &((*symbol_table)->member_wasm_data_data_2), (*symbol_table)->member_wasm_data_size_2);

    TP_WASM_MODULE* wasm_module = &((*symbol_table)->member_wasm_module);

    TP_WASM_MODULE_SECTION** section = wasm_module->member_section;

    if (section){

        for (size_t i = 0; wasm_module->member_section_num > i; ++i){

            if (section[i]) {

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

    if ((*symbol_table)->member_wasm_relocation){

        TP_FREE(*symbol_table, &((*symbol_table)->member_wasm_relocation), (*symbol_table)->member_wasm_relocation_size);
    }

    if ((*symbol_table)->member_local_types){

        TP_FREE(
            *symbol_table, &((*symbol_table)->member_local_types),
            (*symbol_table)->member_local_types_num * sizeof(TP_WASM_LOCAL_TYPE)
        );
    }

// x64 section:

    if ((*symbol_table)->member_stack){

        TP_FREE(*symbol_table, &((*symbol_table)->member_stack), (*symbol_table)->member_stack_size);
    }

// PE COFF section:

    // PE COFF OBJECT/IMAGE File
    if ((*symbol_table)->member_pe_coff_buffer){

        TP_FREE(
            *symbol_table, &((*symbol_table)->member_pe_coff_buffer),
            (*symbol_table)->member_pe_coff_size
        );
    }

    TP_COFF_WRITE* coff = &((*symbol_table)->member_coff);

    // .data
    if (coff->member_data){

        TP_FREE(*symbol_table, &(coff->member_data), coff->member_data_size);
    }

    // .rdata
    if (coff->member_rdata){

        TP_FREE(*symbol_table, &(coff->member_rdata), coff->member_rdata_size);
    }

    // Section Data(Relocations: .data)
    if (coff->member_data_coff_relocations.member_relocations){

        TP_FREE(
            *symbol_table, &(coff->member_data_coff_relocations.member_relocations),
            coff->member_data_coff_relocations.member_size
        );
    }

    // Section Data(Relocations: .rdata)
    if (coff->member_rdata_coff_relocations.member_relocations){

        TP_FREE(
            *symbol_table, &(coff->member_rdata_coff_relocations.member_relocations),
            coff->member_rdata_coff_relocations.member_size
        );
    }

    // Section Data(Relocations: .text)
    if (coff->member_text_coff_relocations.member_relocations){

        TP_FREE(
            *symbol_table, &(coff->member_text_coff_relocations.member_relocations),
            coff->member_text_coff_relocations.member_size
        );
    }

    // COFF Symbol Table
    if (coff->member_coff_symbol){

        TP_FREE(*symbol_table, &(coff->member_coff_symbol), coff->member_coff_symbol_size);
    }

    // COFF String Table
    if (coff->member_string_table){

        TP_FREE(*symbol_table, &(coff->member_string_table), coff->member_string_table_size);
    }

    // Section Table
    if ((*symbol_table)->member_section_table){

        TP_FREE(*symbol_table, &((*symbol_table)->member_section_table), (*symbol_table)->member_section_table_size);
    }

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

    if ( ! tp_close_file(*symbol_table, &((*symbol_table)->member_read_file))){

        TP_PUT_LOG_MSG_TRACE(*symbol_table);
    }

// output file section:

    if ( ! tp_close_file(*symbol_table, &((*symbol_table)->member_parse_tree_file))){

        TP_PUT_LOG_MSG_TRACE(*symbol_table);
    }

    (void)tp_close_file(NULL, &((*symbol_table)->member_write_log_file));

    TP_FREE(NULL, symbol_table, sizeof(TP_SYMBOL_TABLE));
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
    int argc, char** argv, TP_SYMBOL_TABLE* symbol_table, bool* is_disp_usage, bool* is_test)
{
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
                case TP_CONFIG_OPTION_IS_32:
                    symbol_table->member_is_32 = true;
                    break;
                case TP_CONFIG_OPTION_IS_TEST_DISASM_X64:
                    symbol_table->member_is_test_disasm_x64 = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_CURRENT_DIR:
                    symbol_table->member_is_output_current_dir = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_LOG_FILE:
                    symbol_table->member_is_output_log_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_NO_OUTPUT_MESSAGES:
                    symbol_table->member_is_no_output_messages = true;
                    break;
                case TP_CONFIG_OPTION_IS_NO_OUTPUT_FILES:
                    symbol_table->member_is_no_output_files = true;
                    break;
                case TP_CONFIG_OPTION_IS_ORIGIN_WASM:
                    symbol_table->member_is_origin_wasm = true;
                    break;
                case TP_CONFIG_OPTION_IS_SOURCE_CMD_PARAM:
                    symbol_table->member_is_source_cmd_param = true;
                    break;
                case TP_CONFIG_OPTION_IS_TEST_MODE:
                    *is_test = true;
                    symbol_table->member_is_test_mode = true;
                    symbol_table->member_is_output_log_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_WASM_FILE:
                    symbol_table->member_is_output_wasm_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_X64_FILE:
                    symbol_table->member_is_output_x64_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_WASM_TEXT_FILE:
                    symbol_table->member_is_output_wasm_text_file = true;
                    break;
                case TP_CONFIG_OPTION_IS_OUTPUT_PE_COFF_FILE:
                    symbol_table->member_is_output_pe_coff_file = true;
                    break;
                default:
                    goto fail;
                }
            }
        }
    }

    if (command_line_param &&
        (symbol_table->member_is_origin_wasm || symbol_table->member_is_test_mode)){

        goto fail;
    }

    if ((NULL == command_line_param) && (1 == argc)){

        goto fail;
    }

    if (symbol_table->member_is_source_cmd_param){

        size_t length = strlen(command_line_param);

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

        sprintf_s(
            symbol_table->member_input_file_path,
            sizeof(symbol_table->member_input_file_path),
            "%s", command_line_param
        );
    }

    errno_t err = _set_errno(0);

    return true;

fail:

    *is_disp_usage = true;

    fprintf_s(stderr, "usage: PE_COFF [-/][rcmlnwx] [input file] [source code string]\n");
    fprintf_s(stderr, "  -3 : set 32 bits mode.\n");
    fprintf_s(stderr, "  -a : set test of disassembler of x64.\n");
    fprintf_s(stderr, "  -c : set output current directory.\n");
    fprintf_s(stderr, "  -l : set output log file.\n");
    fprintf_s(stderr, "  -m : set no output messages.\n");
    fprintf_s(stderr, "  -n : set no output files.\n");
    fprintf_s(stderr, "  -r : set origin WASM. [input file] is not necessary.\n");
    fprintf_s(stderr, "  -s : set source code command line parameter mode.\n");
    fprintf_s(
        stderr,
        "       need [source code string] up to %d characters. [input file] is not necessary.\n",
        TP_SOURCE_CODE_STRING_LENGTH_MAX
    );
    fprintf_s(stderr, "  -t : set test mode. [input file] is not necessary.\n");
    fprintf_s(stderr, "  -w : set output WASM file.\n");
    fprintf_s(stderr, "  -x : set output x64 file.\n");
    fprintf_s(stderr, "  -y : set output WASM text file.\n");
    fprintf_s(stderr, "  -z : set output PE/COFF file.\n");

    err = _set_errno(0);

    return false;
}

