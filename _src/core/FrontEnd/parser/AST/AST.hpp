
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
#include "../../../Library/lib_manager.hpp"
#include <cstdint>
#include <cstdio>
#include <stdio.h>
#include <utility>

// ======= INSTRUCTIONS ======= //

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

// ENUMS

enum class NodeType : uint8_t;
enum class BodyTypes : uint8_t;

enum class LoopTypes : uint8_t;
enum class MutableTypes : uint8_t;
enum class LiteralTypes : uint8_t;

enum class Operator : uint8_t;

// NODES

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
struct ReturnNode;
struct FnDecl;
struct ErrorDeclNode;

struct ControlNode;
struct ElseNode;
struct ElifNode;
struct IfNode;
struct WhileNode;
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

    WHILE,
    FOR,
    FOR_EACH,
    FOR_DEF,

    RETURN,
    ECHO,
    ACESS_CHANGE,

    IMPORT,
    LIBRARY,
    METHOD,

    // DECLARATIONS
    VAR_DECL,
    FN_DECL,
    NAMESPACE_DECL,
    TYPEDEF_DECL,
    CLASS_DECL,
    STRUCT_DECL,

    // EXPRESSIONS
    LITERAL,
    IDENTIFIER,

    UNARY,
    BINARY,
    ASSIGNMENT,

    MEMBER_ACCESS,
    INDEX_ACCESS,
    FN_CALL,
    TABLE_VALUE,
    ARRAY_VALUE,

    RANGE
};

// Type of Body Insts | Tipos de Instruções de Corpo.
enum class BodyTypes: uint8_t
{
    PROGRAM,
    CONTROL_IF,
    FUNCTION,
    NAMESPACE,
    STRUCT,
    CLASS,
    LOOP_WHILE,
    LOOP_FOR,
    OTHER
};

// Typeof Loop | Tipos de Loop
enum class LoopTypes: uint8_t
{
    WHILE,
    FOR,
    EACH
};

// Mutable Values Types | Valores de Tipos Mutaveis.
enum class MutableTypes : uint8_t
{
    MUT,
    CONST,
    UNK
};

// Object Acess Restrict.
enum class AcessTypes: uint8_t
{
    NONE,
    PRIVATE,
    PUBLIC
};

// Literal Value Types | Tipos de Valores Literais.
enum class LiteralTypes: uint8_t
{
    MONO_STATE,

    INT,
    FLOAT,
    STRING,
    BOOL,
    NONE,
    _NULL,

    ARRAY_INT,
    ARRAY_FLOAT,
    ARRAY_STRING,
    ARRAY_BOOL,
    ARRAY_NONE,
    ARRAY_NULL,

    TABLE_INT,
    TABLE_FLOAT,
    TABLE_STRING,
    TABLE_BOOL,
    TABLE_NONE,
    TABLE_NULL,   
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

// Typeof Echo's | Tipo dos Ecos.
enum class EchoTypes 
{
    COUT, CERR, CWARN
};


// STRUCTS.
struct SAResult;

// Pos of Nodes | Posição dos Nodes
using NodePos = CodePosition;

// Bas AST Node | No de AST base
struct ASTNode
{
    // DATA
    NodePos pos;
    NodeType Type;
    ui16 SymbolId=0;
    bool export_decl=false;

