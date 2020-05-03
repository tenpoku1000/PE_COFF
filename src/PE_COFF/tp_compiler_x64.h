
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_X64_H_)
#define TP_COMPILER_X64_H_

#include "tp_compiler_wasm.h"

// ----------------------------------------------------------------------------------------
// x64 section:

#define TP_WASM_STACK_EMPTY -1
#define TP_WASM_STACK_SIZE_ALLOCATE_UNIT 256
#define TP_WASM_LOCAL_VARIABLE_MAX_DEFAULT 1024
#define TP_WASM_TEMPORARY_VARIABLE_MAX_DEFAULT 1024

typedef enum tp_wasm_stack_pop_{
    TP_WASM_STACK_POP_MODE_DEFAULT,
    TP_WASM_STACK_POP_MODE_PARAM,
    TP_WASM_STACK_POP_MODE_LABEL
}TP_WASM_STACK_POP_MODE;

typedef enum tp_x64_mov_imm_mode_{
    TP_X64_MOV_IMM_MODE_DEFAULT,
    TP_X64_MOV_IMM_MODE_FORCE_IMM32
}TP_X64_MOV_IMM_MODE;

typedef enum tp_x64_add_sub_imm_mode_{
    TP_X64_ADD_SUB_IMM_MODE_DEFAULT,
    TP_X64_ADD_SUB_IMM_MODE_FORCE_IMM32
}TP_X64_ADD_SUB_IMM_MODE;

#define TP_PADDING_MASK (16 - 1)

