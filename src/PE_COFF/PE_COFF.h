﻿
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(PE_COFF_H_)
#define PE_COFF_H_

#include <windows.h>
#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

// ----------------------------------------------------------------------------------------
// COFF File Header

typedef struct TP_COFF_FILE_HEADER_{
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
}TP_COFF_FILE_HEADER;

// Machine Types

#define TP_IMAGE_FILE_MACHINE_UNKNOWN 0x0
#define TP_IMAGE_FILE_MACHINE_AMD64 0x8664

// Characteristics

#define TP_IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define TP_IMAGE_FILE_DEBUG_STRIPPED 0x0200

// ----------------------------------------------------------------------------------------
// Section Table

#define TP_IMAGE_SIZEOF_SHORT_NAME 8

typedef struct TP_SECTION_TABLE_{
    uint8_t Name[TP_IMAGE_SIZEOF_SHORT_NAME];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
}TP_SECTION_TABLE;

// Characteristics(Section Flags)

// .bss    Uninitialized data (free format)
// TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ | TP_IMAGE_SCN_MEM_WRITE

// .data   Initialized data (free format)
// TP_IMAGE_SCN_CNT_INITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ | TP_IMAGE_SCN_MEM_WRITE

// .rdata  Read-only initialized data
// TP_IMAGE_SCN_CNT_INITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ

// .text   Executable code (free format)
// TP_IMAGE_SCN_CNT_CODE | TP_IMAGE_SCN_MEM_EXECUTE | TP_IIMAGE_SCN_MEM_READ

#define TP_IMAGE_SCN_CNT_CODE 0x00000020
#define TP_IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define TP_IMAGE_SCN_LNK_INFO 0x00000200
#define TP_IMAGE_SCN_LNK_REMOVE 0x00000800
#define TP_IMAGE_SCN_LNK_COMDAT 0x00001000
#define TP_IMAGE_SCN_GPREL 0x00008000
#define TP_IMAGE_SCN_ALIGN_1BYTES 0x00100000
#define TP_IMAGE_SCN_ALIGN_2BYTES 0x00200000
#define TP_IMAGE_SCN_ALIGN_4BYTES 0x00300000
#define TP_IMAGE_SCN_ALIGN_8BYTES 0x00400000
#define TP_IMAGE_SCN_ALIGN_16BYTES 0x00500000
#define TP_IMAGE_SCN_ALIGN_32BYTES 0x00600000
#define TP_IMAGE_SCN_ALIGN_64BYTES 0x00700000
#define TP_IMAGE_SCN_ALIGN_128BYTES 0x00800000
#define TP_IMAGE_SCN_ALIGN_256BYTES 0x00900000
#define TP_IMAGE_SCN_ALIGN_512BYTES 0x00A00000
#define TP_IMAGE_SCN_ALIGN_1024BYTES 0x00B00000
#define TP_IMAGE_SCN_ALIGN_2048BYTES 0x00C00000
#define TP_IMAGE_SCN_ALIGN_4096BYTES 0x00D00000
#define TP_IMAGE_SCN_ALIGN_8192BYTES 0x00E00000
#define TP_IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000
#define TP_IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define TP_IMAGE_SCN_MEM_NOT_CACHED 0x04000000
#define TP_IMAGE_SCN_MEM_NOT_PAGED 0x08000000
#define TP_IMAGE_SCN_MEM_SHARED 0x10000000
#define TP_IMAGE_SCN_MEM_EXECUTE 0x20000000
#define TP_IMAGE_SCN_MEM_READ 0x40000000
#define TP_IMAGE_SCN_MEM_WRITE 0x80000000

#define TP_IMAGE_SCN_ALIGN_MASK 0x00F00000

// ----------------------------------------------------------------------------------------
// Section Data(Relocations)

#pragma pack(push, 1)