    // UTILS | UTILIDADES
    inline string GetNodeType()
    {
        switch (Type)
        {
            case NodeType::PROGRAM:        return "<PROGRAM>";
            case NodeType::BODY:           return "<BODY>";
            case NodeType::ERROR:          return "<ERROR>";

            case NodeType::IF_CONTROL:     return "<IF_CONTROL>";
            case NodeType::ELSE_CONTROL:   return "<ELSE_CONTROL>";
            case NodeType::ELIF_CONTROL:   return "<ELIF_CONTROL>";

            case NodeType::WHILE:          return "<WHILE>";
            case NodeType::FOR:            return "<FOR>";
            case NodeType::FOR_EACH:       return "<FOR_EACH>";
            case NodeType::FOR_DEF:        return "<FOR_DEF>";

            case NodeType::RETURN:         return "<RETURN>";
            case NodeType::ECHO:           return "<ECHO>";
            case NodeType::ACESS_CHANGE:   return "<ACESS-NODE>";

            case NodeType::IMPORT:         return "<IMPORT>";
            case NodeType::LIBRARY:        return "<LIBRARY>";
            case NodeType::METHOD:         return "<METHOD>";

            case NodeType::VAR_DECL:       return "<VAR_DECL>";
            case NodeType::FN_DECL:        return "<FN_DECL>";
            case NodeType::NAMESPACE_DECL: return "<NAMESPACE>";
            case NodeType::TYPEDEF_DECL:   return "<TYPE-DEF>";
            case NodeType::STRUCT_DECL:    return "<STRUCT-DECL>";
            case NodeType::CLASS_DECL:     return "<CLASS-DECL>";

            case NodeType::LITERAL:        return "<LITERAL>";
            case NodeType::IDENTIFIER:     return "<IDENTIFIER>";

            case NodeType::UNARY:          return "<UNARY>";
            case NodeType::BINARY:         return "<BINARY>";
            case NodeType::ASSIGNMENT:     return "<ASSIGNMENT>";

            case NodeType::MEMBER_ACCESS:  return "<MEMBER_ACCESS>";
            case NodeType::INDEX_ACCESS:   return "<INDEX_ACCESS>";
            case NodeType::FN_CALL:        return "<FN_CALL>";
            case NodeType::TABLE_VALUE:    return "<TABLE_VALUE>";
            case NodeType::ARRAY_VALUE:    return "<ARRAY_VALUE>";

            case NodeType::RANGE:          return "<RANGE>";
        }

        return "<UNKNOWN>";
    }

    // CONSTRUCTOR | CONSTRUTOR
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
    string,
    bool,
    NoneLitVal,
    NullLitVal,
    nullptr_t
>;
struct ArrayEntry // Entry of Array | Entrada da Tabela.
{
    ExpressionNode* Key;
    ExpressionNode* Value;
};


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
    string Name;

    // CONSTRUCTOR | CONSTRUTOR
    IdentifierNode(NodePos P)
        : ExpressionNode(NodeType::IDENTIFIER, P) {};
};

// UNARY EXPRESSION | Expressão Unária
struct UnaryNode : ExpressionNode
{
    // DATA
    Operator Operator;
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
    ExpressionNode* Member;

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
    bool fallback=false;

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

// FN CALL | Chamada de Função
struct FunctionCall : ExpressionNode
{
    // DATA
    ExpressionNode* Callee;
    vec<ExpressionNode*> Args;

    // CONSTRUCTOR | CONSTRUTOR
    FunctionCall(NodePos P)
        : ExpressionNode(NodeType::FN_CALL, P) {};
};

// ARRAY LITERAL | Tabelas Literais.
struct ArrayValue : ExpressionNode
{
    // DATA
    vec<ExpressionNode*> Args;

    // CONSTRUTOR | CONSTRUCTOR
    ArrayValue(NodePos P)
        : ExpressionNode(NodeType::ARRAY_VALUE, P) {};
};

// TABLE LITERAL | Tabelas.
struct TableValue : ExpressionNode
{
    // DATA
    vec<ArrayEntry> Args;

    // CONSTRUTOR | CONSTRUCTOR
    TableValue(NodePos P)
        : ExpressionNode(NodeType::TABLE_VALUE, P) {};
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
    // DATA | DADOS.
    AcessTypes AcessType = AcessTypes::NONE;
    bool isUDT=false;
    
    // CONSTRUCTOR | CONSTRUTOR 
    DeclarationNode(NodeType T, NodePos P)
        : ASTNode(T, P) {};
};

// Var Decl | Declaração de Variaveis
struct VarDeclNode : DeclarationNode
{
    // DATA
    string Name;
    LiteralTypes InferType = LiteralTypes::MONO_STATE;
    MutableTypes MutType;
    ExpressionNode* Val;
    bool probablyObj=false;

    // CONSTRUCTOR | CONSTRUTOR
    VarDeclNode(NodePos P)
        : DeclarationNode(NodeType::VAR_DECL, P) {}
};

// Functions Decl | Declaração de Função.
struct FnDecl : DeclarationNode
{
    bool haveReturn=false;
    string Name;
    BodyNode* Body;
    vec<ExpressionNode*> Params;

    // CONSTRUCTOR | CONSTRUTOR
    FnDecl(NodePos P)
        : DeclarationNode(NodeType::FN_DECL, P) {};
};

// Namespace Decl | Declarações de NameSpace.
struct NameSpaceDecl : DeclarationNode
{
    // DATA
    string Name;
    BodyNode* Body;
    bool isNested=false;

