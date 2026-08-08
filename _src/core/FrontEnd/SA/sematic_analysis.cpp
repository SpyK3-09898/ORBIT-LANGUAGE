
// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o COdigo em Busca de Erros Semanticos.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "semantic_analysis.hpp" // HEADER FILE | CABEÇALHO.

#include "../parser/parser.hpp"
#include "../parser/AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <algorithm>
#include <cstddef>
#include <string>

// ========= UTILS ========== //

// Semantic Analysis Utils | Utilidades do Analisador.
namespace SAUtils {

    // Create A New Symbol | Cria um Novo Simbolo.
    Symbol* CreateSymbol(const string& Name, ASTNode& Node, SAState& State, SAResult& Res, Arena& Memory)
    {
        Symbol* Sym = Memory.New<Symbol>();
        TypeInfo* TInfo = Memory.New<TypeInfo>();

        Sym->Name = Name;
        Sym->DeclaredScope = State.CurrScope;
        Sym->TInfo = TInfo;

        return Sym;
    }

    string GetStringOfKind(TypeKind K)
    {
        switch (K)
        {
            case TypeKind::UNK:			return "<UNK>";
            case TypeKind::MONO_STATE:	return "<MONO_STATE>";

            case TypeKind::NUMBER:		return "<NUMBER>";
            case TypeKind::STRING:		return "<STRING>";
            case TypeKind::BOOL:		return "<BOOL>";
            case TypeKind::ANY:			return "<ANY>";
            case TypeKind::_NULL:		return "<NULL>";
            case TypeKind::NONE:		return "<NONE>";

            case TypeKind::ARRAY:		return "<ARRAY>";
            case TypeKind::TABLE:		return "<TABLE>";

            case TypeKind::TABLE_INT:	return "<TABLE_INT>";
            case TypeKind::TABLE_FLOAT:	return "<TABLE_FLOAT>";
            case TypeKind::TABLE_STRING:return "<TABLE_STRING>";
            case TypeKind::TABLE_BOOL:	return "<TABLE_BOOL>";
            case TypeKind::TABLE_ANY:	return "<TABLE_ANY>";
            case TypeKind::TABLE_NULL:	return "<TABLE_NULL>";
            case TypeKind::TABLE_NONE:	return "<TABLE_NONE>";

            case TypeKind::STRUCT:		return "<STRUCT>";
            case TypeKind::CLASS:		return "<CLASS>";

            case TypeKind::FN:			return "<FN>";

            default:					return "<UNKNOWN>";
        }
    }
}

// ========== CORE ========== //

// ===== PROGRAM ===== //

// Entry In A New Scope | Entra em um novo Escopo.
Scope* EntryScope(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Mem)
{
    Scope* S = Mem.New<Scope>();
    
    S->Parent = State.CurrScope;
    S->Type = Node.Type;
    S->Owner = &Node;

    State.CurrScope->Next = S;
    State.ScopeStack.push_back(S);

    return S;
}

// Leave The Current Scope | Sai do Escopo Atual.
Scope* LeaveScope(SAState& State, SAResult& Res, RunTimeData& Data)
{
    if (State.CurrScope->Type == BodyTypes::PROGRAM)
    {
        OrbitLog::Warn(
            "semantic_analisys.cpp", 
            "Trying to Close the GlobalScope In(line/index): ~/~"
        );
        return State.CurrScope;
    }

    State.CurrScope = State.ScopeStack.back();
    State.ScopeStack.pop_back();

    return State.CurrScope;
}

// LookUp A General Node | Olha Um Node Geral.
void SemanticAnalizer::LookUpNode(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    switch (Node.Type) {
        
        // PROGRAM
        case NodeType::PROGRAM:  
            LookUpProgram
            (static_cast<ProgramNode&>(Node), State, Res, Data, Memory);
            break;
        case NodeType::BODY:
            LookUpNode
            (static_cast<BodyNode&>(Node), State, Res, Data, Memory);
            break;

        // EXPRESSIONS

        case NodeType::IDENTIFIER:
            LookUpIdentifier
            (static_cast<IdentifierNode&>(Node), State, Res, Data, Memory);
            break;
        case NodeType::ARRAY_VALUE:
            LookUpArray
            (static_cast<ArrayValue&>(Node), State, Res, Data, Memory);
            break;
        case NodeType::TABLE_VALUE:
            LookUpTable
            (static_cast<TableValue&>(Node), State, Res, Data, Memory);
            break;             
        default: {};
    }
}

