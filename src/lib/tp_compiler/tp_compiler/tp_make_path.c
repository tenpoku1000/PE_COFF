
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool make_path_test_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param, char* fname,
    bool is_test, bool is_int_calc_compiler, size_t test_index, time_t now
);
static bool make_path_log_files_main(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir, char* fname,
    bool is_test, bool is_int_calc_compiler
);

bool tp_make_path_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param, char* fname,
    bool is_test, bool is_int_calc_compiler, size_t test_index, time_t now)
{
    if (is_test){

        if ( ! make_path_test_log_files(
            symbol_table, drive, dir_param, fname,
            is_test, is_int_calc_compiler, test_index, now)){

            fprintf_s(stderr, "ERROR: already exists.\n");

            return false;
        }
    }else{

        if ( ! make_path_log_files_main(
            symbol_table, drive, dir_param, fname,
            is_test, is_int_calc_compiler)){

            return false;
        }
    }

    return true;
}

static bool make_path_test_log_files(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir_param, char* fname,
    bool is_test, bool is_int_calc_compiler, size_t test_index, time_t now)
{
    struct tm local_time = { 0 };

    errno_t err = localtime_s(&local_time, &now);

    if (err){

        TP_PRINT_CRT_ERROR(NULL);

        return false;
    }

    char* dir_prefix =
        (is_int_calc_compiler ? TP_WRITE_TEST_DIR_PREFIX_CALC : TP_WRITE_TEST_DIR_PREFIX_C);

    char dir[_MAX_DIR] = { 0 };

    sprintf_s(
        dir, sizeof(dir), "%s%s%04d-%02d-%02d",
        dir_param, dir_prefix,
        local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday
    );

    char path[_MAX_PATH] = { 0 };

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
        dir, sizeof(dir), "%s%s%04d-%02d-%02d\\test_case_%03zd",
        dir_param, dir_prefix, local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday,
        test_index + 1
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

        if ( ! make_path_log_files_main(
            symbol_table, drive, dir, fname,
            is_test, is_int_calc_compiler)){

            return false;
        }

        return true;
    }

    return false;
}

