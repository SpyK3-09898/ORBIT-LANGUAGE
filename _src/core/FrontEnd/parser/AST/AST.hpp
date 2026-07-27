
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
#include <cstdint>
#include <utility>

// ======= INSTRUCTIOSN ======= //

// None Value/Null Value | Valor Nulo e Noni
struct NoneLitVal{};
struct NullLitVal{};

// Instructions Rep | Representação de Instruções.
struct Instruction
{
    vec<Token*> Modifiers;
    vec<Token*> Tokens;

    struct 
    { int modCurr=0; int curr=0; } pos;
    Token* Advance() // Advance to the Next Token | Avança para o Proximo Token.
    {
        if (pos.curr >= Tokens.size())
            return nullptr;

        return Tokens[pos.curr++];
    }
    Token* Peek() // Check The Next Token And Not Consumes | Olha o Proximo Token e Não Consome.
    {
        if (pos.curr >= Tokens.size())
            return nullptr;

        return Tokens[pos.curr];
    }
};
using InstVec = vec<Instruction>;

// ======= PREV ======= //

struct ASTNode;

struct BodyNode;
struct ProgramNode;

struct ExpressionNode;
struct LiteralNode;
struct IdentifierNode;
struct UnaryNode;
struct BinaryNode;
struct AssignmentNode;
struct MemberAccessNode;
struct IndexAccessNode;
struct RangeNode;
struct ErrorExprNode;

struct DeclarationNode;
struct VarDeclNode;
struct ErrorDeclNode;

struct ControlNode;
struct ElseNode;
struct ElifNode;
struct IfNode;
struct ErrorStmtNode;

// ======= NODES ======= //

// ENUMS.

// Typeof Nodes | Tipo dos Nos(Obvio).
enum class NodeType : uint8_t
{
    // PROGRAM
    PROGRAM,
    BODY,
    ERROR,

    // CONTROL
    IF_CONTROL,
    ELSE_CONTROL,
    ELIF_CONTROL,

    // DECLARATIONS
    VAR_DECL,

    // EXPRESSIONS
    LITERAL,
    IDENTIFIER,

    UNARY,
    BINARY,
    ASSIGNMENT,

    MEMBER_ACCESS,
    INDEX_ACCESS,
    CALL,

    RANGE
};

// Type of Body Insts | Tipos de Instruções de Corpo.
enum class BodyTypes
{
    PROGRAM,
    CONTROL_IF,
    FUNCTION,
    LOOP_WHILE,
    LOOP_FOR,
    OTHER
};

// Mutable Values Types | Valores de Tipos Mutaveis.
enum class MutableTypes : uint8_t
{
    MUT,
    CONST,
    UNK
};

// Literal Value Types | Tipos de Valores Literais.
enum class LiteralTypes: uint8_t
{
    INT,
    FLOAT,
    STRING,
    BOOL,
    NONE,
    _NULL
};

// Math Operators | Operadores de Matematica.
enum class Operator: uint8_t
{
    // ARITMETIC | ARITMETICOS.
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POWER,

    // COMP | COMPARAÇOES.
    EQUAL,
    NOT_EQUAL,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,

    // LOGICAL | LOGICOS.
    AND,
    OR,
    NOT,

    // ASSIGN | ASSIGNAÇÃO.
    ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
    POWER_ASSIGN,

    NONE
};

// STRUCTS.


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

// UTIL
inline NodePos MakePosFromToken(Token* Tok)
{  
    return NodePos{
        Tok->pos.indent,
        Tok->pos.start,
        Tok->pos.len,
        Tok->pos.line,
        Tok->pos.collumn
    };
}

// ======= PROGRAM ====== //

// Stack of Nodes | Pilha de Nodes
struct BodyNode : ASTNode
{
    // DATA
    vec<ASTNode*> Data{};
    ASTNode* Father;
    BodyTypes Type;

    // CONSTRUCTOR | CONSTRUTOR
    BodyNode(NodePos P)
        : ASTNode(NodeType::BODY, P) {};
};

// ENTRY-POINT NODE | NO DE PONTO-DE-ENTRADA
struct ProgramNode : ASTNode
{
    // DATA
    BodyNode* Node;

    // CONSTRUCTOR | CONSTRUTOR
    ProgramNode(NodePos P, Arena& Memory)
        : ASTNode(NodeType::PROGRAM, P),
          Node(Memory.New<BodyNode>(P))
    {};
};

// ======= EXPRESSIONS ======= //

// Value of Literals | Valor dos Literais
using LiteralValue = variant<
    i64,
    float,
    str_view,
    bool,
    NoneLitVal,
    NullLitVal,
    nullptr_t
>;

