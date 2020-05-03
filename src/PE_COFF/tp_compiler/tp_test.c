
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

TEST_CASE_TABLE tp_test_case_table[] = {

    { "int32_t value1 = 1 + 2;\n", 3 },
    { "int32_t value1 = 1 - 2;\n", -1 },
    { "int32_t value1 = 1 * 2;\n", 2 },
    { "int32_t value1 = 4 / 2;\n", 2 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = value1 + 2;\n", 3 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = value1 - 2;\n", -1 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = value1 * 2;\n", 2 },

    { "int32_t value1 = 4;\n"
    "int32_t value2 = value1 / 2;\n", 2 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = 2 + value1;\n", 3 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = 2 - value1;\n", 1 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = 2 * value1;\n", 2 },

    { "int32_t value1 = 2;\n"
    "int32_t value2 = 4 / value1;\n", 2 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = 2;\n"
    "int32_t value3 = value1 + value2;\n", 3 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = 2;\n"
    "int32_t value3 = value1 - value2;\n", -1 },

    { "int32_t value1 = 1;\n"
    "int32_t value2 = 2;\n"
    "int32_t value3 = value1 * value2;\n", 2 },

    { "int32_t value1 = 4;\n"
    "int32_t value2 = 2;\n"
    "int32_t value3 = value1 / value2;\n", 2 },

    { "int32_t value1 = (1 + 2) * 3;\n"
    "int32_t value2 = 2 + (3 * value1);\n"
    "value1 = value2 + 100;\n", 129 },

    { NULL, 0 }
};

static bool move_test_log_files(char* drive, char* dir_param, bool is_test, time_t now);

bool tp_test_compiler(
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    char* drive, char* dir, time_t now)
{
    bool status = true;

    bool is_test_mode = true;

    size_t size = (sizeof(tp_test_case_table) / sizeof(TEST_CASE_TABLE));

    for (size_t i = 0; size > i; ++i){

        uint8_t* source_code = tp_test_case_table[i].member_source_code;

        if (NULL == source_code){

            break;
        }

        int32_t return_value = 0;

        if ( ! tp_compiler_main(
            argc, argv, msg_buffer, msg_buffer_size,
            &is_test_mode, i, &return_value, drive, dir, now)){

            status = false;

            fprintf_s(stderr, "ERROR: compile failed. test case No.%03zd.\n", i + 1);
        }else{

            int32_t correct_value = tp_test_case_table[i].member_return_value;

            if (return_value == correct_value){

                fprintf_s(stderr, "SUCCESS: test case No.%03zd.\n", i + 1);
            }else{

                fprintf_s(
                    stderr, "ERROR: test case No.%03zd: "
                    "return value=(%d), correct value=(%d),\n"
                    "source code=(%s).\n",
                    i + 1, return_value, correct_value, source_code
                );
            }
        }
    }

    (void)move_test_log_files(drive, dir, is_test_mode, now);

    return status;
}

static bool move_test_log_files(char* drive, char* dir_param, bool is_test, time_t now)
{
    if ( ! is_test){

        return true;
    }

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

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    for (size_t i = 1; TP_TEST_FNAME_NUM_MAX >= i; ++i){

        char new_path[_MAX_PATH];
        memset(new_path, 0, sizeof(new_path));

        memset(dir, 0, sizeof(dir));

        sprintf_s(
            dir, sizeof(dir), "%s\\test_%04d-%02d-%02d_%03zd",
            dir_param, local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday, i
        );

        err = _makepath_s(new_path, sizeof(new_path), drive, dir, NULL, NULL);

        if (err){

            TP_PRINT_CRT_ERROR(NULL);

            return false;
        }

        DWORD attributes = GetFileAttributesA(new_path);

        if (-1 != attributes){

            continue;
        }

        SetLastError(NO_ERROR);

        if ( ! MoveFileA(path, new_path)){

            TP_GET_LAST_ERROR(NULL);

            return false;
        }

        return true;
    }

    fprintf_s(stderr, "ERROR: overflow move directory suffix number.\n");

    return false;
}

