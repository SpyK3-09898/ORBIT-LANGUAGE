
// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o COdigo em Busca de Erros Semanticos.
// Developed By: SpyK3(2026) | License: GitHub(MIT).


// INCLUDE HEADERS 'N DEPENDENCES
#include "semantic_analysis.hpp"

#include "../parser/parser.hpp"
#include "../parser/AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

#include <cstddef>
#include <filesystem>
#include <string>

// ======= CORE ======= //

// ===== UTILS ===== //

TypeInfo* SemanticAnalizer::GetExpressionType(ExpressionNode* Node)
{
    if (!Node)
        return nullptr;

    switch (Node->Type)
    {
        case NodeType::LITERAL:
        {
            LiteralNode* Lit = static_cast<LiteralNode*>(Node);

            TypeInfo* Type = M_Memory->New<TypeInfo>();

            if (holds_alternative<i64>(Lit->Value))
                Type->Kind = TypeKind::INT;

            else if (holds_alternative<float>(Lit->Value))
                Type->Kind = TypeKind::FLOAT;

            else if (holds_alternative<str_view>(Lit->Value))
                Type->Kind = TypeKind::STRING;

            else if (holds_alternative<bool>(Lit->Value))
                Type->Kind = TypeKind::BOOL;

            else if (holds_alternative<NoneLitVal>(Lit->Value))
                Type->Kind = TypeKind::NONE;

            else if (holds_alternative<NullLitVal>(Lit->Value))
                Type->Kind = TypeKind::NULLVAL;

            return Type;
        }


        case NodeType::IDENTIFIER:
        {
            IdentifierNode* Id = static_cast<IdentifierNode*>(Node);

            Symbol* Sym = SAState.CurrScope->LookUp(Id->Name);

            if (!Sym)
                return nullptr;

            return Sym->Type;
        }


        case NodeType::ARRAY_VALUE:
        {
            TypeInfo* Type = M_Memory->New<TypeInfo>();

            Type->Kind = TypeKind::ARRAY;

            return Type;
        }


        case NodeType::TABLE_VALUE:
        {
            TypeInfo* Type = M_Memory->New<TypeInfo>();

            Type->Kind = TypeKind::TABLE;

            return Type;
        }


        case NodeType::MEMBER_ACCESS:
        {
            MemberAccessNode* Access = static_cast<MemberAccessNode*>(Node);

            TypeInfo* Object = GetExpressionType(Access->Object);

            if (!Object)
                return nullptr;

            if (Object->Kind != TypeKind::STRUCT &&
                Object->Kind != TypeKind::CLASS)
                return nullptr;


            IdentifierNode* Member =
                static_cast<IdentifierNode*>(Access->Member);


            auto It = Object->Members.find(Member->Name);

            if (It == Object->Members.end())
                return nullptr;


            return It->second;
        }


        default:
            return nullptr;
    }
}

// ===== SCOPES ===== //

// Analize the Node | Analisa o Nó.
void SemanticAnalizer::Visit(ASTNode* Node)
{
    if (Node == nullptr)
        return;

    switch (Node->Type)
    {
        // PROGRAM
        case NodeType::PROGRAM:
            VisitProgram(static_cast<ProgramNode*>(Node));
            break;

        case NodeType::BODY:
            VisitBody(static_cast<BodyNode*>(Node));
            break;

        // DECLARATIONS
        case NodeType::VAR_DECL:
            VisitVarDecl(static_cast<VarDeclNode*>(Node));
            break;

        case NodeType::FN_DECL:
            VisitFnDecl(static_cast<FnDecl*>(Node));
            break;

        // CONTROL
        case NodeType::IF_CONTROL:
            VisitIf(static_cast<IfNode*>(Node));
            break;

        case NodeType::ELSE_CONTROL:
            VisitElse(static_cast<ElseNode*>(Node));
            break;

        case NodeType::ELIF_CONTROL:
            VisitElif(static_cast<ElifNode*>(Node));
            break;

        case NodeType::WHILE:
            VisitWhile(static_cast<WhileNode*>(Node));
            break;

        case NodeType::FOR:
            VisitFor(static_cast<ForNode*>(Node));
            break;

        case NodeType::FOR_EACH:
            VisitForEach(static_cast<ForEachNode*>(Node));
            break;

        case NodeType::FOR_DEF:
            VisitForDef(static_cast<ForDefNode*>(Node));
            break;

        case NodeType::RETURN:
            VisitReturn(static_cast<ReturnNode*>(Node));
            break;

        // EXPRESSIONS
        case NodeType::LITERAL:
            VisitLiteral(static_cast<LiteralNode*>(Node));
            break;

        case NodeType::IDENTIFIER:
            VisitIdentifier(static_cast<IdentifierNode*>(Node));
            break;

        case NodeType::UNARY:
            VisitUnary(static_cast<UnaryNode*>(Node));
            break;

        case NodeType::BINARY:
            VisitBinary(static_cast<BinaryNode*>(Node));
            break;

        case NodeType::ASSIGNMENT:
            VisitAssignment(static_cast<AssignmentNode*>(Node));
            break;

        case NodeType::MEMBER_ACCESS:
            VisitMemberAccess(static_cast<MemberAccessNode*>(Node));
            break;

        case NodeType::INDEX_ACCESS:
            VisitIndexAccess(static_cast<IndexAccessNode*>(Node));
            break;

        case NodeType::FN_CALL:
            VisitFunctionCall(static_cast<FunctionCall*>(Node));
            break;

        case NodeType::TABLE_VALUE:
            VisitTable(static_cast<TableValue*>(Node));
            break;

        case NodeType::ARRAY_VALUE:
            VisitArray(static_cast<ArrayValue*>(Node));
            break;

        case NodeType::RANGE:
            VisitRange(static_cast<RangeNode*>(Node));
            break;

        // ERRORS
        case NodeType::ERROR:
            break;
    }
}

