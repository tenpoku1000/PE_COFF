
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// C IR(type check).

static bool normalize_C_IR_type_specifier_basic_common(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_TYPE_SPECIFIER* type_specifier
);

bool tp_check_C_IR_type_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_OBJECT* type)
{
    switch (type->member_type.member_type){
    case TP_C_TYPE_TYPE_BASIC:{

        TP_C_TYPE_SPECIFIER* type_specifier = &(type->member_type.
            member_body.member_type_basic.member_type_specifier);

        if ( ! normalize_C_IR_type_specifier_basic_common(
            symbol_table, grammer_context, type_specifier)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE** type_declaration =
            &(type->member_type.member_body.
            member_type_declaration_statement.member_declaration);

        if (NULL == (*type_declaration)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        switch ((*type_declaration)->member_type){
        case TP_C_TYPE_TYPE_BASIC:{

            TP_C_TYPE_SPECIFIER* type_specifier =
                &((*type_declaration)->member_body.member_type_basic.member_type_specifier);

            if ( ! normalize_C_IR_type_specifier_basic_common(
                symbol_table, grammer_context, type_specifier)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        }
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool normalize_C_IR_type_specifier_basic_common(
    TP_SYMBOL_TABLE* symbol_table,
    TP_GRAMMER_CONTEXT grammer_context, TP_C_TYPE_SPECIFIER* type_specifier)
{
    if (TP_C_TYPE_SPECIFIER_NONE == *type_specifier){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_SPECIFIER type_specifier_value = TP_MASK_C_TYPE_SPECIFIER(*type_specifier);

    bool is_signed = (TP_C_TYPE_SPECIFIER_SIGNED & type_specifier_value);
    bool is_unsigned = (TP_C_TYPE_SPECIFIER_UNSIGNED & type_specifier_value);

    if (is_signed && is_unsigned){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: type error.")
        );

        return false;
    }

    if ((TP_C_TYPE_SPECIFIER_VOID & (*type_specifier)) &&
        ((~TP_C_TYPE_SPECIFIER_VOID) & (*type_specifier))){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: type error.")
        );

        return false;
    }

    // int
    if (is_signed || is_unsigned){

        if ( ! type_specifier_value){

            return true;
        }
    }
    if (TP_C_TYPE_SPECIFIER_INT == type_specifier_value){

        return true;
    }

    // char
    if (TP_C_TYPE_SPECIFIER_CHAR == type_specifier_value){

        if ((false == is_signed) && (false == is_unsigned)){

            (*type_specifier) |= TP_C_TYPE_SPECIFIER_SIGNED;
        }

        return true;
    }

    // Other types.
    if (TP_C_TYPE_SPECIFIER_INT & type_specifier_value){

        (*type_specifier) ^= TP_C_TYPE_SPECIFIER_INT;
        type_specifier_value ^= TP_C_TYPE_SPECIFIER_INT;
    }

    switch (type_specifier_value){
    case TP_C_TYPE_SPECIFIER_SHORT: // short
//      break;
    case TP_C_TYPE_SPECIFIER_LONG1: // long
//      break;
    case TP_C_TYPE_SPECIFIER_LONG2: // long long
        return true;
    default:
        break;
    }

    if ((TP_GRAMMER_CONTEXT_FUNCTION_ARGS == grammer_context) ||
        (TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE == grammer_context)){

        if (TP_C_TYPE_SPECIFIER_VOID & (*type_specifier)){

            return true;
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: type error.")
    );

    return false;
}

