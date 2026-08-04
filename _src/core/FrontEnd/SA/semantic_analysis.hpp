
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
    PARAM,
    STRUCT,
    CLASS,
    ENUM,
    MODULE,
    LIBRARIE 
};

// Scope Repr | Representação de Escopos.
struct Scope
{
    Scope* Parent  =  nullptr;
    Scope* Next    =  nullptr;
    ASTNode* Owner;
    unord_map<str_view, Symbol*> Symbols;

    // HELPERS

    // Find Symbol ONLY in THIS Scope
    // Encontra o Simbolo APENAS NESTE Escopo.
    Symbol* FindSymLocal(string& sym)
    {
        for (pair<str_view, Symbol*> P : Symbols)
            if (P.first == sym)
                return P.second;
        return nullptr;
    }    

    // Find Symbol in Scopes | Encontra Simbolo no Escopo.
    Symbol* FindSym(string& sym)
    {
        for (pair<str_view, Symbol*> P : Symbols)
            if (P.first == sym)
                return P.second;
            else
                if (Parent) return Parent->FindSym(sym);
                else return nullptr;
        return nullptr;
    }
};

// Type Informations | Infos dos Tipos.
struct TypeInfo
{

};

// Symbol Repr | Representação dos Simbolos.
struct Symbol
{
    str_view Name;
    SymbolTypes Type = SymbolTypes::UNK;
    TypeInfo* TInfo;

    NodePos Pos;
    Scope* DeclaredScope;

    MutableTypes Mut;

    bool inited = false;
    ui32 read_count  = 0;
    ui32 write_count = 0;
};

// STATE & RESULT | ESTADO E RESULTADO.
struct SAState 
{
    vec<Scope*> ScopeStack = {};
    Scope* CurrScope = nullptr;
};

struct SAResult 
{
    Scope* GlobalScope = nullptr;
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

        //void LookUpLiteral(LiteralNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpUnary(UnaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpBinary(BinaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpAssignment(AssignmentNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpMemberAccess(MemberAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpIndexAccess(IndexAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpRange(RangeNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpFunctionCall(FunctionCall& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpArray(ArrayValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpTable(TableValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);

        //void LookUpVarDecl(VarDeclNode& Node);
        //void LookUpFunction(FnDecl& Node);

        //void LookUpIf(IfNode& Node);
        //void LookUpElif(ElifNode& Node);
        //void LookUpElse(ElseNode& Node);
        //void LookUpWhile(WhileNode& Node);
        //void LookUpFor(ForNode& Node);
        //void LookUpForEach(ForEachNode& Node);
        //void LookUpForDef(ForDefNode& Node);
        //void LookUpReturn(ReturnNode& Node);
};