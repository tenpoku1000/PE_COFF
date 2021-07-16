
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

TEST_CASE_TABLE tp_C_test_case_table[] = {
    // No.1
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1 + 2;\n"
      "    return value1;\n"
      "}\n", 3 },

    // No.2
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1 - 2;\n"
      "    return value1;\n"
      "}\n", -1 },

    // No.3
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1 * 2;\n"
      "    return value1;\n"
      "}\n", 2 },

    // No.4
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 4 / 2;\n"
      "    return value1;\n"
      "}\n", 2 },

    // No.5
    { "int calc(void)\n"
       "{\n"
       "    int value1 = 1;\n"
       "    int value2 = value1 + 2;\n"
       "    return value2;\n"
       "}\n", 3 },

    // No.6
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = value1 - 2;\n"
      "    return value2;\n"
      "}\n", -1 },

    // No.7
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = value1 * 2;\n"
      "    return value2;\n"
      "}\n", 2 },

    // No.8
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 4;\n"
      "    int value2 = value1 / 2;\n"
      "    return value2;\n"
      "}\n", 2 },

    // No.9
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = 2 + value1;\n"
      "    return value2;\n"
      "}\n", 3 },

    // No.10
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = 2 - value1;\n"
      "    return value2;\n"
      "}\n", 1 },

    // No.11
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = 2 * value1;\n"
      "    return value2;\n"
      "}\n", 2 },

    // No.12
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 2;\n"
      "    int value2 = 4 / value1;\n"
      "    return value2;\n"
      "}\n", 2 },

    // No.13
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = 2;\n"
      "    int value3 = value1 + value2;\n"
      "    return value3;\n"
      "}\n", 3 },

    // No.14
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = 2;\n"
      "    int value3 = value1 - value2;\n"
      "    return value3;\n"
      "}\n", -1 },

    // No.15
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 1;\n"
      "    int value2 = 2;\n"
      "    int value3 = value1 * value2;\n"
      "    return value3;\n"
      "}\n", 2 },

    // No.16
    { "int calc(void)\n"
      "{\n"
      "    int value1 = 4;\n"
      "    int value2 = 2;\n"
      "    int value3 = value1 / value2;\n"
      "    return value3;\n"
      "}\n", 2 },

    // No.17
    { "int calc(void)\n"
      "{\n"
      "    int value1 = (1 + 2) * 3;\n"
      "    int value2 = 2 + (3 * value1);\n"
      "    value1 = value2 + 100;\n"
      "    return value1;\n"
      "}\n", 129 },

    // No.18
    { "int calc(void)\n"
      "{\n"
      "    int val\\\n"
      "ue1 = (1 + 2) * 3;\n"
      "    int value2 = 2 + (3 * valu\\\n"
      "e1);\n"
      "    value1 = value2 + 100;\n"
      "    return value1;\n"
      "}\n", 129 },

    // No.19
    { "int calc(void)\n"
      "{\n"
      "    int v\\\n"
      "al\\\n"
      "ue1 = (1 + 2) * 3;\n"
      "    int value2 = 2 + (3 * val\\\n"
      "u\\\n"
      "e1);\n"
      "    value1 = value2 + 100;\n"
      "    return value1;\n"
      "}\n", 129 },

    // No.20
    { "int calc_body(void)\n"
      "{\n"
      "    int value1 = (1 + 2) * 3;\n"
      "    int value2 = 2 + (3 * value1);\n"
      "    value1 = value2 + 100;\n"
      "    return value1;\n"
      "}\n"
      "int calc(void)\n"
      "{\n"
      "    return calc_body();\n"
      "}\n", 129 },

    // No.21
    { "int calc_body(int value1, int value2)\n"
      "{\n"
      "    value1 = value2 + 100;\n"
      "    return value1;\n"
      "}\n"
      "int calc(void)\n"
      "{\n"
      "    int value1 = (1 + 2) * 3;\n"
      "    int value2 = 2 + (3 * value1);\n"
      "    return calc_body(value1, value2);\n"
      "}\n", 129 },

    // No.22
    { "int calc_body(int value1, int value2, int value3)\n"
      "{\n"
      "    value1 = value2 + value3;\n"
      "    return value1;\n"
      "}\n"
      "int calc(void)\n"
      "{\n"
      "    int value1 = 0;\n"
      "    int value2 = 2 + (3 * value1);\n"
      "    return calc_body(value1 = (1 + 2) * 3, value2, 100);\n"
      "}\n", 102 },

    // No.23
    { "int g_value0 = 0;\n"
      "\n"
      "int calc_body(int value1, int value2, int value3, int value4)\n"
      "{\n"
      "    value1 = value2 + value3 + value4;\n"
      "    g_value0 = value1;\n"
      "    return g_value0;\n"
      "}\n"
      "\n"
      "int g_value1 = 0;\n"
      "int g_value3 = 49;\n"
      "\n"
      "int calc(void)\n"
      "{\n"
      "    int value2 = 2 + (3 * g_value1);\n"
      "    calc_body(g_value1 = (1 + 2) * 3, value2, g_value3, 51);\n"
      "    return value2;\n"
      "}\n", 2 },

    // No.24
    { "int g_value0 = 0;\n"
      "int value1 = 1;\n"
      "\n"
      "int calc(void)\n"
      "{\n"
      "    int value1 = 2;\n"
      "    {\n"
      "        int value1 = 3;\n"
      "        {\n"
      "            int value1 = 4;\n"
      "            {\n"
      "                int value1 = 5;\n"
      "                g_value0 = value1;\n"
      "            }\n"
      "        }\n"
      "    }\n"
      "    return g_value0;\n"
      "}\n", 5 },

    // No.25
    { "int plus(int x, int y)\n"
      "{\n"
      "    return x + y;\n"
      "}\n"
      "\n"
      "int minus(int x, int y)\n"
      "{\n"
      "    return x - y;\n"
      "}\n"
      "\n"
      "int calc(void)\n"
      "{\n"
      "    return plus(1, minus(3, plus(1, 1) + 1) + 1) + 1;\n"
      "}\n", 3 },

    // No.26
    { "int plus(int x, int y)\n"
      "{\n"
      "    return x + y;\n"
      "}\n"
      "\n"
      "int minus(int x, int y)\n"
      "{\n"
      "    return x - y;\n"
      "}\n"
      "\n"
      "int calc(void)\n"
      "{\n"
      "    int v = 1;\n"
      "    int w = 3;\n"
      "    return plus(v, minus(w, plus(1, 1) + 1) + 1) + 1;\n"
      "}\n", 3 },

    // No.27
    { "int x(int i, int j)\n"
      "{\n"
      "    return i + j;\n"
      "}\n"
      "\n"
      "int y(int i, int j)\n"
      "{\n"
      "    return i * j;\n"
      "}\n"
      "\n"
      "int z(int i, int j)\n"
      "{\n"
      "    return i / j;\n"
      "}\n"
      "\n"
      "int calc(void)\n"
      "{\n"
      "    int v = 100;\n"
      "    int w =  x(y(z(v + 1, 2), 3), y(4, 5));\n"
      "    return w;\n"
      "}\n", 170 },

    // No.28
    { "int calc_body2(int value1)\n"
      "{\n"
      "    return value1 = value1 + 100;\n"
      "}\n"
      "\n"
      "int calc_body(int value1, int value2, int value3, int value4)\n"
      "{\n"
      "    return calc_body2(value1 = value2 + value3 + value4);\n"
      "}\n"
      "\n"
      "int g_value1 = 0;\n"
      "int g_value3 = 49;\n"
      "\n"
      "int calc(void)\n"
      "{\n"
      "    int value2 = 2 + (3 * g_value1);\n"
      "    return calc_body(g_value1 = (1 + 2) * 3, value2, g_value3, 51);\n"
      "}\n", 202 },

    { NULL, 0 }
};

