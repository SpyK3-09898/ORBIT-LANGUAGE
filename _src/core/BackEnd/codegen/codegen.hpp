
// ========== CODE-GENNERATOR =========== //
// Parse '_AST' And Generate ByteCodes.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMANTICAS.
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

// State of Ifs | Estados dos Ifs
struct IfCompileState
{
    vec<ByteInstruction*> EndJumps;
    ByteInstruction* PreviousJumpFalse = nullptr;
};

// State of ByteCode Generator | Estado do Gerador de ByteCodes.
struct CodeGenState 
{
    unord_map<ui16, ui32> Locals;
    unord_map<Symbol*, ui32> LocalsSym;
    unord_map<Symbol*, i64> Functions;
    Symbol* CurrNamespace = nullptr;
    int currChunk = 0;
    int localCount=0;

    vec<IfCompileState> IfStates;

    // UTILS:

    // Return if Have a Local | Retorna se Tem um Local.
    bool HasLocal(ui16 Id)
    {
        return Locals.find(Id) != Locals.end();
    }

    // Get Slot of Local | Pega o Slot do Local.
    ui32 GetLocal(ui16 Id)
    {
        auto It = Locals.find(Id);

        if (It != Locals.end())
            return It->second;

        OrbitLog::Error(
            "codegen.cpp",
            "Cannot Find Local, Id: "+std::to_string(static_cast<int>(Id)),
            true,
            404
        );

        return 0;
    }

    // Create A New Local | Cria um Novo Local.
    ui32 CreateLocal(ui16 Id)
    {
        auto It = Locals.find(Id);

        if (It != Locals.end())
            return It->second;

        ui32 Index = localCount++;
        Locals[Id] = Index;

        return Index;
    }

    // Create A New Local Whit Symbols | Cria Um Novo Local Com Simbolos.
    ui32 CreateLocalForSym(Symbol* Sym, ui16 Id)
    {
        ui32 Index = CreateLocal(Id);
        if (Sym)
            LocalsSym[Sym] = Index;
        return Index;
    }

    // Get Local Whit Symbol | Pega O Local Pelo Simbolo.
    ui32 GetLocalForSym(Symbol* Sym, ui16 Id)
    {
        if (Sym)
        {
            auto It = LocalsSym.find(Sym);
            if (It != LocalsSym.end())
                return It->second;
        }
        return GetLocal(Id);
    }

};


// Util | Utilidade.
string ByteValueToString(const ByteValue& Value);

// MAIN CLASS | CLASSE PRINCIPAL
class CodeGenerator
{
    public:
    
        ByteCode InitCG(ParseResult& PRes, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    private:
    
    // OTHERS
    void CompileNode(ASTNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memoryy, Symbol* Owner=nullptr);

    // PROGRAMS:
    void CompileProgram(ProgramNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileBody(BodyNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);

    // EXPRESSIONS:
    void CompileLValue(ExpressionNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory, bool base=false);
    void CompileLiteral(LiteralNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileIdentifier(IdentifierNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
    void CompileUnary(UnaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileBinary(BinaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileAssignment(AssignmentNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileMemberAccess(MemberAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
    void CompileIndexAccess(IndexAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory, Symbol* Owner=nullptr);
    void CompileFunctionCall(FunctionCall* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileTableValue(TableValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileArrayValue(ArrayValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileRange(RangeNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileErrorExpr(ErrorExprNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);

    // DECLARATIONS:
    void CompileVarDecl(VarDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileFnDecl(FnDecl* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileNameSpaceDecl(NameSpaceDecl* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileErrorDecl(ErrorDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);

    // CONTROL-FLOW:
    void CompileIf(IfNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileElse(ElseNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileElif(ElifNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileWhile(WhileNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileFor(ForNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileForEach(ForEachNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileForDef(ForDefNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileReturn(ReturnNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileEcho(EchoNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileLibraryDef(LibraryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
    void CompileErrorStmt(ErrorStmtNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory);
};

// EOF
