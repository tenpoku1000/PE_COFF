
// Copyright (C) 2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_WASM_UEFI_ENTRY_POINT "efi_main"
#define TP_WASM_UEFI_ENTRY_POINT_LENGTH 8

#define TP_WASM_MODULE_SECTION_CODE_COUNT 1
#define TP_WASM_MODULE_SECTION_CODE_LOCAL_COUNT 1
#define TP_WASM_MODULE_SECTION_CODE_VAR_COUNT 1

#define TP_WASM_CALL_TYPE_0 0
#define TP_WASM_CALL_TYPE_1 1

// Variable constants
static const uint32_t EFI_SUCCESS = 0;
static const uint32_t ST = 1;
static const uint32_t RT = 2;
static const uint32_t IN_ = 3;
static const uint32_t OUT_ = 4;
static const uint32_t LOCAL_STATUS = 0;
static const uint32_t KEY = 1;
static uint32_t STRING_LITERAL = 0;

// Variable offset(WASM)
static uint32_t OFFSET_OF_EFI_SUCCESS = 0;
static uint32_t OFFSET_OF_ST = 0;
static uint32_t OFFSET_OF_RT = 0;
static uint32_t OFFSET_OF_IN = 0;
static uint32_t OFFSET_OF_OUT = 0;
static uint32_t OFFSET_OF_LOCAL_STATUS = 0;
static uint32_t OFFSET_OF_KEY = 0;
static uint32_t OFFSET_OF_STRING_LITERAL = 0;

// Variable offset(COFF)
static uint32_t COFF_OFFSET_OF_EFI_SUCCESS = 0;
static uint32_t COFF_OFFSET_OF_ST = 0;
static uint32_t COFF_OFFSET_OF_RT = 0;
static uint32_t COFF_OFFSET_OF_IN = 0;
static uint32_t COFF_OFFSET_OF_OUT = 0;
static uint32_t COFF_OFFSET_OF_STRING_LITERAL = 0;

// Member offset
static const uint32_t OFFSET_OF_ST_RuntimeServices = 0x58;
static const uint32_t OFFSET_OF_ST_ConIn = 0x30;
static const uint32_t OFFSET_OF_ST_ConOut = 0x40;
static const uint32_t OFFSET_OF_OUT_OutputString = 0x8;
static const uint32_t OFFSET_OF_IN_ReadKeyStroke = 0x8;
static const uint32_t OFFSET_OF_RT_ResetSystem = 0x68;

// Global symbols 
static uint8_t* SYMBOL_OF_EFI_SUCCESS = "EFI_SUCCESS";
static uint32_t SYMBOL_LENGTH_OF_EFI_SUCCESS = 11;
static uint8_t* SYMBOL_OF_ST = "ST";
static uint32_t SYMBOL_LENGTH_OF_ST = 2;
static uint8_t* SYMBOL_OF_RT = "RT";
static uint32_t SYMBOL_LENGTH_OF_RT = 2;
static uint8_t* SYMBOL_OF_IN = "IN";
static uint32_t SYMBOL_LENGTH_OF_IN = 2;
static uint8_t* SYMBOL_OF_OUT = "OUT";
static uint32_t SYMBOL_LENGTH_OF_OUT = 2;
static uint8_t* symbol = "When you press any key, the system will reboot.\n";
static rsize_t symbol_length = 0;

// Symbol table index
static uint32_t SYMBOL_INDEX_OF_EFI_SUCCESS = 0;
static uint32_t SYMBOL_INDEX_OF_ST = 0;
static uint32_t SYMBOL_INDEX_OF_RT = 0;
static uint32_t SYMBOL_INDEX_OF_IN = 0;
static uint32_t SYMBOL_INDEX_OF_OUT = 0;
static uint32_t SYMBOL_INDEX_OF_STRING_LITERAL = 0;

static bool make_section_code_origin_wasm_UEFI64_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer, size_t* body_size
);

