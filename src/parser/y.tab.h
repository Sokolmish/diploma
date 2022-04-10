/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 10 "parser.y"

    struct LinesWarpMap;

#line 52 "y.tab.h"

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PTR_OP = 258,
    INC_OP = 259,
    DEC_OP = 260,
    LEFT_OP = 261,
    RIGHT_OP = 262,
    LE_OP = 263,
    GE_OP = 264,
    EQ_OP = 265,
    NE_OP = 266,
    AND_OP = 267,
    OR_OP = 268,
    MUL_ASSIGN = 269,
    DIV_ASSIGN = 270,
    MOD_ASSIGN = 271,
    ADD_ASSIGN = 272,
    SUB_ASSIGN = 273,
    LEFT_ASSIGN = 274,
    RIGHT_ASSIGN = 275,
    AND_ASSIGN = 276,
    XOR_ASSIGN = 277,
    OR_ASSIGN = 278,
    SIZEOF = 279,
    TYPEDEF = 280,
    EXTERN = 281,
    STATIC = 282,
    AUTO = 283,
    REGISTER = 284,
    INLINE = 285,
    RESTRICT = 286,
    CHAR = 287,
    SHORT = 288,
    INT = 289,
    LONG = 290,
    SIGNED = 291,
    UNSIGNED = 292,
    FLOAT = 293,
    DOUBLE = 294,
    CONST = 295,
    VOLATILE = 296,
    VOID = 297,
    STRUCT = 298,
    UNION = 299,
    ENUM = 300,
    ELLIPSIS = 301,
    CASE = 302,
    DEFAULT = 303,
    IF = 304,
    SWITCH = 305,
    WHILE = 306,
    DO = 307,
    FOR = 308,
    GOTO = 309,
    CONTINUE = 310,
    BREAK = 311,
    RETURN = 312,
    IFX = 313,
    ELSE = 314,
    IDENTIFIER = 315,
    TYPE_NAME = 316,
    CONSTANT = 317,
    STRING_LITERAL = 318
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 47 "parser.y"

    int boolean;
    int enum_val;
    struct AST_Literal num_literal;
    string_id_t str;
    string_id_t ident_id;

    struct AST_Expr *expr;

    struct AST_ArgumentsList *args_list;
    struct AST_StringsSeq *str_seq;

    struct AST_DeclSpecifiers *decl_specifiers;
    struct AST_TypeSpecifier *type_specifier;
    struct AST_StructDeclarationList *struct_decl_lst;
    struct AST_StructDeclaration *struct_decl;
    struct AST_SpecsQualsList *spec_qual_lst;
    struct AST_StructDeclarator *struct_delarator;
    struct AST_StructDeclaratorList *struct_delarator_lst;
    struct AST_UStructSpec *struct_or_union_spec;
    struct AST_Enumerator *enumerator;
    struct AST_EnumeratorList *enumerator_lst;
    struct AST_EnumSpecifier *enum_spec;

    struct AST_InitDeclarator *init_decltor;
    struct AST_InitDeclaratorList *init_decltor_lst;
    struct AST_Declaration *declaration;
    struct AST_DirDeclarator *dir_decltor;
    struct AST_Pointer *pointer;
    struct AST_Declarator *declarator;
    struct AST_ParameterDeclaration *param_declaration;
    struct AST_ParameterTypeList *param_type_lst;
    struct AST_ParameterList *param_lst;
    struct AST_TypeName *type_name;
    struct AST_DirAbstrDeclarator *dir_abstract_decl;
    struct AST_AbstrDeclarator *abstract_decl;
    struct AST_TypeQuals *type_quals;

    struct AST_Designator *designator;
    struct AST_InitializerList *initializer_lst;
    struct AST_Initializer *initializer;

    struct AST_Stmt *stmt;
    struct AST_LabeledStmt *label_stmt;
    struct AST_BlockItemList *block_item_lst;
    struct AST_CompoundStmt *compound_stmt;
    struct AST_ExprStmt *expr_stmt;
    struct AST_SelectionStmt *select_stmt;
    struct AST_IterStmt *iter_stmt;
    struct AST_JumpStmt *jump_stmt;

    struct AST_FunctionDef *func_def;
    struct AST_TranslationUnit *trans_unit;

#line 182 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (yyscan_t scanner, struct AST_TranslationUnit **parser_result, const struct LinesWarpMap *warps);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
