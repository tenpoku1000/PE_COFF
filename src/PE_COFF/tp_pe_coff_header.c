
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#include "PE_COFF.h"

static bool make_PE_file_PE_OPTIONAL_HEADER64(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);
static bool make_PE_file_COFF_FILE_HEADER(TP_SYMBOL_TABLE* symbol_table, FILE* write_file);

bool tp_make_PE_file_PE_HEADER64(TP_SYMBOL_TABLE* symbol_table, FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // PE File header

    if (NULL == symbol_table->member_pe_coff_buffer){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // PE File DOS Header
    symbol_table->member_dos_header_read = (TP_PE_DOS_HEADER*)(symbol_table->member_pe_coff_buffer);

    if ( ! IS_PE_IMAGE_FILE(symbol_table)){

        // COFF File Header
        if ( ! make_PE_file_COFF_FILE_HEADER(symbol_table, write_file)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    // File address of PE File header.
    symbol_table->member_pe_header64_read
        = (TP_PE_HEADER64_READ*)(symbol_table->member_pe_coff_buffer + symbol_table->member_dos_header_read->e_lfanew);

    // PE\0\0
    if (TP_PE_HEADER64_SIGNATURE != symbol_table->member_pe_header64_read->Signature){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // COFF File Header
    if ( ! make_PE_file_COFF_FILE_HEADER(symbol_table, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // PE File Optional header
    if ( ! make_PE_file_PE_OPTIONAL_HEADER64(symbol_table, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool make_PE_file_PE_OPTIONAL_HEADER64(TP_SYMBOL_TABLE* symbol_table, FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // PE File Optional header
    bool is_print = symbol_table->member_is_output_log_file;

    TP_PE_OPTIONAL_HEADER64* optional_header = &(symbol_table->member_pe_header64_read->OptionalHeader);

     // PE32+
    if (TP_IMAGE_OPTIONAL_HEADER64_MAGIC != optional_header->Magic){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (is_print){

        fprintf(write_file,
            "TP_PE_OPTIONAL_HEADER64:\n"
            "    uint16_t Magic(%x);\n"
            "    uint8_t MajorLinkerVersion(%x);\n"
            "    uint8_t MinorLinkerVersion(%x);\n"
            "    uint32_t SizeOfCode(%x);\n"
            "    uint32_t SizeOfInitializedData(%x);\n"
            "    uint32_t SizeOfUninitializedData(%x);\n"
            "    uint32_t AddressOfEntryPoint(%x);\n"
            "    uint32_t BaseOfCode(%x);\n"
            "    uint64_t ImageBase(%zx);\n"
            "    uint32_t SectionAlignment(%x);\n"
            "    uint32_t FileAlignment(%x);\n"
            "    uint16_t MajorOperatingSystemVersion(%x);\n"
            "    uint16_t MinorOperatingSystemVersion(%x);\n"
            "    uint16_t MajorImageVersion(%x);\n"
            "    uint16_t MinorImageVersion(%x);\n"
            "    uint16_t MajorSubsystemVersion(%x);\n"
            "    uint16_t MinorSubsystemVersion(%x);\n"
            "    uint32_t Win32VersionValue(%x);\n"
            "    uint32_t SizeOfImage(%x);\n"
            "    uint32_t SizeOfHeaders(%x);\n"
            "    uint32_t CheckSum(%x);\n"
            "    uint16_t Subsystem(%x);\n"
            "    uint16_t DllCharacteristics(%x);\n"
            "    uint64_t SizeOfStackReserve(%zx);\n"
            "    uint64_t SizeOfStackCommit(%zx);\n"
            "    uint64_t SizeOfHeapReserve(%zx);\n"
            "    uint64_t SizeOfHeapCommit(%zx);\n"
            "    uint32_t LoaderFlags(%x);\n"
            "    uint32_t NumberOfRvaAndSizes(%x);\n\n",
            optional_header->Magic,
            optional_header->MajorLinkerVersion,
            optional_header->MinorLinkerVersion,
            optional_header->SizeOfCode,
            optional_header->SizeOfInitializedData,
            optional_header->SizeOfUninitializedData,
            optional_header->AddressOfEntryPoint,
            optional_header->BaseOfCode,
            optional_header->ImageBase,
            optional_header->SectionAlignment,
            optional_header->FileAlignment,
            optional_header->MajorOperatingSystemVersion,
            optional_header->MinorOperatingSystemVersion,
            optional_header->MajorImageVersion,
            optional_header->MinorImageVersion,
            optional_header->MajorSubsystemVersion,
            optional_header->MinorSubsystemVersion,
            optional_header->Win32VersionValue,
            optional_header->SizeOfImage,
            optional_header->SizeOfHeaders,
            optional_header->CheckSum,
            optional_header->Subsystem,
            optional_header->DllCharacteristics,
            optional_header->SizeOfStackReserve,
            optional_header->SizeOfStackCommit,
            optional_header->SizeOfHeapReserve,
            optional_header->SizeOfHeapCommit,
            optional_header->LoaderFlags,
            optional_header->NumberOfRvaAndSizes
        );

        // Subsystem

        switch (optional_header->Subsystem){
        case TP_IMAGE_SUBSYSTEM_WINDOWS_GUI:
            fprintf(write_file, "uint16_t Subsystem : TP_IMAGE_SUBSYSTEM_WINDOWS_GUI\n");
            break;
        case TP_IMAGE_SUBSYSTEM_WINDOWS_CUI:
            fprintf(write_file, "uint16_t Subsystem : TP_IMAGE_SUBSYSTEM_WINDOWS_CUI\n");
            break;
        case TP_IMAGE_SUBSYSTEM_EFI_APPLICATION:
            fprintf(write_file, "uint16_t Subsystem : TP_IMAGE_SUBSYSTEM_EFI_APPLICATION\n");
            break;
        case TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
            fprintf(write_file, "uint16_t Subsystem : TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER\n");
            break;
        case TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
            fprintf(write_file, "uint16_t Subsystem : TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER\n");
            break;
        default:
            fprintf(write_file, "uint16_t Subsystem : OTHER\n");
            break;
        }

        fprintf(write_file, "\n");

        // DllCharacteristics

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NX_COMPAT){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NX_COMPAT\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NO_ISOLATION\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NO_SEH){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NO_SEH\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NO_BIND){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_NO_BIND\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_APPCONTAINER){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_APPCONTAINER\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_WDM_DRIVER){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_WDM_DRIVER\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_GUARD_CF){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_GUARD_CF\n");
        }

        if (optional_header->DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE){

            fprintf(write_file, "uint16_t DllCharacteristics & TP_IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE\n");
        }

        fprintf(write_file, "\n");
    }

    return true;
}

static bool make_PE_file_COFF_FILE_HEADER(TP_SYMBOL_TABLE* symbol_table, FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF File Header
    bool is_print = symbol_table->member_is_output_log_file;

    if (NULL == symbol_table->member_pe_coff_buffer){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (IS_PE_IMAGE_FILE(symbol_table)){

        symbol_table->member_coff_file_header = &(symbol_table->member_pe_header64_read->FileHeader);

        TP_PE_OPTIONAL_HEADER64* optional_header = &(symbol_table->member_pe_header64_read->OptionalHeader);

        uint16_t size_of_optional_header
            = (uint16_t)(sizeof(TP_PE_OPTIONAL_HEADER64) + (optional_header->NumberOfRvaAndSizes * sizeof(TP_PE_DATA_DIRECTORY)));

        if (symbol_table->member_coff_file_header->SizeOfOptionalHeader != size_of_optional_header){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }
    }else{

        symbol_table->member_coff_file_header = (TP_COFF_FILE_HEADER*)(symbol_table->member_pe_coff_buffer);
    }

    if (is_print){

        fprintf(write_file,
            "TP_COFF_FILE_HEADER:\n"
            "    uint16_t Machine(%x);\n"
            "    uint16_t NumberOfSections(%x);\n"
            "    uint32_t TimeDateStamp(%x);\n"
            "    uint32_t PointerToSymbolTable(%x);\n"
            "    uint32_t NumberOfSymbols(%x);\n"
            "    uint16_t SizeOfOptionalHeader(%x);\n"
            "    uint16_t Characteristics(%x);\n\n",
            symbol_table->member_coff_file_header->Machine,
            symbol_table->member_coff_file_header->NumberOfSections,
            symbol_table->member_coff_file_header->TimeDateStamp,
            symbol_table->member_coff_file_header->PointerToSymbolTable,
            symbol_table->member_coff_file_header->NumberOfSymbols,
            symbol_table->member_coff_file_header->SizeOfOptionalHeader,
            symbol_table->member_coff_file_header->Characteristics
        );

        // Machine Types

        switch (symbol_table->member_coff_file_header->Machine){
        case TP_IMAGE_FILE_MACHINE_UNKNOWN:
            fprintf(write_file, "uint16_t Machine : TP_IMAGE_FILE_MACHINE_UNKNOWN\n");
            break;
        case TP_IMAGE_FILE_MACHINE_AMD64:
            fprintf(write_file, "uint16_t Machine : TP_IMAGE_FILE_MACHINE_AMD64\n");
            break;
        default:
            fprintf(write_file, "uint16_t Machine : OTHER\n");
            break;
        }

        fprintf(write_file, "\n");

        // Characteristics

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_RELOCS_STRIPPED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_RELOCS_STRIPPED\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_EXECUTABLE_IMAGE){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_EXECUTABLE_IMAGE\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_LINE_NUMS_STRIPPED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_LINE_NUMS_STRIPPED\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_LOCAL_SYMS_STRIPPED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_LOCAL_SYMS_STRIPPED\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_AGGRESIVE_WS_TRIM){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_AGGRESIVE_WS_TRIM\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_LARGE_ADDRESS_AWARE){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_LARGE_ADDRESS_AWARE\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_BYTES_REVERSED_LO){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_BYTES_REVERSED_LO\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_32BIT_MACHINE){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_32BIT_MACHINE\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_DEBUG_STRIPPED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_DEBUG_STRIPPED\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_NET_RUN_FROM_SWAP){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_NET_RUN_FROM_SWAP\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_SYSTEM){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_SYSTEM\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_DLL){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_DLL\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_UP_SYSTEM_ONLY){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_UP_SYSTEM_ONLY\n");
        }

        if (symbol_table->member_coff_file_header->Characteristics & TP_IMAGE_FILE_BYTES_REVERSED_HI){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_BYTES_REVERSED_HI\n");
        }

        fprintf(write_file, "\n");
    }

    return true;
}

