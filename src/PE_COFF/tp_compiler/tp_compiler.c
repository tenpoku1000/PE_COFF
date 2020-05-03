
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_compiler(int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size)
{
    SetLastError(NO_ERROR);

    errno_t err = _set_errno(0);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    bool is_test_mode = false;

    time_t now = time(NULL);

    char drive[_MAX_DRIVE];
    memset(drive, 0, sizeof(drive));

    char dir[_MAX_DIR];
    memset(dir, 0, sizeof(dir));

    if ( ! tp_compiler_main(
        argc, argv, msg_buffer, msg_buffer_size,
        &is_test_mode, 0, NULL, drive, dir, now)){

        _CrtDumpMemoryLeaks();

        return false;
    }

    if (is_test_mode){

        if ( ! tp_test_compiler(
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
    bool* is_test_mode, size_t test_index, int32_t* return_value,
    char* drive, char* dir, time_t now)
{
    TP_SYMBOL_TABLE* symbol_table = (TP_SYMBOL_TABLE*)calloc(1, sizeof(TP_SYMBOL_TABLE));

    if (NULL == symbol_table){

        TP_PRINT_CRT_ERROR(NULL);

        goto error_proc;
    }

    bool is_disp_usage = false;

    bool is_test = (is_test_mode ? *is_test_mode : false);

    if ( ! tp_init_symbol_table(
        symbol_table, argc, argv, &is_disp_usage,
        is_test_mode, test_index, now, drive, dir, msg_buffer, msg_buffer_size)){

        if ( ! is_disp_usage){

            fprintf(stderr, "ERROR: Initialize failed.\n");
        }

        tp_free_memory_and_file(&symbol_table);

        return false;
    }

    if (is_test_mode){

        if ((false == is_test) && *is_test_mode){

            // switch to test mode.
            tp_free_memory_and_file(&symbol_table);

            return true;
        }
    }

    bool is_origin_wasm = symbol_table->member_is_origin_wasm;
    bool is_pe_coff = symbol_table->member_is_output_pe_coff_file;
    bool is_64 = (false == symbol_table->member_is_32);

    uint8_t* entry_point_symbol = TP_WASM_MODULE_SECTION_EXPORT_NAME_2_EXPR;

    if (is_pe_coff){

        entry_point_symbol = TP_WASM_MODULE_SECTION_EXPORT_NAME_2_UEFI;
    }

    if (is_origin_wasm){

        goto origin_wasm;
    }else{

        if (is_test_mode && *is_test_mode){

            if ( ! tp_make_token(
                symbol_table,
                tp_test_case_table[test_index].member_source_code,
                strlen(tp_test_case_table[test_index].member_source_code))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }
        }else{

            if (symbol_table->member_is_source_cmd_param){

                if ( ! tp_make_token(
                    symbol_table,
                    symbol_table->member_source_code, strlen(symbol_table->member_source_code))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto error_proc;
                }
            }else{

                if ( ! tp_make_token(symbol_table, NULL, 0)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto error_proc;
                }
            }
        }

        if ( ! tp_make_parse_tree(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto error_proc;
        }

        if ( ! tp_semantic_analysis(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto error_proc;
        }

origin_wasm:
        ;

        if (is_pe_coff){

            if ( ! tp_make_wasm(symbol_table, entry_point_symbol, is_origin_wasm, is_pe_coff, is_64)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }

            if ( ! tp_make_x64_code(
                symbol_table, entry_point_symbol, return_value, is_pe_coff,
                TP_COFF_OBJECT_DEFAULT_FNAME, TP_COFF_OBJECT_DEFAULT_EXT)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }

            if ( ! tp_make_PE_file(
                symbol_table, TP_COFF_OBJECT_DEFAULT_FNAME, TP_COFF_OBJECT_DEFAULT_EXT, entry_point_symbol)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }

            if ( ! tp_make_PE_file(symbol_table, NULL, NULL, NULL)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }
        }else{

            if ( ! tp_make_wasm(symbol_table, entry_point_symbol, is_origin_wasm, is_pe_coff, is_64)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }

            if ( ! tp_make_x64_code(
                symbol_table, entry_point_symbol, return_value, is_pe_coff, NULL, NULL)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }
        }
    }

    tp_free_memory_and_file(&symbol_table);

    return true;

error_proc:

    if (symbol_table){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: Compile failed.")
        );
    }

    tp_free_memory_and_file(&symbol_table);

    return false;
}