    // CONSTRUCTOR | CONSTRUTOR
    NameSpaceDecl(NodePos& P)
        : DeclarationNode(NodeType::NAMESPACE_DECL, P) {};
};

// Class Decl | Declarações de Classes.
struct StructDeclNode : DeclarationNode
{
    // DATA | DADOS.
    BodyNode* Body;
    ExpressionNode* Extend;
    string Name;

    // CONSTRUCTOR | CONSTRUTOR
    StructDeclNode(NodePos& P)
        : DeclarationNode(NodeType::STRUCT_DECL, P) {};    
};

// Class Decl | Declarações de Classes.
struct ClassDeclNode : DeclarationNode
{
    // DATA | DADOS.
    BodyNode* Body;
    ExpressionNode* Extend;
    string Name;

    // CONSTRUCTOR | CONSTRUTOR
    ClassDeclNode(NodePos& P)
        : DeclarationNode(NodeType::CLASS_DECL, P) {};    
};

// Aliases Decl | Declaração de Apelidos.
struct TypeDefDeclNode : DeclarationNode
{
    // DATA
    string Name;    

    // CONSTRUCTOR | CONSTRUTOR
    TypeDefDeclNode(NodePos& P)
        : DeclarationNode(NodeType::TYPEDEF_DECL, P) {};
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

// Elif Control Statement | Controle de Instrução 'Elif'.
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
    bool alwaysTrue=false;
    ExpressionNode* Cond;
    BodyNode* IfBody;
    ElseNode* ElseBody;
    vec<ElifNode*> ElifBodyStack;

    // CONSTRUCTOR | CONSTRUTOR
    IfNode(NodePos P)
        : ControlNode(NodeType::IF_CONTROL, P) {};
};

// While Control Statement | Controle de Instrução 'While'.
struct WhileNode : ControlNode
{
    // DATA
    ExpressionNode* Cond;
    BodyNode* Body;
    LoopTypes Type;
    bool alwaysExecuteFirst=false;

    // CONSTRUCTOR | CONSTRUTOR
    WhileNode(NodePos P)
        : ControlNode(NodeType::WHILE, P) {};
};

// ForEach Control Statement | Controle de Instrução 'ForEach'
struct ForEachNode : ControlNode
{
    // DATA
    IdentifierNode* Identifier;
    ExpressionNode* Index;
    ArrayValue* Value;

    BodyNode* Body;
    LoopTypes Type;

    // CONSTRUCTOR | CONSTRUTOR
    ForEachNode(NodePos P)
        : ControlNode(NodeType::FOR_EACH, P) {};
};

// For Control Statement | Controle de Instrução 'For'
struct ForDefNode : ControlNode
{
    // DATA
    IdentifierNode* Identifier;
    LiteralNode* Value;
    ExpressionNode* Cond;

    BodyNode* Body;
    LoopTypes Type;

    // CONSTRUCTOR | CONSTRUTOR
    ForDefNode(NodePos P)
        : ControlNode(NodeType::FOR_DEF, P) {};
};

// For Control Statement | Controle de Instrução 'For'
struct ForNode : ControlNode
{
    // DATA
    IdentifierNode* Identifier;
    RangeNode* End;
    BodyNode* Body;
    LoopTypes Type;

    // CONSTRUCTOR | CONSTRUTOR
    ForNode(NodePos P)
        : ControlNode(NodeType::FOR, P) {};
};

// Function Return | Retorno de Funçoes.
struct ReturnNode : ControlNode
{
    // DATA
    ExpressionNode* Value;
    bool isIf=false;

    // CONSTRUCTOR | CONSTRUTOR
    ReturnNode(NodePos P)
        : ControlNode(NodeType::RETURN, P) {};
};

// Debug-Echo | Eco.
struct EchoNode : ControlNode
{
    // DATA | DADOS
    EchoTypes Type;
    ExpressionNode* Value;

    // CONSTRUCTOR | CONSTRUTOR
    EchoNode(NodePos P)
        : ControlNode(NodeType::ECHO, P) {};
};

// Acess Change Statement | Mudança de Acesso.
struct AcessChangeNode : ControlNode
{
    // DATA | DADOS
    AcessTypes AcessType;