TEST_CASE_TABLE tp_test_case_table[] = {

    { "int value1 = 1 + 2;\n", 3 },    // No.1
    { "int value1 = 1 - 2;\n", -1 },   // No.2
    { "int value1 = 1 * 2;\n", 2 },    // No.3
    { "int value1 = 4 / 2;\n", 2 },    // No.4

    { "int value1 = 1;\n"
    "int value2 = value1 + 2;\n", 3 }, // No.5

    { "int value1 = 1;\n"
    "int value2 = value1 - 2;\n", -1 },// No.6

    { "int value1 = 1;\n"
    "int value2 = value1 * 2;\n", 2 }, // No.7

    { "int value1 = 4;\n"
    "int value2 = value1 / 2;\n", 2 }, // No.8

    { "int value1 = 1;\n"
    "int value2 = 2 + value1;\n", 3 }, // No.9

    { "int value1 = 1;\n"
    "int value2 = 2 - value1;\n", 1 }, // No.10

    { "int value1 = 1;\n"
    "int value2 = 2 * value1;\n", 2 }, // No.11

    { "int value1 = 2;\n"
    "int value2 = 4 / value1;\n", 2 }, // No.12

    { "int value1 = 1;\n"
    "int value2 = 2;\n"
    "int value3 = value1 + value2;\n", 3 }, // No.13

    { "int value1 = 1;\n"
    "int value2 = 2;\n"
    "int value3 = value1 - value2;\n", -1 },// No.14

    { "int value1 = 1;\n"
    "int value2 = 2;\n"
    "int value3 = value1 * value2;\n", 2 }, // No.15

    { "int value1 = 4;\n"
    "int value2 = 2;\n"
    "int value3 = value1 / value2;\n", 2 }, // No.16

    { "int value1 = (1 + 2) * 3;\n"
    "int value2 = 2 + (3 * value1);\n"
    "value1 = value2 + 100;\n", 129 },   // No.17

    { "int val\\\n"
      "ue1 = (1 + 2) * 3;\n"
      "int value2 = 2 + (3 * valu\\\n"
      "e1);\n"
      "value1 = value2 + 100;\n", 129 }, // No.18

    { "int v\\\n"
      "al\\\n"
      "ue1 = (1 + 2) * 3;\n"
      "int value2 = 2 + (3 * val\\\n"
      "u\\\n"
      "e1);\n"
      "value1 = value2 + 100;\n", 129 }, // No.19

    { NULL, 0 }
};

