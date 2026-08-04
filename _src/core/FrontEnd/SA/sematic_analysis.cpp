
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

// ===== UTILS ===== //

namespace SAUtils {

    // Create A New Scope | Cria Um Novo Escopo.
    Scope* EntryScope(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
    {
        // Create Scope | Cria o Escopo
        Scope* S = Memory.New<Scope>();
        S->Owner = &Node;
        
        // Set Scope | Define o Escopo.
        State.CurrScope = S;
        State.ScopeStack.back()->Next = S;
        S->Parent = State.ScopeStack.back();
        State.ScopeStack.push_back(S);

        // Return.
        return S;
    }

    // Remove The Last Scope | Remove o Ultimo Escopo.
    Scope* LeaveScope(SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
    {

        // Take Last | Pega o Ultimo.
        Scope* S = State.CurrScope;
        if (S->Owner->Type == NodeType::PROGRAM)
        {
            OrbitLog::Warn("semantic_analysis.cpp", "Try To Close the <GLOBAL> Scope");
            return S;
        }
        S->Next = nullptr;

        // Updt Stack | Atualiza a Stack.
        State.ScopeStack.pop_back();
        State.CurrScope = State.ScopeStack.back();

        // Return.
        return State.CurrScope;
    }
}

// ===== NODES ====== //

// --- PROGRAM --- //

// LookUp Any Node Type | Checa Qualquer Tipo de Nó.
void SemanticAnalizer::LookUpNode(
    ASTNode& Node, 
    SAState& State, 
    SAResult& Res,
    RunTimeData& Data,
    Arena& Memory    
)
{
    switch (Node.Type) {
        
        case NodeType::PROGRAM: 
            LookUpProgram(static_cast<ProgramNode&>(Node), State, Res, Data, Memory);
            break;    
        case NodeType::BODY:
            LookUpBody(static_cast<BodyNode&>(Node), State, Res, Data, Memory);
            break;

        default: {};
    }
}

// LookUp Entry Point | Checa o Ponto de Entrada.
void SemanticAnalizer::LookUpProgram(
    ProgramNode& Node, 
    SAState& State, 
    SAResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    // Create Global Scope | Cria o Escopo Global.
    Res.GlobalScope = Memory.New<Scope>();
    Res.GlobalScope->Owner = &Node; // Define State | Define o Estado:
    State.ScopeStack.push_back(Res.GlobalScope);
    State.CurrScope = Res.GlobalScope;

    // LookUp AST
    for (ASTNode* N : Node.Node->Data)
        LookUpNode(*N, State, Res, Data, Memory);
}

// LookUp Body Nodes | Checa os BodyNodes.
void SemanticAnalizer::LookUpBody(
    BodyNode& Node, 
    SAState& State, 
    SAResult& Res,
    RunTimeData& Data,
    Arena& Memory    
)
{
    SAUtils::EntryScope(Node,State, Res, Data, Memory); 
    for (ASTNode* Node : Node.Data)
        LookUpNode(*Node, State, Res, Data, Memory);
    SAUtils::LeaveScope(State, Res, Data, Memory);
}

// --- EXPRESSION --- //

void SemanticAnalizer::LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (!State.CurrScope->FindSym(Node.Name)) 
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Unknown <IDENTIFIER>",
            "Used of Undeclared <IDENTIFIER>: "+Node.Name,
            "Create Or Change Identifier Name",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
}

// ======= ENTRY-POINT | PONTO-DE-ENTRADA ======= //
SAResult SemanticAnalizer::InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: SemanticAnalysis");

    SAResult Res;
    SAState State;
    
    LookUpNode(*PRes.AST, State, Res, Data, Memory);

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: SemanticAnalysis. .. ...");
    
    return Res;
}