// UEFI: original WASM64
TP_WASM_MODULE_SECTION* tp_make_section_code_origin_wasm_UEFI64(TP_SYMBOL_TABLE* symbol_table)
{
    symbol_table->member_wasm_local_pos = 0;

    TP_WASM_MODULE_SECTION* section = NULL;
    uint8_t* section_buffer = NULL;

    if ( ! tp_wasm_add_global_variable_i64(
        symbol_table, EFI_SUCCESS, SYMBOL_OF_EFI_SUCCESS, SYMBOL_LENGTH_OF_EFI_SUCCESS,
        0, &OFFSET_OF_EFI_SUCCESS, &COFF_OFFSET_OF_EFI_SUCCESS, &SYMBOL_INDEX_OF_EFI_SUCCESS)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_global_variable_i32(
        symbol_table, ST, SYMBOL_OF_ST, SYMBOL_LENGTH_OF_ST,
        0, &OFFSET_OF_ST, &COFF_OFFSET_OF_ST, &SYMBOL_INDEX_OF_ST)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_global_variable_i32(
        symbol_table, RT, SYMBOL_OF_RT, SYMBOL_LENGTH_OF_RT,
        0, &OFFSET_OF_RT, &COFF_OFFSET_OF_RT, &SYMBOL_INDEX_OF_RT)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_global_variable_i32(
        symbol_table, IN_, SYMBOL_OF_IN, SYMBOL_LENGTH_OF_IN,
        0, &OFFSET_OF_IN, &COFF_OFFSET_OF_IN, &SYMBOL_INDEX_OF_IN)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_global_variable_i32(
        symbol_table, OUT_, SYMBOL_OF_OUT, SYMBOL_LENGTH_OF_OUT,
        0, &OFFSET_OF_OUT, &COFF_OFFSET_OF_OUT, &SYMBOL_INDEX_OF_OUT)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_local_variable_i64(symbol_table, LOCAL_STATUS, &OFFSET_OF_LOCAL_STATUS)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_local_variable_i32(symbol_table, KEY, &OFFSET_OF_KEY)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    symbol_length = strlen(symbol);
    uint16_t* string = L"When you press any key, the system will reboot.\n";
    rsize_t string_length = wcslen(string);

    if ( ! tp_wasm_add_string_literal(
        symbol_table, &STRING_LITERAL, symbol, symbol_length, string, string_length,
        &OFFSET_OF_STRING_LITERAL, &COFF_OFFSET_OF_STRING_LITERAL, &SYMBOL_INDEX_OF_STRING_LITERAL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_wasm_add_function(
        symbol_table, TP_WASM_UEFI_ENTRY_POINT, TP_WASM_UEFI_ENTRY_POINT_LENGTH)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    // unsigned long long efi_main(void* image_handle, struct EFI_SYSTEM_TABLE* system_table)
    // (func $efi_main (type 0) (param i64 i64) (result i64)
    //   (local i64)
    size_t body_size = 0;

    if ( ! make_section_code_origin_wasm_UEFI64_code(symbol_table, NULL, &body_size)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    uint32_t count = TP_WASM_MODULE_SECTION_CODE_COUNT;
    uint32_t payload_len = tp_encode_ui32leb128(NULL, 0, count);
    payload_len += tp_encode_ui32leb128(NULL, 0, (uint32_t)body_size);
    payload_len += (uint32_t)body_size;

    TP_MAKE_WASM_SECTION_BUFFER(
        symbol_table, section, section_buffer, TP_WASM_SECTION_TYPE_CODE, payload_len
    );

    // unsigned long long efi_main(void* image_handle, struct EFI_SYSTEM_TABLE* system_table)
    // (func $efi_main (type 0) (param i64 i64) (result i64)
    //   (local i64)
    size_t index = tp_encode_ui32leb128(section_buffer, 0, TP_WASM_SECTION_TYPE_CODE);
    index += tp_encode_ui32leb128(section_buffer, index, payload_len);
    index += tp_encode_ui32leb128(section_buffer, index, count);
    index += tp_encode_ui32leb128(section_buffer, index, (uint32_t)body_size);

    if ( ! make_section_code_origin_wasm_UEFI64_code(symbol_table, section_buffer, &index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    return section;

fail:
    if (section){

        TP_FREE(symbol_table, &(section->member_name_len_name_payload_data), section->member_section_size);

        TP_FREE(symbol_table, &section, sizeof(TP_WASM_MODULE_SECTION));
    }

    return NULL;
}

static bool make_section_code_origin_wasm_UEFI64_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* section_buffer, size_t* body_size)
{
    // unsigned long long efi_main(void* image_handle, struct EFI_SYSTEM_TABLE* system_table)
    // (func $efi_main (type 0) (param i64 i64) (result i64)
    //   (local i64)
    uint32_t local_count = TP_WASM_MODULE_SECTION_CODE_LOCAL_COUNT;
    uint32_t var_count = TP_WASM_MODULE_SECTION_CODE_VAR_COUNT;
    uint32_t var_type = TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32;

    size_t index = *body_size;
    index += tp_encode_ui32leb128(section_buffer, index, local_count);
    index += tp_encode_ui32leb128(section_buffer, index, var_count);
    index += tp_encode_ui32leb128(section_buffer, index, var_type);

    // Prologue
    TP_WASM_CODE_BEGIN;
    symbol_table->member_wasm_relocation_begin = index;
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_get_global_code(section_buffer, index, 0));
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_i64_const_code(section_buffer, index, 16));
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_i64_sub_code(section_buffer, index));
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_tee_local_code(section_buffer, index, 2));
    TP_WASM_CODE_PROLOGUE(symbol_table, index, tp_make_set_global_code(section_buffer, index, 0));

    // ST = system_table;
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1032 */OFFSET_OF_ST
        ), COFF_OFFSET_OF_ST, SYMBOL_OF_ST, SYMBOL_LENGTH_OF_ST, SYMBOL_INDEX_OF_ST
    );
    TP_WASM_CODE(symbol_table, index, tp_make_get_local_code(section_buffer, index, 1));
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_store_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );

    // RT = ST->RuntimeServices;
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1048 */OFFSET_OF_RT
        ), COFF_OFFSET_OF_RT, SYMBOL_OF_RT, SYMBOL_LENGTH_OF_RT, SYMBOL_INDEX_OF_RT
     );
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1032 */OFFSET_OF_ST
        ), COFF_OFFSET_OF_ST, SYMBOL_OF_ST, SYMBOL_LENGTH_OF_ST, SYMBOL_INDEX_OF_ST
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 0x58 */OFFSET_OF_ST_RuntimeServices
        )
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_store_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );

    // IN = ST->ConIn;
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1048 */OFFSET_OF_IN
        ), COFF_OFFSET_OF_IN, SYMBOL_OF_IN, SYMBOL_LENGTH_OF_IN, SYMBOL_INDEX_OF_IN
    );
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1032 */OFFSET_OF_ST
        ), COFF_OFFSET_OF_ST, SYMBOL_OF_ST, SYMBOL_LENGTH_OF_ST, SYMBOL_INDEX_OF_ST
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 0x30 */OFFSET_OF_ST_ConIn
        )
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_store_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );

    // OUT = ST->ConOut;
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1056 */OFFSET_OF_OUT
        ), COFF_OFFSET_OF_OUT, SYMBOL_OF_OUT, SYMBOL_LENGTH_OF_OUT, SYMBOL_INDEX_OF_OUT
    );
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1032 */OFFSET_OF_ST
        ), COFF_OFFSET_OF_ST, SYMBOL_OF_ST, SYMBOL_LENGTH_OF_ST, SYMBOL_INDEX_OF_ST
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 0x40 */OFFSET_OF_ST_ConOut
        )
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_store_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );

    // OUT
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1056 */OFFSET_OF_OUT
        ), COFF_OFFSET_OF_OUT, SYMBOL_OF_OUT, SYMBOL_LENGTH_OF_OUT, SYMBOL_INDEX_OF_OUT
    );
    TP_WASM_CODE_ARG(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        ), 0, TP_WASM_RETURN_VOID
    );

    // L"When you press any key, the system will reboot.\n"
    TP_WASM_CODE_STRING_LITERAL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1064 */OFFSET_OF_STRING_LITERAL
        ), COFF_OFFSET_OF_STRING_LITERAL,
        symbol, symbol_length, SYMBOL_INDEX_OF_STRING_LITERAL, 1, TP_WASM_RETURN_VOID
    );

    // OUT->OutputString
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1056 */OFFSET_OF_OUT
        ), COFF_OFFSET_OF_OUT, SYMBOL_OF_OUT, SYMBOL_LENGTH_OF_OUT, SYMBOL_INDEX_OF_OUT
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 0x8 */OFFSET_OF_OUT_OutputString
        )
    );

    // OUT->OutputString(OUT, L"When you press any key, the system will reboot.\n");
    TP_WASM_CODE_CALL(
        symbol_table, index, tp_make_call_indirect_code(
            section_buffer, index, TP_WASM_CALL_TYPE_0, 0
        ), TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
    );
    TP_WASM_CODE(symbol_table, index, tp_make_drop_code(section_buffer, index));
 
    // unsigned long long local_status = EFI_SUCCESS;
    TP_WASM_CODE_BASE(symbol_table, index, tp_make_get_local_code(section_buffer, index, 2));
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1024 */OFFSET_OF_EFI_SUCCESS
        ), COFF_OFFSET_OF_EFI_SUCCESS,
        SYMBOL_OF_EFI_SUCCESS, SYMBOL_LENGTH_OF_EFI_SUCCESS, SYMBOL_INDEX_OF_EFI_SUCCESS
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_store_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 8 */OFFSET_OF_LOCAL_STATUS
        )
    );

    // do{
    // loop  ;; label = @1
    TP_WASM_CODE(symbol_table, index, tp_make_loop_code(section_buffer, index, TP_WASM_BLOCK_TYPE_VOID));

        // IN
        TP_WASM_CODE_REL64(
            symbol_table, index, tp_make_i64_const_code(
                section_buffer, index, /* 1048 */OFFSET_OF_IN
            ), COFF_OFFSET_OF_IN, SYMBOL_OF_IN, SYMBOL_LENGTH_OF_IN, SYMBOL_INDEX_OF_IN
        );
        TP_WASM_CODE_ARG(
            symbol_table, index, tp_make_i64_load_code(
                section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
            ), 0, TP_WASM_RETURN_VOID
        );

        // struct EFI_INPUT_KEY key;
        // &key
        TP_WASM_CODE_BASE(symbol_table, index, tp_make_get_local_code(section_buffer, index, 2));
        TP_WASM_CODE(
            symbol_table, index, tp_make_i64_const_code(section_buffer, index, /* 16 */OFFSET_OF_KEY)
        );
        TP_WASM_CODE_ARG(
            symbol_table, index, tp_make_i64_add_code(section_buffer, index), 1, TP_WASM_RETURN_VOID
        );

        // IN->ReadKeyStroke
        TP_WASM_CODE_REL64(
            symbol_table, index, tp_make_i64_const_code(
                section_buffer, index, /* 1048 */OFFSET_OF_IN
            ), COFF_OFFSET_OF_IN, SYMBOL_OF_IN, SYMBOL_LENGTH_OF_IN, SYMBOL_INDEX_OF_IN
        );
        TP_WASM_CODE(
            symbol_table, index, tp_make_i64_load_code(
                section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
            )
        );
        TP_WASM_CODE(
            symbol_table, index, tp_make_i64_load_code(
                section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 0x8 */OFFSET_OF_IN_ReadKeyStroke
            )
        );

        // local_status = IN->ReadKeyStroke(IN, &key);
        TP_WASM_CODE_CALL(
            symbol_table, index, tp_make_call_indirect_code(
                section_buffer, index, TP_WASM_CALL_TYPE_0, 0
            ), TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
        );

    // }while (EFI_SUCCESS != local_status);
        TP_WASM_CODE_BASE(symbol_table, index, tp_make_get_local_code(section_buffer, index, 2));
        TP_WASM_CODE(
            symbol_table, index, tp_make_i64_load_code(
                section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 8 */OFFSET_OF_LOCAL_STATUS
            )
        );
        TP_WASM_CODE(symbol_table, index, tp_make_i64_ne_code(section_buffer, index));
        // br_if 0 (;@1;)
        TP_WASM_CODE(symbol_table, index, tp_make_br_if_code(section_buffer, index, 0/* relative_depth */));
        // end
        TP_WASM_CODE(symbol_table, index, tp_make_end_code(section_buffer, index));

    // EfiResetCold
    TP_WASM_CODE_ARG(
        symbol_table, index, tp_make_i32_const_code(section_buffer, index, 0), 0, TP_WASM_RETURN_VOID
    );

    // EFI_SUCCESS
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1024 */OFFSET_OF_EFI_SUCCESS
        ), COFF_OFFSET_OF_EFI_SUCCESS,
        SYMBOL_OF_EFI_SUCCESS, SYMBOL_LENGTH_OF_EFI_SUCCESS, SYMBOL_INDEX_OF_EFI_SUCCESS
    );
    TP_WASM_CODE_ARG(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        ), 1, TP_WASM_RETURN_VOID
    );

    // 0
    TP_WASM_CODE_ARG(
        symbol_table, index, tp_make_i64_const_code(section_buffer, index, 0), 2, TP_WASM_RETURN_VOID
    );

    // NULL
    TP_WASM_CODE_ARG(
        symbol_table, index, tp_make_i64_const_code(section_buffer, index, 0), 3, TP_WASM_RETURN_VOID
    );

    // RT->ResetSystem
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1048 */OFFSET_OF_RT
        ), COFF_OFFSET_OF_RT, SYMBOL_OF_RT, SYMBOL_LENGTH_OF_RT, SYMBOL_INDEX_OF_RT
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        )
    );
    TP_WASM_CODE(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, /* 0x68 */OFFSET_OF_RT_ResetSystem
        )
    );

    // RT->ResetSystem(0/* EfiResetCold */, EFI_SUCCESS, 0, 0/* NULL */);
    TP_WASM_CODE_CALL(
        symbol_table, index, tp_make_call_indirect_code(
            section_buffer, index, TP_WASM_CALL_TYPE_1, 0
        ), TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
    );
    TP_WASM_CODE(symbol_table, index, tp_make_drop_code(section_buffer, index));

    // Epilogue
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_get_local_code(section_buffer, index, 2));
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_i64_const_code(section_buffer, index, 16));
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_i64_add_code(section_buffer, index));
    TP_WASM_CODE_EPILOGUE(symbol_table, index, tp_make_set_global_code(section_buffer, index, 0));

    // return EFI_SUCCESS;
    TP_WASM_CODE_REL64(
        symbol_table, index, tp_make_i64_const_code(
            section_buffer, index, /* 1024 */OFFSET_OF_EFI_SUCCESS
        ), COFF_OFFSET_OF_EFI_SUCCESS,
        SYMBOL_OF_EFI_SUCCESS, SYMBOL_LENGTH_OF_EFI_SUCCESS, SYMBOL_INDEX_OF_EFI_SUCCESS
    );
    TP_WASM_CODE_ARG(
        symbol_table, index, tp_make_i64_load_code(
            section_buffer, index, TP_WASM_MEM_ALIGN_64, 0
        ), TP_WASM_ARG_INDEX_EAX,
        TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64
    );
    TP_WASM_CODE(symbol_table, index, tp_make_ret_code(section_buffer, index));

    // end
    TP_WASM_CODE(symbol_table, index, tp_make_end_code(section_buffer, index));

    *body_size = index;

    return true;

fail:
    return false;
}