typedef struct TP_COFF_RELOCATIONS_{
    union{
        uint32_t VirtualAddress;
        uint32_t RelocCount; // When IMAGE_SCN_LNK_NRELOC_OVFL is set.
    } UNION;
    uint32_t SymbolTableIndex;
    uint16_t Type;
}TP_COFF_RELOCATIONS;

#pragma pack(pop)

// Type Indicators

#define TP_IMAGE_REL_AMD64_ABSOLUTE 0x0000
#define TP_IMAGE_REL_AMD64_ADDR64 0x0001
#define TP_IMAGE_REL_AMD64_ADDR32 0x0002
#define TP_IMAGE_REL_AMD64_ADDR32NB 0x0003
#define TP_IMAGE_REL_AMD64_REL32 0x0004
#define TP_IMAGE_REL_AMD64_REL32_1 0x0005
#define TP_IMAGE_REL_AMD64_REL32_2 0x0006
#define TP_IMAGE_REL_AMD64_REL32_3 0x0007
#define TP_IMAGE_REL_AMD64_REL32_4 0x0008
#define TP_IMAGE_REL_AMD64_REL32_5 0x0009
#define TP_IMAGE_REL_AMD64_SECTION 0x000A
#define TP_IMAGE_REL_AMD64_SECREL 0x000B
#define TP_IMAGE_REL_AMD64_SECREL7 0x000C
#define TP_IMAGE_REL_AMD64_TOKEN 0x000D
#define TP_IMAGE_REL_AMD64_SREL32 0x000E
#define TP_IMAGE_REL_AMD64_PAIR 0x000F
#define TP_IMAGE_REL_AMD64_SSPAN32 0x0010

// Unknown new values.
#define TP_IMAGE_REL_AMD64_EHANDLER 0x0011
#define TP_IMAGE_REL_AMD64_IMPORT_BR 0x0012
#define TP_IMAGE_REL_AMD64_IMPORT_CALL 0x0013
#define TP_IMAGE_REL_AMD64_CFG_BR 0x0014
#define TP_IMAGE_REL_AMD64_CFG_BR_REX 0x0015
#define TP_IMAGE_REL_AMD64_CFG_CALL 0x0016
#define TP_IMAGE_REL_AMD64_INDIR_BR 0x0017
#define TP_IMAGE_REL_AMD64_INDIR_BR_REX 0x0018
#define TP_IMAGE_REL_AMD64_INDIR_CALL 0x0019
#define TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_FIRST 0x0020
#define TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_LAST 0x002F

// ----------------------------------------------------------------------------------------
// COFF Symbol Table(Auxiliary Symbol Records)

#define TP_IMAGE_SIZEOF_SYMBOL 18

// Auxiliary Format 1: Function Definitions
typedef struct TP_AUX_SYMBOL_RECORD_FUNC_DEF_{
    uint32_t TagIndex;
    uint32_t TotalSize;
    uint32_t PointerToLinenumber;
    uint32_t PointerToNextFunction;
    uint8_t Unused[2];
}TP_AUX_SYMBOL_RECORD_FUNC_DEF;

// Auxiliary Format 2: .bf and .ef Symbols
typedef struct TP_AUX_SYMBOL_RECORD_FUNC_LINE_{
    uint8_t Unused1[4];
    uint16_t Linenumber;
    uint8_t Unused2[6];
    uint32_t PointerToNextFunction;
    uint8_t Unused3[2];
}TP_AUX_SYMBOL_RECORD_FUNC_LINE;

// Auxiliary Format 3: Weak Externals
typedef struct TP_AUX_SYMBOL_RECORD_WEAK_EXTERN_ {
    uint32_t WeakDefaultSymIndex;
    uint32_t Characteristics;
    uint8_t Unused[10];
}TP_AUX_SYMBOL_RECORD_WEAK_EXTERN;

#define TP_IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define TP_IMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define TP_IMAGE_WEAK_EXTERN_SEARCH_ALIAS      3
// Unknown new value.
#define TP_IMAGE_WEAK_EXTERN_ANTI_DEPENDENCY   4