// LookUp A ProgramNode | Olha um ProgramNode.
void SemanticAnalizer::LookUpProgram(ProgramNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    Scope* GlobalScope = Memory.New<Scope>();
    
    GlobalScope->Owner = nullptr;
    GlobalScope->Type = BodyTypes::PROGRAM;

    State.ScopeStack.push_back(GlobalScope);
    State.CurrScope = GlobalScope;
}

// LookUp BodyNode | Olha um BodyBode.
void SemanticAnalizer::LookUpBody(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    EntryScope(Node, State, Res, Data, Memory);
    for (ASTNode* N : Node.Data)
        LookUpNode(*N, State, Res, Data, Memory);
    LeaveScope(State, Res, Data);
}

// ===== EXPRESSIONS ===== //

// Get Expression Types | Pega o Tipo das Expressoes.
TypeInfo*GetExpressionType(ExpressionNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    TypeInfo* TInfo = Memory.New<TypeInfo>();
    switch (Node.Type) {
    
        case NodeType::LITERAL:
        {
            LiteralNode& Lit = static_cast<LiteralNode&>(Node);

            if (holds_alt<i64>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SybTypeKind::INT;
            }
            else if (holds_alt<float>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SybTypeKind::FLOAT;
            }
            else if (holds_alt<str_view>(Lit.Value))
            {
                TInfo->Kind = TypeKind::STRING;
            }
            else if (holds_alt_value<bool>(Lit.Value, true))
            {
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SybTypeKind::TRUE;
            }
            else if (holds_alt_value<bool>(Lit.Value, false))
            {
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SybTypeKind::FALSE;
            }
            else if (holds_alt<NoneLitVal>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NONE;
            }
            else if (holds_alt<NullLitVal>(Lit.Value))
            {
                TInfo->Kind = TypeKind::_NULL;
            }
            else if (holds_alt<nullptr_t>(Lit.Value))
            {
                TInfo->Kind = TypeKind::MONO_STATE;
            }

            return TInfo;
        }

        case NodeType::ARRAY_VALUE:
            TInfo->Kind = TypeKind::ARRAY;
            break;
            
        case NodeType::IDENTIFIER:
        {
            IdentifierNode& Id = static_cast<IdentifierNode&>(Node);
            if (!State.CurrScope->FindSym(Id.Name))
            {
                OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
                "Used a Undeclared <IDENTIFIER>",
                "Ident: "+Id.Name+" Dont Exists",
                "Declare Ident or Use a Valid Identifier",
                Node.pos.line, Node.pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                    { TInfo->Kind = TypeKind::UNK; return TInfo; };
            }

            Symbol* Sym = State.CurrScope->FindSym(Id.Name);
            if (!Sym->inited)
                { TInfo->Kind = TypeKind::NONE; return TInfo; };
            
            Sym->read_count++;
            return Sym->TInfo;
        }

        default: {};
    }
    return TInfo;
}

// LookUp IdentifierNode | Olha um IdentifierNode.
void SemanticAnalizer::LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Error Prev | Prevenção de Erros.
    if (!State.CurrScope->FindSym(Node.Name))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Used a Undeclared <IDENTIFIER>",
            "Ident: "+Node.Name+" Dont Exists",
            "Declare Ident or Use a Valid Identifier",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
}

// LookUp Array Values | Olha um ArrayValue.
void SemanticAnalizer::LookUpArray(ArrayValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    for (ExpressionNode* Val : Node.Args)
        LookUpNode(*Val, State, Res, Data, Memory);
}

// LookUp Table Values | Olha um TableValue.
void SemanticAnalizer::LookUpTable(TableValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    for (ArrayEntry E : Node.Args)
    {
        // Error Prev | Prevenção de Erros:
        if 
        (
            GetExpressionType
            (*E.Key, State, Res, Data, Memory)
            ->Kind != TypeKind::STRING
        )
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Expected <STRING>, But Got: "+SAUtils::GetStringOfKind
                (GetExpressionType
                (*E.Key, State, Res, Data, Memory)
                ->Kind),
                "<ARRAY>s Need a <STRING> To Refer a Value",
                "Add a Valid Type or Convert",
                E.Key->pos.line, E.Key->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        }
        LookUpNode(*E.Value, State, Res, Data, Memory);
    }
}

// ========== ENTRY-POINT ========== //
// Entry-Point of SA Program | Ponto-de-Entrada do programa do SA.
SAResult SemanticAnalizer::InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: SemanticAnalysis");

    SAResult Res;
    SAState State;

    LookUpNode(*PRes.AST, State, Res, Data, Memory);

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: SemanticAnalysis. .. ...");
    
    return std::move(Res);
}