    // CONSTRUCTOR | CONSTRUTOR
    AcessChangeNode(NodePos P)
        : ControlNode(NodeType::ACESS_CHANGE, P) {};
};

// Error Statements Nodes | Errors de Nos de Statement.
struct ErrorStmtNode : ControlNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ErrorStmtNode(NodePos P)
        : ControlNode(NodeType::ERROR, P) {};
};

// ======= SPECIALS ======= //

// Control Statement Base Node | No de Instrução de Controle Base.
struct SpecialNode : ASTNode
{
    // CONSTRUCTOR | CONSTRUTOR
    SpecialNode(NodeType T, NodePos P)
        : ASTNode(T, P) {}
};

// Library Defines | Definições de Bibliotecas.
struct LibraryNode : SpecialNode
{
    // DATA
    string Name;

    // CONSTRUCTOR
    LibraryNode(NodePos& Pos)
        : SpecialNode(NodeType::LIBRARY, Pos) {};
};

// Import Defines MetaData | MetaDados de Definições de Importações.
struct ImportNode : SpecialNode
{
    // DATA
    ExpressionNode* Path;
    ExpressionNode* Base;
    ExpressionNode* Bottom;
    string Alias  = "";
    string Origin = "ORBIT";
    string LibName;
    vec<string> Modules{}; 

    // CONSTRUCTOR
    ImportNode(NodePos& Pos)
        : SpecialNode(NodeType::IMPORT, Pos) {};
};

// Error Special Nodes | Nós de Erros de Instruções Especiais.
struct ErrorSpecialNode : SpecialNode
{
    // CONSTRUCTOR | CONSTRUTOR
    ErrorSpecialNode(NodePos P)
        : SpecialNode(NodeType::ERROR, P) {};    
};

// ======== AST ======== //

// Curr State of Parser | Estado Atual do Parser
struct ParseState
{
    int lastIndent=0;
    bool SingleStatement = false;
    bool consumedInst=false;
    NodePos Pos; // Current Pos of Parser | Posição Atual do Lexer.
    vec<IfNode*>    IfStack;
    vec<BodyNode*>  Bodys;
    vec<AcessTypes> AcessStack;
    BodyNode*       CurrBody;

    // Return if Type is in Stack | Retorna se o Tipo Tem na Stack.
    bool HaveTypeInBodyStack(BodyTypes Type)
    {
        for (BodyNode* B : Bodys)
            if (B->Type is Type)
                return true;
        return false;
    };

    // Return Index of Type in Stack | Retorna o Indice do Tipo na Stack
    int TypeBodyStackIndex(BodyTypes Type)
    {
        int i=0;
        for (BodyNode* B : Bodys)
            if (B->Type is Type)
                return i;
            else
                i++;
        return 0;
    }
};

// Result Of Parser | Resultado do Parser
struct ParseResult
{
    ASTNode* AST;
    LibraryNode* LibDef;
    unord_map<string, pair<ParseResult, SAResult*>> Contexts;
    vec<ImportNode*> ImportRefs{};
};

// ========== NAMESPACES ========= //

// Utils of Parser | Utilidades do Parser.
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
        if (State.AcessStack.size() > 0)
            State.AcessStack.pop_back();
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

    // OTHERS PARSER UTILS | OUTRAS UTILIDADES DO PARSER
    namespace Comm {
        
        // Return the Typeof Explicit Type Assign
        // Retorna o Tipo dos Tipos Explicitamente Infernidos
        inline pair<LiteralTypes, int> InferType(vec<Token*>& Tokens, RunTimeData& Data)
        {
            pair<LiteralTypes, int> DEF_RET = {LiteralTypes::MONO_STATE, 1};

            if (Tokens.size() == 0)
                return DEF_RET;

            if (Tokens[0]->Type is_not TokenType::LIT_TYPE)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "<LIT_TYPE> Expected After ':'",
                    "Expected <LIT_TYPE> After ':' But Found: " + Tokens[0]->GetType(),
                    "Complete the Statement or Delete ':'",
                    Tokens[0]->pos.line,
                    Tokens[0]->pos.collumn
                );
                if (!Data.flags.debugMode)
                {
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                    return DEF_RET;
                }
            }

            string Lex = Tokens[0]->Lexeme(Data);

            // LITERALS | LITERAIS.
            if (Lex is "Int") return {LiteralTypes::INT, 1};
            else if (Lex is "Float") return {LiteralTypes::FLOAT, 1};
            else if (Lex is "Bool") return {LiteralTypes::BOOL, 1};
            else if (Lex is "String") return {LiteralTypes::STRING, 1};
            else if (Lex is "None") return {LiteralTypes::NONE, 1};
            else if (Lex is "Null") return {LiteralTypes::_NULL, 1};