// Auxiliary Format 4: Files
typedef struct TP_AUX_SYMBOL_RECORD_FILE_{
    uint8_t FileName[TP_IMAGE_SIZEOF_SYMBOL];
}TP_AUX_SYMBOL_RECORD_FILE;

// Auxiliary Format 5: Section Definitions
typedef struct TP_AUX_SYMBOL_RECORD_SECTION_{
    uint32_t Length;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t CheckSum;
    int16_t Number;
    uint8_t Selection;
    uint8_t Unused[3];
}TP_AUX_SYMBOL_RECORD_SECTION;

// COMDAT Sections

#define TP_IMAGE_COMDAT_SELECT_NODUPLICATES 1
#define TP_IMAGE_COMDAT_SELECT_ANY 2
#define TP_IMAGE_COMDAT_SELECT_SAME_SIZE 3
#define TP_IMAGE_COMDAT_SELECT_EXACT_MATCH 4
#define TP_IMAGE_COMDAT_SELECT_ASSOCIATIVE 5
#define TP_IMAGE_COMDAT_SELECT_LARGEST 6
// Unknown new value.
#define TP_IMAGE_COMDAT_SELECT_NEWEST 7

// Auxiliary Symbol Record
typedef union TP_AUX_SYMBOL_RECORD_{
    // Auxiliary Format 1: Function Definitions
    TP_AUX_SYMBOL_RECORD_FUNC_DEF func_def;
    // Auxiliary Format 2: .bf and .ef Symbols
    TP_AUX_SYMBOL_RECORD_FUNC_LINE func_line;
    // Auxiliary Format 3: Weak Externals
    TP_AUX_SYMBOL_RECORD_WEAK_EXTERN weak_extern;
    // Auxiliary Format 4: Files
    TP_AUX_SYMBOL_RECORD_FILE file;
    // Auxiliary Format 5: Section Definitions
    TP_AUX_SYMBOL_RECORD_SECTION section;
}TP_AUX_SYMBOL_RECORD;

// ----------------------------------------------------------------------------------------
// COFF Symbol Table

#pragma pack(push, 1)

typedef  struct TP_COFF_SYMBOL_TABLE_{
    union SYMBOL_NAME_REPRESENTATION_{
        uint8_t ShortName[TP_IMAGE_SIZEOF_SHORT_NAME];
        struct{
            uint32_t Zeroes; // if 0, use LongName
            uint32_t Offset; // An offset into the string table.
        } Name;
    } UNION;
    uint32_t Value;
    int16_t SectionNumber;
    uint16_t Type;
    uint8_t StorageClass;
    uint8_t NumberOfAuxSymbols;
    TP_AUX_SYMBOL_RECORD AuxiliarySymbolRecords[];
}TP_COFF_SYMBOL_TABLE;

#pragma pack(pop)

// Section Number Values
#define TP_IMAGE_SYM_UNDEFINED 0
#define TP_IMAGE_SYM_ABSOLUTE  -1
#define TP_IMAGE_SYM_DEBUG  -2
#define TP_IMAGE_SYM_SECTION_MAX 0xfeff

// Type Representation
#define TP_IMAGE_SYM_TYPE_NULL 0
#define TP_IMAGE_SYM_DTYPE_NULL 0
#define TP_IMAGE_SYM_DTYPE_FUNCTION 2

// Storage Class
#define TP_IMAGE_SYM_CLASS_END_OF_FUNCTION 0xff
#define TP_IMAGE_SYM_CLASS_NULL 0
#define TP_IMAGE_SYM_CLASS_EXTERNAL 2
#define TP_IMAGE_SYM_CLASS_STATIC 3
#define TP_IMAGE_SYM_CLASS_FUNCTION 101
#define TP_IMAGE_SYM_CLASS_FILE 103

#endif

