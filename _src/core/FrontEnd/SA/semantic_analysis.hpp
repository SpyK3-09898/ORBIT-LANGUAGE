
// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o COdigo em Busca de Erros Semanticos.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../parser/parser.hpp"
#include "../parser/AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <cstddef>

// ======= PREV ======= //
struct Scope;
struct Symbol;

// ======= ENUMS ====== //

// Typeof Symbols | Tipo dos Simbolos(Obvio).
enum class SymbolTypes: ui8
{
    UNK,
    VAR,
    FN,
    IDENTIFIER,
    ARRAY,
    TABLE,
    PARAM,
    STRUCT,
    CLASS,
    ENUM,
    MODULE,
    LIBRARIE 
};

// Kindof Types | kind dos Tipos.
enum class TypeKind
{
    // UNKNOW | DESCONHECIDO
    UNK,
    MONO_STATE,

    // LIT
    NUMBER,
    STRING,
    BOOL,
    ANY,
    _NULL,
    NONE,

    // LISTS | LISTAS.
    ARRAY,
    TABLE,

    TABLE_INT,
    TABLE_FLOAT,
    TABLE_STRING,
    TABLE_BOOL,
    TABLE_ANY,
    TABLE_NULL,
    TABLE_NONE,

    // OBJ
    STRUCT,
    CLASS,

    // OTHERS | OUTROS.
    FN
};

// Kindof SubTypes | Kind dos SubTipos.
enum class SybTypeKind
{
    NONE,
    INT,
    FLOAT,
    TRUE,
    FALSE
};

// Type Informations | Infos dos Tipos.
struct TypeInfo
{
    TypeKind Kind = TypeKind::MONO_STATE;
    SybTypeKind SubKind = SybTypeKind::NONE;
};

// Symbol Repr | Representação dos Simbolos.
struct Symbol
{
    TypeInfo* TInfo;
    Scope* DeclaredScope;

    vec<pair<string, TypeInfo*>> Objs;

    str_view Name;
    NodePos Pos;

    SymbolTypes Type = SymbolTypes::UNK;
    MutableTypes Mut = MutableTypes::MUT;

    ui32 read_count  = 0;
    ui32 write_count = 0;
    bool inited = false;
};

// Scope Repr | Representação de Escopos.
struct Scope
{
    Scope* Parent  =  nullptr;
    Scope* Next    =  nullptr;
    ASTNode* Owner;
    unord_map<str_view, Symbol*> Symbols;
    BodyTypes Type;

    // HELPERS

    // Find Symbol ONLY in THIS Scope
    // Encontra o Simbolo APENAS NESTE Escopo.
    Symbol* FindSymLocal(string& sym)
    {
        for (auto& P : Symbols)
            if (P.first == sym)
                return P.second;
        return nullptr;
    }    

    // Find Symbol in Scopes | Encontra Simbolo no Escopo.
    Symbol* FindSym(string& sym)
    {
        for (auto& P : Symbols)
            if (P.first == sym)
                return P.second;
        if (Parent)
            return Parent->FindSym(sym);
        return nullptr;
    }

    // Find Symbol Whit Types in Scope 
    // Encontra o Simbolo de tal Tipo no Escopo.
    Symbol* FindSymbol(string& sym, SymbolTypes Type)
    {
        for (auto& P : Symbols)
            if (P.first == sym && P.second->Type == Type)
                return P.second;
        if (Parent)
            return Parent->FindSymbol(sym, Type);
        return nullptr;        
    }
};

// STATE & RESULT | ESTADO E RESULTADO.
struct SAState 
{
    Scope* CurrScope = nullptr;
    vec<Scope*> ScopeStack = {};
    vec<pair<int, ASTNode*>> NodesChecked{};
    int logInd=0;
};

struct SAResult 
{
    Scope* GlobalScope = nullptr;
    unord_map<ASTNode*, TypeInfo> ExpressionTypes;
    unord_map<str_view, Symbol*> SymbolTable;
};

// MAIN CLASS | CLASSE PRINCIPAL.
class SemanticAnalizer
{
    public:
        SAResult InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory);
    private:

        void LookUpNode(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);

        void LookUpProgram(ProgramNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpBody(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);

        void LookUpLiteral(LiteralNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpUnary(UnaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpBinary(BinaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpAssignment(AssignmentNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpMemberAccess(MemberAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpIndexAccess(IndexAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpRange(RangeNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpFunctionCall(FunctionCall& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpArray(ArrayValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpTable(TableValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpIValue(ExpressionNode* Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);

        //void LookUpVarDecl(VarDeclNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpFunction(FnDecl& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);

        //void LookUpIf(IfNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpElif(ElifNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpElse(ElseNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpWhile(WhileNode& Node);
        //void LookUpFor(ForNode& Node);
        //void LookUpForEach(ForEachNode& Node);
        //void LookUpForDef(ForDefNode& Node);
        //void LookUpReturn(ReturnNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
};