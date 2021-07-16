
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_do_preprocess(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    //（4） 前処理指令を実行し，マクロ呼出しを展開する。さらに，_Pragma 単項演算子式を実行する。
    //      字句連結（6.10.3.3 参照）の結果として生成される文字の並びが国際文字名の構文規則に一致する場合，
    //      その動作は未定義とする。#include 前処理指令に指定された名前をもつヘッダ又はソースファイルに対して，
    //      フェーズ（1）からフェーズ（4）までの処理を再帰的に行い，すべての前処理指令を削除する。
/*

A.1.8 Header names

(6.4.7) header-name:
< h-char-sequence >
" q-char-sequence "

(6.4.7) h-char-sequence:
h-char
h-char-sequence h-char

(6.4.7) h-char:
any member of the source character set except
the new-line character and >

(6.4.7) q-char-sequence:
q-char
q-char-sequence q-char

(6.4.7) q-char:
any member of the source character set except
the new-line character and "

A.3 Preprocessing directives

(6.10) preprocessing-file:
groupopt

(6.10) group:
group-part
group group-part

(6.10) group-part:
if-section
control-line
text-line
# non-directive

(6.10) if-section:
if-group elif-groupsopt else-groupopt endif-line

(6.10) if-group:
# if constant-expression new-line groupopt
# ifdef identifier new-line groupopt
# ifndef identifier new-line groupopt

(6.10) elif-groups:
elif-group
elif-groups elif-group

(6.10) elif-group:
# elif constant-expression new-line groupopt

(6.10) else-group:
# else new-line groupopt

(6.10) endif-line:
# endif new-line

(6.10) control-line:
# include pp-tokens new-line
# define identifier replacement-list new-line
# define identifier lparen identifier-listopt )
replacement-list new-line
# define identifier lparen ... ) replacement-list new-line
# define identifier lparen identifier-list , ... )
replacement-list new-line
# undef identifier new-line
# line pp-tokens new-line
# error pp-tokensopt new-line
# pragma pp-tokensopt new-line
# new-line

(6.10) text-line:
pp-tokensopt new-line

(6.10) non-directive:
pp-tokens new-line

(6.10) lparen:
a ( character not immediately preceded by white-space

(6.10) replacement-list:
pp-tokensopt

(6.10) pp-tokens:
preprocessing-token
pp-tokens preprocessing-token

(6.10) new-line:
the new-line character

A.2.2 Declarations

(6.7.6) identifier-list:
identifier
identifier-list , identifier

A.2.1 Expressions

(6.5.1) primary-expression:
identifier
constant
string-literal
( expression )
generic-selection
(6.5.1.1) generic-selection:
_Generic ( assignment-expression , generic-assoc-list )
(6.5.1.1) generic-assoc-list:
generic-association
generic-assoc-list , generic-association
(6.5.1.1) generic-association:
type-name : assignment-expression
default : assignment-expression
(6.5.2) postfix-expression:
primary-expression
postfix-expression [ expression ]
postfix-expression ( argument-expression-listopt )
postfix-expression . identifier
postfix-expression -> identifier
postfix-expression ++
postfix-expression --
( type-name ) { initializer-list }
( type-name ) { initializer-list , }
(6.5.2) argument-expression-list:
assignment-expression
argument-expression-list , assignment-expression
(6.5.3) unary-expression:
postfix-expression
++ unary-expression
-- unary-expression
unary-operator cast-expression
sizeof unary-expression
sizeof ( type-name )
_Alignof ( type-name )
§A.2.1 Languagesyntax summary 465
ISO/IEC 9899:201x Committee Draft — April 12, 2011 N1570
(6.5.3) unary-operator: one of
& * + - ~ !
(6.5.4) cast-expression:
unary-expression
( type-name ) cast-expression
(6.5.5) multiplicative-expression:
cast-expression
multiplicative-expression * cast-expression
multiplicative-expression / cast-expression
multiplicative-expression % cast-expression
(6.5.6) additive-expression:
multiplicative-expression
additive-expression + multiplicative-expression
additive-expression - multiplicative-expression
(6.5.7) shift-expression:
additive-expression
shift-expression << additive-expression
shift-expression >> additive-expression
(6.5.8) relational-expression:
shift-expression
relational-expression < shift-expression
relational-expression > shift-expression
relational-expression <= shift-expression
relational-expression >= shift-expression
(6.5.9) equality-expression:
relational-expression
equality-expression == relational-expression
equality-expression != relational-expression
(6.5.10) AND-expression:
equality-expression
AND-expression & equality-expression
(6.5.11) exclusive-OR-expression:
AND-expression
exclusive-OR-expression ^ AND-expression
466 Languagesyntax summary §A.2.1
N1570 Committee Draft — April 12, 2011 ISO/IEC 9899:201x
(6.5.12) inclusive-OR-expression:
exclusive-OR-expression
inclusive-OR-expression | exclusive-OR-expression
(6.5.13) logical-AND-expression:
inclusive-OR-expression
logical-AND-expression && inclusive-OR-expression
(6.5.14) logical-OR-expression:
logical-AND-expression
logical-OR-expression || logical-AND-expression
(6.5.15) conditional-expression:
logical-OR-expression
logical-OR-expression ? expression : conditional-expression
(6.5.16) assignment-expression:
conditional-expression
unary-expression assignment-operator assignment-expression
(6.5.16) assignment-operator: one of
= *= /= %= += -= <<= >>= &= ^= |=
(6.5.17) expression:
assignment-expression
expression , assignment-expression
(6.6) constant-expression:
conditional-expression

*/
    return true;
}