static bool move_test_log_files(
    bool is_int_calc_compiler, char* drive, char* dir_param, bool is_test, time_t now
);

bool tp_test_compiler(
    bool is_int_calc_compiler, size_t test_case_index,
    int argc, char** argv, uint8_t* msg_buffer, size_t msg_buffer_size,
    char* drive, char* dir, time_t now)
{
    bool status = true;
    bool is_test_mode = true;

    size_t test_success_num = 0;
    size_t test_failure_num = 0;
    size_t test_num = (is_int_calc_compiler ?
        (sizeof(tp_test_case_table) / sizeof(TEST_CASE_TABLE)) :
        (sizeof(tp_C_test_case_table) / sizeof(TEST_CASE_TABLE)));

    if (test_case_index){

        if (test_num < test_case_index){

            TP_PUT_LOG_MSG(
                NULL, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Bad index number of test case[test_num(%1) < test_case_index(%2)]."),
                TP_LOG_PARAM_UINT64_VALUE(test_num),
                TP_LOG_PARAM_UINT64_VALUE(test_case_index)
            );

            return false;
        }

        test_num = test_case_index;

        --test_case_index;
    }

    for (size_t i = test_case_index; test_num > i; ++i){

        uint8_t* source_code = (is_int_calc_compiler ?
            tp_test_case_table[i].member_source_code :
            tp_C_test_case_table[i].member_source_code);

        if (NULL == source_code){

            break;
        }

        int32_t correct_value = (is_int_calc_compiler ?
            tp_test_case_table[i].member_return_value :
            tp_C_test_case_table[i].member_return_value);

        int32_t return_value = 0;

        if ( ! tp_compiler_main(
            argc, argv, msg_buffer, msg_buffer_size,
            &is_test_mode, &is_int_calc_compiler, &test_case_index,
            i, &return_value, drive, dir, now)){

            status = false;

            ++test_failure_num;

            fprintf_s(
                stderr, "ERROR: compile failed. "
                "test case No.%03zd: correct value=(%d),\n"
                "source code:\n%s\n", i + 1, correct_value, source_code
            );
        }else{

            if (return_value == correct_value){

                ++test_success_num;

                fprintf_s(stderr, "SUCCESS: test case No.%03zd.\n\n", i + 1);
            }else{

                ++test_failure_num;

                fprintf_s(
                    stderr, "ERROR: test case No.%03zd: "
                    "return value=(%d), correct value=(%d),\n"
                    "source code:\n%s\n",
                    i + 1, return_value, correct_value, source_code
                );
            }
        }
    }

    fprintf_s(
        stderr,
        "%s: total(%zd), ok(%zd), ng(%zd)\n",
        (is_int_calc_compiler ? "test_calc" : "test_C"),
        (test_case_index ? 1 : (test_num - 1)),
        test_success_num, test_failure_num
    );

    (void)move_test_log_files(is_int_calc_compiler, drive, dir, is_test_mode, now);

    return status;
}

static bool move_test_log_files(
    bool is_int_calc_compiler, char* drive, char* dir_param, bool is_test, time_t now)
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

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    for (size_t i = 1; TP_TEST_FNAME_NUM_MAX >= i; ++i){

        char new_path[_MAX_PATH] = { 0 };

        memset(dir, 0, sizeof(dir));

        sprintf_s(
            dir, sizeof(dir), "%s%s%04d-%02d-%02d_%03zd",
            dir_param, dir_prefix,
            local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday, i
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

