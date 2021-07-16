
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define IS_TP_NONZERO_DIGIT(c) ((0 != (c)) && isdigit(c))

typedef enum TP_INTEGER_SUFFIX_
{
    TP_INTEGER_SUFFIX_NULL,
    TP_INTEGER_SUFFIX_U,
    TP_INTEGER_SUFFIX_L,
    TP_INTEGER_SUFFIX_UL,
    TP_INTEGER_SUFFIX_LL,
    TP_INTEGER_SUFFIX_ULL
}TP_INTEGER_SUFFIX;

typedef enum TP_FLOATING_SUFFIX_
{
    TP_FLOATING_SUFFIX_NULL,
    TP_FLOATING_SUFFIX_F,
    TP_FLOATING_SUFFIX_L
}TP_FLOATING_SUFFIX;

static bool convert_pp_number_to_integer(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string, bool* is_match
);
static bool set_integer_value(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    int base, TP_INTEGER_SUFFIX suffix_type, bool* is_match
);
static bool convert_pp_number_to_integer_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match
);
static bool convert_pp_number_to_octal_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match
);
static bool convert_pp_number_to_hexadecimal_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match
);
static bool integer_suffix(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match
);
static bool convert_pp_number_to_floating(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string, bool* is_match
);
static bool set_floating_value(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX suffix_type, bool* is_match
);
static bool convert_pp_number_to_decimal_floating_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX* suffix_type, bool* is_match
);
static bool convert_pp_number_to_hexadecimal_floating_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX* suffix_type, bool* is_match
);
static bool floating_suffix(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX* suffix_type, bool* is_match
);

