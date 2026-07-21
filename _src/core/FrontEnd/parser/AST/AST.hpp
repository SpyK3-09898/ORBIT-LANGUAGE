
// ============ AST ========== //
// ABSTRACT SYNTAX TREE | ARVORE SINTATICA ABSTRATA
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../RunTimeData.hpp"

// ======= INSTRUCTIOSN ======= //

struct Instruction
{
    vec<Token*> Modifiers;
    vec<Token*> Tokens;
};

using InstVec = vec<Instruction>;
// ======= NODES ======= //

// Typeof Nodes | Tipo dos Nos(Obvio).
enum class NodeType: uint8_t
{
    BODY
};

// Pos of Nodes | Posição dos Nodes
struct NodePos
{
    int indent = 0;
    ui32 start = 0;
    ui32 len = 0;

    ui32 line = 0;
    ui32 collumn = 0;
};

// Bas AST Node | No de AST base
struct ASTNode
{
    // DATA
    NodePos pos;
    NodeType Type;

    // CONSTRUCTOR | COSNTRUTOR
    ASTNode(NodeType T, NodePos P)
        : pos(P), Type(T) {};
};

// PROGRAM

// Stack of Nodes | Pilha de Nodes
struct BodyNode : ASTNode
{
    // DATA
    vec<ASTNode> Data;

    // CONSTRUCTOR | CONSTRUTOR
    BodyNode(NodePos P)
        : ASTNode(NodeType::BODY, P) {};
};

// ======= AST ======= //

// Curr State of Parser | Estado Atual do Parser
struct ParseState
{
    NodePos Pos; // Current Pos of Parser | Posição Atual do Lexer.
    vec<uniq_ptr<BodyNode>> Bodys;
    BodyNode* CurrBody;
};

// Result Of Parser | Resultado do Parser
struct ParseResult
{
    uniq_ptr<ASTNode> AST;
};

// ========== NAMESPACES ========= //

namespace ParserUtils {

    // Add A New Elem in BodyStack | Adiciona um Novo Elemento na Pilha de Bodys.
    inline void UpdateBodyStack(BodyNode& Node, ParseState& State, RunTimeData& Data)
    {
        if (State.Bodys.size() == 1) {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <SCOPE> Closing", 
                "Trying to Close a <GLOBAL_SCOPE>", 
                "Remove <END>",
                Node.pos.line,
                Node.pos.collumn
            );
            if (Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
        } else {
            State.Bodys.push_back(make_uniq<BodyNode>(Node));
            State.CurrBody = State.Bodys.back().get();
        }
    }

    // Pop The Body Stack | Retira o Ultimo Elemento da Pilha de Body.
    inline void PopBodyStack(ParseState& State)
    {
        State.Bodys.pop_back();
        State.CurrBody = State.Bodys.back().get();

    }

    // Update State Position | Atualiza a Pos do Node
    inline void UpdateStatePos(ASTNode& Node, ParseState& State)
    {
        State.Pos = Node.pos;
    }

    // Create A New Node | Cria Um Node.
    template<typename T>
    inline T* MakeNode(NodeType Type, ParseState& State, ParseResult& Res, Arena& Memory)
    {
        T* Node = Memory.New<T>(Type, State.Pos);
        State.CurrBody->Data.push_back(Node);

        return Node;
    }
}