static bool make_path_log_files_main(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir, char* fname,
    bool is_test, bool is_int_calc_compiler)
{
    char file_name[_MAX_FNAME] = { 0 };

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

    if (is_test){

        if ( ! tp_make_path(
            NULL, drive, dir, TP_SRC_FILE_PREFIX,
            TP_WRITE_TEST_SRC_DEFAULT_FILE_NAME,
            (is_int_calc_compiler ?
                TP_WRITE_TEST_SRC_DEFAULT_EXT_NAME :
                TP_WRITE_TEST_SRC_DEFAULT_C_EXT_NAME
            ), symbol_table->member_test_src_file_path,
            sizeof(symbol_table->member_test_src_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_READ_UNICODE_DATA_DEFAULT_FILE_NAME,
        TP_READ_UNICODE_DATA_DEFAULT_EXT_NAME,
        symbol_table->member_read_unicode_data_path,
        sizeof(symbol_table->member_read_unicode_data_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_UNICODE_DATA_DEFAULT_FILE_NAME,
        TP_WRITE_UNICODE_DATA_DEFAULT_EXT_NAME,
        symbol_table->member_unicode_data_path,
        sizeof(symbol_table->member_unicode_data_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_UNICODE_DATA_INDEX_DEFAULT_FILE_NAME,
        TP_WRITE_UNICODE_DATA_INDEX_DEFAULT_EXT_NAME,
        symbol_table->member_unicode_data_index_path,
        sizeof(symbol_table->member_unicode_data_index_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FIRST_SET_DEFAULT_FILE_NAME_1,
        TP_WRITE_FIRST_SET_DEFAULT_EXT_NAME,
        symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1],
        sizeof(symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FIRST_SET_DEFAULT_FILE_NAME_1_MAX_NUM,
        TP_WRITE_FIRST_SET_DEFAULT_EXT_NAME,
        symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1_MAX_NUM],
        sizeof(symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1_MAX_NUM]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FIRST_SET_DEFAULT_FILE_NAME_C,
        TP_WRITE_FIRST_SET_DEFAULT_EXT_NAME,
        symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C],
        sizeof(symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FIRST_SET_DEFAULT_FILE_NAME_C_MAX_NUM,
        TP_WRITE_FIRST_SET_DEFAULT_EXT_NAME,
        symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C_MAX_NUM],
        sizeof(symbol_table->member_first_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C_MAX_NUM]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FOLLOW_SET_DEFAULT_FILE_NAME_1,
        TP_WRITE_FOLLOW_SET_DEFAULT_EXT_NAME,
        symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1],
        sizeof(symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FOLLOW_SET_DEFAULT_FILE_NAME_1_MAX_NUM,
        TP_WRITE_FOLLOW_SET_DEFAULT_EXT_NAME,
        symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1_MAX_NUM],
        sizeof(symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1_MAX_NUM]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FOLLOW_SET_DEFAULT_FILE_NAME_C,
        TP_WRITE_FOLLOW_SET_DEFAULT_EXT_NAME,
        symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C],
        sizeof(symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_WRITE_FOLLOW_SET_DEFAULT_FILE_NAME_C_MAX_NUM,
        TP_WRITE_FOLLOW_SET_DEFAULT_EXT_NAME,
        symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C_MAX_NUM],
        sizeof(symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_C_MAX_NUM]))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
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
        TP_PP_TOKEN_DEFAULT_FILE_NAME, TP_PP_TOKEN_DEFAULT_EXT_NAME,
        symbol_table->member_pp_token_file_path,
        sizeof(symbol_table->member_pp_token_file_path))){

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
        (fname[0] ? fname : TP_WASM_DEFAULT_FILE_NAME),
        TP_WASM_DEFAULT_EXT_NAME,
        symbol_table->member_wasm_file_path,
        sizeof(symbol_table->member_wasm_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (fname[0]){

        sprintf_s(file_name, sizeof(file_name), "%s_wasm", fname);
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? file_name : TP_WASM_TEXT_DEFAULT_FILE_NAME),
        TP_WASM_TEXT_DEFAULT_EXT_NAME,
        symbol_table->member_wasm_text_file_path,
        sizeof(symbol_table->member_wasm_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? fname : TP_X64_TEXT_DEFAULT_FILE_NAME),
        TP_X64_DEFAULT_EXT_NAME,
        symbol_table->member_x64_file_path,
        sizeof(symbol_table->member_x64_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (fname[0]){

        sprintf_s(file_name, sizeof(file_name), "%s_bin", fname);
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? file_name : TP_X64_TEXT_DEFAULT_FILE_NAME),
        TP_X64_TEXT_DEFAULT_EXT_NAME,
        symbol_table->member_x64_text_file_path,
        sizeof(symbol_table->member_x64_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? fname : TP_COFF_OBJECT_DEFAULT_FNAME),
        TP_COFF_OBJECT_DEFAULT_EXT,
        symbol_table->member_coff_file_path,
        sizeof(symbol_table->member_coff_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (fname[0]){

        sprintf_s(file_name, sizeof(file_name), "%s_obj", fname);
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? file_name : TP_COFF_OBJECT_TEXT_DEFAULT_FNAME),
        TP_COFF_OBJECT_TEXT_DEFAULT_EXT,
        symbol_table->member_coff_text_file_path,
        sizeof(symbol_table->member_coff_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? file_name : TP_COFF_CODE_DEFAULT_FNAME),
        TP_COFF_CODE_DEFAULT_EXT,
        symbol_table->member_coff_code_file_path,
        sizeof(symbol_table->member_coff_code_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (fname[0]){

        sprintf_s(file_name, sizeof(file_name), "%s_obj_bin", fname);
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        (fname[0] ? file_name : TP_COFF_CODE_TEXT_DEFAULT_FNAME),
        TP_COFF_CODE_TEXT_DEFAULT_EXT,
        symbol_table->member_coff_code_text_file_path,
        sizeof(symbol_table->member_coff_code_text_file_path))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    switch (symbol_table->member_subsystem){
    case TP_IMAGE_SUBSYSTEM_WINDOWS_GUI:
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_DLL_DEFAULT_FNAME, TP_PE_DLL_DEFAULT_EXT,
            symbol_table->member_pe_file_path,
            sizeof(symbol_table->member_pe_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_DLL_TEXT_DEFAULT_FNAME, TP_PE_DLL_TEXT_DEFAULT_EXT,
            symbol_table->member_pe_text_file_path,
            sizeof(symbol_table->member_pe_text_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_DLL_CODE_DEFAULT_FNAME, TP_PE_DLL_CODE_DEFAULT_EXT,
            symbol_table->member_pe_code_file_path,
            sizeof(symbol_table->member_pe_code_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_DLL_CODE_TEXT_DEFAULT_FNAME, TP_PE_DLL_CODE_TEXT_DEFAULT_EXT,
            symbol_table->member_pe_code_text_file_path,
            sizeof(symbol_table->member_pe_code_text_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    case TP_IMAGE_SUBSYSTEM_WINDOWS_CUI:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    case TP_IMAGE_SUBSYSTEM_EFI_APPLICATION:
//      break;
    case TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
//      break;
    case TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_UEFI_DEFAULT_FNAME, TP_PE_UEFI_DEFAULT_EXT,
            symbol_table->member_pe_file_path,
            sizeof(symbol_table->member_pe_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_UEFI_TEXT_DEFAULT_FNAME, TP_PE_UEFI_TEXT_DEFAULT_EXT,
            symbol_table->member_pe_text_file_path,
            sizeof(symbol_table->member_pe_text_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_UEFI_CODE_DEFAULT_FNAME, TP_PE_UEFI_CODE_DEFAULT_EXT,
            symbol_table->member_pe_code_file_path,
            sizeof(symbol_table->member_pe_code_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_UEFI_CODE_TEXT_DEFAULT_FNAME, TP_PE_UEFI_CODE_TEXT_DEFAULT_EXT,
            symbol_table->member_pe_code_text_file_path,
            sizeof(symbol_table->member_pe_code_text_file_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ( ! tp_make_path(
        symbol_table, drive, dir, NULL,
        TP_PE_DLL_DEFAULT_FNAME, TP_PE_DLL_DEFAULT_EXT,
        symbol_table->member_load_dll_file_path,
        sizeof(symbol_table->member_load_dll_file_path))){

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
    char tmp_fname[_MAX_FNAME] = { 0 };

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
    char base_dir[_MAX_PATH] = { 0 };

    HMODULE handle = GetModuleHandleA(NULL);

    if (0 == handle){

        TP_GET_LAST_ERROR(symbol_table);

        return false;
    }

    DWORD module_status = GetModuleFileNameA(handle, base_dir, sizeof(base_dir));

    if (0 == module_status){

        TP_GET_LAST_ERROR(symbol_table);

        return false;
    }

    errno_t err = _splitpath_s(base_dir, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    return true;
}

bool tp_get_current_drive_dir(
    TP_SYMBOL_TABLE* symbol_table, char* drive, char* dir)
{
    char base_dir[_MAX_PATH] = { 0 };

    DWORD status = GetCurrentDirectoryA(sizeof(base_dir), base_dir);

    if (0 == status){

        TP_GET_LAST_ERROR(symbol_table);

        return false;
    }

    errno_t err = _splitpath_s(base_dir, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    return true;
}

