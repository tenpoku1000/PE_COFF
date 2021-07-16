
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef enum TP_OPERAND_NUM_{
    TP_OPERAND_NUM_0 = 0,
    TP_OPERAND_NUM_1,
    TP_OPERAND_NUM_2
}TP_OPERAND_NUM;

typedef enum TP_IS_NEED_OP_REG_{
    TP_IS_NOT_NEED_OP_REG = 0,
    TP_IS_NEED_OP_REG32,
    TP_IS_NEED_OP_REG64
}TP_IS_NEED_OP_REG;

typedef enum TP_IS_NEED_REL_{
    TP_IS_NOT_NEED_REL = 0,
    TP_IS_NEED_REL8,
    TP_IS_NEED_REL32
}TP_IS_NEED_REL;

typedef enum TP_IS_NEED_DISP_{
    TP_IS_NOT_NEED_DISP = 0,
    TP_IS_NEED_DISP8,
    TP_IS_NEED_DISP32
}TP_IS_NEED_DISP;

typedef enum TP_IS_NEED_IMM_{
    TP_IS_NOT_NEED_IMM = 0,
    TP_IS_NEED_IMM8,
    TP_IS_NEED_IMM32,
    TP_IS_NEED_IMM64
}TP_IS_NEED_IMM;

typedef enum TP_EXCEPTIONAL_ADDRESSING_MODE_{
    TP_EXCEPTIONAL_ADDRESSING_MODE_NONE = 0,
    TP_EXCEPTIONAL_ADDRESSING_MODE_1,
    TP_EXCEPTIONAL_ADDRESSING_MODE_2
}TP_EXCEPTIONAL_ADDRESSING_MODE;

#define TP_DISASM_COLUMS 7

#define TP_DISASM_STRING_CALL "call"
#define TP_DISASM_STRING_LEA "lea"
#define TP_DISASM_STRING_NONE "none"
#define TP_DISASM_STRING_SIZE 128

typedef struct TP_DISASM_PARAMS_{
    bool member_is_bad;
    bool member_is_unknown;
    uint8_t* member_opcode;  // NOTE: must not free memory.
    TP_OPERAND_NUM member_operand_num;
    uint8_t member_disasm_string[TP_DISASM_STRING_SIZE];
    uint8_t member_additional_string[TP_DISASM_STRING_SIZE];

    // opcode
    uint32_t member_match_bytes;
    bool member_is_need_mod_rm;
    TP_IS_NEED_IMM member_is_need_imm;
    TP_IS_NEED_OP_REG member_is_need_op_reg;
    TP_IS_NEED_REL member_is_need_rel;
    bool member_is_rex;
    bool member_is_rex_W;
    bool member_is_rex_R;
    bool member_is_rex_X;
    bool member_is_rex_B;
    uint8_t member_x64_opcode_1;
    uint8_t* member_x64_opcode_2;  // NOTE: must not free memory.
    bool member_is_reg_dst;

    // ModR/M
    uint8_t member_mod;
    bool member_is_mod11;
    uint8_t member_rm;
    bool member_is_RIP;
    bool member_is_need_sib;
    TP_IS_NEED_DISP member_is_need_disp;
    uint8_t* member_register_name_reg;  // NOTE: must not free memory.
    uint8_t* member_register_name_rm;  // NOTE: must not free memory.

    // SIB
    uint8_t member_sib_base;
    uint8_t* member_register_name_base;  // NOTE: must not free memory.
    uint8_t member_sib_index;
    uint8_t* member_register_name_index;  // NOTE: must not free memory.
    uint8_t member_sib_scale;
    TP_EXCEPTIONAL_ADDRESSING_MODE member_exceptional_addressing_mode;
}TP_DISASM_PARAMS;

