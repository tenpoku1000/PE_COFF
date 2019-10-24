
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#include "PE_COFF.h"

static bool make_PE_file_COFF_SYMBOL_TABLE_AuxiliarySymbolRecords(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables
);
static bool make_PE_file_COFF_SYMBOL_TABLE_func_def(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_file_COFF_SYMBOL_TABLE_func_line(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_file_COFF_SYMBOL_TABLE_weak_extern(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_file_COFF_SYMBOL_TABLE_file(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);
static bool make_PE_file_COFF_SYMBOL_TABLE_section(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num
);

bool tp_make_PE_file_COFF_SYMBOL_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table

    rsize_t num = symbol_table->member_coff_file_header->NumberOfSymbols;

    if ( ! tp_seek_PE_COFF_file(symbol_table, (long)(symbol_table->member_coff_file_header->PointerToSymbolTable), 0)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    symbol_table->member_coff_symbol_table = (TP_COFF_SYMBOL_TABLE*)TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);

    for (rsize_t i = 0; num > i; ++i){

        if ( ! tp_make_PE_file_COFF_SYMBOL_TABLE_content(
            symbol_table, write_file, &(symbol_table->member_coff_symbol_table[i]), i)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        i += symbol_table->member_coff_symbol_table[i].NumberOfAuxSymbols;
    }

    return true;
}

bool tp_make_PE_file_COFF_SYMBOL_TABLE_content(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables, rsize_t index)
{
    bool is_print = symbol_table->member_is_output_log_file;

    if (is_print){

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
                (symbol_table->member_string_table + one_of_coff_symbol_tables->UNION.Name.Offset) : "",
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

        uint8_t type_lsb = TP_IMAGE_SYM_TYPE_LSB(one_of_coff_symbol_tables);

        if (TP_IMAGE_SYM_TYPE_NULL == type_lsb){

            fprintf(write_file, "uint16_t Type_LSB is TP_IMAGE_SYM_TYPE_NULL.\n");
        }else{

            fprintf(write_file, "uint16_t Type_LSB is OTHER.\n");
        }

        uint8_t type_msb = TP_IMAGE_SYM_TYPE_MSB(one_of_coff_symbol_tables);

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
    }

    if ( ! make_PE_file_COFF_SYMBOL_TABLE_AuxiliarySymbolRecords(
        symbol_table, write_file, one_of_coff_symbol_tables)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool make_PE_file_COFF_SYMBOL_TABLE_AuxiliarySymbolRecords(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_COFF_SYMBOL_TABLE* one_of_coff_symbol_tables)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table(Auxiliary Symbol Records)
    bool is_print = symbol_table->member_is_output_log_file;

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
        if ( ! make_PE_file_COFF_SYMBOL_TABLE_func_def(
            symbol_table, write_file, AuxiliarySymbolRecords, num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_FUNCTION == one_of_coff_symbol_tables->StorageClass) &&
        ((0 == strncmp(".bf", one_of_coff_symbol_tables->UNION.ShortName, 5)) ||
        (0 == strncmp(".ef", one_of_coff_symbol_tables->UNION.ShortName, 5)))){

        // Auxiliary Format 2: .bf and .ef Symbols
        if ( ! make_PE_file_COFF_SYMBOL_TABLE_func_line(
            symbol_table, write_file, AuxiliarySymbolRecords, num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_EXTERNAL == one_of_coff_symbol_tables->StorageClass) &&
        (TP_IMAGE_SYM_UNDEFINED == one_of_coff_symbol_tables->SectionNumber) &&
        (0 == one_of_coff_symbol_tables->Value)){

        // Auxiliary Format 3: Weak Externals
        if ( ! make_PE_file_COFF_SYMBOL_TABLE_weak_extern(
            symbol_table, write_file, AuxiliarySymbolRecords, num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_FILE == one_of_coff_symbol_tables->StorageClass) &&
        (0 == strncmp(".file", one_of_coff_symbol_tables->UNION.ShortName, 5))){

        // Auxiliary Format 4: Files
        if ( ! make_PE_file_COFF_SYMBOL_TABLE_file(
            symbol_table, write_file, AuxiliarySymbolRecords, num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else if ((TP_IMAGE_SYM_CLASS_STATIC == one_of_coff_symbol_tables->StorageClass)){

        // Auxiliary Format 5: Section Definitions
        if ( ! make_PE_file_COFF_SYMBOL_TABLE_section(
            symbol_table, write_file, AuxiliarySymbolRecords, num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else{

        if (is_print){

            fprintf(write_file, "Unknown Auxiliary Format.\n");
        }
    }

    if (is_print){

        fprintf(write_file, "\n");
    }

    return true;
}

static bool make_PE_file_COFF_SYMBOL_TABLE_func_def(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 1: Function Definitions
    if ( ! symbol_table->member_is_output_log_file){

        return true;
    }

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

static bool make_PE_file_COFF_SYMBOL_TABLE_func_line(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 2: .bf and .ef Symbols
    if ( ! symbol_table->member_is_output_log_file){

        return true;
    }

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

static bool make_PE_file_COFF_SYMBOL_TABLE_weak_extern(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 3: Weak Externals
    if ( ! symbol_table->member_is_output_log_file){

        return true;
    }

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

static bool make_PE_file_COFF_SYMBOL_TABLE_file(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 4: Files
    if ( ! symbol_table->member_is_output_log_file){

        return true;
    }

    TP_AUX_SYMBOL_RECORD_FILE* file = (TP_AUX_SYMBOL_RECORD_FILE*)AuxiliarySymbolRecords;

    fprintf(write_file, "TP_AUX_SYMBOL_RECORD_FILE: uint8_t FileName("); 

    for (uint32_t i = 0; num > i; ++i){

        fprintf(write_file, "%.*s", TP_IMAGE_SIZEOF_SYMBOL, file[i].FileName); 
    }

    fprintf(write_file, ");\n\n"); 

    return true;
}

static bool make_PE_file_COFF_SYMBOL_TABLE_section(
    TP_SYMBOL_TABLE* symbol_table,
    FILE* write_file, TP_AUX_SYMBOL_RECORD* AuxiliarySymbolRecords, uint32_t num)
{
    // Auxiliary Format 5: Section Definitions
    if ( ! symbol_table->member_is_output_log_file){

        return true;
    }

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

