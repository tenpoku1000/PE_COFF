
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(PE_COFF_H_)
#define PE_COFF_H_

#include <windows.h>
#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

// ----------------------------------------------------------------------------------------
// PE File DOS Header

typedef struct TP_PE_DOS_HEADER_{
    uint16_t e_magic; // Magic number
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    int32_t e_lfanew; // File address of PE File header.
}TP_PE_DOS_HEADER;

#define TP_PE_DOS_HEADER_MAGIC 0x5A4D // MZ

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

#define TP_IMAGE_FILE_RELOCS_STRIPPED 0x0001
#define TP_IMAGE_FILE_EXECUTABLE_IMAGE 0x0002
#define TP_IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004
#define TP_IMAGE_FILE_LOCAL_SYMS_STRIPPED 0x0008
#define TP_IMAGE_FILE_AGGRESIVE_WS_TRIM 0x0010
#define TP_IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define TP_IMAGE_FILE_BYTES_REVERSED_LO 0x0080
#define TP_IMAGE_FILE_32BIT_MACHINE 0x0100
#define TP_IMAGE_FILE_DEBUG_STRIPPED 0x0200
#define TP_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP 0x0400
#define TP_IMAGE_FILE_NET_RUN_FROM_SWAP 0x0800
#define TP_IMAGE_FILE_SYSTEM 0x1000
#define TP_IMAGE_FILE_DLL 0x2000
#define TP_IMAGE_FILE_UP_SYSTEM_ONLY 0x4000
#define TP_IMAGE_FILE_BYTES_REVERSED_HI 0x8000

// ----------------------------------------------------------------------------------------
// PE File Data Directory

typedef struct TP_PE_DATA_DIRECTORY_{
    uint32_t VirtualAddress;
    uint32_t Size;
}TP_PE_DATA_DIRECTORY;

// ----------------------------------------------------------------------------------------
// PE File Optional header

typedef struct TP_PE_OPTIONAL_HEADER64_{
    uint16_t Magic;
    uint8_t MajorLinkerVersion;
    uint8_t MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint64_t SizeOfStackReserve;
    uint64_t SizeOfStackCommit;
    uint64_t SizeOfHeapReserve;
    uint64_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    TP_PE_DATA_DIRECTORY DataDirectory[];
}TP_PE_OPTIONAL_HEADER64;

#define TP_IMAGE_OPTIONAL_HEADER64_MAGIC 0x20b // PE32+

#define TP_PE_SECTION_ALIGNMENT 0x1000
#define TP_PE_FILE_ALIGNMENT 0x200

#define TP_PE_PADDING_SECTION_ALIGNMENT(value) (rsize_t)(-((int64_t)value) & (TP_PE_SECTION_ALIGNMENT - 1))
#define TP_PE_PADDING_FILE_ALIGNMENT(value) (rsize_t)(-((int64_t)value) & (TP_PE_FILE_ALIGNMENT - 1))

// Subsystem

#define TP_IMAGE_SUBSYSTEM_WINDOWS_GUI 2
#define TP_IMAGE_SUBSYSTEM_WINDOWS_CUI 3
#define TP_IMAGE_SUBSYSTEM_EFI_APPLICATION 10
#define TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER 11
#define TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER 12

// DllCharacteristics

#define TP_IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA 0x0020
#define TP_IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040
#define TP_IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY 0x0080
#define TP_IMAGE_DLLCHARACTERISTICS_NX_COMPAT 0x0100
#define TP_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200
#define TP_IMAGE_DLLCHARACTERISTICS_NO_SEH 0x0400
#define TP_IMAGE_DLLCHARACTERISTICS_NO_BIND 0x0800
#define TP_IMAGE_DLLCHARACTERISTICS_APPCONTAINER 0x1000
#define TP_IMAGE_DLLCHARACTERISTICS_WDM_DRIVER 0x2000
#define TP_IMAGE_DLLCHARACTERISTICS_GUARD_CF 0x4000
#define TP_IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000

// Data Directory