// Base Expression Node | Node Base de Expressão
struct ExpressionNode : ASTNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ExpressionNode(NodeType T, NodePos P)
        : ASTNode(T, P) {};
};

struct LiteralNode : ExpressionNode
{
    // DATA
    LiteralValue Value;

    // CONSTRUCTOR | CONSTRUTOR
    LiteralNode(NodePos P)
        : ExpressionNode(NodeType::LITERAL, P) {};
};

// IDENTIFIER | Identificador
struct IdentifierNode : ExpressionNode
{
    // DATA
    str_view Name;

    // CONSTRUCTOR | CONSTRUTOR
    IdentifierNode(NodePos P)
        : ExpressionNode(NodeType::IDENTIFIER, P) {};
};

// UNARY EXPRESSION | Expressão Unária
struct UnaryNode : ExpressionNode
{
    // DATA
    Token* Operator;
    ExpressionNode* Operand;

    // CONSTRUCTOR | CONSTRUTOR
    UnaryNode(NodePos P)
        : ExpressionNode(NodeType::UNARY, P) {};
};

// BINARY EXPRESSION | Expressão Binária
struct BinaryNode : ExpressionNode
{
    // DATA
    Operator Op;

    ExpressionNode* L;
    ExpressionNode* R;

    // CONSTRUCTOR | CONSTRUTOR
    BinaryNode(NodePos P)
        : ExpressionNode(NodeType::BINARY, P) {};
};

// ASSIGNMENT EXPRESSION | Expressão de Atribuição
struct AssignmentNode : ExpressionNode
{
    // DATA
    Token* Operator;

    ExpressionNode* Left;
    ExpressionNode* Right;

    // CONSTRUCTOR | CONSTRUTOR
    AssignmentNode(NodePos P)
        : ExpressionNode(NodeType::ASSIGNMENT, P) {};
};

// MEMBER ACCESS | Acesso de Membro
struct MemberAccessNode : ExpressionNode
{
    // DATA
    ExpressionNode* Object;
    Token* Member;

    // CONSTRUCTOR | CONSTRUTOR
    MemberAccessNode(NodePos P)
        : ExpressionNode(NodeType::MEMBER_ACCESS, P) {};
};

// INDEX ACCESS | Acesso por Índice
struct IndexAccessNode : ExpressionNode
{
    // DATA
    ExpressionNode* Object;
    ExpressionNode* Index;

    // CONSTRUCTOR | CONSTRUTOR
    IndexAccessNode(NodePos P)
        : ExpressionNode(NodeType::INDEX_ACCESS, P) {};
};

// RANGE EXPRESSION | Expressão de Intervalo
struct RangeNode : ExpressionNode
{
    // DATA
    ExpressionNode* Begin;
    ExpressionNode* End;

    // CONSTRUCTOR | CONSTRUTOR
    RangeNode(NodePos P)
        : ExpressionNode(NodeType::RANGE, P) {};
};

// ERRORS | ERROS
struct ErrorExprNode : ExpressionNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ErrorExprNode(NodePos P)
        : ExpressionNode(NodeType::ERROR, P) {};
};

// ======= DECLARATION ======== //

// Base Decl Node | No de Decl Base
struct DeclarationNode : ASTNode
{
    // CONSTRUCTOR | COSBNTRUTOR 
    DeclarationNode(NodeType T, NodePos P)
        : ASTNode(T, P) {};
};

// Var Decl | Declaração de Variaveis
struct VarDeclNode : DeclarationNode
{
    // DATA
    str_view Name;
    LiteralTypes InferType;
    MutableTypes MutType;
    ExpressionNode* Val;

    // CONSTRUCTOR | CONSTRUTOR
    VarDeclNode(NodePos P)
        : DeclarationNode(NodeType::VAR_DECL, P) {}
};

// ERRORS | ERROS
struct ErrorDeclNode : DeclarationNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ErrorDeclNode(NodePos Pos)
        : DeclarationNode(NodeType::ERROR, Pos) {};
};

// ======== STATEMENTS ========= //

// Control Statement Base Node | No de Instrução de Controle Base.
struct ControlNode : ASTNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ControlNode(NodeType T, NodePos P)
        : ASTNode(T, P) {}
};


// Else Control Statement | Controle de Instrução 'Else'.
struct ElseNode : ControlNode
{
    // DATA.
    BodyNode* Body;

    // CONSTRUCTOR | CONSTRUTOR.
    ElseNode(NodePos P)
        : ControlNode(NodeType::ELSE_CONTROL, P) {};
};

// IElif Control Statement | Controle de Instrução 'Elif'.
struct ElifNode : ControlNode
{
    // DATA
    ExpressionNode* Cond;
    BodyNode* Body;