static bool print_disasm_lines(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint32_t x64_code_pos_begin, uint32_t x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static bool set_opcode_string(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static uint8_t* get_opcode_string(TP_DISASM_PARAMS* disasm_params);
static bool set_operand_string(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static bool set_operand_string_ModRM(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static bool set_operand_string_SIB(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static bool set_operand_string_EXCEPTIONAL_ADDRESSING_MODE(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static bool set_operand_string_DISP_IMM(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params
);
static uint8_t* get_register_name(
    uint8_t byte, bool is_rex_W, bool is_rex_B_or_R_or_X
);

typedef struct TP_DISASM_TEST_{
    uint8_t member_bin_num;
    uint8_t member_bin[10];
    uint8_t* member_string;
    uint8_t* member_additional_string;
}TP_DISASM_TEST;

static TP_DISASM_TEST global_disasm_test[] = {
    {  1, { 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "nop", NULL },
    {  1, { 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "int 3", NULL },
    {  1, { 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "push rbp", NULL },
    {  1, { 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "(bad)", NULL  },
    {  3, { 0xfc, 0xfc, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "(unknown)", NULL  },
    {  7, { 0x48, 0x81, 0xec, 0x98, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 }, "sub rsp,0x198", NULL  },
    {  5, { 0x48, 0x8d, 0x6c, 0x24, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 }, "lea rbp,[rsp+0x20]", NULL  },
    {  7, { 0x48, 0x89, 0x15, 0xf9, 0xdf, 0xff, 0xff, 0x00, 0x00, 0x00 },
            "mov QWORD PTR [rip+0xffffffffffffdff9],rdx", "# 0xffffffffffffe000"  },
    {  7, { 0x4c, 0x8b, 0x05, 0xf2, 0xdf, 0xff, 0xff, 0x00, 0x00, 0x00 },
            "mov r8,QWORD PTR [rip+0xffffffffffffdff2]", "# 0xffffffffffffdff9"  },
    {  7, { 0x48, 0x8d, 0x15, 0xa0, 0xef, 0xff, 0xff, 0x00, 0x00, 0x00 },
            "lea rdx,[rip+0xffffffffffffefa0]", "# 0xffffffffffffefa7"  },
    {  2, { 0xff, 0xd3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "call rbx", NULL  },
    {  5, { 0xe8, 0x4a, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 },
            "call QWORD PTR [rip+0xffffffffffffff4a]", "# 0xffffffffffffff4f"  },
    {  3, { 0x48, 0x8b, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov rsi,rbp", NULL  },
    { 10, { 0x48, 0xbf, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov rdi,0x10", NULL  },
    {  3, { 0x48, 0x03, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "add rsi,rdi", NULL  },
    {  4, { 0x48, 0x8d, 0x14, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "lea rdx,[rsi+none*1]", NULL  },
    {  5, { 0x48, 0x8b, 0x74, 0x27, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov rsi,QWORD PTR [rdi+none*1+0x8]", NULL  },
    {  3, { 0x49, 0x2b, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "sub rax,r8", NULL  },
    {  2, { 0x75, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "jne 0xc0", NULL  },
    {  5, { 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov ecx,0x0", NULL  },
    { 10, { 0x49, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov r8,0x0", NULL  },
    {  5, { 0x49, 0x8b, 0x7c, 0x22, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov rdi,QWORD PTR [r10+none*1+0x68]", NULL  },
    {  2, { 0xf7, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "imul ecx", NULL  },
    {  3, { 0x0f, 0xaf, 0xca, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "imul ecx,edx", NULL  },
    {  4, { 0x89, 0x44, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov DWORD PTR [rbp+none*1+0x0],eax", NULL  },
    {  4, { 0x8b, 0x54, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov edx,DWORD PTR [rbp+none*1+0x0]", NULL  },
    {  4, { 0x89, 0x44, 0x25, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov DWORD PTR [rbp+none*1+0x4],eax", NULL  },
    {  4, { 0x8b, 0x44, 0x25, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov eax,DWORD PTR [rbp+none*1+0x4]", NULL  },
    {  5, { 0xb9, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "mov ecx,0x64", NULL  },
    {  2, { 0x03, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "add eax,ecx", NULL  },
    {  2, { 0xf7, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "idiv ecx", NULL  },
    {  2, { 0x33, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "xor eax,ecx", NULL  },
    {  7, { 0x48, 0x81, 0xc4, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "add rsp,0x50", NULL  },
    {  1, { 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "pop rbp", NULL  },
    {  1, { 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, "ret", NULL  },
    {  0, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, NULL, NULL  },
};
/*
tp_make_x64_disasm.c(469): SUCCESS: nop, nop
tp_make_x64_disasm.c(469): SUCCESS: int 3, int 3
tp_make_x64_disasm.c(469): SUCCESS: push rbp, push rbp
tp_make_x64_disasm.c(424): === x64 disasm is (bad). ===
tp_make_x64_disasm.c(469): SUCCESS: (bad), (bad)
tp_make_x64_disasm.c(411): === x64 disasm is (unknown). ===
tp_make_x64_disasm.c(469): SUCCESS: (unknown), (unknown)
tp_make_x64_disasm.c(469): SUCCESS: sub rsp,0x198, sub rsp,0x198
tp_make_x64_disasm.c(469): SUCCESS: lea rbp,[rsp+0x20], lea rbp,[rsp+0x20]
tp_make_x64_disasm.c(461): SUCCESS: mov QWORD PTR [rip+0xffffffffffffdff9],rdx, mov QWORD PTR [rip+0xffffffffffffdff9],rdx # 0xffffffffffffe000
tp_make_x64_disasm.c(461): SUCCESS: mov r8,QWORD PTR [rip+0xffffffffffffdff2], mov r8,QWORD PTR [rip+0xffffffffffffdff2] # 0xffffffffffffdff9
tp_make_x64_disasm.c(461): SUCCESS: lea rdx,[rip+0xffffffffffffefa0], lea rdx,[rip+0xffffffffffffefa0] # 0xffffffffffffefa7
tp_make_x64_disasm.c(469): SUCCESS: call rbx, call rbx
tp_make_x64_disasm.c(461): SUCCESS: call QWORD PTR [rip+0xffffffffffffff4a], call QWORD PTR [rip+0xffffffffffffff4a] # 0xffffffffffffff4f
tp_make_x64_disasm.c(469): SUCCESS: mov rsi,rbp, mov rsi,rbp
tp_make_x64_disasm.c(469): SUCCESS: mov rdi,0x10, mov rdi,0x10
tp_make_x64_disasm.c(469): SUCCESS: add rsi,rdi, add rsi,rdi
tp_make_x64_disasm.c(469): SUCCESS: lea rdx,[rsi+none*1], lea rdx,[rsi+none*1]
tp_make_x64_disasm.c(469): SUCCESS: mov rsi,QWORD PTR [rdi+none*1+0x8], mov rsi,QWORD PTR [rdi+none*1+0x8]
tp_make_x64_disasm.c(469): SUCCESS: sub rax,r8, sub rax,r8
tp_make_x64_disasm.c(469): SUCCESS: jne 0xc0, jne 0xc0
tp_make_x64_disasm.c(469): SUCCESS: mov ecx,0x0, mov ecx,0x0
tp_make_x64_disasm.c(469): SUCCESS: mov r8,0x0, mov r8,0x0
tp_make_x64_disasm.c(469): SUCCESS: mov rdi,QWORD PTR [r10+none*1+0x68], mov rdi,QWORD PTR [r10+none*1+0x68]
tp_make_x64_disasm.c(469): SUCCESS: imul ecx, imul ecx
tp_make_x64_disasm.c(469): SUCCESS: imul ecx,edx, imul ecx,edx
tp_make_x64_disasm.c(469): SUCCESS: mov DWORD PTR [rbp+none*1+0x0],eax, mov DWORD PTR [rbp+none*1+0x0],eax
tp_make_x64_disasm.c(469): SUCCESS: mov edx,DWORD PTR [rbp+none*1+0x0], mov edx,DWORD PTR [rbp+none*1+0x0]
tp_make_x64_disasm.c(469): SUCCESS: mov DWORD PTR [rbp+none*1+0x4],eax, mov DWORD PTR [rbp+none*1+0x4],eax
tp_make_x64_disasm.c(469): SUCCESS: mov eax,DWORD PTR [rbp+none*1+0x4], mov eax,DWORD PTR [rbp+none*1+0x4]
tp_make_x64_disasm.c(469): SUCCESS: mov ecx,0x64, mov ecx,0x64
tp_make_x64_disasm.c(469): SUCCESS: add eax,ecx, add eax,ecx
tp_make_x64_disasm.c(469): SUCCESS: idiv ecx, idiv ecx
tp_make_x64_disasm.c(469): SUCCESS: xor eax,ecx, xor eax,ecx
tp_make_x64_disasm.c(469): SUCCESS: add rsp,0x50, add rsp,0x50
tp_make_x64_disasm.c(469): SUCCESS: pop rbp, pop rbp
tp_make_x64_disasm.c(469): SUCCESS: ret, ret
tp_make_x64_disasm.c(259): test_disasm: total(35), ok(35), ng(0)
*/

bool tp_test_disasm_x64(TP_SYMBOL_TABLE* symbol_table)
{
    int32_t total = 0;
    int32_t ok = 0;
    int32_t ng = 0;

    for (rsize_t i = 0; ; ++i){

        if (NULL == global_disasm_test[i].member_string){

            break;
        }

        ++total;

        if ( ! tp_disasm_x64(
            symbol_table, NULL,
            global_disasm_test[i].member_bin, global_disasm_test[i].member_bin_num,
            global_disasm_test[i].member_string,
            global_disasm_test[i].member_additional_string)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            ++ng;

            continue;
        }

        ++ok;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("test_disasm: total(%1), ok(%2), ng(%3)"),
        TP_LOG_PARAM_INT32_VALUE(total),
        TP_LOG_PARAM_INT32_VALUE(ok),
        TP_LOG_PARAM_INT32_VALUE(ng)
    );

    return true;
}

bool tp_disasm_x64(
    TP_SYMBOL_TABLE* symbol_table, char* path,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint8_t* disasm_string, uint8_t* disasm_additional_string)
{
    bool status = false;

    FILE* write_file = NULL;

    if (path){

        if ( ! tp_open_write_file(symbol_table, path, &write_file)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    uint32_t x64_code_pos_begin = 0;
    bool first_print = true;

    for (uint32_t x64_code_body_pos = 0;
        x64_code_buffer_size > x64_code_body_pos; ++x64_code_body_pos){

        // Skip of zero fill bytes.
        if (write_file &&
            (x64_code_buffer_size > (x64_code_body_pos + TP_DISASM_COLUMS))){

            uint8_t zero_fill_bytes[TP_DISASM_COLUMS] = { 0 };

            if (0 == memcmp(
                zero_fill_bytes, &(x64_code_buffer[x64_code_body_pos]), TP_DISASM_COLUMS)){

                if (first_print){

                    fprintf(write_file, "...\n");

                    first_print = false;
                }
                continue;
            }
        }

        TP_DISASM_PARAMS disasm_params = {
            .member_opcode = "(null)",
            .member_operand_num = TP_OPERAND_NUM_2,
            .member_register_name_reg = "(null)",
            .member_register_name_rm = "(null)",
            .member_register_name_base = "(null)",
            .member_register_name_index = "(null)"
        };

        uint8_t byte = x64_code_buffer[x64_code_body_pos];

        // REX
        if (0x40 == (byte & 0xf0)){

            // 40h ～ 4fh: 0100 WRXB
            uint8_t rex = (byte & 0x0f);

            disasm_params.member_is_rex = true;
            if (rex & 0x08){  disasm_params.member_is_rex_W = true; }
            if (rex & 0x04){  disasm_params.member_is_rex_R = true; }
            if (rex & 0x02){  disasm_params.member_is_rex_X = true; }
            if (rex & 0x01){  disasm_params.member_is_rex_B = true; }
        }

        x64_code_pos_begin = x64_code_body_pos;

        // opcode
        if ( ! set_opcode_string(
            symbol_table, x64_code_buffer,
            x64_code_buffer_size, &x64_code_body_pos,
            &disasm_params)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        if (disasm_params.member_is_unknown){

            if ((NULL == path) && disasm_string){

                goto test;
            }

            if ( ! print_disasm_lines(
                symbol_table, write_file,
                x64_code_buffer, x64_code_buffer_size,
                x64_code_pos_begin, x64_code_body_pos,
                &disasm_params)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            continue;
        }

        if (disasm_params.member_is_bad){

            if ((NULL == path) && disasm_string){

                goto test;
            }

            if ( ! print_disasm_lines(
                symbol_table, write_file,
                x64_code_buffer, x64_code_buffer_size,
                x64_code_pos_begin, x64_code_body_pos,
                &disasm_params)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            continue;
        }

        // operand
        if ( ! set_operand_string(
            symbol_table, x64_code_buffer,
            x64_code_buffer_size, &x64_code_body_pos,
            &disasm_params)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        // test
        if ((NULL == path) && disasm_string){
test:
            if (disasm_params.member_is_unknown){

                sprintf_s(
                    disasm_params.member_disasm_string,
                    sizeof(disasm_params.member_disasm_string), "(unknown)"
                );

                TP_PUT_LOG_PRINT(
                    symbol_table,
                    TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("=== x64 disasm is (unknown). ===")
                );
            }

            if (disasm_params.member_is_bad){

                sprintf_s(
                    disasm_params.member_disasm_string,
                    sizeof(disasm_params.member_disasm_string), "(bad)"
                );

                TP_PUT_LOG_PRINT(
                    symbol_table,
                    TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("=== x64 disasm is (bad). ===")
                );
            }

            if (strcmp(disasm_string, disasm_params.member_disasm_string)){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: %1, %2"),
                    TP_LOG_PARAM_STRING(disasm_string),
                    TP_LOG_PARAM_STRING(disasm_params.member_disasm_string)
                );

                goto fail;
            }else if (disasm_additional_string &&
                *(disasm_params.member_additional_string) &&
                strcmp(disasm_additional_string,
                    disasm_params.member_additional_string)){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: %1 %2, %3"),
                    TP_LOG_PARAM_STRING(disasm_string),
                    TP_LOG_PARAM_STRING(disasm_additional_string),
                    TP_LOG_PARAM_STRING(disasm_params.member_additional_string)
                );

                goto fail;
            }

            if (disasm_additional_string){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("SUCCESS: %1, %2 %3"),
                    TP_LOG_PARAM_STRING(disasm_string),
                    TP_LOG_PARAM_STRING(disasm_params.member_disasm_string),
                    TP_LOG_PARAM_STRING(disasm_params.member_additional_string)
                );
            }else{

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("SUCCESS: %1, %2"),
                    TP_LOG_PARAM_STRING(disasm_string),
                    TP_LOG_PARAM_STRING(disasm_params.member_disasm_string)
                );
            }

            return true;
        }

        // print of disasm_string.
        if (write_file){

            if ( ! print_disasm_lines(
                symbol_table, write_file,
                x64_code_buffer, x64_code_buffer_size,
                x64_code_pos_begin, x64_code_body_pos,
                &disasm_params)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
        }else{

            TP_PUT_LOG_MSG_IRE(symbol_table);

            goto fail;
        }
    }

    status = true;

fail:
    if (path){

        if ( ! tp_close_file(symbol_table, &write_file)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

//          return false;
        }
    }

    return status;
}

static bool print_disasm_lines(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint32_t x64_code_pos_begin, uint32_t x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    if (NULL == write_file){

        return true;
    }

    if (x64_code_body_pos < x64_code_pos_begin){

        TP_PUT_LOG_MSG_IRE(symbol_table);

        return false;
    }

    if (disasm_params->member_is_unknown){

        sprintf_s(
            disasm_params->member_disasm_string,
            sizeof(disasm_params->member_disasm_string), "(unknown)"
        );
        disasm_params->member_is_unknown = false;

        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("=== x64 disasm is (unknown). ===")
        );
    }

    if (disasm_params->member_is_bad){

        sprintf_s(
            disasm_params->member_disasm_string,
            sizeof(disasm_params->member_disasm_string), "(bad)"
        );
        disasm_params->member_is_bad = false;
        
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("=== x64 disasm is (bad). ===")
        );
    }

    uint32_t x64_code_pos_begin_line = x64_code_pos_begin;
    uint32_t x64_code_pos_cur = x64_code_pos_begin;

    uint32_t num = x64_code_body_pos - x64_code_pos_begin + 1;
    uint32_t line_num = num / TP_DISASM_COLUMS;
    uint32_t line_mod = num % TP_DISASM_COLUMS;

    if ((0 == line_num) && (0 == line_mod)){

        TP_PUT_LOG_MSG_IRE(symbol_table);

        return false;
    }

    if (0 == num){

        fprintf(
            write_file, "%s\n",
            disasm_params->member_disasm_string
        );

        return true;
    }

    // 0x00000092:  48 bf 10 00 00 00 00  mov rdi,0x10
    for (uint32_t i = 0; line_num > i; ++i){

        fprintf(write_file, "0x%08x:", x64_code_pos_begin_line);

        x64_code_pos_begin_line += TP_DISASM_COLUMS;

        for (uint32_t j = 0; TP_DISASM_COLUMS > j; ++j){

            fprintf(write_file, "  %02x", x64_code_buffer[x64_code_pos_cur]);

            ++x64_code_pos_cur;
        }

        if (0 == i){

            if (*(disasm_params->member_additional_string)){

                fprintf(
                    write_file, "  %s  %s\n",
                    disasm_params->member_disasm_string,
                    disasm_params->member_additional_string
                );
            }else{

                fprintf(write_file, "  %s\n", disasm_params->member_disasm_string);
            }
        }else{

            fprintf(write_file, "\n");
        }
    }

    // 0x00000099:  00 00 00 
    if (line_mod){

        fprintf(write_file, "0x%08x:", x64_code_pos_begin_line);

        for (uint32_t i = 0; line_mod > i; ++i){

            fprintf(write_file, "  %02x", x64_code_buffer[x64_code_pos_cur]);

            ++x64_code_pos_cur;
        }

        if (line_num){

            fprintf(write_file, "\n");
        }else{

            for (uint32_t i = line_mod; TP_DISASM_COLUMS > i; ++i){

                fprintf(write_file, "    ");
            }

            if (*(disasm_params->member_additional_string)){

                fprintf(
                    write_file, "  %s  %s\n",
                    disasm_params->member_disasm_string,
                    disasm_params->member_additional_string
                );
            }else{

                fprintf(write_file, "  %s\n", disasm_params->member_disasm_string);
            }
        }
    }

    return true;
}

static bool set_opcode_string(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    // opcode
    disasm_params->member_is_need_mod_rm = true;
    disasm_params->member_is_need_imm = TP_IS_NOT_NEED_IMM;
    disasm_params->member_is_need_op_reg = TP_IS_NOT_NEED_OP_REG;
    disasm_params->member_is_need_rel = TP_IS_NOT_NEED_REL;

    if (disasm_params->member_is_rex){

        ++(*x64_code_body_pos);

        if (x64_code_buffer_size <= *x64_code_body_pos){

            disasm_params->member_is_bad = true;
            return true;
        }
    }

    disasm_params->member_x64_opcode_1 = x64_code_buffer[*x64_code_body_pos];

    if (x64_code_buffer_size > ((*x64_code_body_pos) + 1)){

        disasm_params->member_x64_opcode_2 =
            &(x64_code_buffer[((*x64_code_body_pos) + 1)]);
    }

    uint8_t* opcode = get_opcode_string(disasm_params);

    if (2 < disasm_params->member_match_bytes){

        TP_PUT_LOG_MSG_IRE(symbol_table);

        return false;
    }

    if (NULL == opcode){

        (*disasm_params).member_is_unknown = true;

        return true;
    }

    disasm_params->member_opcode = opcode;

    if (disasm_params->member_is_need_mod_rm ||
        (TP_IS_NOT_NEED_IMM != disasm_params->member_is_need_imm)){

        if (x64_code_buffer_size <
            (*x64_code_body_pos) + disasm_params->member_match_bytes){

            disasm_params->member_is_bad = true;
            return true;
        }
    }

    if (0 != strcmp(TP_DISASM_STRING_CALL, disasm_params->member_opcode)){

        if (disasm_params->member_is_need_mod_rm ||
            (TP_IS_NOT_NEED_IMM != disasm_params->member_is_need_imm) ||
            (TP_IS_NOT_NEED_REL != disasm_params->member_is_need_rel)){

            *x64_code_body_pos += disasm_params->member_match_bytes;
        }
    }else{

        if (disasm_params->member_is_need_mod_rm){

            *x64_code_body_pos += disasm_params->member_match_bytes;
        }
    }

    (*disasm_params).member_is_unknown = false;

    return true;
}

static uint8_t* get_opcode_string(TP_DISASM_PARAMS* disasm_params)
{
    disasm_params->member_match_bytes = 1;
    disasm_params->member_is_need_mod_rm = true;
    disasm_params->member_is_need_imm = TP_IS_NOT_NEED_IMM;
    disasm_params->member_is_need_op_reg = TP_IS_NOT_NEED_OP_REG;
    disasm_params->member_is_need_rel = TP_IS_NOT_NEED_REL;

    switch (disasm_params->member_x64_opcode_1 & 0xf8){
    case TP_X64_OPCODE_MOV_IMM:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_is_need_imm = TP_IS_NEED_IMM32;
        disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
        return "mov";
    case TP_X64_OPCODE_MOV_IMM_32_64:
        disasm_params->member_is_need_mod_rm = false;
        if (disasm_params->member_is_rex_W){
            disasm_params->member_is_need_imm = TP_IS_NEED_IMM64;
            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG64;
        }else{
            disasm_params->member_is_need_imm = TP_IS_NEED_IMM32;
            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
        }
        return "mov";
    case TP_X64_OPCODE_PUSH:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG64;
        disasm_params->member_is_rex_W = true;
        disasm_params->member_operand_num = TP_OPERAND_NUM_1;
        return "push";
    case TP_X64_OPCODE_POP:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG64;
        disasm_params->member_is_rex_W = true;
        disasm_params->member_operand_num = TP_OPERAND_NUM_1;
        return "pop";
    default:
        break;
    }

    if (disasm_params->member_x64_opcode_1 & 0x02){

        disasm_params->member_is_reg_dst = true;
    }

    switch (disasm_params->member_x64_opcode_1){
    case TP_X64_OPCODE_NOP:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_operand_num = TP_OPERAND_NUM_0;
        return "nop";
    case TP_X64_OPCODE_INT_3:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_operand_num = TP_OPERAND_NUM_0;
        return "int 3";
    case TP_X64_OPCODE_MOV_REG: return "mov";
    case TP_X64_OPCODE_MOV_DST_MEM: return "mov";
//  case TP_X64_OPCODE_MOV_SRC_MEM_REG: return "mov";
    case TP_X64_OPCODE_MOV_IMM_MEM:
        disasm_params->member_is_need_imm = TP_IS_NEED_IMM32;
        return "mov";

    case TP_X64_OPCODE_ADD_REG: return "add";
    case TP_X64_OPCODE_ADD_DST_MEM: return "add";
//  case TP_X64_OPCODE_ADD_SRC_MEM_REG: return "add";

    case TP_X64_OPCODE_SUB: return "sub";
    case TP_X64_OPCODE_SUB_DST_MEM: return "sub";
//  case TP_X64_OPCODE_SUB_SRC_MEM_REG: return "sub";

    case TP_X64_OPCODE_XOR: return "xor";
    case TP_X64_OPCODE_XOR_DST_MEM: return "xor";
//  case TP_X64_OPCODE_XOR_SRC_MEM_REG: return "xor";

    case TP_X64_OPCODE_LEA: return "lea";
    case TP_X64_OPCODE_JNE_REL8:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_is_need_rel = TP_IS_NEED_REL8;
        disasm_params->member_operand_num = TP_OPERAND_NUM_1;
        return "jne";
    case TP_X64_OPCODE_CALL_INDIRECT:
        disasm_params->member_is_rex_W = true;
        disasm_params->member_operand_num = TP_OPERAND_NUM_1;
        return "call";
    case TP_X64_OPCODE_CALL_RIP:
        disasm_params->member_is_RIP = true;
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_is_need_rel = TP_IS_NEED_REL32;
        disasm_params->member_operand_num = TP_OPERAND_NUM_1;
        return "call";
    case TP_X64_OPCODE_RET:
        disasm_params->member_is_need_mod_rm = false;
        disasm_params->member_operand_num = TP_OPERAND_NUM_0;
        return "ret";

    default:
        break;
    }

    if (NULL == disasm_params->member_x64_opcode_2){

        disasm_params->member_match_bytes = 0;

        return NULL;
    }

    if (TP_X64_OPCODE_IDIV_IMUL_EAX_1 ==
        disasm_params->member_x64_opcode_1){

        disasm_params->member_operand_num = TP_OPERAND_NUM_1;

        if (disasm_params->member_is_rex_W){

            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG64;
        }else{

            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
        }

        uint8_t reg_opcode = (((*(disasm_params->member_x64_opcode_2)) >> 3) & 0x07);

        switch (reg_opcode){
        case TP_X64_OPCODE_IMUL_EAX_2: return "imul";
        case TP_X64_OPCODE_IDIV_2: return "idiv";
        default:
            break;
        }

        return NULL;
    }

    disasm_params->member_match_bytes = 1;

    if ((*(disasm_params->member_x64_opcode_2)) & 0x01){

        disasm_params->member_is_reg_dst = true;
    }

    switch (disasm_params->member_x64_opcode_1){
    case TP_X64_OPCODE_IMUL_1:
//  case TP_X64_OPCODE_JNE_REL32_1:
        switch (*(disasm_params->member_x64_opcode_2)){
        case TP_X64_OPCODE_IMUL_2:
            disasm_params->member_match_bytes = 2;
            return "imul";
        case TP_X64_OPCODE_JNE_REL32_2:
            disasm_params->member_is_need_mod_rm = false;
            disasm_params->member_is_need_rel = TP_IS_NEED_REL32;
            disasm_params->member_match_bytes = 2;
            disasm_params->member_operand_num = TP_OPERAND_NUM_1;
            return "jne";
        default:
            break;
        }
        return NULL;
    case TP_X64_OPCODE_ADD_SUB_IMM8_1:
        if (TP_X64_OPCODE_ADD_IMM8_IMM32_2 ==
            ((*(disasm_params->member_x64_opcode_2)) & 0xf8)){
            disasm_params->member_is_need_mod_rm = false;
            disasm_params->member_match_bytes = 2;
            disasm_params->member_is_need_imm = TP_IS_NEED_IMM8;
            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
            return "add";
        }
        if (TP_X64_OPCODE_SUB_IMM8_IMM32_2 ==
            ((*(disasm_params->member_x64_opcode_2)) & 0xf8)){
            disasm_params->member_is_need_mod_rm = false;
            disasm_params->member_match_bytes = 2;
            disasm_params->member_is_need_imm = TP_IS_NEED_IMM8;
            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
            return "sub";
        }
        return NULL;
    case TP_X64_OPCODE_ADD_SUB_IMM32_1:
        if (TP_X64_OPCODE_ADD_IMM8_IMM32_2 ==
            ((*(disasm_params->member_x64_opcode_2)) & 0xf8)){
            disasm_params->member_is_need_mod_rm = false;
            disasm_params->member_match_bytes = 2;
            disasm_params->member_is_need_imm = TP_IS_NEED_IMM32;
            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
            return "add";
        }
        if (TP_X64_OPCODE_SUB_IMM8_IMM32_2 ==
            ((*(disasm_params->member_x64_opcode_2)) & 0xf8)){
            disasm_params->member_is_need_mod_rm = false;
            disasm_params->member_match_bytes = 2;
            disasm_params->member_is_need_imm = TP_IS_NEED_IMM32;
            disasm_params->member_is_need_op_reg = TP_IS_NEED_OP_REG32;
            return "sub";
        }
        return NULL;
    default:
        break;
    }

    disasm_params->member_match_bytes = 0;

    return NULL;
}

static bool set_operand_string(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    if (false == disasm_params->member_is_need_mod_rm){

        if (x64_code_buffer_size < *x64_code_body_pos){

            disasm_params->member_is_bad = true;
            return true;
        }

        if (0 == strcmp(TP_DISASM_STRING_CALL, disasm_params->member_opcode)){

            // Address displacement & Immediate data
            if ( ! set_operand_string_DISP_IMM(
                symbol_table,
                x64_code_buffer, x64_code_buffer_size, x64_code_body_pos,
                disasm_params)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
            return true;
        }

        switch (disasm_params->member_is_need_op_reg){
        case TP_IS_NEED_OP_REG32:
//          break;
        case TP_IS_NEED_OP_REG64:
            if (TP_IS_NOT_NEED_REL != disasm_params->member_is_need_rel){

                disasm_params->member_is_bad = true;
                return true;
            }

            uint8_t op_reg = ((2 == disasm_params->member_match_bytes) ?
                ((*(disasm_params->member_x64_opcode_2)) & 0x07) :
                (disasm_params->member_x64_opcode_1 & 0x07)
            );
            uint8_t* register_name = get_register_name(
                op_reg,
                disasm_params->member_is_rex_W,
                disasm_params->member_is_rex_B
            );

            if (NULL == register_name){

                TP_PUT_LOG_MSG_IRE(symbol_table);
                return false;
            }

            int32_t disp32 = 0;
            int64_t disp64 = 0;

            switch (disasm_params->member_is_need_imm){
            case TP_IS_NOT_NEED_IMM:
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s", disasm_params->member_opcode, register_name
                );
                break;
            case TP_IS_NEED_IMM8:
                if (x64_code_buffer_size <
                    (*x64_code_body_pos + (uint8_t)sizeof(int8_t))){

                    disasm_params->member_is_bad = true;
                    return true;
                }
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,0x%x", disasm_params->member_opcode, register_name,
                    x64_code_buffer[*x64_code_body_pos]
                );
                break;
            case TP_IS_NEED_IMM32:
                if (x64_code_buffer_size <
                    (*x64_code_body_pos + (uint32_t)sizeof(int32_t))){

                    disasm_params->member_is_bad = true;
                    return true;
                }
                memcpy(&disp32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int32_t));
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,0x%x", disasm_params->member_opcode, register_name, disp32
                );
                (*x64_code_body_pos) += (uint32_t)(sizeof(int32_t) - 1);
                break;
            case TP_IS_NEED_IMM64:
                if (x64_code_buffer_size <
                    (*x64_code_body_pos + (uint64_t)sizeof(int64_t))){

                    disasm_params->member_is_bad = true;
                    return true;
                }
                memcpy(&disp64, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int64_t));
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,0x%zx", disasm_params->member_opcode, register_name, disp64
                );
                (*x64_code_body_pos) += (uint32_t)(sizeof(int64_t) - 1);
                break;
            default:
                TP_PUT_LOG_MSG_IRE(symbol_table);
                return false;
            }
            break;
        case TP_IS_NOT_NEED_OP_REG:
            switch (disasm_params->member_is_need_rel){
            case TP_IS_NEED_REL8:{
                uint8_t rel8 = x64_code_buffer[*x64_code_body_pos] + *x64_code_body_pos + 1;
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s 0x%x", disasm_params->member_opcode, rel8
                );
                break;
            }
            case TP_IS_NEED_REL32:{
                uint32_t rel32 = 0;
                if (x64_code_buffer_size < (*x64_code_body_pos + (uint32_t)sizeof(rel32))){

                    disasm_params->member_is_bad = true;
                    return true;
                }
                memcpy(&rel32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(uint32_t));
                rel32 += (*x64_code_body_pos + 4);
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s 0x%x", disasm_params->member_opcode, rel32
                );
                (*x64_code_body_pos) += (uint32_t)(sizeof(rel32) - 1);
                break;
            }
            case TP_IS_NOT_NEED_REL:
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s", disasm_params->member_opcode
                );
                break;
            default:
                TP_PUT_LOG_MSG_IRE(symbol_table);
                return false;
            }
            break;
        default:
            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }

        return true;
    }else{

        // ModR/M
        if ( ! set_operand_string_ModRM(
            symbol_table,
            x64_code_buffer, x64_code_buffer_size, x64_code_body_pos,
            disasm_params)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (disasm_params->member_is_bad){ return true; }

        if (TP_OPERAND_NUM_2 != disasm_params->member_operand_num){

            return true;
        }

        // SIB
        if ( ! set_operand_string_SIB(
            symbol_table,
            x64_code_buffer, x64_code_buffer_size, x64_code_body_pos,
            disasm_params)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (disasm_params->member_is_bad){ return true; }

        // Exceptional addressing mode
        if ( ! set_operand_string_EXCEPTIONAL_ADDRESSING_MODE(
            symbol_table,
            x64_code_buffer, x64_code_buffer_size, x64_code_body_pos,
            disasm_params)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (disasm_params->member_is_bad){ return true; }

        if (TP_EXCEPTIONAL_ADDRESSING_MODE_NONE !=
            disasm_params->member_exceptional_addressing_mode){

            return true;
        }

        // Address displacement & Immediate data
        if ( ! set_operand_string_DISP_IMM(
            symbol_table,
            x64_code_buffer, x64_code_buffer_size, x64_code_body_pos,
            disasm_params)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool set_operand_string_ModRM(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    // ModR/M
    if (x64_code_buffer_size < *x64_code_body_pos){

        disasm_params->member_is_bad = true;
        return true;
    }

    disasm_params->member_mod = 0;
    disasm_params->member_is_mod11 = false;
    disasm_params->member_rm = 0;
    disasm_params->member_is_RIP = false;
    disasm_params->member_is_need_sib = false;
    disasm_params->member_is_need_disp = TP_IS_NOT_NEED_DISP;
    disasm_params->member_register_name_reg = NULL;
    disasm_params->member_register_name_rm = NULL;

    uint8_t mod_rm_byte = x64_code_buffer[*x64_code_body_pos];
    uint8_t reg_opcode = ((mod_rm_byte >> 3) & 0x07);
    disasm_params->member_mod = ((mod_rm_byte >> 6) & 0x03);
    disasm_params->member_is_mod11 = (0x03 == disasm_params->member_mod);
    disasm_params->member_rm = (mod_rm_byte & 0x07);

    disasm_params->member_register_name_reg = get_register_name(
        reg_opcode,
        disasm_params->member_is_rex_W,
        disasm_params->member_is_rex_R
    );

    switch (disasm_params->member_mod){
    case 0x00:
        switch (disasm_params->member_rm){
        case 0x04:
            disasm_params->member_is_need_sib = true;
            break;
        case 0x05:
            disasm_params->member_is_RIP = true;
//          disasm_params->member_is_need_disp = TP_IS_NEED_DISP32;
            break;
        default:
            disasm_params->member_register_name_rm =
                get_register_name(
                    disasm_params->member_rm,
                    disasm_params->member_is_rex_W,
                    disasm_params->member_is_rex_B
                );
            break;
        }
        break;
    case 0x01:
        disasm_params->member_is_need_disp = TP_IS_NEED_DISP8;

        if (0x04 == disasm_params->member_rm){

            disasm_params->member_is_need_sib = true;
        }else{

            disasm_params->member_register_name_rm = get_register_name(
                disasm_params->member_rm,
                disasm_params->member_is_rex_W,
                disasm_params->member_is_rex_B
            );
        }
        break;
    case 0x02:
        disasm_params->member_is_need_disp = TP_IS_NEED_DISP32;

        if (0x04 == disasm_params->member_rm){

            disasm_params->member_is_need_sib = true;
        }else{

            disasm_params->member_register_name_rm = get_register_name(
                disasm_params->member_rm,
                disasm_params->member_is_rex_W,
                disasm_params->member_is_rex_B
            );
        }
        break;
    case 0x03:
        disasm_params->member_register_name_rm = get_register_name(
            disasm_params->member_rm,
            disasm_params->member_is_rex_W,
            disasm_params->member_is_rex_B
        );
        break;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    if ((false == disasm_params->member_is_need_sib) &&
        (false == disasm_params->member_is_RIP) &&
        (TP_IS_NOT_NEED_DISP == disasm_params->member_is_need_disp) &&
        (TP_IS_NOT_NEED_IMM == disasm_params->member_is_need_imm)){

        if (0 == strcmp(TP_DISASM_STRING_CALL, disasm_params->member_opcode)){

            sprintf_s(
                disasm_params->member_disasm_string,
                sizeof(disasm_params->member_disasm_string),
                "%s %s",
                disasm_params->member_opcode,
                disasm_params->member_register_name_rm
            );
        }else if (TP_X64_OPCODE_IDIV_IMUL_EAX_1 ==
            disasm_params->member_x64_opcode_1){

            switch (reg_opcode){
            case TP_X64_OPCODE_IMUL_EAX_2: break;
            case TP_X64_OPCODE_IDIV_2: break;
            default:
                disasm_params->member_is_bad = true;
                return true;
            }
            sprintf_s(
                disasm_params->member_disasm_string,
                sizeof(disasm_params->member_disasm_string),
                "%s %s",
                disasm_params->member_opcode,
                disasm_params->member_register_name_rm
            );
        }else{

            // d  | src         | dst
            // -------------------------------
            // 0  | reg field   | r/m field
            // -------------------------------
            // 1  | r/m field   | reg field
            sprintf_s(
                disasm_params->member_disasm_string,
                sizeof(disasm_params->member_disasm_string),
                "%s %s,%s",
                disasm_params->member_opcode,
                (disasm_params->member_is_reg_dst ?
                    disasm_params->member_register_name_reg :
                    disasm_params->member_register_name_rm),
                (disasm_params->member_is_reg_dst ?
                    disasm_params->member_register_name_rm :
                    disasm_params->member_register_name_reg)
            );
        }
    }

    return true;
}

static bool set_operand_string_SIB(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    // SIB
    disasm_params->member_sib_index = 0;
    disasm_params->member_register_name_index = NULL;
    disasm_params->member_sib_scale = 0;
    disasm_params->member_sib_base = 0;
    disasm_params->member_register_name_base = NULL;
    disasm_params->member_exceptional_addressing_mode =
        TP_EXCEPTIONAL_ADDRESSING_MODE_NONE;

    if (false == disasm_params->member_is_need_sib){

        return true;
    }

    ++(*x64_code_body_pos);

    if (x64_code_buffer_size < *x64_code_body_pos){

        disasm_params->member_is_bad = true;
        return true;
    }

    uint8_t sib_byte = x64_code_buffer[*x64_code_body_pos];

    uint8_t sib_scale_byte = ((sib_byte >> 6) & 0x03);
    disasm_params->member_sib_index = ((sib_byte >> 3) & 0x07);
    disasm_params->member_sib_base = (sib_byte & 0x07);

    switch (sib_scale_byte){
    case 0x00: disasm_params->member_sib_scale = 1; break;
    case 0x01: disasm_params->member_sib_scale = 2; break;
    case 0x02: disasm_params->member_sib_scale = 4; break;
    case 0x03: disasm_params->member_sib_scale = 8; break;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    if (0x04 == disasm_params->member_sib_index){

        disasm_params->member_register_name_index = TP_DISASM_STRING_NONE;

        if ((0x05 == disasm_params->member_sib_base) &&
            (0x00 == disasm_params->member_mod)){

            disasm_params->member_exceptional_addressing_mode =
                TP_EXCEPTIONAL_ADDRESSING_MODE_2;
            return true;
        }
    }else{

        disasm_params->member_register_name_index =
            get_register_name(
                disasm_params->member_sib_index,
                disasm_params->member_is_rex_W,
                disasm_params->member_is_rex_X
            );

        if ((0x05 == disasm_params->member_sib_base) &&
            (0x11 != disasm_params->member_mod)){

            disasm_params->member_exceptional_addressing_mode =
                TP_EXCEPTIONAL_ADDRESSING_MODE_1;
            return true;
        }
    }

    disasm_params->member_register_name_base = get_register_name(
        disasm_params->member_sib_base,
        disasm_params->member_is_rex_W,
        disasm_params->member_is_rex_B
    );

    switch (disasm_params->member_mod){
    case 0x00:
        if ((TP_IS_NOT_NEED_DISP != disasm_params->member_is_need_disp)){

            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        if (TP_IS_NOT_NEED_IMM == disasm_params->member_is_need_imm){

            // d  | src       | dst
            // ---------------------------
            // 0  | reg field | SIB byte
            // ---------------------------
            // 1  | SIB byte  | reg field
            if (disasm_params->member_is_reg_dst){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,[%s+%s*%d]",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_reg,
                    disasm_params->member_register_name_base,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale
                );
            }else{

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s [%s+%s*%d],%s",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_base,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    disasm_params->member_register_name_reg
                );
            }
            return true;
        }
        break;
    case 0x01:
        if ((TP_IS_NEED_DISP8 != disasm_params->member_is_need_disp)){

            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        break;
    case 0x02:
        if ((TP_IS_NEED_DISP32 != disasm_params->member_is_need_disp)){

            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        break;
    case 0x03:
//      break;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    return true;
}

static bool set_operand_string_EXCEPTIONAL_ADDRESSING_MODE(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    // Exceptional addressing mode
    int32_t disp32 = 0;

    switch (disasm_params->member_exceptional_addressing_mode){
    case TP_EXCEPTIONAL_ADDRESSING_MODE_NONE:
        break;
    case TP_EXCEPTIONAL_ADDRESSING_MODE_1:
        if (disasm_params->member_is_mod11 ||
            (TP_IS_NOT_NEED_IMM != disasm_params->member_is_need_imm)){

            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        switch (disasm_params->member_mod){
        case 0x00:
            if (x64_code_buffer_size <
                (*x64_code_body_pos + (uint32_t)sizeof(int32_t))){

                disasm_params->member_is_bad = true;
                return true;
            }
            ++(*x64_code_body_pos);
            memcpy(&disp32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int32_t));

            // (SIB.Index * scale) + disp32
            // ------------------------------------
            // d  | src       | dst
            // ------------------------------------
            // 0  | reg field | SIB byte
            // ------------------------------------
            // 1  | SIB byte  | reg field
            if (disasm_params->member_is_reg_dst){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,[%s*%d+0x%x]",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_reg,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    disp32
                );
            }else{

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s [%s*%d+0x%x],%s",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    disp32,
                    disasm_params->member_register_name_reg
                );
            }
            (*x64_code_body_pos) += ((uint32_t)sizeof(int32_t) - 1);
            break;
        case 0x01:
            if (x64_code_buffer_size <
                (*x64_code_body_pos + (uint32_t)sizeof(uint8_t))){

                disasm_params->member_is_bad = true;
                return true;
            }
            ++(*x64_code_body_pos);
            // (SIB.Index * scale) + disp8 + [RBP]
            // ------------------------------------
            // d  | src       | dst
            // ------------------------------------
            // 0  | reg field | SIB byte
            // ------------------------------------
            // 1  | SIB byte  | reg field
            if (disasm_params->member_is_reg_dst){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,[%s*%d+0x%x+rbp]",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_reg,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    x64_code_buffer[*x64_code_body_pos]
                );
            }else{

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s [%s*%d+0x%x+rbp],%s",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    x64_code_buffer[*x64_code_body_pos],
                    disasm_params->member_register_name_reg
                );
            }
            break;
        case 0x02:
            if (x64_code_buffer_size <
                (*x64_code_body_pos + (uint32_t)sizeof(int32_t))){

                disasm_params->member_is_bad = true;
                return true;
            }
            ++(*x64_code_body_pos);
            memcpy(&disp32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int32_t));
            // (SIB.Index * scale) + disp32 + [RBP]
            // ------------------------------------
            // d  | src       | dst
            // ------------------------------------
            // 0  | reg field | SIB byte
            // ------------------------------------
            // 1  | SIB byte  | reg field
            if (disasm_params->member_is_reg_dst){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,[%s*%d+0x%x+rbp]",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_reg,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale, disp32
                );
            }else{

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s [%s*%d+0x%x+rbp],%s",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale, disp32,
                    disasm_params->member_register_name_reg
                );
            }
            (*x64_code_body_pos) += ((uint32_t)sizeof(int32_t) - 1);
            break;
        case 0x03:
//          break;
        default:
            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        return true;
    case TP_EXCEPTIONAL_ADDRESSING_MODE_2:
        if ((0x00 != disasm_params->member_mod) ||
            (0x05 != disasm_params->member_sib_base) ||
            (0x04 != disasm_params->member_sib_index)){

            TP_PUT_LOG_MSG_IRE(symbol_table);
             return false;
        }
        if (x64_code_buffer_size <
                (*x64_code_body_pos + (uint32_t)sizeof(int32_t))){

            disasm_params->member_is_bad = true;
            return true;
        }
        ++(*x64_code_body_pos);
        memcpy(&disp32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int32_t));
        // ------------------------------------
        // d  | src       | dst
        // ------------------------------------
        // 0  | reg field | disp32
        // ------------------------------------
        // 1  | disp32    | reg field
        if (disasm_params->member_is_reg_dst){

            sprintf_s(
                disasm_params->member_disasm_string,
                sizeof(disasm_params->member_disasm_string),
                "%s %s,0x%x",
                disasm_params->member_opcode,
                disasm_params->member_register_name_reg,
                disp32
            );
        }else{

            sprintf_s(
                disasm_params->member_disasm_string,
                sizeof(disasm_params->member_disasm_string),
                "%s 0x%x,%s",
                disasm_params->member_opcode,
                disp32,
                disasm_params->member_register_name_reg
            );
        }
        (*x64_code_body_pos) += ((uint32_t)sizeof(int32_t) - 1);
        return true;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    return true;
}

static bool set_operand_string_DISP_IMM(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer,
    uint32_t x64_code_buffer_size, uint32_t* x64_code_body_pos,
    TP_DISASM_PARAMS* disasm_params)
{
    // Address displacement & Immediate data
    int32_t disp32 = 0;
    int64_t disp64 = 0;
    int64_t imm64 = 0;

    // RIP
    if (disasm_params->member_is_RIP){

        goto disp32_proc;
    }

    if (0 == strcmp(TP_DISASM_STRING_CALL, disasm_params->member_opcode)){

        disasm_params->member_is_bad = true;

        return true;
    }

    disasm_params->member_register_name_base = get_register_name(
        disasm_params->member_sib_base,
        true, // 64 bits
        disasm_params->member_is_rex_B
    );

    // Address displacement
    switch (disasm_params->member_is_need_disp){
    case TP_IS_NOT_NEED_DISP:
        break;
    case TP_IS_NEED_DISP8:
        if (x64_code_buffer_size <
            (*x64_code_body_pos + (uint32_t)sizeof(uint8_t))){

            disasm_params->member_is_bad = true;
            return true;
        }
        ++(*x64_code_body_pos);
        disp32 = x64_code_buffer[*x64_code_body_pos];
        break;
    case TP_IS_NEED_DISP32:
disp32_proc:
        if (x64_code_buffer_size <
            (*x64_code_body_pos + (uint32_t)sizeof(int32_t))){

            disasm_params->member_is_bad = true;
            return true;
        }
        ++(*x64_code_body_pos);
        memcpy(&disp32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int32_t));
        disp64 = disp32;

        // RIP
        if (disasm_params->member_is_RIP){

            int64_t rel64 = disp64 + *x64_code_body_pos + 4;

            if (0 == strcmp(disasm_params->member_opcode, TP_DISASM_STRING_LEA)){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,[rip+0x%016zx]",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_reg,
                    disp64
                );
            }else if (0 == strcmp(disasm_params->member_opcode, TP_DISASM_STRING_CALL)){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s QWORD PTR [rip+0x%016zx]",
                    disasm_params->member_opcode,
                    disp64
                );
            }else{
                // d  | src       | dst
                // ---------------------------
                // 0  | reg field | disp32
                // ---------------------------
                // 1  | disp32    | reg field
                if (disasm_params->member_is_reg_dst){

                    sprintf_s(
                        disasm_params->member_disasm_string,
                        sizeof(disasm_params->member_disasm_string),
                        disasm_params->member_is_rex_W ?
                            "%s %s,QWORD PTR [rip+0x%016zx]" :
                            "%s %s,DWORD PTR [rip+0x%016zx]",
                        disasm_params->member_opcode,
                        disasm_params->member_register_name_reg,
                        disp64
                    );
                }else{

                    sprintf_s(
                        disasm_params->member_disasm_string,
                        sizeof(disasm_params->member_disasm_string),
                        disasm_params->member_is_rex_W ?
                            "%s QWORD PTR [rip+0x%016zx],%s" :
                            "%s DWORD PTR [rip+0x%016zx],%s",
                        disasm_params->member_opcode,
                        disp64,
                        disasm_params->member_register_name_reg
                    );
                }
            }

            sprintf_s(
                disasm_params->member_additional_string,
                sizeof(disasm_params->member_additional_string),
                "# 0x%016zx", rel64
            );

            (*x64_code_body_pos) += ((uint32_t)sizeof(int32_t) - 1);

            return true;
        }else{

            (*x64_code_body_pos) += ((uint32_t)sizeof(int32_t) - 1);
        }
        break;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    // Immediate data
    switch (disasm_params->member_is_need_imm){
    case TP_IS_NOT_NEED_IMM:
        break;
    case TP_IS_NEED_IMM8:
        if (x64_code_buffer_size <
            (*x64_code_body_pos + (uint32_t)sizeof(uint8_t))){

            disasm_params->member_is_bad = true;
            return true;
        }
        ++(*x64_code_body_pos);
        imm64 = x64_code_buffer[*x64_code_body_pos];
        break;
    case TP_IS_NEED_IMM32:{
        int32_t disp32 = 0;
        if (x64_code_buffer_size <
            (*x64_code_body_pos + (uint32_t)sizeof(int32_t))){

            disasm_params->member_is_bad = true;
            return true;
        }
        ++(*x64_code_body_pos);
        memcpy(&disp32, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int32_t));
        imm64 = disp32;
        (*x64_code_body_pos) += ((uint32_t)sizeof(int32_t) - 1);
        break;
    }
    case TP_IS_NEED_IMM64:
        if (x64_code_buffer_size <
            (*x64_code_body_pos + (uint32_t)sizeof(int64_t))){

            disasm_params->member_is_bad = true;
            return true;
        }
        ++(*x64_code_body_pos);
        memcpy(&imm64, &(x64_code_buffer[*x64_code_body_pos]), sizeof(int64_t));
        (*x64_code_body_pos) += ((uint32_t)sizeof(int64_t) - 1);
        break;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    // Address displacement & Immediate data
    switch (disasm_params->member_is_need_imm){
    case TP_IS_NOT_NEED_IMM:
        switch (disasm_params->member_is_need_disp){
        case TP_IS_NOT_NEED_DISP:
            if (false == disasm_params->member_is_need_sib){
                // d  | src       | dst
                // ---------------------------
                // 0  | reg field | r/m field
                // ---------------------------
                // 1  | r/m field | reg field
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,%s",
                    disasm_params->member_opcode,
                    (disasm_params->member_is_reg_dst ?
                        disasm_params->member_register_name_reg :
                        disasm_params->member_register_name_rm),
                    (disasm_params->member_is_reg_dst ?
                        disasm_params->member_register_name_rm :
                        disasm_params->member_register_name_reg)
                );
        }else{
                if (0 == strcmp(TP_DISASM_STRING_LEA, disasm_params->member_opcode)){

                    sprintf_s(
                        disasm_params->member_disasm_string,
                        sizeof(disasm_params->member_disasm_string),
                        "%s %s,[%s+%s*%d]",
                        disasm_params->member_opcode,
                        disasm_params->member_register_name_reg,
                        disasm_params->member_register_name_base,
                        disasm_params->member_register_name_index,
                        disasm_params->member_sib_scale
                    );
                }else{
                    // d  | src       | dst
                    // ---------------------------
                    // 0  | reg field | SIB byte
                    // ---------------------------
                    // 1  | SIB byte  | reg field
                    if (disasm_params->member_is_reg_dst){

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            (disasm_params->member_is_rex_W ?
                                "%s %s,QWORD PTR [%s+%s*%d]" :
                                "%s %s,DWORD PTR [%s+%s*%d]"),
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_reg,
                            disasm_params->member_register_name_base,
                            disasm_params->member_register_name_index,
                            disasm_params->member_sib_scale
                        );
                    }else{

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            (disasm_params->member_is_rex_W ?
                                "%s QWORD PTR [%s+%s*%d],%s" :
                                "%s DWORD PTR [%s+%s*%d],%s"),
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_base,
                            disasm_params->member_register_name_index,
                            disasm_params->member_sib_scale,
                            disasm_params->member_register_name_reg
                        );
                    }
                }
            }
            break;
        case TP_IS_NEED_DISP8:
//          break;
        case TP_IS_NEED_DISP32:
            if (false == disasm_params->member_is_need_sib){

                if (0 == strcmp(TP_DISASM_STRING_LEA, disasm_params->member_opcode)){

                    sprintf_s(
                        disasm_params->member_disasm_string,
                        sizeof(disasm_params->member_disasm_string),
                        "%s %s,[%s+0x%x]",
                        disasm_params->member_opcode,
                        disasm_params->member_register_name_reg,
                        disasm_params->member_register_name_rm,
                        disp32
                    );
                }else{
                    // d  | src       | dst
                    // ---------------------------
                    // 0  | reg field | r/m field
                    // ---------------------------
                    // 1  | r/m field | reg field
                    if (disasm_params->member_is_reg_dst){

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            (disasm_params->member_is_rex_W ?
                                "%s %s,QWORD PTR [%s+0x%x]" :
                                "%s %s,DWORD PTR [%s+0x%x]"),
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_reg,
                            disasm_params->member_register_name_rm,
                            disp32
                        );
                    }else{

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                           (disasm_params->member_is_rex_W ?
                               "%s QWORD PTR [%s+0x%x],%s" :
                               "%s DWORD PTR [%s+0x%x],%s"),
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_rm,
                            disp32,
                            disasm_params->member_register_name_reg
                        );
                    }
                }
            }else{

                if (0 == strcmp(TP_DISASM_STRING_LEA, disasm_params->member_opcode)){

                    if (0 == strcmp(
                        TP_DISASM_STRING_NONE, disasm_params->member_register_name_index)){

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            "%s %s,[%s+0x%x]",
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_reg,
                            disasm_params->member_register_name_base,
                            disp32
                        );
                    }else{

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            "%s %s,[%s+%s*%d+0x%x]",
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_reg,
                            disasm_params->member_register_name_base,
                            disasm_params->member_register_name_index,
                            disasm_params->member_sib_scale,
                            disp32
                        );
                    }
                }else{
                    // d  | src       | dst
                    // ---------------------------
                    // 0  | reg field | SIB byte
                    // ---------------------------
                    // 1  | SIB byte  | reg field
                    if (disasm_params->member_is_reg_dst){

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            (disasm_params->member_is_rex_W ?
                                "%s %s,QWORD PTR [%s+%s*%d+0x%x]" :
                                "%s %s,DWORD PTR [%s+%s*%d+0x%x]"),
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_reg,
                            disasm_params->member_register_name_base,
                            disasm_params->member_register_name_index,
                            disasm_params->member_sib_scale,
                            disp32
                        );
                    }else{

                        sprintf_s(
                            disasm_params->member_disasm_string,
                            sizeof(disasm_params->member_disasm_string),
                            (disasm_params->member_is_rex_W ?
                                "%s QWORD PTR [%s+%s*%d+0x%x],%s" :
                                "%s DWORD PTR [%s+%s*%d+0x%x],%s"),
                            disasm_params->member_opcode,
                            disasm_params->member_register_name_base,
                            disasm_params->member_register_name_index,
                            disasm_params->member_sib_scale,
                            disp32,
                            disasm_params->member_register_name_reg
                        );
                    }
                }
            }
            break;
        default:
            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        break;
    case TP_IS_NEED_IMM8:
//      break;
    case TP_IS_NEED_IMM32:
//      break;
    case TP_IS_NEED_IMM64:
        switch (disasm_params->member_is_need_disp){
        case TP_IS_NOT_NEED_DISP:
            if (false == disasm_params->member_is_need_sib){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    "%s %s,0x%zx",
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_rm,
                    imm64
                );
            }else{
                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    (disasm_params->member_is_rex_W ?
                        "%s QWORD PTR [%s+%s*%d],0x%zx" :
                        "%s DWORD PTR [%s+%s*%d],0x%zx"),
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_base,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    imm64
                );
            }
            break;
        case TP_IS_NEED_DISP8:
//          break;
        case TP_IS_NEED_DISP32:
            if (false == disasm_params->member_is_need_sib){

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    (disasm_params->member_is_rex_W ?
                        "%s QWORD PTR [%s+0x%x],0x%zx" :
                        "%s DWORD PTR [%s+0x%x],0x%zx"),
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_rm,
                    disp32, imm64
                );
            }else{

                sprintf_s(
                    disasm_params->member_disasm_string,
                    sizeof(disasm_params->member_disasm_string),
                    (disasm_params->member_is_rex_W ?
                        "%s QWORD PTR [%s+%s*%d+0x%x],0x%zx" :
                        "%s DWORD PTR [%s+%s*%d+0x%x],0x%zx"),
                    disasm_params->member_opcode,
                    disasm_params->member_register_name_base,
                    disasm_params->member_register_name_index,
                    disasm_params->member_sib_scale,
                    disp32, imm64
                );
            }
            break;
        default:
            TP_PUT_LOG_MSG_IRE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_IRE(symbol_table);
        return false;
    }

    return true;
}

static uint8_t* get_register_name(
    uint8_t byte, bool is_rex_W, bool is_rex_B_or_R_or_X)
{
    if (is_rex_W){

        if (is_rex_B_or_R_or_X){

            switch (byte){
            case 0: return "r8";
            case 1: return "r9";
            case 2: return "r10";
            case 3: return "r11";
            case 4: return "r12";
            case 5: return "r13";
            case 6: return "r14";
            case 7: return "r15";
            default: return NULL;
            }
        }else{

            switch (byte){
            case 0: return "rax";
            case 1: return "rcx";
            case 2: return "rdx";
            case 3: return "rbx";
            case 4: return "rsp";
            case 5: return "rbp";
            case 6: return "rsi";
            case 7: return "rdi";
            default: return NULL;
            }
        }
    }else{

        if (is_rex_B_or_R_or_X){

            switch (byte){
            case 0: return "r8d";
            case 1: return "r9d";
            case 2: return "r10d";
            case 3: return "r11d";
            case 4: return "r12d";
            case 5: return "r13d";
            case 6: return "r14d";
            case 7: return "r15d";
            default: return NULL;
            }
        }else{

            switch (byte){
            case 0: return "eax";
            case 1: return "ecx";
            case 2: return "edx";
            case 3: return "ebx";
            case 4: return "esp";
            case 5: return "ebp";
            case 6: return "esi";
            case 7: return "edi";
            default: return NULL;
            }
        }
    }

    return NULL;
}