            // LISTS | LISTAS
            else if (Lex is "Array")
            {
                if (Tokens.size() < 4)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Incomplete Array Type",
                        "Expected Array<TYPE> But Found Incomplete Declaration",
                        "Complete with Array<Type>",
                        Tokens[0]->pos.line,
                        Tokens[0]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 1};
                }

                if (Tokens[1]->Type is_not TokenType::LESS)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "'<' Expected",
                        "'<' Expected After 'Array' Infer",
                        "Add a '<' After 'Array'",
                        Tokens[1]->pos.line,
                        Tokens[1]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 1};
                }

                if (Tokens[2]->Type is_not TokenType::LIT_TYPE)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "<TYPE> Expected",
                        "<TYPE> Expected After 'Array<' Infer",
                        "Add a <TYPE> After 'Array<'",
                        Tokens[2]->pos.line,
                        Tokens[2]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 2};
                }

                if (Tokens[3]->Type is_not TokenType::GREATER)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "'>' Expected",
                        "'>' Expected After 'Array<TYPE' Infer",
                        "Add a '>' After 'Array<TYPE'",
                        Tokens[3]->pos.line,
                        Tokens[3]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 3};
                }

                string Inner = Tokens[2]->Lexeme(Data);

                if (Inner is "Int") return {LiteralTypes::ARRAY_INT, 4};
                else if (Inner is "Float") return {LiteralTypes::ARRAY_FLOAT, 4};
                else if (Inner is "Bool") return {LiteralTypes::ARRAY_BOOL, 4};
                else if (Inner is "String") return {LiteralTypes::ARRAY_STRING, 4};
                else if (Inner is "None") return {LiteralTypes::ARRAY_NONE, 4};
                else if (Inner is "Null") return {LiteralTypes::ARRAY_NULL, 4};

                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid Type Inside Array",
                    "Unknown Type Inside Array<>: " + Inner,
                    "Use a Valid Literal Type (Int, Float, Bool, String, None, Null)",
                    Tokens[2]->pos.line,
                    Tokens[2]->pos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                return {LiteralTypes::_NULL, 4};
            }
            else if (Lex is "List")
            {
                if (Tokens.size() < 4)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Incomplete List Type",
                        "Expected List<TYPE> But Found Incomplete Declaration",
                        "Complete with List<Type>",
                        Tokens[0]->pos.line,
                        Tokens[0]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 1};
                }

                if (Tokens[1]->Type is_not TokenType::LESS)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "'<' Expected",
                        "'<' Expected After 'List' Infer",
                        "Add a '<' After 'List'",
                        Tokens[1]->pos.line,
                        Tokens[1]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 1};
                }

                if (Tokens[2]->Type is_not TokenType::LIT_TYPE)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "<TYPE> Expected",
                        "<TYPE> Expected After 'List<' Infer",
                        "Add a <TYPE> After 'List<'",
                        Tokens[2]->pos.line,
                        Tokens[2]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 2};
                }

                if (Tokens[3]->Type is_not TokenType::GREATER)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "'>' Expected",
                        "'>' Expected After 'List<TYPE' Infer",
                        "Add a '>' After 'List<TYPE'",
                        Tokens[3]->pos.line,
                        Tokens[3]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return {LiteralTypes::_NULL, 3};
                }

                string Inner = Tokens[2]->Lexeme(Data);

                if (Inner is "Int") return {LiteralTypes::TABLE_INT, 4};
                else if (Inner is "Float") return {LiteralTypes::TABLE_FLOAT, 4};
                else if (Inner is "Bool") return {LiteralTypes::TABLE_BOOL, 4};
                else if (Inner is "String") return {LiteralTypes::TABLE_STRING, 4};
                else if (Inner is "None") return {LiteralTypes::TABLE_NONE, 4};
                else if (Inner is "Null") return {LiteralTypes::TABLE_NULL, 4};

                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid Type Inside List",
                    "Unknown Type Inside List<>: " + Inner,
                    "Use a Valid Literal Type (Int, Float, Bool, String, None, Null)",
                    Tokens[2]->pos.line,
                    Tokens[2]->pos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                return {LiteralTypes::_NULL, 4};
            }

            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Unknown Type",
                "Unknown Type After ':': " + Lex,
                "Use a Valid Type (Int, Float, Bool, String, None, Null, Array, List)",
                Tokens[0]->pos.line,
                Tokens[0]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return DEF_RET;
        }
    }
}