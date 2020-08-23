
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool make_path_test_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param,
    bool is_test, size_t test_index, time_t now
);
static bool make_path_log_files_main(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir, bool is_test
);

bool tp_make_path_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param, bool is_test, size_t test_index, time_t now)
{
    if (is_test){

        if ( ! make_path_test_log_files(symbol_table, drive, dir_param, is_test, test_index, now)){

            fprintf_s(stderr, "ERROR: already exists.\n");

            return false;
        }
    }else{

        if ( ! make_path_log_files_main(symbol_table, drive, dir_param, is_test)){

            return false;
        }
    }

    return true;
}

static bool make_path_test_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param, bool is_test, size_t test_index, time_t now)
{
    struct tm local_time = { 0 };

    errno_t err = localtime_s(&local_time, &now);

    if (err){

        TP_PRINT_CRT_ERROR(NULL);

        return false;
    }

    char dir[_MAX_DIR];
    memset(dir, 0, sizeof(dir));

    sprintf_s(
        dir, sizeof(dir), "%s\\test_%04d-%02d-%02d",
        dir_param, local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday
    );

    char path[_MAX_PATH];
    memset(path, 0, sizeof(path));

    err = _makepath_s(path, sizeof(path), drive, dir, NULL, NULL);

    if (err){

        TP_PRINT_CRT_ERROR(NULL);

        return false;
    }

    DWORD attributes = GetFileAttributesA(path);

    if (-1 == attributes){

        SetLastError(NO_ERROR);

        if ( ! CreateDirectoryA(path, NULL)){

            TP_GET_LAST_ERROR(NULL);

            return false;
        }
    }

    if (TP_TEST_FNAME_NUM_MAX < test_index){

        fprintf_s(stderr, "ERROR: overflow test case(%03zd).\n", test_index);

        return false;
    }

    memset(dir, 0, sizeof(dir));

    sprintf_s(
        dir, sizeof(dir), "%s\\test_%04d-%02d-%02d\\test_case_%03zd",
        dir_param, local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday, test_index + 1
    );

    memset(path, 0, sizeof(path));

    err = _makepath_s(path, sizeof(path), drive, dir, NULL, NULL);

    if (err){

        TP_PRINT_CRT_ERROR(NULL);

        return false;
    }

    attributes = GetFileAttributesA(path);

    if (-1 == attributes){

        SetLastError(NO_ERROR);

        if ( ! CreateDirectoryA(path, NULL)){

            TP_GET_LAST_ERROR(NULL);

            return false;
        }

        if ( ! make_path_log_files_main(symbol_table, drive, dir, is_test)){

            return false;
        }

        return true;
    }

    return false;
}

static bool make_path_log_files_main(TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir, bool is_test)
{
    if ( ! tp_make_path(
        NULL, drive, dir, TP_LOG_FILE_PREFIX,
        TP_WRITE_LOG_DEFAULT_FILE_NAME, TP_WRITE_LOG_DEFAULT_EXT_NAME,
        symbol_table->member_write_log_file_path,
        sizeof(symbol_table->member_write_log_file_path))){

//      TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (symbol_table->member_is_output_log_file){

        FILE* write_log_file = NULL;

        if ( ! tp_open_write_file(NULL, symbol_table->member_write_log_file_path, &write_log_file)){

//          TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        symbol_table->member_write_log_file = write_log_file;

        if (0 != setvbuf(
            symbol_table->member_write_log_file, symbol_table->member_log_msg_buffer, _IOFBF,
            sizeof(symbol_table->member_log_msg_buffer))){

            TP_PRINT_CRT_ERROR(NULL);

            return false;
        }
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, TP_LOG_FILE_PREFIX,
        TP_TOKEN_DEFAULT_FILE_NAME, TP_TOKEN_DEFAULT_EXT_NAME,
        symbol_table->member_token_file_path,
        sizeof(symbol_table->member_token_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, TP_LOG_FILE_PREFIX,
        TP_PARSE_TREE_DEFAULT_FILE_NAME, TP_PARSE_TREE_DEFAULT_EXT_NAME,
        symbol_table->member_parse_tree_file_path,
        sizeof(symbol_table->member_parse_tree_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, TP_LOG_FILE_PREFIX,
        TP_OBJECT_HASH_DEFAULT_FILE_NAME, TP_OBJECT_HASH_DEFAULT_EXT_NAME,
        symbol_table->member_object_hash_file_path,
        sizeof(symbol_table->member_object_hash_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WASM_DEFAULT_FILE_NAME, TP_WASM_DEFAULT_EXT_NAME,
        symbol_table->member_wasm_file_path,
        sizeof(symbol_table->member_wasm_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WASM_TEXT_DEFAULT_FILE_NAME, TP_WASM_TEXT_DEFAULT_EXT_NAME,
        symbol_table->member_wasm_text_file_path,
        sizeof(symbol_table->member_wasm_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_X64_DEFAULT_FILE_NAME, TP_X64_DEFAULT_EXT_NAME,
        symbol_table->member_x64_file_path,
        sizeof(symbol_table->member_x64_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_X64_TEXT_DEFAULT_FILE_NAME, TP_X64_TEXT_DEFAULT_EXT_NAME,
        symbol_table->member_x64_text_file_path,
        sizeof(symbol_table->member_x64_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_COFF_CODE_TEXT_DEFAULT_FILE_NAME, TP_COFF_CODE_TEXT_DEFAULT_EXT_NAME,
        symbol_table->member_coff_code_text_file_path,
        sizeof(symbol_table->member_coff_code_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_PE_CODE_TEXT_DEFAULT_FILE_NAME, TP_PE_CODE_TEXT_DEFAULT_EXT_NAME,
        symbol_table->member_pe_code_text_file_path,
        sizeof(symbol_table->member_pe_code_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_PE_UEFI_DEFAULT_FNAME, TP_PE_UEFI_DEFAULT_EXT,
        symbol_table->member_pe_uefi_path,
        sizeof(symbol_table->member_pe_uefi_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_make_path(
    TP_SYMBOL_TABLE* symbol_table,
    char* drive, char* dir, char* prefix, char* fname, char* ext,
    char* path, size_t path_size)
{
    char tmp_fname[_MAX_FNAME];
    memset(tmp_fname, 0, sizeof(tmp_fname));

    if (prefix){

        sprintf_s(tmp_fname, sizeof(tmp_fname), "%s_%s", prefix, fname);
    }else{

        sprintf_s(tmp_fname, sizeof(tmp_fname), "%s", fname);
    }

    errno_t err = _makepath_s(path, path_size, drive, dir, tmp_fname, ext);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    return true;
}

bool tp_get_drive_dir(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir)
{
    char base_dir[_MAX_PATH];
    memset(base_dir, 0, sizeof(base_dir));

    HMODULE handle = GetModuleHandleA(NULL);

    if (0 == handle){

        TP_GET_LAST_ERROR(symbol_table);

        return false;
    }

    DWORD module_status = GetModuleFileNameA(handle, base_dir, sizeof(base_dir));

    if (0 == module_status){

        return false;
    }

    errno_t err = _splitpath_s(base_dir, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    return true;
}

