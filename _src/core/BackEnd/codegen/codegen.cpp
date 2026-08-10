
// ========== CODE-GEN =========== //
// Parse '_AST' And Generate ByteCodes.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "codegen.hpp" // HEADER FILE | CABEÇALHO
#include "../byte_code.hpp"

#include "../../FrontEnd/SA/semantic_analysis.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

// ========= CORE ======== //

// --- PROGRAM --- //

// Compile Entry-Point | Compila o Ponto-de-Entrada.
void CodeGenerator::CompileProgram(ProgramNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{ CompileBody(Node->Node, State, BC, SARes, Data, Memory); }

// Compile Bodys | Compila BodyNodes.
void CodeGenerator::CompileBody(BodyNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    for (ASTNode* N : Node->Data)
        CompileNode(N, State, BC, Data, Memory);
    return;
}

// --- CONTROL FLOW --- //

// Compile If | Compila If.
void CodeGenerator::CompileIf(IfNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Else | Compila Else.
void CodeGenerator::CompileElse(ElseNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Elif | Compila Elif.
void CodeGenerator::CompileElif(ElifNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile While | Compila While.
void CodeGenerator::CompileWhile(WhileNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile For | Compila For.
void CodeGenerator::CompileFor(ForNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile For Each | Compila For Each.
void CodeGenerator::CompileForEach(ForEachNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile For Definition | Compila Definição de For.
void CodeGenerator::CompileForDef(ForDefNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Return | Compila Retorno.
void CodeGenerator::CompileReturn(ReturnNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Error Statement | Compila Erro de Statement.
void CodeGenerator::CompileErrorStmt(ErrorStmtNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// --- DECLARATIONS --- //

// Compile Variable Declaration | Compila Declaração de Variável.
void CodeGenerator::CompileVarDecl(VarDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Function Declaration | Compila Declaração de Função.
void CodeGenerator::CompileFnDecl(FnDecl* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Error Declaration | Compila Erro de Declaração.
void CodeGenerator::CompileErrorDecl(ErrorDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// --- EXPRESSIONS --- //

// Compile Literal | Compila Literal.
void CodeGenerator::CompileLiteral(LiteralNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Identifier | Compila Identificador.
void CodeGenerator::CompileIdentifier(IdentifierNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Unary | Compila Unário.
void CodeGenerator::CompileUnary(UnaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Binary | Compila Binário.
void CodeGenerator::CompileBinary(BinaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Assignment | Compila Atribuição.
void CodeGenerator::CompileAssignment(AssignmentNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Member Access | Compila Acesso de Membro.
void CodeGenerator::CompileMemberAccess(MemberAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Index Access | Compila Acesso por Índice.
void CodeGenerator::CompileIndexAccess(IndexAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Function Call | Compila Chamada de Função.
void CodeGenerator::CompileFunctionCall(FunctionCall* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Table Value | Compila Valor de Tabela.
void CodeGenerator::CompileTableValue(TableValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Array Value | Compila Valor de Array.
void CodeGenerator::CompileArrayValue(ArrayValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Range | Compila Intervalo.
void CodeGenerator::CompileRange(RangeNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Error Expression | Compila Erro de Expressão.
void CodeGenerator::CompileErrorExpr(ErrorExprNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// ========= ENTRY-POINT ========= //

// Compile a Node | Compila Um Nó.
void CodeGenerator::CompileNode(
    ASTNode* Node, 
    CodeGenState& State, 
    ByteCode& BC, 
    RunTimeData& Data, 
    Arena& Memory
)
{
    switch (Node->Type) {
    
        default: {};
    }
}

// Entry-Point of CodeGen Program 
// Ponto de Entrada do Programa da Geração de ByteCodes.
ByteCode CodeGenerator::InitCG(
    ParseResult& PRes, 
    SAResult& SARes,
    RunTimeData& Data, 
    Arena& Memory
)
{
    if (Data.flags.debugMode)
        PrintLn("STARTING TASK: Compile ORBIT");
    CodeGenState State;
    ByteCode BC;

    CompileNode(PRes.AST, State, BC, Data, Memory);

    if (Data.flags.debugMode)
        PrintLn("ENDOF TASK: 'Compile ORBIT'. .. ...");
    return BC;
}