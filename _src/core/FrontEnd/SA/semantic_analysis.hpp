
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
    NAMESPACE,
    PARAM,
    ARRAY,
    TABLE,
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
    NAMESPACE,
    MODULE,
    LIBRARIE,
    ITERATOR,

    // OTHERS | OUTROS.
    FN
};

// Kindof SubTypes | Kind dos SubTipos.
enum class SubTypeKind
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
    SubTypeKind SubKind = SubTypeKind::NONE;
    Symbol* Father;
};

// Symbol Repr | Representação dos Simbolos.
struct Symbol
{
    TypeInfo* TInfo;
    TypeInfo* InferType;
    Scope* DeclaredScope;
    Scope* LinkedScope;

    vec<pair<string, TypeInfo*>> Objs;

    string Name;
    CodePosition Pos;

    SymbolTypes Type = SymbolTypes::UNK;
    MutableTypes Mut = MutableTypes::MUT;

    ui32 read_count  = 0;
    ui32 write_count = 0;
    ui16 Id          = 0;
    bool inited      = false;
};

// Scope Repr | Representação de Escopos.
struct Scope
{
    // DATA | DADOS
    Scope* Parent  =  nullptr;
    Scope* Next    =  nullptr;
    ASTNode* Owner;
    unord_map<string, Symbol*> Symbols;
    BodyTypes Type;

    // HELPERS

    // Find Symbol ONLY in THIS Scope
    // Encontra o Simbolo APENAS NESTE Escopo.
    Symbol* FindSymLocal(const string& sym)
    {
        for (auto& P : Symbols)
            if (P.first == sym)
                return P.second;
        return nullptr;
    }    

    // Find Symbol in Scopes | Encontra Simbolo no Escopo.
    Symbol* FindSym(const string& sym)
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
    Symbol* FindSymbol(const string& sym, SymbolTypes Type)
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
    int logInd  = 0;
    ui16 nextId = 1;

    struct {
        bool extensionDefined = false;
        bool libraryDefined   = false;
        bool methodDefined    = false;
    } Flags;
};

struct SAResult 
{
    Scope* GlobalScope = nullptr;
    unord_map<ASTNode*, TypeInfo> ExpressionTypes;
    unord_map<str_view, Symbol*> SymbolTable;
    unord_map<ui16, Symbol*> Symbols;
};

// MAIN CLASS | CLASSE PRINCIPAL.
class SemanticAnalizer
{
    public:
        SAResult InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory);
    private:

        void LookUpNode(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);

        void LookUpProgram(ProgramNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpBody(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Scope* S = nullptr);

        void LookUpLiteral(LiteralNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpUnary(UnaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpBinary(BinaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpAssignment(AssignmentNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpMemberAccess(MemberAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpIndexAccess(IndexAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpRange(RangeNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpFunctionCall(FunctionCall& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpArray(ArrayValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpTable(TableValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);

        void LookUpVarDecl(VarDeclNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpFunction(FnDecl& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpNameSpace(NameSpaceDecl& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);

        void LookUpIf(IfNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpElif(ElifNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpElse(ElseNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpWhile(WhileNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpFor(ForNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        //void LookUpForEach(ForEachNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        //void LookUpForDef(ForDefNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory);
        void LookUpReturn(ReturnNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpEcho(EchoNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
        void LookUpLibraryDef(LibraryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
};

// EOF