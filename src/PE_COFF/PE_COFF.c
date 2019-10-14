
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#include "PE_COFF.h"

// ----------------------------------------------------------------------------------------
// PE COFF OBJECT File
#define IS_EOF_PE_COFF() (pe_coff_size <= pe_coff_current_offset)

static uint8_t* pe_coff_buffer = NULL;
static rsize_t pe_coff_size = 0;
static rsize_t pe_coff_current_offset = 0;

// ----------------------------------------------------------------------------------------
// COFF File Header
// Section Table
// COFF Symbol Table
static TP_COFF_FILE_HEADER* coff_file_header = NULL;
static TP_SECTION_TABLE* section_table = NULL;
static TP_COFF_SYMBOL_TABLE* coff_symbol_table = NULL;

// ----------------------------------------------------------------------------------------
// COFF String Table
static rsize_t string_table_offset = 0;
static uint32_t string_table_size = 0;
static uint8_t* string_table = NULL;

static bool make_PE_COFF_dump_file_main(FILE* write_file);
static bool make_PE_COFF_dump_file_COFF_FILE_HEADER(FILE* write_file);
static bool make_PE_COFF_dump_file_COFF_STRING_TABLE(FILE* write_file);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE(FILE* write_file);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_content(
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables, rsize_t index
);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_AuxiliarySymbolRecords(
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables
);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_func_def(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_func_line(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_weak_extern(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_file(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_section(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_COFF_dump_file_SECTION_TABLE(FILE* write_file);
static bool make_PE_COFF_dump_file_SECTION_TABLE_section_data(FILE* write_file, TP_SECTION_TABLE* section);
static bool make_PE_COFF_dump_file_SECTION_TABLE_section_data_RawData(FILE* write_file, TP_SECTION_TABLE* section);
static bool make_PE_COFF_dump_file_SECTION_TABLE_section_data_Relocations(FILE* write_file, TP_SECTION_TABLE* section);
static bool make_PE_COFF_dump_file(char* read_path, char* write_path);
static bool seek_PE_COFF_file(long seek_position, long line_bytes);
static bool read_file(char* path);
static bool write_data(uint8_t* data, rsize_t size, char* fname, char* ext);
static bool make_path(char* path, size_t path_size, char* fname, char* ext);

int main()
{
    char read_path[_MAX_PATH];
    memset(read_path, 0, sizeof(read_path));

    if ( ! make_path(read_path, sizeof(read_path), "efi_main", "obj")){

        fprintf(stderr, "make_path() failed.\n");

        return EXIT_FAILURE;
    }

    char write_path[_MAX_PATH];
    memset(write_path, 0, sizeof(write_path));

    if ( ! make_path(write_path, sizeof(write_path), "PE_COFF", "txt")){

        fprintf(stderr, "make_path() failed.\n");

        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;

    if ( ! make_PE_COFF_dump_file(read_path, write_path)){

        fprintf(stderr, "make_PE_COFF_dump_file() failed.\n");

        status = EXIT_FAILURE;
    }

    if (pe_coff_buffer){

        SecureZeroMemory(pe_coff_buffer, pe_coff_size);
        free(pe_coff_buffer);
        pe_coff_buffer = NULL;
    }

    return status;
}

static bool make_PE_COFF_dump_file_main(FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF File Header
    if ( ! make_PE_COFF_dump_file_COFF_FILE_HEADER(write_file)){

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // COFF String Table
    if ( ! make_PE_COFF_dump_file_COFF_STRING_TABLE(write_file)){

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table
    if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE(write_file)){

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // Section Table
    if ( ! make_PE_COFF_dump_file_SECTION_TABLE(write_file)){

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // PE COFF OBJECT File

    if ((NULL == pe_coff_buffer) || (0 == pe_coff_size)){

        return false;
    }

    if ( ! write_data(pe_coff_buffer, pe_coff_size, "PE_COFF", "obj")){

        return false;
    }

    return true;
}

static bool make_PE_COFF_dump_file_COFF_FILE_HEADER(FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF File Header

    if (NULL == pe_coff_buffer){

        return false;
    }

    coff_file_header = (TP_COFF_FILE_HEADER*)pe_coff_buffer;

    fprintf(write_file,
        "TP_COFF_FILE_HEADER:\n"
        "    uint16_t Machine(%x);\n"
        "    uint16_t NumberOfSections(%x);\n"
        "    uint32_t TimeDateStamp(%x);\n"
        "    uint32_t PointerToSymbolTable(%x);\n"
        "    uint32_t NumberOfSymbols(%x);\n"
        "    uint16_t SizeOfOptionalHeader(%x);\n"
        "    uint16_t Characteristics(%x);\n\n",
        coff_file_header->Machine,
        coff_file_header->NumberOfSections,
        coff_file_header->TimeDateStamp,
        coff_file_header->PointerToSymbolTable,
        coff_file_header->NumberOfSymbols,
        coff_file_header->SizeOfOptionalHeader,
        coff_file_header->Characteristics
    );

    // Machine Types

    switch (coff_file_header->Machine){
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

    if (coff_file_header->Characteristics & TP_IMAGE_FILE_LARGE_ADDRESS_AWARE){

        fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_LARGE_ADDRESS_AWARE\n");
    }

    if (coff_file_header->Characteristics & TP_IMAGE_FILE_DEBUG_STRIPPED){

        fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_FILE_DEBUG_STRIPPED\n");
    }

    uint16_t characteristics_mask = ~(TP_IMAGE_FILE_LARGE_ADDRESS_AWARE | TP_IMAGE_FILE_DEBUG_STRIPPED);

    if (coff_file_header->Characteristics & characteristics_mask){

        fprintf(write_file, "uint16_t Characteristics : OTHER\n");
    }

    fprintf(write_file, "\n");

    coff_file_header->Characteristics = TP_IMAGE_FILE_DEBUG_STRIPPED;

    return true;
}


static bool make_PE_COFF_dump_file_COFF_STRING_TABLE(FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF String Table

    rsize_t offset = coff_file_header->NumberOfSymbols * sizeof(TP_COFF_SYMBOL_TABLE);

    if ( ! seek_PE_COFF_file((long)(coff_file_header->PointerToSymbolTable), (long)offset)){

        return false;
    }

    if (IS_EOF_PE_COFF()){

        return false;
    }

    memcpy(&string_table_size, (pe_coff_buffer + pe_coff_current_offset), sizeof(uint32_t));

    offset += sizeof(uint32_t);

    if ( ! seek_PE_COFF_file((long)(coff_file_header->PointerToSymbolTable), (long)offset)){

        return false;
    }

    if (IS_EOF_PE_COFF()){

        if (sizeof(uint32_t) == string_table_size){

            return true;
        }

        return false;
    }

    string_table = pe_coff_buffer + pe_coff_current_offset;

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE(FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table

    rsize_t num = coff_file_header->NumberOfSymbols;

    if ( !  seek_PE_COFF_file((long)(coff_file_header->PointerToSymbolTable), 0)){

        return false;
    }

    if (IS_EOF_PE_COFF()){

        return false;
    }

    coff_symbol_table = (TP_COFF_SYMBOL_TABLE*)(pe_coff_buffer + pe_coff_current_offset);

    for (rsize_t i = 0; num > i; ++i){

        if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_content(
            write_file, &(coff_symbol_table[i]), i)){

            return false;
        }

        i += coff_symbol_table[i].NumberOfAuxSymbols;
    }

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_content(
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables, rsize_t index)
{
    fprintf(write_file,
        "TP_COFF_SYMBOL_TABLE(%zx):\n"
        "    uint8_t ShortName(%.*s);\n"
        "    uint32_t Zeroes(%x);\n"
        "    uint32_t Offset(%x);\n"
        "    uint8_t LongName(%s);\n"
        "    uint32_t Value(%x);\n"
        "    int16_t SectionNumber(%x);\n"
        "    uint16_t Type(%x);\n"
        "    uint8_t StorageClass(%x);\n"
        "    uint8_t NumberOfAuxSymbols(%x);\n\n",
        index,
        TP_IMAGE_SIZEOF_SHORT_NAME,
        one_of_coff_symbol_tables->UNION.ShortName,
        one_of_coff_symbol_tables->UNION.Name.Zeroes, // if 0, use LongName
        one_of_coff_symbol_tables->UNION.Name.Offset, // An offset into the string table.
        (0 == one_of_coff_symbol_tables->UNION.Name.Zeroes) ?
            (string_table + one_of_coff_symbol_tables->UNION.Name.Offset) : "",
        one_of_coff_symbol_tables->Value,
        one_of_coff_symbol_tables->SectionNumber,
        one_of_coff_symbol_tables->Type,
        one_of_coff_symbol_tables->StorageClass,
        one_of_coff_symbol_tables->NumberOfAuxSymbols
    ); 

    // Section Number Values

    switch (one_of_coff_symbol_tables->SectionNumber){
    case TP_IMAGE_SYM_UNDEFINED:
        fprintf(write_file, "int16_t SectionNumber is TP_IMAGE_SYM_UNDEFINED.\n");
        break;
    case TP_IMAGE_SYM_ABSOLUTE:
        fprintf(write_file, "int16_t SectionNumber is TP_IMAGE_SYM_ABSOLUTE.\n");
        break;
    case TP_IMAGE_SYM_DEBUG:
        fprintf(write_file, "int16_t SectionNumber is TP_IMAGE_SYM_DEBUG.\n");
        break;
    default:
        break;
    }

    fprintf(write_file, "\n");

    // Type Representation

    uint8_t type_lsb = (uint8_t)(one_of_coff_symbol_tables->Type & 0xf);

    if (TP_IMAGE_SYM_TYPE_NULL == type_lsb){

        fprintf(write_file, "uint16_t Type_LSB is TP_IMAGE_SYM_TYPE_NULL.\n");
    }else{

        fprintf(write_file, "uint16_t Type_LSB is OTHER.\n");
    }

    uint8_t type_msb = (uint8_t)((one_of_coff_symbol_tables->Type & 0xf0) >> 8);

    switch (type_msb){
    case TP_IMAGE_SYM_DTYPE_NULL:
        fprintf(write_file, "uint16_t Type_MSB is TP_IMAGE_SYM_DTYPE_NULL.\n");
        break;
    case TP_IMAGE_SYM_DTYPE_FUNCTION:
        fprintf(write_file, "uint16_t Type_MSB is TP_IMAGE_SYM_DTYPE_FUNCTION.\n");
        break;
    default:
        fprintf(write_file, "uint16_t Type_MSB is OTHER.\n");
        break;
    }

    fprintf(write_file, "\n");

    // Storage Class

    switch (one_of_coff_symbol_tables->StorageClass){
    case TP_IMAGE_SYM_CLASS_END_OF_FUNCTION:
        fprintf(write_file, "uint8_t StorageClass is TP_IMAGE_SYM_CLASS_END_OF_FUNCTION.\n");
        break;
    case TP_IMAGE_SYM_CLASS_NULL:
        fprintf(write_file, "uint8_t StorageClass is TP_IMAGE_SYM_CLASS_NULL.\n");
        break;
    case TP_IMAGE_SYM_CLASS_EXTERNAL:
        fprintf(write_file, "uint8_t StorageClass is TP_IMAGE_SYM_CLASS_EXTERNAL.\n");
        break;
    case TP_IMAGE_SYM_CLASS_STATIC:
        fprintf(write_file, "uint8_t StorageClass is TP_IMAGE_SYM_CLASS_STATIC.\n");
        break;
    case TP_IMAGE_SYM_CLASS_FUNCTION:
        fprintf(write_file, "uint8_t StorageClass is TP_IMAGE_SYM_CLASS_FUNCTION.\n");
        break;
    case TP_IMAGE_SYM_CLASS_FILE:
        fprintf(write_file, "uint8_t StorageClass is TP_IMAGE_SYM_CLASS_FILE.\n");
        break;
    default:
        fprintf(write_file, "uint8_t StorageClass is OTHER.\n");
        break;
    }

    fprintf(write_file, "\n");

    if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_AuxiliarySymbolRecords(
        write_file, one_of_coff_symbol_tables)){

        return false;
    }

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_AuxiliarySymbolRecords(
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table(Auxiliary Symbol Records)

    uint32_t num = one_of_coff_symbol_tables->NumberOfAuxSymbols;

    if (0 == num){

        return true;
    }

    uint8_t type_msb = (uint8_t)((one_of_coff_symbol_tables->Type & 0xf0) >> 8);

    TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords = one_of_coff_symbol_tables->AuxiliarySymbolRecords;

    if ((TP_IMAGE_SYM_CLASS_EXTERNAL == one_of_coff_symbol_tables->StorageClass) &&
        (TP_IMAGE_SYM_UNDEFINED < one_of_coff_symbol_tables->SectionNumber) &&
        (TP_IMAGE_SYM_DTYPE_FUNCTION == type_msb)){

        // Auxiliary Format 1: Function Definitions
        if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_func_def(
            write_file, AuxiliarySymbolRecords, num)){

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_FUNCTION == one_of_coff_symbol_tables->StorageClass) &&
        ((0 == strncmp(".bf", one_of_coff_symbol_tables->UNION.ShortName, 5)) ||
        (0 == strncmp(".ef", one_of_coff_symbol_tables->UNION.ShortName, 5)))){

        // Auxiliary Format 2: .bf and .ef Symbols
        if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_func_line(
            write_file, AuxiliarySymbolRecords, num)){

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_EXTERNAL == one_of_coff_symbol_tables->StorageClass) &&
        (TP_IMAGE_SYM_UNDEFINED == one_of_coff_symbol_tables->SectionNumber) &&
        (0 == one_of_coff_symbol_tables->Value)){

        // Auxiliary Format 3: Weak Externals
        if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_weak_extern(
            write_file, AuxiliarySymbolRecords, num)){

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_FILE == one_of_coff_symbol_tables->StorageClass) &&
        (0 == strncmp(".file", one_of_coff_symbol_tables->UNION.ShortName, 5))){

        // Auxiliary Format 4: Files
        if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_file(
            write_file, AuxiliarySymbolRecords, num)){

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_STATIC == one_of_coff_symbol_tables->StorageClass)){

        // Auxiliary Format 5: Section Definitions
        if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_section(
            write_file, AuxiliarySymbolRecords, num)){

            return false;
        }
    }else{

        fprintf(write_file, "Unknown Auxiliary Format.\n");
    }

    fprintf(write_file, "\n");

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_func_def(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 1: Function Definitions
    TP_AUX_SYMBOL_RECORD_FUNC_DEF* func_def = (TP_AUX_SYMBOL_RECORD_FUNC_DEF*)AuxiliarySymbolRecords;

    for (uint32_t i = 0; num > i; ++i){

        fprintf(write_file,
            "TP_AUX_SYMBOL_RECORD_FUNC_DEF(%x):\n"
            "    uint32_t TagIndex(%x);\n"
            "    uint32_t TotalSize(%x);\n"
            "    uint32_t PointerToLinenumber(%x);\n"
            "    uint32_t PointerToNextFunction(%x);\n"
            "    uint8_t Unused(%.*s);\n\n",
            i,
            func_def[i].TagIndex,
            func_def[i].TotalSize,
            func_def[i].PointerToLinenumber,
            func_def[i].PointerToNextFunction,
            (int)sizeof(func_def[i].Unused),
            func_def[i].Unused
        );
    }

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_func_line(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 2: .bf and .ef Symbols
    TP_AUX_SYMBOL_RECORD_FUNC_LINE* func_line = (TP_AUX_SYMBOL_RECORD_FUNC_LINE*)AuxiliarySymbolRecords;

    for (uint32_t i = 0; num > i; ++i){

        fprintf(write_file,
            "TP_AUX_SYMBOL_RECORD_FUNC_LINE(%x):\n"
            "    uint8_t Unused1(%.*s);\n"
            "    uint16_t Linenumber(%x);\n"
            "    uint8_t Unused2(%.*s);\n"
            "    uint32_t PointerToNextFunction(%x);\n"
            "    uint8_t Unused3(%.*s);\n\n",
            i,
            (int)sizeof(func_line[i].Unused1),
            func_line[i].Unused1,
            func_line[i].Linenumber,
            (int)sizeof(func_line[i].Unused2),
            func_line[i].Unused2,
            func_line[i].PointerToNextFunction,
            (int)sizeof(func_line[i].Unused3),
            func_line[i].Unused3
        );
    }

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_weak_extern(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 3: Weak Externals
    TP_AUX_SYMBOL_RECORD_WEAK_EXTERN* weak_extern = (TP_AUX_SYMBOL_RECORD_WEAK_EXTERN*)AuxiliarySymbolRecords;

    for (uint32_t i = 0; num > i; ++i){

        fprintf(write_file,
            "TP_AUX_SYMBOL_RECORD_WEAK_EXTERN(%x):\n"
            "    uint32_t WeakDefaultSymIndex(%x);\n"
            "    uint32_t Characteristics(%x);\n"
            "    uint8_t Unused(%.*s);\n\n",
            i,
           weak_extern[i].WeakDefaultSymIndex,
           weak_extern[i].Characteristics,
           (int)sizeof(weak_extern[i].Unused),
           weak_extern[i].Unused
       ); 

        switch (weak_extern[i].Characteristics){
        case TP_IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY:
            fprintf(write_file, "    uint32_t Characteristics is TP_IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY.\n"); 
            break;
        case TP_IMAGE_WEAK_EXTERN_SEARCH_LIBRARY:
            fprintf(write_file, "    uint32_t Characteristics is TP_IMAGE_WEAK_EXTERN_SEARCH_LIBRARY.\n"); 
            break;
        case TP_IMAGE_WEAK_EXTERN_SEARCH_ALIAS:
            fprintf(write_file, "    uint32_t Characteristics is TP_IMAGE_WEAK_EXTERN_SEARCH_ALIAS.\n"); 
            break;
        case TP_IMAGE_WEAK_EXTERN_ANTI_DEPENDENCY:
            fprintf(write_file, "    uint32_t Characteristics is TP_IMAGE_WEAK_EXTERN_ANTI_DEPENDENCY.\n"); 
            break;
        default:
            fprintf(write_file, "    uint32_t Characteristics is unknown value.\n"); 
            break;
        }

        fprintf(write_file, "\n");
    }

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_file(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 4: Files
    TP_AUX_SYMBOL_RECORD_FILE* file = (TP_AUX_SYMBOL_RECORD_FILE*)AuxiliarySymbolRecords;

    fprintf(write_file, "TP_AUX_SYMBOL_RECORD_FILE: uint8_t FileName("); 

    for (uint32_t i = 0; num > i; ++i){

        fprintf(write_file, "%.*s", TP_IMAGE_SIZEOF_SYMBOL, file[i].FileName); 
    }

    fprintf(write_file, ");\n\n"); 

    return true;
}

static bool make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_section(
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 5: Section Definitions
    TP_AUX_SYMBOL_RECORD_SECTION* section = (TP_AUX_SYMBOL_RECORD_SECTION*)AuxiliarySymbolRecords;

    for (uint32_t i = 0; num > i; ++i){

        fprintf(write_file,
            "TP_AUX_SYMBOL_RECORD_SECTION(%x):\n"
            "    uint32_t Length(%x);\n"
            "    uint16_t NumberOfRelocations(%x);\n"
            "    uint16_t NumberOfLinenumbers(%x);\n"
            "    uint32_t CheckSum(%x);\n"
            "    int16_t Number(%x);\n"
            "    uint8_t Selection(%x);\n"
            "    uint8_t Unused(%.*s);\n\n",
            i,
            section[i].Length,
            section[i].NumberOfRelocations,
            section[i].NumberOfLinenumbers,
            section[i].CheckSum,
            section[i].Number,
            section[i].Selection,
            (int)sizeof(section[i].Unused),
            section[i].Unused
        ); 

        switch (section[i].Selection){
        case 0:
            break;
        case TP_IMAGE_COMDAT_SELECT_NODUPLICATES:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_NODUPLICATES.\n"); 
            break;
        case TP_IMAGE_COMDAT_SELECT_ANY:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_ANY.\n"); 
            break;
        case TP_IMAGE_COMDAT_SELECT_SAME_SIZE:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_SAME_SIZE.\n"); 
            break;
        case TP_IMAGE_COMDAT_SELECT_EXACT_MATCH:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_EXACT_MATCH.\n"); 
            break;
        case TP_IMAGE_COMDAT_SELECT_ASSOCIATIVE:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_ASSOCIATIVE.\n"); 
            break;
        case TP_IMAGE_COMDAT_SELECT_LARGEST:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_LARGEST.\n"); 
            break;
        case TP_IMAGE_COMDAT_SELECT_NEWEST:
            fprintf(write_file, "    uint8_t Selection is TP_IMAGE_COMDAT_SELECT_NEWEST.\n"); 
            break;
        default:
            fprintf(write_file, "    uint8_t Selection is unknown value.\n"); 
            break;
        }

        fprintf(write_file, "\n");
    }

    return true;
}


static bool make_PE_COFF_dump_file_SECTION_TABLE(FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // Section Table

    if (NULL == coff_file_header){

        return false;
    }

    if ( !  seek_PE_COFF_file(0, (long)sizeof(TP_COFF_FILE_HEADER))){

        return false;
    }

    rsize_t num = coff_file_header->NumberOfSections;

    section_table = (TP_SECTION_TABLE*)calloc(num, sizeof(TP_SECTION_TABLE));

    if (NULL == section_table){

        return false;
    }

    if (IS_EOF_PE_COFF()){

        return false;
    }

    section_table = (TP_SECTION_TABLE*)(pe_coff_buffer + pe_coff_current_offset);

    for (rsize_t i = 0; num > i; ++i){

        uint8_t* Name = section_table[i].Name;

        if ( ! ((0 == strncmp(Name, ".text", 5)) ||
//          (0 == strncmp(Name, ".tls", 4)) ||
//          (0 == strncmp(Name, ".drectve", 8)) ||
            (0 == strncmp(Name, ".bss", 4)) ||
            (0 == strncmp(Name, ".data", 5)) ||
            (0 == strncmp(Name, ".rdata", 6)))){

            sprintf_s(Name, TP_IMAGE_SIZEOF_SHORT_NAME, "%c", '0' + (int)i);

            section_table[i].SizeOfRawData = 0;
            section_table[i].PointerToRawData = 0;
            section_table[i].PointerToRelocations = 0;
            section_table[i].PointerToLinenumbers = 0;
            section_table[i].NumberOfRelocations = 0;
            section_table[i].NumberOfLinenumbers = 0;

            section_table[i].Characteristics = TP_IMAGE_SCN_LNK_REMOVE;

            continue;
        }

         uint32_t Characteristics = section_table[i].Characteristics;

        fprintf(write_file,
            "TP_SECTION_TABLE(%zu):\n"
            "    uint8_t Name(%.*s);\n"
            "    uint32_t VirtualSize(%x);\n"
            "    uint32_t VirtualAddress(%x);\n"
            "    uint32_t SizeOfRawData(%x);\n"
            "    uint32_t PointerToRawData(%x);\n"
            "    uint32_t PointerToRelocations(%x);\n"
            "    uint32_t PointerToLinenumbers(%x);\n"
            "    uint16_t NumberOfRelocations(%x);\n"
            "    uint16_t NumberOfLinenumbers(%x);\n"
            "    uint32_t Characteristics(%x);\n\n",
            i + 1,
            TP_IMAGE_SIZEOF_SHORT_NAME,
            Name,
            section_table[i].VirtualSize,
            section_table[i].VirtualAddress,
            section_table[i].SizeOfRawData,
            section_table[i].PointerToRawData,
            section_table[i].PointerToRelocations,
            section_table[i].PointerToLinenumbers,
            section_table[i].NumberOfRelocations,
            section_table[i].NumberOfLinenumbers,
            Characteristics
        ); 

        // Characteristics(Section Flags)

        if (Characteristics & TP_IMAGE_SCN_CNT_CODE){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_CNT_CODE\n");
        }

        if (Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA\n");
        }

        if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA\n");
        }

        if (Characteristics & TP_IMAGE_SCN_LNK_INFO){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_LNK_INFO\n");
        }

        if (Characteristics & TP_IMAGE_SCN_LNK_REMOVE){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_LNK_REMOVE\n");
        }

        if (Characteristics & TP_IMAGE_SCN_LNK_COMDAT){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_LNK_COMDAT\n");
        }

        if (Characteristics & TP_IMAGE_SCN_GPREL){

            fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_GPREL\n");
        }

        switch (Characteristics & TP_IMAGE_SCN_ALIGN_MASK){
        case TP_IMAGE_SCN_ALIGN_1BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_1BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_2BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_2BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_4BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_4BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_8BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_8BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_16BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_16BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_32BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_32BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_64BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_64BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_128BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_128BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_256BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_256BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_512BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_512BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_1024BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_1024BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_2048BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_2048BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_4096BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_4096BYTES.\n");
            break;
        case TP_IMAGE_SCN_ALIGN_8192BYTES:
            fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_8192BYTES.\n");
            break;
        default:
            break;
        }

        if (Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_DISCARDABLE){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_DISCARDABLE\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_NOT_CACHED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_NOT_CACHED\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_NOT_PAGED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_NOT_PAGED\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_SHARED){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_SHARED\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_EXECUTE){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_EXECUTE\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_READ){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_READ\n");
        }

        if (Characteristics & TP_IMAGE_SCN_MEM_WRITE){

            fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_WRITE\n");
        }

        fprintf(write_file, "\n");

        if ( ! make_PE_COFF_dump_file_SECTION_TABLE_section_data(write_file, &(section_table[i]))){

            return false;
        }
    }

    return true;
}

static bool make_PE_COFF_dump_file_SECTION_TABLE_section_data(FILE* write_file, TP_SECTION_TABLE* section)
{
    // Section Data(RawData)
    if ( ! make_PE_COFF_dump_file_SECTION_TABLE_section_data_RawData(write_file, section)){

        return false;
    }

    // Section Data(Relocations)
    if ( ! make_PE_COFF_dump_file_SECTION_TABLE_section_data_Relocations(write_file, section)){

        return false;
    }

    return true;
}

static bool make_PE_COFF_dump_file_SECTION_TABLE_section_data_RawData(FILE* write_file, TP_SECTION_TABLE* section)
{
    // ----------------------------------------------------------------------------------------
    // Section Data(RawData)

    uint32_t PointerToRawData = section->PointerToRawData;

    if (0 == PointerToRawData){

        return true;
    }

    if ( !  seek_PE_COFF_file((long)PointerToRawData, 0)){

        return false;
    }

    if (IS_EOF_PE_COFF()){

        return false;
    }

    uint8_t* raw_data = pe_coff_buffer + pe_coff_current_offset;

    rsize_t size = section->SizeOfRawData;

    if (section->Characteristics & TP_IMAGE_SCN_CNT_CODE){

        if ( ! write_data(raw_data, size, "PE_COFF", "bin")){

            return false;
        }
    }

    fprintf(write_file, "section_data_RawData:\n    "); 

    for (rsize_t i = 0, count = 0; size > i; ++i){

        fprintf(write_file, "%02x ", raw_data[i]); 

        if (15 == count){

            count = 0;

            fprintf(write_file, "\n    "); 
        }else{

            ++count;
        }
    }

    fprintf(write_file, "\n    "); 

    for (rsize_t i = 0, count = 0; size > i; ++i){

        fprintf(write_file, "%c ", raw_data[i]); 

        if (15 == count){

            count = 0;

            fprintf(write_file, "\n    "); 
        }else{

            ++count;
        }
    }

    fprintf(write_file, "\n\n"); 

    return true;
}

static bool make_PE_COFF_dump_file_SECTION_TABLE_section_data_Relocations(FILE* write_file, TP_SECTION_TABLE* section)
{
    // ----------------------------------------------------------------------------------------
    // Section Data(Relocations)

    if ( !  seek_PE_COFF_file((long)(section->PointerToRelocations), 0)){

        return false;
    }

    if (IS_EOF_PE_COFF()){

        return false;
    }

    rsize_t num = section->NumberOfRelocations;

    rsize_t size = num * sizeof(TP_COFF_RELOCATIONS);

    TP_COFF_RELOCATIONS* relocations = (TP_COFF_RELOCATIONS*)calloc(num, sizeof(TP_COFF_RELOCATIONS));

    if (NULL == relocations){

        return false;
    }

    memcpy(relocations, (pe_coff_buffer + pe_coff_current_offset), size);

    for (rsize_t i = 0; num > i; ++i){

        fprintf(write_file,
            "TP_COFF_RELOCATIONS(%zu):\n"
            "    uint32_t VirtualAddress(%x);\n"
            "    uint32_t RelocCount(%x);\n"
            "    uint32_t SymbolTableIndex(%x);\n"
            "    uint16_t Type(%x);\n\n",
            i,
            relocations[i].UNION.VirtualAddress,
            relocations[i].UNION.RelocCount, // When IMAGE_SCN_LNK_NRELOC_OVFL is set.
            relocations[i].SymbolTableIndex,
            relocations[i].Type
        ); 

        // Type Indicators

        switch (relocations[i].Type){
        case TP_IMAGE_REL_AMD64_ABSOLUTE:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ABSOLUTE.\n");
            break;
        case TP_IMAGE_REL_AMD64_ADDR64:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ADDR64.\n");
            break;
        case TP_IMAGE_REL_AMD64_ADDR32:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ADDR32.\n");
            break;
        case TP_IMAGE_REL_AMD64_ADDR32NB:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ADDR32NB.\n");
            break;
        case TP_IMAGE_REL_AMD64_REL32:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32.\n");
            break;
        case TP_IMAGE_REL_AMD64_REL32_1:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_1.\n");
            break;
        case TP_IMAGE_REL_AMD64_REL32_2:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_2.\n");
            break;
        case TP_IMAGE_REL_AMD64_REL32_3:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_3.\n");
            break;
        case TP_IMAGE_REL_AMD64_REL32_4:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_4.\n");
            break;
        case TP_IMAGE_REL_AMD64_REL32_5:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_5.\n");
            break;
        case TP_IMAGE_REL_AMD64_SECTION:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SECTION.\n");
            break;
        case TP_IMAGE_REL_AMD64_SECREL:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SECREL.\n");
            break;
        case TP_IMAGE_REL_AMD64_SECREL7:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SECREL7.\n");
            break;
        case TP_IMAGE_REL_AMD64_TOKEN:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_TOKEN.\n");
            break;
        case TP_IMAGE_REL_AMD64_SREL32:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SREL32.\n");
            break;
        case TP_IMAGE_REL_AMD64_PAIR:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_PAIR.\n");
            break;
        case TP_IMAGE_REL_AMD64_SSPAN32:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SSPAN32.\n");
            break;
        case TP_IMAGE_REL_AMD64_EHANDLER:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_EHANDLER.\n");
            break;
        case TP_IMAGE_REL_AMD64_IMPORT_BR:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_IMPORT_BR.\n");
            break;
        case TP_IMAGE_REL_AMD64_IMPORT_CALL:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_IMPORT_CALL.\n");
            break;
        case TP_IMAGE_REL_AMD64_CFG_BR:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_CFG_BR.\n");
            break;
        case TP_IMAGE_REL_AMD64_CFG_BR_REX:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_CFG_BR_REX.\n");
            break;
        case TP_IMAGE_REL_AMD64_CFG_CALL:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_CFG_CALL.\n");
            break;
        case TP_IMAGE_REL_AMD64_INDIR_BR:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR.\n");
            break;
        case TP_IMAGE_REL_AMD64_INDIR_BR_REX:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR_REX.\n");
            break;
        case TP_IMAGE_REL_AMD64_INDIR_CALL:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_CALL.\n");
            break;
        case TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_FIRST:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_FIRST.\n");
            break;
        case TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_LAST:
            fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_LAST.\n");
            break;
        default:
            fprintf(write_file, "uint16_t Type is OTHER.\n");
            break;
        }

        fprintf(write_file, "\n");

        if (TP_IMAGE_REL_AMD64_REL32 == relocations[i].Type){

            rsize_t symbols = coff_file_header->NumberOfSymbols;

            for (rsize_t j = 0; symbols > j; ++j){

                if (relocations[i].SymbolTableIndex == j){

                    if ( ! make_PE_COFF_dump_file_COFF_SYMBOL_TABLE_content(
                        write_file, &(coff_symbol_table[j]), j)){

                        SecureZeroMemory(relocations, size);
                        free(relocations);
                        relocations = NULL;

                        return false;
                    }

                    break;
                }

                j += coff_symbol_table[j].NumberOfAuxSymbols;
            }
        }
    }

    SecureZeroMemory(relocations, size);
    free(relocations);
    relocations = NULL;

    return true;
}

static bool make_PE_COFF_dump_file(char* read_path, char* write_path)
{
    if ( ! read_file(read_path)){

        return false;
    }

    FILE* write_file = NULL;

    errno_t err = fopen_s(&write_file, write_path, "w");

    if (NULL == write_file){

        return false;
    }

    bool status = make_PE_COFF_dump_file_main(write_file);

    int fclose_error = fclose(write_file);

    if (EOF == fclose_error){

        clearerr(write_file);

        return false;
    }

    write_file = NULL;

    return status;
}

static bool seek_PE_COFF_file(long seek_position, long line_bytes)
{
    rsize_t offset = (rsize_t)(seek_position) + (rsize_t)(line_bytes);

    pe_coff_current_offset = offset;

    return true;
}

static bool read_file(char* path)
{
    int fd = 0;

    errno_t _sopen_s_error = _sopen_s(&fd, path, _O_RDONLY | _O_BINARY, _SH_DENYWR, 0);

    if (_sopen_s_error){

        return false;
    }
  
    FILE* read_file = _fdopen(fd, "rb");

    if (NULL == read_file){

        return false;
    }

    struct stat stbuf = { 0 };

    int fstat_error = fstat(fd, &stbuf);

    if (-1 == fstat_error){

        clearerr(read_file);
        (void)fclose(read_file);
        read_file = NULL;

        return false;
    }

    bool is_eof = false;

    size_t read_count = stbuf.st_size;

    uint8_t* read_buffer = (uint8_t*)calloc(read_count, sizeof(uint8_t));

    if (NULL == read_buffer){

        (void)fclose(read_file);
        read_file = NULL;

        return false;
    }

    size_t fread_bytes = fread(
        read_buffer, sizeof(uint8_t), read_count, read_file
    );

    if (read_count > fread_bytes){

        int ferror_error = ferror(read_file);

        if (ferror_error){

            clearerr(read_file);
            (void)fclose(read_file);
            read_file = NULL;

            return false;
        }

        int feof_error = feof(read_file);

        if (feof_error){

            is_eof = true;
        }

        (void)fclose(read_file);
        read_file = NULL;

        return false;
    }

    int fclose_error = fclose(read_file);

    if (EOF == fclose_error){

        clearerr(read_file);
        read_file = NULL;

        return false;
    }

    read_file = NULL;

    pe_coff_buffer = read_buffer;

    pe_coff_size = fread_bytes;

    return true;
}

static bool write_data(uint8_t* data, rsize_t size, char* fname, char* ext)
{
    char write_path[_MAX_PATH];
    memset(write_path, 0, sizeof(write_path));

    if ( ! make_path(write_path, sizeof(write_path), fname, ext)){

        return false;
    }

    FILE* write_code = NULL;

    errno_t err = fopen_s(&write_code, write_path, "wb");

    if (NULL == write_code){

        return false;
    }

    size_t fwrite_bytes = fwrite(data, sizeof(uint8_t), size, write_code);

    if (size > fwrite_bytes){

        int ferror_error = ferror(write_code);

        if (ferror_error){

            clearerr(write_code);
        }

        (void)fclose(write_code);

        return false;
    }

    int fclose_error = fclose(write_code);

    if (EOF == fclose_error){

        return false;
    }

    write_code = NULL;

    return true;
}

static bool make_path(char* path, size_t path_size, char* fname, char* ext)
{
    errno_t err = 0;

    char base_dir[_MAX_PATH];
    memset(base_dir, 0, sizeof(base_dir));
    char drive[_MAX_DRIVE];
    memset(drive, 0, sizeof(drive));
    char dir[_MAX_DIR];
    memset(dir, 0, sizeof(dir));

    HMODULE handle = GetModuleHandleA(NULL);

    if (0 == handle){

        goto error_out;
    }

    DWORD status = GetModuleFileNameA(handle, base_dir, sizeof(base_dir));

    if (0 == status){

        goto error_out;
    }

    err = _splitpath_s(base_dir, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (err){

        goto error_out;
    }

    err = _makepath_s(path, path_size, drive, dir, fname, ext);

    if (err){

        goto error_out;
    }

    return true;

error_out:

    return false;
}

