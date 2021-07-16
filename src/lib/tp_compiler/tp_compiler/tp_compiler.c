
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool check_cpuid(TP_SYMBOL_TABLE* symbol_table);

bool tp_compiler(int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size)
{
    SetLastError(NO_ERROR);

    if ( ! SetThreadErrorMode(0, NULL)){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    if ( ! SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    if ( ! SetDllDirectoryA("")){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    if ( ! SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT)){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    errno_t err = _set_errno(0);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    bool is_test_mode = false;
    bool is_int_calc_compiler = false;
    size_t test_case_index = 0;

    time_t now = time(NULL);

    char drive[_MAX_DRIVE] = { 0 };
    char dir[_MAX_DIR] = { 0 };

    if ( ! tp_compiler_main(
        argc, argv, msg_buffer, msg_buffer_size,
        &is_test_mode, &is_int_calc_compiler, &test_case_index,
        0, NULL, drive, dir, now)){

        _CrtDumpMemoryLeaks();

        return false;
    }

    if (is_test_mode){

        if ( ! tp_test_compiler(
            is_int_calc_compiler, test_case_index,
            argc, argv, msg_buffer, msg_buffer_size, drive, dir, now)){

            _CrtDumpMemoryLeaks();

            return false;
        }
    }

    _CrtDumpMemoryLeaks();

    return true;
}

bool tp_compiler_main(
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    bool* is_test_mode, bool* is_int_calc_compiler, size_t* test_case_index,
    size_t test_index, int32_t* return_value, char* drive, char* dir, time_t now)
{
    TP_SYMBOL_TABLE* symbol_table = (TP_SYMBOL_TABLE*)TP_CALLOC(NULL, 1, sizeof(TP_SYMBOL_TABLE));

    if (NULL == symbol_table){

        TP_PRINT_CRT_ERROR(NULL);

        goto fail;
    }

    bool is_disp_usage = false;
    bool is_test = (is_test_mode ? *is_test_mode : false);

    if ( ! tp_init_symbol_table(
        symbol_table, argc, argv, &is_disp_usage,
        is_test_mode, is_int_calc_compiler, test_case_index,
        test_index, now, drive, dir, msg_buffer, msg_buffer_size)){

        if ( ! is_disp_usage){

            fprintf(stderr, "ERROR: Initialize failed.\n");
        }

        tp_free_memory_and_file(&symbol_table);

        return false;
    }

    if ( ! check_cpuid(symbol_table)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        tp_free_memory_and_file(&symbol_table);

        return false;
    }

    if (symbol_table->member_is_load_and_exec_dll){

        TP_PUT_LOG_PRINT(
            symbol_table, TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("Execute of dll."),
        );

        int return_value = 0;

        if ( ! tp_load_and_exec_dll(
            symbol_table, TP_PE_DLL_DEFAULT_FUNC, &return_value)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail_skip_msg;
        }

        tp_free_memory_and_file(&symbol_table);

        return true;
    }

    if (symbol_table->member_is_output_unicode_character_database ||
        symbol_table->member_is_test_unicode_character_database){

        if (symbol_table->member_is_output_unicode_character_database){

            if ( ! tp_unicode_data_gen(symbol_table, TP_UNICODE_DATA_GEN_KIND_DATA)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
            }

            if ( ! tp_unicode_data_gen(symbol_table, TP_UNICODE_DATA_GEN_KIND_INDEX)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
            }
        }

        if (symbol_table->member_is_test_unicode_character_database){

            if ( ! tp_test_unicode_data_index(symbol_table)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
            }
        }

        tp_free_memory_and_file(&symbol_table);

        return true;
    }

    if (symbol_table->member_is_calc_first_follow_set){

        if ( ! tp_calc_first_follow_set_entry_point(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }

        tp_free_memory_and_file(&symbol_table);

        return true;
    }

    if (symbol_table->member_is_test_disasm_x64){

        if ( ! tp_test_disasm_x64(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }

        tp_free_memory_and_file(&symbol_table);

        return true;
    }

    if (is_test_mode){

        if ((false == is_test) && *is_test_mode){

            // switch to test mode.
            tp_free_memory_and_file(&symbol_table);

            return true;
        }
    }

    bool is_origin_wasm = symbol_table->member_is_origin_wasm;

    uint8_t* entry_point_symbol = TP_WASM_MODULE_SECTION_EXPORT_NAME_2_EXPR;

    if ( ! (symbol_table->member_is_int_calc_compiler)){

        switch (symbol_table->member_subsystem){
        case TP_IMAGE_SUBSYSTEM_WINDOWS_GUI:
            if (symbol_table->member_is_test_mode){

                break;
            }
            if ( ! (symbol_table->member_is_output_pe_coff_file)){

                break;
            }
            entry_point_symbol = TP_PE_DLL_ENTRY_POINT;
            break;
        case TP_IMAGE_SUBSYSTEM_WINDOWS_CUI:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        case TP_IMAGE_SUBSYSTEM_EFI_APPLICATION:
//          break;
        case TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
//          break;
        case TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
            entry_point_symbol = TP_WASM_MODULE_SECTION_EXPORT_NAME_2_UEFI;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
    }

    if (is_origin_wasm){

        goto origin_wasm;
    }else{

        if (is_test_mode && *is_test_mode){

            uint8_t* source_code = ((*is_int_calc_compiler) ?
                tp_test_case_table[test_index].member_source_code :
                tp_C_test_case_table[test_index].member_source_code);

            size_t source_code_length = strlen(source_code);

            FILE* write_test_src_file = NULL;

            if ( ! tp_open_write_file_text(
                symbol_table, symbol_table->member_test_src_file_path,
                &write_test_src_file)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            fprintf(write_test_src_file, "%s\n", source_code);

            if ( ! tp_close_file(symbol_table, &write_test_src_file)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            if ( ! tp_make_token(
                symbol_table, source_code, source_code_length)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
        }else{

            if (symbol_table->member_is_source_cmd_param){

                if ( ! tp_make_token(
                    symbol_table, symbol_table->member_source_code,
                    (strlen(symbol_table->member_source_code) + 1))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto fail;
                }
            }else{

                if ( ! tp_make_token(symbol_table, NULL, 0)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto fail;
                }
            }
        }

        if ( ! tp_make_parse_tree(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        if ( ! tp_semantic_analysis(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

origin_wasm:
        ;

        if ( ! tp_make_wasm(symbol_table, entry_point_symbol, is_origin_wasm)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        if ( ! tp_make_x64_code(
            symbol_table, entry_point_symbol, return_value)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }
    }

    TP_PUT_LOG_PRINT(
        symbol_table, TP_MSG_FMT("End of compile: error(%1), warning(%2)"),
        TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_error_count),
        TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_warning_count)
    );

    TP_PUT_LOG_PRINT(
        symbol_table, TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("Compile succeeded."),
    );

    tp_free_memory_and_file(&symbol_table);

    return true;

fail:
    if (symbol_table){

        TP_PUT_LOG_PRINT(
            symbol_table, TP_MSG_FMT("End of compile: error(%1), warning(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_error_count),
            TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_warning_count)
        );

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: Compile failed.")
        );
    }

fail_skip_msg:
    tp_free_memory_and_file(&symbol_table);

    return false;
}

static bool check_cpuid(TP_SYMBOL_TABLE* symbol_table)
{
    int cpuid_info[TP_CPUID_INFO_SIZE] = { 0 };

    __cpuid(cpuid_info, 0x1);

    if ((0x01 << TP_CPUID_POPCNT_SUPPORT) &
        cpuid_info[TP_CPUID_ECX_INDEX]){

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"),
        TP_LOG_PARAM_STRING("ERROR: POPCNT unsupported.")
    );

    return false;
}