#define TP_IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define TP_IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define TP_IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define TP_IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define TP_IMAGE_DIRECTORY_ENTRY_SECURITY 4
#define TP_IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define TP_IMAGE_DIRECTORY_ENTRY_DEBUG 6
#define TP_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 7
#define TP_IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8
#define TP_IMAGE_DIRECTORY_ENTRY_TLS 9
#define TP_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define TP_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
#define TP_IMAGE_DIRECTORY_ENTRY_IAT 12
#define TP_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13
#define TP_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14
#define TP_IMAGE_DIRECTORY_ENTRY_REVERSED 15

// ----------------------------------------------------------------------------------------
// PE File Directory(Based relocation)

typedef struct TP_PE_BASE_RELOCATION_{
    uint32_t VirtualAddress;
    uint32_t SizeOfBlock;
    uint16_t TypeOffset[];
}TP_PE_BASE_RELOCATION;

#define TP_IMAGE_REL_BASED_TYPE(value) (((value) & 0xF000) >> 12)
#define TP_IMAGE_REL_BASED_OFFSET(value) ((value) & 0x0FFF)

// Based relocation types

#define TP_IMAGE_REL_BASED_ABSOLUTE 0
#define TP_IMAGE_REL_BASED_HIGH 1
#define TP_IMAGE_REL_BASED_LOW 2
#define TP_IMAGE_REL_BASED_HIGHLOW 3
#define TP_IMAGE_REL_BASED_HIGHADJ 4
#define TP_IMAGE_REL_BASED_DIR64 10

// ----------------------------------------------------------------------------------------
// PE File header

typedef struct TP_PE_HEADER64_READ_{
    uint32_t Signature;
    TP_COFF_FILE_HEADER FileHeader;
    TP_PE_OPTIONAL_HEADER64 OptionalHeader;
}TP_PE_HEADER64_READ;

typedef struct TP_PE_HEADER64_WRITE_{
    TP_PE_DOS_HEADER DosHeader;
    uint32_t Signature;
    TP_COFF_FILE_HEADER FileHeader;
    TP_PE_OPTIONAL_HEADER64 OptionalHeader;
}TP_PE_HEADER64_WRITE;

#define TP_PE_HEADER64_SIGNATURE 0x00004550 // PE\0\0

#define TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM 16

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

// NumberOfRelocations

#define TP_NRELOC_OVFL_NUM 0xFFFF

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
#define TP_IMAGE_SYM_TYPE_LSB(symbol) (uint8_t)((symbol)->Type & 0xf);
#define TP_IMAGE_SYM_TYPE_MSB(symbol) (uint8_t)(((symbol)->Type & 0xf0) >> 4);

#define TP_IMAGE_SYM_UNDEFINED 0
#define TP_IMAGE_SYM_ABSOLUTE -1
#define TP_IMAGE_SYM_DEBUG -2
#define TP_IMAGE_SYM_SECTION_MAX 0xfeff // Type Representation
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

// ----------------------------------------------------------------------------------------
// Environment

// Section Data(Relocations)
typedef struct TP_COFF_RELOCATIONS_ARRAY_{
    TP_COFF_RELOCATIONS* member_relocations;
    rsize_t member_size;
    rsize_t member_num;
}TP_COFF_RELOCATIONS_ARRAY;

typedef struct TP_SYMBOL_TABLE_{
// config section:
    bool member_is_output_log_file;

// message section:
    FILE* member_disp_log_file;

// PE COFF section:
    // PE COFF OBJECT/IMAGE File
    uint8_t* member_pe_coff_buffer;
    rsize_t member_pe_coff_size;
    rsize_t member_pe_coff_current_offset;

    // PE File header
    TP_PE_DOS_HEADER* member_dos_header_read;  // NOTE: member_dos_header_read must not free memory.
    TP_PE_HEADER64_READ* member_pe_header64_read;  // NOTE: member_pe_header64_read must not free memory.

    // COFF File Header
    TP_COFF_FILE_HEADER* member_coff_file_header;  // NOTE: member_coff_file_header must not free memory.

    // Section Table
    TP_SECTION_TABLE* member_section_table;
    rsize_t member_section_table_size;
    rsize_t member_section_table_num;

    // Section Data(Relocations)
    TP_COFF_RELOCATIONS_ARRAY* member_coff_relocations;
    rsize_t member_coff_relocations_size;

    // COFF Symbol Table
    TP_COFF_SYMBOL_TABLE* member_coff_symbol_table;  // NOTE: member_coff_symbol_table must not free memory.

    // COFF String Table
    rsize_t member_string_table_offset;
    uint32_t member_string_table_size;
    uint8_t* member_string_table;  // NOTE: member_string_table must not free memory.
}TP_SYMBOL_TABLE;

