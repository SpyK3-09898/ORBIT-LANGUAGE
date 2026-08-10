
// ========== CODE-GEN =========== //
// Parse '_AST' And Generate ByteCodes.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../byte_code.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"
#include "../../FrontEnd/SA/semantic_analysis.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

// STRUCTS
struct CodeGenState {};

// MAIN CLASS | CLASSE PRINCIPAL
class CodeGenerator
{
    public:
    
        ByteCode InitCG(ParseResult& PRes, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    private:
    
    // OTHERS

    void CompileNode(ASTNode* Node, CodeGenState& State, ByteCode& BC, RunTimeData& Data, Arena& Memory);


    // PROGRAMS:

    void CompileProgram(ProgramNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileBody(BodyNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);


    // EXPRESSIONS:

    void CompileLiteral(LiteralNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileIdentifier(IdentifierNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileUnary(UnaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileBinary(BinaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileAssignment(AssignmentNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileMemberAccess(MemberAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileIndexAccess(IndexAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileFunctionCall(FunctionCall* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileTableValue(TableValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileArrayValue(ArrayValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileRange(RangeNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileErrorExpr(ErrorExprNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);


    // DECLARATIONS:

    void CompileVarDecl(VarDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileFnDecl(FnDecl* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileErrorDecl(ErrorDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);


    // CONTROL FLOW:

    void CompileIf(IfNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileElse(ElseNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileElif(ElifNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileWhile(WhileNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileFor(ForNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileForEach(ForEachNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileForDef(ForDefNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileReturn(ReturnNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileErrorStmt(ErrorStmtNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
};