#define TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size) \
\
    do{ \
        if (0 == (tmp_x64_code_size)){ \
\
            TP_PUT_LOG_MSG( \
                (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: 0 == tmp_x64_code_size at %1 function."), \
                TP_LOG_PARAM_STRING(__func__) \
            ); \
\
            return 0; \
        } \
\
        (x64_code_size) += (tmp_x64_code_size); \
    }while (false)

#define TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size) \
\
    do{ \
        if (false == (status)){ \
\
            TP_PUT_LOG_MSG( \
                (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: 0 == tmp_x64_code_size at %1 function."), \
                TP_LOG_PARAM_STRING(__func__) \
            ); \
\
            return 0; \
        } \
\
        (x64_code_size) += (tmp_x64_code_size); \
    }while (false)

typedef enum tp_x86_32_register_{
    TP_X86_32_REGISTER_EAX = 0,
    TP_X86_32_REGISTER_ECX,
    TP_X86_32_REGISTER_EDX,
    TP_X86_32_REGISTER_EBX,
    TP_X86_32_REGISTER_ESP,
    TP_X86_32_REGISTER_EBP,
    TP_X86_32_REGISTER_ESI,
    TP_X86_32_REGISTER_EDI,
    TP_X86_32_REGISTER_NULL,
    TP_X86_32_REGISTER_NUM = 8
}TP_X86_32_REGISTER;

typedef enum tp_x64_32_register_{
    TP_X64_32_REGISTER_R8D = 0,
    TP_X64_32_REGISTER_R9D,
    TP_X64_32_REGISTER_R10D,
    TP_X64_32_REGISTER_R11D,
    TP_X64_32_REGISTER_R12D,
    TP_X64_32_REGISTER_R13D,
    TP_X64_32_REGISTER_R14D,
    TP_X64_32_REGISTER_R15D,
    TP_X64_32_REGISTER_NULL,
    TP_X64_32_REGISTER_NUM = 8
}TP_X64_32_REGISTER;

typedef enum tp_x64_64_register_{
    TP_X64_64_REGISTER_RAX = 0,
    TP_X64_64_REGISTER_RCX,
    TP_X64_64_REGISTER_RDX,
    TP_X64_64_REGISTER_RBX,
    TP_X64_64_REGISTER_RSP,
    TP_X64_64_REGISTER_INDEX_NONE = 4,
    TP_X64_64_REGISTER_RBP,
    TP_X64_64_REGISTER_RSI,
    TP_X64_64_REGISTER_RDI,
    TP_X64_64_REGISTER_R8,
    TP_X64_64_REGISTER_R9,
    TP_X64_64_REGISTER_R10,
    TP_X64_64_REGISTER_R11,
    TP_X64_64_REGISTER_R12,
    TP_X64_64_REGISTER_R13,
    TP_X64_64_REGISTER_R14,
    TP_X64_64_REGISTER_R15,
    TP_X64_64_REGISTER_NULL
}TP_X64_64_REGISTER;

typedef enum tp_x64_item_kind_{
    TP_X64_ITEM_KIND_NONE = 0,
    TP_X64_ITEM_KIND_X86_32_REGISTER,
    TP_X64_ITEM_KIND_X64_32_REGISTER,
    TP_X64_ITEM_KIND_MEMORY
}TP_X64_ITEM_KIND;

typedef enum tp_x64_item_memory_kind_{
    TP_X64_ITEM_MEMORY_KIND_NONE = 0,
    TP_X64_ITEM_MEMORY_KIND_LOCAL,
    TP_X64_ITEM_MEMORY_KIND_GLOBAL,
    TP_X64_ITEM_MEMORY_KIND_TEMP
}TP_X64_ITEM_MEMORY_KIND;

typedef union tp_x64_item_{
    TP_X86_32_REGISTER member_x86_32_register;
    TP_X64_32_REGISTER member_x64_32_register;
}TP_X64_ITEM;

typedef struct tp_wasm_stack_element_{
    uint32_t member_wasm_opcode;

    // Control flow operators
    uint32_t member_sig;
    uint32_t member_relative_depth;
    uint32_t member_x64_code_offset;

    // Call operators
    uint32_t member_type_index;
    uint32_t member_reserved;

    // Variable access
    uint32_t member_local_index;
    uint32_t member_global_index;

    // Memory-related operators
    uint32_t member_flags;
    uint32_t member_mem_offset;

    // Constants
    int32_t member_i32;
    int64_t member_i64;

    TP_WASM_RELOCATION* member_wasm_relocation;

    TP_X64_ITEM_KIND member_x64_item_kind;
    TP_X64_ITEM member_x64_item;
    TP_X64_ITEM_MEMORY_KIND member_x64_memory_kind;
    TP_X64_ITEM_KIND member_x64_memory_item_kind;
    int32_t member_offset;
    int32_t member_offset_backup;

    bool member_is_stack_base;
}TP_WASM_STACK_ELEMENT;

typedef enum tp_x64_allocate_mode_{
    TP_X64_ALLOCATE_DEFAULT,
    TP_X64_ALLOCATE_MEMORY
}TP_X64_ALLOCATE_MODE;

typedef enum tp_x64_nv64_register_{
    TP_X64_NV64_REGISTER_NULL = 0,
    TP_X64_NV64_REGISTER_RBX = TP_X64_64_REGISTER_RBX,
    TP_X64_NV64_REGISTER_RSI = TP_X64_64_REGISTER_RSI,
    TP_X64_NV64_REGISTER_RDI,
    TP_X64_NV64_REGISTER_R12 = TP_X64_64_REGISTER_R12,
    TP_X64_NV64_REGISTER_R13,
    TP_X64_NV64_REGISTER_R14,
    TP_X64_NV64_REGISTER_R15,
    TP_X64_NV64_REGISTER_NUM = 7,
    TP_X64_NV64_REGISTER_RBX_INDEX = 0,
    TP_X64_NV64_REGISTER_RSI_INDEX,
    TP_X64_NV64_REGISTER_RDI_INDEX,
    TP_X64_NV64_REGISTER_R12_INDEX,
    TP_X64_NV64_REGISTER_R13_INDEX,
    TP_X64_NV64_REGISTER_R14_INDEX,
    TP_X64_NV64_REGISTER_R15_INDEX
}TP_X64_NV64_REGISTER;

typedef enum tp_x64_v64_register_{
    TP_X64_V64_REGISTER_NULL = TP_X64_64_REGISTER_RAX - 1,
    TP_X64_V64_REGISTER_RAX = TP_X64_64_REGISTER_RAX,
    TP_X64_V64_REGISTER_RCX = TP_X64_64_REGISTER_RCX,
    TP_X64_V64_REGISTER_RDX,
    TP_X64_V64_REGISTER_R8 = TP_X64_64_REGISTER_R8,
    TP_X64_V64_REGISTER_R9,
    TP_X64_V64_REGISTER_R10,
    TP_X64_V64_REGISTER_R11,
    TP_X64_V64_REGISTER_NUM = 7,
    TP_X64_V64_REGISTER_RAX_INDEX = 0,
    TP_X64_V64_REGISTER_RCX_INDEX,
    TP_X64_V64_REGISTER_RDX_INDEX,
    TP_X64_V64_REGISTER_R8_INDEX,
    TP_X64_V64_REGISTER_R9_INDEX,
    TP_X64_V64_REGISTER_R10_INDEX,
    TP_X64_V64_REGISTER_R11_INDEX
}TP_X64_V64_REGISTER;

typedef enum tp_x64_{
    TP_X64_MOV,
    TP_X64_MOV_RIP,
    TP_X64_MOV_32_TO_64,
    TP_X64_ADD,
    TP_X64_SUB,
    TP_X64_IMUL,
    TP_X64_IDIV,
    TP_X64_XOR,
    TP_X64_NULL
}TP_X64;

typedef enum tp_x64_direction_{
    TP_X64_DIRECTION_SOURCE_REGISTER,
    TP_X64_DIRECTION_SOURCE_MEMORY
}TP_X64_DIRECTION;

#endif