// Entry in a New Scope | Entra em Um Novo Escopo.
void SemanticAnalizer::EnterScope(BodyTypes Kind)
{
    Scope* New = M_Memory->New<Scope>();

    New->Parent = SAState.CurrScope;
    New->Kind = Kind;

    SAState.currScopeLvl++;
    SAState.CurrScope = New;
}

// Back to a Father Scope | Volta Para o Escopo pai.
void SemanticAnalizer::LeaveScope(BodyTypes Kind)
{
    Scope* Curr = SAState.CurrScope;
    if (!Curr)
        OrbitLog::Error("semantic_analysis.cpp", "Try to Back to a Null Scope", true);
    SAState.CurrScope = Curr;
    SAState.currScopeLvl--;
}

// ===== VISITS ===== //

// --- PROGRAM --- //

// PROGRAM NODE | NO DE PROGRAMAS 
void SemanticAnalizer::VisitProgram(ProgramNode* Node)
{
    for (ASTNode* Child : Node->Node->Data)
    {
        SemanticAnalizer::Visit(Child);
    }
}

// BODY NODE | NO DE BODYS
void SemanticAnalizer::VisitBody(BodyNode* Node)
{
    for(ASTNode* Child : Node->Data)
    {
        Visit(Child);
    }
}

// --- DECLARATION --- //

// Visit Var Declarations | Visita Criações de Variaveis.
void SemanticAnalizer::VisitVarDecl(VarDeclNode* Node)
{
    // Error Prev | Prevenção de Erros
    if(SAState.CurrScope->LookUp(Node->Name))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "<IDENTIFIER> Already Exists",
            "Ident: "+Node->Name+" Already Exists, First Declareated Here: "
            +std::to_string(SAState.CurrScope->LookUp(Node->Name)->Pos.line)
            +":"+std::to_string(SAState.CurrScope->LookUp(Node->Name)->Pos.collumn),
            "Add a Diferent Name, Ex: "+Node->Name+"2",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return ;
    }

    // Validate Expression and Create Symbol | Valida Expressoes e Cria o Simbolo.
    bool inited=false;
    if (Node->Val)
        { VisitExpression(Node->Val); inited=true; }
    Symbol* S = M_Memory->New<Symbol>();
    S->initialized = inited;

    // Set Symbol | Define o Simbolo.
    S->Mut  = Node->MutType;
    S->Pos  = Node->pos;
    S->name = Node->Name;

    // Finalize | Finaliza.
    SAState.CurrScope->Symbols[S->name] = S;
}

// Visit Identifiers Using | Visita Uso de Indentificadores.
void SemanticAnalizer::VisitIdentifier(IdentifierNode* Node)
{
    Symbol* Sym = SAState.CurrScope->LookUp(Node->Name);
    if (!Sym)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Use a Undefined <IDENTIFIER>",
            "Ident: "+Node->Name+" Non Exists",
            "Create or Change the Symbol",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    Sym->read_count++;
}

// Visit Member Acess Using | Visita Acesso de membros.
void SemanticAnalizer::VisitMemberAccess(MemberAccessNode* Node, bool isBase)
{
    // ERROR PREV | PREVENÇÃO DE ERROS
    if (!holds_alt<IValueTypes>(Node->Member))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Acess Using A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Member->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    if (!holds_alt<IValueTypes>(Node->Object))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Acess A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Member->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    // SET | DEFINE
    if (Node->Member->Type == NodeType::MEMBER_ACCESS)
    {
        MemberAccessNode* Mb = static_cast<MemberAccessNode*>(Node->Member);
        VisitMemberAccess(Mb, false);
    }
    if (isBase)
    {
        IdentifierNode* Id = static_cast<IdentifierNode*>(Node->Object);
        VisitIdentifier(Id);
    }
}

// Visit Index Acess Using | Visita o Acesso Per Indice
void SemanticAnalizer::VisitIndexAccess(IndexAccessNode* Node, bool isAssign)
{
    // ERROR PREV | PREVENÇÃO DE ERROS.
    if (!holds_alt<IValueTypes>(Node->Object))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Assign A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Object->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    if ()
}

// Visit Assign Nodes | Visita Nós de Atribuição.
void SemanticAnalizer::VisitAssignment(AssignmentNode* Node)
{
    // ERROR PREV | PREVENÇÃO DE ERROS.
    if (holds_alt<IValueTypes>(Node->Left->Type))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Assign A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Left->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }

    // ANALIZE | ANALIZA
    if (Node->Left->Type == NodeType::MEMBER_ACCESS)
        VisitMemberAccess(static_cast<MemberAccessNode*>(Node->Left));
    else if (Node->Left->Type == NodeType::INDEX_ACCESS)
        VisitIndexAccess(static_cast<IndexAccessNode*>(Node->Left));    
    else {
        Symbol* Sym = SAState.CurrScope->LookUp(
            static_cast<IdentifierNode*>(Node->Left)->Name
        );

        Sym->write_count++;
        Sym->initialized = true;
    }
    VisitExpression(Node->Right);
}

// ======= ENTRY-POINT ======= //
// Entry-Point of SA Program | Ponto-de-Entrada do Programa de AS
SAResult SemanticAnalizer::InitSA(ParseResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: Semantic Analizing");

    M_Memory = &Memory;
    M_Data = &Data;

    SARes = SAResult{};
    
    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: Semantic Analizing. .. ...");
    return SARes;
}