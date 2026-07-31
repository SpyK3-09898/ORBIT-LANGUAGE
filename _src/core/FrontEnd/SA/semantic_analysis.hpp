
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

// CORE

// Def Symbol | Simbolo Padrão.
struct Symbol
{
    // DATA 
    string name;
    MutableTypes Mut = MutableTypes::UNK;

    // USING | USOS
    ui32 read_count  = 0;
    ui32 write_count = 0;
    ui32 call_count  = 0;

    // DERIVATED | DERIVADOS
    bool is_live=false;
};

// Current Base Scope | Base do Escopo Atual 
struct Scope
{

    // DATA
    unord_map<string, Symbol*> Symbols;
    Scope* Parent;
    BodyTypes Kind;

    // UTILS | UTILIDADES.

    // Return If Have This Symbol In Scope & Parents 
    // Retorna se Tem o Simbolo no Escopo ou Nos Pais.
    Symbol* LookUp(const string& N) const 
    {
        auto it = Symbols.find(N);
        if(it != Symbols.end())
            return it->second;
        if (Parent)
            return Parent->LookUp(N);

        return nullptr;
    }
    // Return If Have This Symbol In Scope
    // Retorna se Tem o Simbolo Neste Escopo.
    Symbol* LookupLocal(const string& name) const
    {
        auto it = Symbols.find(name);
        return (it != Symbols.end()) ? it->second : nullptr;
    }
};

// RESULT

// Result of AS | Resultado do AS
struct SAResult
{
    
};

// MAIN CLASS | CLASSE PRINCIPAL
class SemanticAnalizer
{
    // DATA
    private:

        // Current State of SA | Estado Atual do AS
        struct
        {
            Scope* CurrScope   = nullptr;

            Arena* Memory      = nullptr;
            RunTimeData* Data  = nullptr;
        } SAState;

    // PUBLICS
    public:

        SAResult InitSA(ParseResult& Res, RunTimeData& Data, Arena& Memory);

    // VISIT FUNCTIONS
    private:

        void Visit(ASTNode* Node);

        // PROGRAM
        void VisitProgram(ProgramNode* Node);
        void VisitBody(BodyNode* Node);

        // DECLARATIONS
        void VisitVarDecl(VarDeclNode* Node);
        void VisitFnDecl(FnDecl* Node);

        // CONTROL
        void VisitIf(IfNode* Node);
        void VisitElse(ElseNode* Node);
        void VisitElif(ElifNode* Node);

        void VisitWhile(WhileNode* Node);
        void VisitFor(ForNode* Node);
        void VisitForEach(ForEachNode* Node);
        void VisitForDef(ForDefNode* Node);

        void VisitReturn(ReturnNode* Node);

        // EXPRESSIONS
        void VisitLiteral(LiteralNode* Node);
        void VisitIdentifier(IdentifierNode* Node);

        void VisitUnary(UnaryNode* Node);
        void VisitBinary(BinaryNode* Node);
        void VisitAssignment(AssignmentNode* Node);

        void VisitMemberAccess(MemberAccessNode* Node);
        void VisitIndexAccess(IndexAccessNode* Node);
        void VisitFunctionCall(FunctionCall* Node);

        void VisitArray(ArrayValue* Node);
        void VisitTable(TableValue* Node);

        void VisitRange(RangeNode* Node);

        // ERRORS
        void VisitErrorExpr(ErrorExprNode* Node);
        void VisitErrorDecl(ErrorDeclNode* Node);
        void VisitErrorStmt(ErrorStmtNode* Node);     
};

// EOF