#define TP_MESSAGE_BUFFER_SIZE 1024

#define TP_PUT_LOG_MSG_ILE(symbol_table) \
    fprintf((symbol_table)->member_disp_log_file, "Internal linker error(%s:%d).\n", __func__, __LINE__)
#define TP_PUT_LOG_MSG_TRACE(symbol_table) \
    fprintf((symbol_table)->member_disp_log_file, "TRACE: %s function\n", __func__)
#define TP_GET_LAST_ERROR(symbol_table) \
    fprintf((symbol_table)->member_disp_log_file, "GET_LAST_ERROR=%d(%s:%d)\n", GetLastError(), __func__, __LINE__); SetLastError(0)
#define TP_PRINT_CRT_ERROR(symbol_table) \
    fprintf((symbol_table)->member_disp_log_file, "CRT_ERROR=%d(%s:%d)\n", errno, __func__, __LINE__); (void)_set_errno(0)

#define IS_PE_IMAGE_FILE(symbol_table) \
    ((symbol_table)->member_dos_header_read && (TP_PE_DOS_HEADER_MAGIC == (symbol_table)->member_dos_header_read->e_magic))

#define IS_EOF_PE_COFF(symbol_table) \
    ((symbol_table)->member_pe_coff_size <= (symbol_table)->member_pe_coff_current_offset)

#define TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table) \
    ((symbol_table)->member_pe_coff_buffer + (symbol_table)->member_pe_coff_current_offset)

#define TP_COFF_OBJECT_DEFAULT_FNAME "efi_main"
#define TP_COFF_OBJECT_DEFAULT_EXT "obj"

#define TP_COFF_CODE_DEFAULT_FNAME "efi_main"
#define TP_COFF_CODE_DEFAULT_EXT "bin"

#define TP_PE_CODE_DEFAULT_FNAME "bootx64"
#define TP_PE_CODE_DEFAULT_EXT "bin"

#define TP_PE_UEFI_DEFAULT_FNAME "bootx64"
#define TP_PE_UEFI_DEFAULT_EXT "efi"

#define TP_PE_COFF_TEXT_DEFAULT_EXT "txt"

// ----------------------------------------------------------------------------------------
// Convert from COFF Object to PE Image. 
bool tp_make_PE_file_buffer(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* entry_point_symbol);

// ----------------------------------------------------------------------------------------
// PE File header
bool tp_make_PE_file_PE_HEADER64(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

// ----------------------------------------------------------------------------------------
// PE File Data Directory
bool tp_make_PE_file_PE_DATA_DIRECTORY(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_PE_OPTIONAL_HEADER64* optional_header
);
bool tp_make_PE_file_PE_BASE_RELOCATION(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* raw_data, uint32_t data_size
);

// ----------------------------------------------------------------------------------------
// Section Table
bool tp_make_PE_file_SECTION_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

// ----------------------------------------------------------------------------------------
// COFF Symbol Table
bool tp_make_PE_file_COFF_SYMBOL_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);
bool tp_make_PE_file_COFF_SYMBOL_TABLE_content(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables, rsize_t index
);

// ----------------------------------------------------------------------------------------
// COFF String Table
bool tp_make_PE_file_COFF_STRING_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

// ----------------------------------------------------------------------------------------
// Utilities
bool tp_make_PE_file_raw_data(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* raw_data, rsize_t size
);
bool tp_seek_PE_COFF_file(TP_SYMBOL_TABLE* symbol_table, long seek_position, long line_bytes);
bool tp_write_data(TP_SYMBOL_TABLE* symbol_table, uint8_t* data, rsize_t size, char* fname, char* ext);

#endif