    // CONSTRUCTOR | CONSTRUTOR
    ElifNode(NodePos P)
        : ControlNode(NodeType::ELIF_CONTROL, P) {};
};

// If Control Statement | Controle de Instrução 'Ifs'.
struct IfNode : ControlNode
{
    // DATA
    ExpressionNode* Cond;
    BodyNode* IfBody;
    ElseNode* ElseBody;
    vec<BodyNode*> ElifBodyStack;

    // CONSTRUCTOR | CONSTRUTOR
    IfNode(NodePos P)
        : ControlNode(NodeType::IF_CONTROL, P) {};
};

// Error Statements Nodes | Errors de Nos de Statement.
struct ErrorStmtNode : ControlNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ErrorStmtNode(NodePos P)
        : ControlNode(NodeType::ERROR, P) {};
};

// ======== AST ======== //

// Curr State of Parser | Estado Atual do Parser
struct ParseState
{
    int lastIndent=0;
    bool SingleStatement = false;
    NodePos Pos; // Current Pos of Parser | Posição Atual do Lexer.
    vec<BodyNode*> Bodys;
    BodyNode* CurrBody;
};

// Result Of Parser | Resultado do Parser
struct ParseResult
{
    ASTNode* AST;
};

// ========== NAMESPACES ========= //

namespace ParserUtils {

    // Add A New Elem in BodyStack | Adiciona um Novo Elemento na Pilha de Bodys.
    inline void UpdateBodyStack(BodyNode* Node, ParseState& State, RunTimeData& Data)
    {
        State.Bodys.push_back(Node);
        State.CurrBody = State.Bodys.back();
        State.lastIndent = Node->pos.indent;
    }

    // Pop The Body Stack | Retira o Ultimo Elemento da Pilha de Body.
    inline void PopBodyStack(ParseState& State, RunTimeData& Data)
    {
        if (State.Bodys.size() == 1)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Pop <STACK> Fail",
                "Attemp to Close <GLOBAL_STACK>",
                "~",
                State.Pos.line,
                State.Pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
        } else {
            State.Bodys.pop_back();
            State.CurrBody = State.Bodys.back();
        }
    }

    // Update State Position | Atualiza a Pos do Node
    inline void UpdateStatePos(variant<Token*, ASTNode*> Val, ParseState& State)
    {
        if (std::holds_alternative<ASTNode*>(Val))
        {
            State.Pos = std::get<ASTNode*>(Val)->pos;
        }
        else
        {
            State.Pos = MakePosFromToken(std::get<Token*>(Val));
        }
    }

    // Add A Statement | Adiciona a Instrução.
    template<typename T>
    void AddInst(ASTNode* Node, ParseState& State, ParseResult& Res, Arena& Memory)
    {
        State.CurrBody->Data.push_back(Node);
    }

    // Create A New Node | Cria Um Node.
    template<typename T>
    inline T* MakeNode(ParseState& State, ParseResult& Res, Arena& Memory)
    {
        T* Node = Memory.New<T>(State.Pos);
        return Node;
    }

    namespace Comm {
        
        // Return the Typeof Explicit Type Assign
        // Retorna o Tipo dos Tipos Explicitamente Infernidos
        inline pair<LiteralTypes, int> InferType(vec<Token*>& Tokens, RunTimeData& Data)
        {
            pair<LiteralTypes, int> DEF_RET = {LiteralTypes::_NULL, 2};
            if (Tokens.size() == 0)
                return DEF_RET;
            if (Tokens[1]->Type is TokenType::COLON)
            {
                if (Tokens.size() == 1) 
                    return {LiteralTypes::_NULL, 1};
                else
                    if (Tokens[2]->Type is_not TokenType::LIT_TYPE)
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Parsing",
                            "<LIT_TYPE> Expected After ':'",
                            "Expected <LIT_TYPE> Afte ': ' But Found: "+Tokens[2]->GetType(),
                            "Complete the Statement or Delete ';'",
                            Tokens[2]->pos.line,
                            Tokens[2]->pos.collumn
                        );
                        if (!Data.flags.debugMode)
                        {
                            OrbitLog::SyntaxLog::ThrowLog(Data);
                            return DEF_RET;
                        }
                        string Lex = Tokens[2]->Lexeme(Data);
                        if (Lex is "Int") return {LiteralTypes::INT, 2};
                        else if (Lex is "Float") return {LiteralTypes::FLOAT, 2};
                        else if (Lex is "Bool") return {LiteralTypes::BOOL, 2};
                        else if (Lex is "String") return {LiteralTypes::STRING, 2};
                        else if (Lex is "None") return {LiteralTypes::NONE, 2};
                        else if (Lex is "Null") return {LiteralTypes::_NULL, 2};  
                    }
            } else
                return DEF_RET;
            return DEF_RET;
        }
    }
}