bool tp_convert_pp_number(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    bool is_match = false;

    if ( ! convert_pp_number_to_integer(
        symbol_table, token, token->member_string, &is_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_match){

        token->member_symbol = TP_SYMBOL_CONSTANT;
        token->member_symbol_kind = TP_SYMBOL_KIND_INTEGER_CONSTANT;

        return true;
    }

    if ( ! convert_pp_number_to_floating(
        symbol_table, token, token->member_string, &is_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_match){

        token->member_symbol = TP_SYMBOL_CONSTANT;
        token->member_symbol_kind = TP_SYMBOL_KIND_FLOATING_CONSTANT;

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: pp-number(%1) convert failed."),
        TP_LOG_PARAM_STRING(token->member_string)
    );

    return false;
}

static bool convert_pp_number_to_integer(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string, bool* is_match)
{
    int base = 10;
    TP_INTEGER_SUFFIX suffix_type = TP_INTEGER_SUFFIX_NULL;
    bool is_local_match = false;

    if ( ! convert_pp_number_to_integer_constant(
        symbol_table, token, string, &suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        if ( ! set_integer_value(symbol_table, token, string, base, suffix_type, is_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    if ( ! convert_pp_number_to_octal_constant(
        symbol_table, token, string, &suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        base = 8;

        if ( ! set_integer_value(symbol_table, token, string, base, suffix_type, is_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    if ( ! convert_pp_number_to_hexadecimal_constant(
        symbol_table, token, string, &suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        base = 16;

        if ( ! set_integer_value(symbol_table, token, string, base, suffix_type, is_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    return true;
}

static bool set_integer_value(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    int base, TP_INTEGER_SUFFIX suffix_type, bool* is_match)
{
    char* error_first_char = NULL;

    uint64_t value = (uint64_t)strtoull(string, &error_first_char, base);

    if (string == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtoull(%1) convert failed."),
            TP_LOG_PARAM_STRING(string)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

        return false;
    }

    switch (suffix_type){
    case TP_INTEGER_SUFFIX_NULL:
        if (10 == base){
            if (INT_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_INT;
                token->member_value.member_int_value = (int)value;
            }else if (LONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_INT;
                token->member_value.member_long_int_value = (long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_LONG_LONG_INT;
                token->member_value.member_long_long_int_value = (long long int)value;
            }
        }else{
            if (INT_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_INT;
                token->member_value.member_int_value = (int)value;
            }else if (UINT_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_INT;
                token->member_value.member_unsigned_int_value = (unsigned int)value;
            }else if (LONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_INT;
                token->member_value.member_long_int_value = (long int)value;
            }else if (ULONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_INT;
                token->member_value.member_unsigned_long_int_value = (unsigned long int)value;
            }else if (LLONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_LONG_INT;
                token->member_value.member_long_long_int_value = (long long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT;
                token->member_value.member_unsigned_long_long_int_value = (unsigned long long int)value;
            }
        }
        break;
    case TP_INTEGER_SUFFIX_U:
            if (UINT_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_INT;
                token->member_value.member_unsigned_int_value = (unsigned int)value;
            }else if (ULONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_INT;
                token->member_value.member_unsigned_long_int_value = (unsigned long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT;
                token->member_value.member_unsigned_long_long_int_value = (unsigned long long int)value;
            }
        break;
    case TP_INTEGER_SUFFIX_L:
        if (10 == base){
            if (LONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_INT;
                token->member_value.member_long_int_value = (long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_LONG_LONG_INT;
                token->member_value.member_long_long_int_value = (long long int)value;
            }
        }else{
            if (LONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_INT;
                token->member_value.member_long_int_value = (long int)value;
            }else if (ULONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_INT;
                token->member_value.member_unsigned_long_int_value = (unsigned long int)value;
            }else if (LLONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_LONG_INT;
                token->member_value.member_long_long_int_value = (long long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT;
                token->member_value.member_unsigned_long_long_int_value = (unsigned long long int)value;
            }
        }
        break;
    case TP_INTEGER_SUFFIX_UL:
            if (ULONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_INT;
                token->member_value.member_unsigned_long_int_value = (unsigned long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT;
                token->member_value.member_unsigned_long_long_int_value = (unsigned long long int)value;
            }
        break;
    case TP_INTEGER_SUFFIX_LL:
        if (10 == base){
                token->member_value_type = TP_VALUE_TYPE_LONG_LONG_INT;
                token->member_value.member_long_long_int_value = (long long int)value;
        }else{
            if (LLONG_MAX >= value){
                token->member_value_type = TP_VALUE_TYPE_LONG_LONG_INT;
                token->member_value.member_long_long_int_value = (long long int)value;
            }else{
                token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT;
                token->member_value.member_unsigned_long_long_int_value = (unsigned long long int)value;
            }
        }
        break;
    case TP_INTEGER_SUFFIX_ULL:
        token->member_value_type = TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT;
        token->member_value.member_unsigned_long_long_int_value = (unsigned long long int)value;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    *is_match = true;

    return true;
}

static bool convert_pp_number_to_integer_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match)
{
    // decimal-constant integer-suffixopt

    // decimal-constant: nonzero-digit digit*
    // nonzero-digit: one of 123456789

    if ( ! IS_TP_NONZERO_DIGIT(*string)){

        return true;
    }

    ++string;

    while (isdigit(*string)){

        ++string;
    }

    bool is_local_match = false;

    if ( ! integer_suffix(symbol_table, token, string, suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        *is_match = true;

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: Bad integer suffix(%1)."),
        TP_LOG_PARAM_STRING(string)
    );

    return false;
}

static bool convert_pp_number_to_octal_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match)
{
    // octal-constant integer-suffixopt

    // octal-constant: 0 octal-digit*
    // octal-digit: one of 01234567

    if ('0' != *string){

        return true;
    }

    ++string;

    while (IS_TP_OCTAL_DIGIT(*string)){

        ++string;
    }

    bool is_local_match = false;

    if ( ! integer_suffix(symbol_table, token, string, suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        *is_match = true;

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: Bad integer suffix(%1)."),
        TP_LOG_PARAM_STRING(string)
    );

    return false;
}

static bool convert_pp_number_to_hexadecimal_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match)
{
    // hexadecimal-constant integer-suffixopt

    // hexadecimal-constant: hexadecimal-prefix hexadecimal-digit+
    // hexadecimal-prefix: one of 0x 0X
    // hexadecimal-digit: one of 0123456789 abcdef ABCDEF

    if ('0' != *string){

        return true;
    }

    ++string;

    if (('x' != *string) && ('X' != *string)){

        return true;
    }

    bool is_local_match = false;

    for (++string; ; ++string){

        if ( ! isxdigit(*string)){

            break;
        }

        is_local_match = true;
    }

    if (false == is_local_match){

        return true;
    }

    is_local_match = false;

    if ( ! integer_suffix(symbol_table, token, string, suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        *is_match = true;

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: Bad integer suffix(%1)."),
        TP_LOG_PARAM_STRING(string)
    );

    return false;
}

static bool integer_suffix(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_INTEGER_SUFFIX* suffix_type, bool* is_match)
{
    if ('\0' == *string){

        *suffix_type = TP_INTEGER_SUFFIX_NULL;
        *is_match = true;

        return true;
    }

    // unsigned-suffix long-suffixopt
    // unsigned-suffix: one of u U
    // long-suffix: one of l L

    if (('u' == *string) || ('U' == *string)){

        ++string;

        if ('\0' == *string){

            *suffix_type = TP_INTEGER_SUFFIX_U;
            *is_match = true;

            return true;
        }

        if (('l' == *string) || ('L' == *string)){

            ++string;

            if ('\0' == *string){

                *suffix_type = TP_INTEGER_SUFFIX_UL;
                *is_match = true;

                return true;
            }

            // unsigned-suffix long-long-suffix
            // unsigned-suffix: one of u U
            // long-long-suffix: one of ll LL

            if (('l' == *string) || ('L' == *string)){

                *suffix_type = TP_INTEGER_SUFFIX_ULL;
                *is_match = true;

                return true;
            }
        }
    }

    // long-suffix unsigned-suffixopt
    // long-suffix: one of l L
    // unsigned-suffix: one of u U

    if (('l' == *string) || ('L' == *string)){

        ++string;

        if ('\0' == *string){

            *suffix_type = TP_INTEGER_SUFFIX_L;
            *is_match = true;

            return true;
        }

        if (('u' == *string) || ('U' == *string)){

            ++string;

            if ('\0' == *string){

                *suffix_type = TP_INTEGER_SUFFIX_UL;
                *is_match = true;

                return true;
            }
        }

        // long-long-suffix unsigned-suffixopt
        // long-long-suffix: one of ll LL
        // unsigned-suffix: one of u U

        if (('l' == *string) || ('L' == *string)){

            ++string;

            if ('\0' == *string){

                *suffix_type = TP_INTEGER_SUFFIX_LL;
                *is_match = true;

                return true;
            }

            if (('u' == *string) || ('U' == *string)){

                ++string;

                if ('\0' == *string){

                    *suffix_type = TP_INTEGER_SUFFIX_ULL;
                    *is_match = true;

                    return true;
                }
            }
        }
    }

    return true;
}

static bool convert_pp_number_to_floating(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string, bool* is_match)
{
    TP_FLOATING_SUFFIX suffix_type = TP_FLOATING_SUFFIX_NULL;
    bool is_local_match = false;

    if ( ! convert_pp_number_to_decimal_floating_constant(
        symbol_table, token, string, &suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        if ( ! set_floating_value(symbol_table, token, string, suffix_type, is_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    if ( ! convert_pp_number_to_hexadecimal_floating_constant(
        symbol_table, token, string, &suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        if ( ! set_floating_value(symbol_table, token, string, suffix_type, is_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    return true;
}

static bool set_floating_value(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX suffix_type, bool* is_match)
{
    char* error_first_char = NULL;

    long double value = strtold(string, &error_first_char);

    if (string == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtold(%1) convert failed."),
            TP_LOG_PARAM_STRING(string)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

        return false;
    }

    switch (suffix_type){
    case TP_FLOATING_SUFFIX_NULL:
        if (isgreater(value, DBL_MAX) || isless(value, DBL_MIN)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: double type overflow(value = %1)."),
                TP_LOG_PARAM_STRING(string)
            );

            return false;
        }else{
            token->member_value_type = TP_VALUE_TYPE_DOUBLE;
            token->member_value.member_double_value = (double)value;
        }
        break;
    case TP_FLOATING_SUFFIX_F:
        if (isgreater(value, FLT_MAX) || isless(value, FLT_MIN)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: float type overflow(value = %1)."),
                TP_LOG_PARAM_STRING(string)
            );

            return false;
        }else{
            token->member_value_type = TP_VALUE_TYPE_FLOAT;
            token->member_value.member_float_value = (float)value;
        }
        break;
    case TP_FLOATING_SUFFIX_L:
        token->member_value_type = TP_VALUE_TYPE_LONG_DOUBLE;
        token->member_value.member_long_double_value = value;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    *is_match = true;

    return true;
}

static bool convert_pp_number_to_decimal_floating_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX* suffix_type, bool* is_match)
{
    // decimal-floating-constant:
    //     (fractional-constant exponent-partopt floating-suffixopt) |
    //     (digit-sequence exponent-part floating-suffixopt)

    bool is_local_match = false;
    bool is_must_exponent_part = false;

    // fractional-constant:
    //     (digit-sequenceopt . digit-sequence) |
    //     (digit-sequence .)
    if (isdigit(*string)){

        // digit-sequence: digit*
        for (++string; isdigit(*string); ++string){ }

        if ('.' != *string){

            is_must_exponent_part = true;

            goto exponent_part;
        }

        // digit-sequence: digit*
        for (++string; isdigit(*string); ++string){ }

    }else if ('.' == *string){

        // digit-sequence: digit+
        for (++string; ; ++string){

            if ( ! isdigit(*string)){

                ++string;

                break;
            }

            is_local_match = true;
        }

        if (false == is_local_match){

            return true;
        }
    }else{

        return true;
    }

exponent_part:

    is_local_match = false;

    // exponent-part: (e | E) signopt digit-sequence
    // sign: one of + -
    if (('e' == *string) || ('E' == *string)){

        ++string;

        if (('+' == *string) || ('-' == *string)){

            ++string;
        }

        // digit-sequence: digit+
        for (++string; ; ++string){

            if ( ! isdigit(*string)){

                ++string;

                break;
            }

            is_local_match = true;
        }

        if (false == is_local_match){

            return true;
        }
    }else if (is_must_exponent_part){

        return true;
    }

    is_local_match = false;

    if ( ! floating_suffix(symbol_table, token, string, suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        *is_match = true;
    }

    return true;
}

static bool convert_pp_number_to_hexadecimal_floating_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX* suffix_type, bool* is_match)
{
    // hexadecimal-floating-constant:
    //     hexadecimal-prefix hexadecimal-fractional-constant
    //         binary-exponent-part floating-suffixopt
    //     hexadecimal-prefix hexadecimal-digit-sequence
    //         binary-exponent-part floating-suffixopt

    // hexadecimal-prefix: one of 0x 0X
    if ('0' != *string){

        return true;
    }

    ++string;

    if (('x' == *string) || ('X' == *string)){

        ++string;
    }

    // hexadecimal-fractional-constant:
    //     (hexadecimal-digit-sequenceopt .) |
    //     hexadecimal-digit-sequence |
    //     (hexadecimal-digit-sequence .)
    // hexadecimal-digit-sequence: hexadecimal-digit+
    if (isxdigit(*string)){

        // digit-sequence: digit*
        for (++string; isxdigit(*string); ++string){ }

        if ('.' != *string){

            goto binary_exponent_part;
        }
    }else if ('.' == *string){

        ;
    }else{

        return true;
    }

    ++string;

binary_exponent_part:
    ;

    bool is_local_match = false;

    // binary-exponent-part: (p | P) signopt digit-sequence
    // sign: one of + -
    // digit-sequence: digit+
    if (('p' == *string) || ('P' == *string)){

        ++string;

        if (('+' == *string) || ('-' == *string)){

            ++string;
        }

        // digit-sequence: digit+
        for (++string; ; ++string){

            if ( ! isdigit(*string)){

                ++string;

                break;
            }

            is_local_match = true;
        }

        if (false == is_local_match){

            return true;
        }
    }

    is_local_match = false;

    if ( ! floating_suffix(symbol_table, token, string, suffix_type, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        *is_match = true;
    }

    return true;
}

static bool floating_suffix(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* string,
    TP_FLOATING_SUFFIX* suffix_type, bool* is_match)
{
    if ('\0' == *string){

        *suffix_type = TP_FLOATING_SUFFIX_NULL;
        *is_match = true;

        return true;
    }

    // floating-suffix: one of flFL

    if (('f' == *string) || ('F' == *string)){

        ++string;

        if ('\0' == *string){

            *suffix_type = TP_FLOATING_SUFFIX_F;
            *is_match = true;

            return true;
        }
    }

    if (('l' == *string) || ('L' == *string)){

        ++string;

        if ('\0' == *string){

            *suffix_type = TP_FLOATING_SUFFIX_L;
            *is_match = true;

            return true;
        }
    }

    return true;
}

