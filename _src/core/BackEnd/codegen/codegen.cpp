
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
#include <string>
#include <variant>

// ========= UTILS ========== //

// Utils of Code Generator | Utilidades do Gerador de Codigo.
namespace CodeGenUtils {

    // Create A New Instruction | Cria uma Nova Instrução.
    ByteInstruction* CreateInst(ASTNode* Node, OpCode Code, ByteValue R1, ByteValue R2, RunTimeData& Data, Arena& Memory)
    {
        ByteInstruction* Inst = Memory.New<ByteInstruction>();

        Inst->C = Code;
        Inst->R1 = R1;
        Inst->R2 = R2;
        Inst->Pos = Node->pos;

        return Inst;
    }

    // Create A New Chunk | Cria uma Nova Chunk.
    int CreateChunk(ByteCode& BC, RunTimeData& Data, Arena& Memory)
    {
        Chunk* C = Memory.New<Chunk>();
        BC.Chunks.push_back(C);

        return BC.Chunks.size() - 1;
    }
}

// ========= CORE ======== //

// --- PROGRAM --- //

// Compile Entry-Point | Compila o Ponto-de-Entrada.
void CodeGenerator::CompileProgram(ProgramNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{ CompileBody(Node->Node, State, BC, SARes, Data, Memory); }

// Compile Bodys | Compila BodyNodes.
void CodeGenerator::CompileBody(BodyNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    for (ASTNode* N : Node->Data)
        CompileNode(N, State, BC, SARes, Data, Memory);
    return;
}

// --- CONTROL FLOW --- //

// Compile If | Compila If.
void CodeGenerator::CompileIf(IfNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Init
    State.IfStates.push_back({});

    // Data1
    IfCompileState& IfState = State.IfStates.back();
    CompileNode(Node->Cond, State, BC, SARes, Data, Memory);
    ByteInstruction* JumpFalse = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP_IF_FALSE, 0, 0, Data, Memory);
    
    //  Data2
    JumpFalse->R1 = static_cast<i64>(-1);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpFalse);
    IfState.PreviousJumpFalse = JumpFalse;
    CompileNode(Node->IfBody, State, BC, SARes, Data, Memory);
    ByteInstruction* JumpEnd = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP, 0, 0, Data, Memory);

    JumpEnd->R1 = static_cast<i64>(-1);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpEnd);
    IfState.EndJumps.push_back(JumpEnd);

    // Compile Elifs and Elses | Compila os Elifs e Elses.
    for (ElifNode* Elif : Node->ElifBodyStack)
        CompileElif(Elif, State, BC, SARes, Data, Memory);
    if (Node->ElseBody)
        CompileElse(Node->ElseBody, State, BC, SARes, Data, Memory);
    int EndAddress =
        BC.Chunks[State.currChunk]->Instructions.size();

    // Jumps | Pulos
    for (ByteInstruction* Jump : IfState.EndJumps)
        Jump->R1 = static_cast<i64>(EndAddress);
    IfState.PreviousJumpFalse->R1 =
        static_cast<i64>(EndAddress);
    State.IfStates.pop_back();
};

// Compile Else | Compila Else.
void CodeGenerator::CompileElse(ElseNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Init
    if (!Node)
        return;

    IfCompileState& IfState = State.IfStates.back();

    // Generate Codes | GEra os Codigos.
    ui32 ElseAddress =
        BC.Chunks[State.currChunk]->Instructions.size();
    IfState.PreviousJumpFalse->R1 =
        static_cast<i64>(ElseAddress);
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);
};

// Compile Elif | Compila Elif.
void CodeGenerator::CompileElif(ElifNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (!Node)
        return;

    // Data
    IfCompileState& IfState = State.IfStates.back();
    ui32 ElifAddress =
        BC.Chunks[State.currChunk]->Instructions.size();
    IfState.PreviousJumpFalse->R1 =
        static_cast<i64>(ElifAddress);

    // Compile
    CompileNode(Node->Cond, State, BC, SARes, Data, Memory);
    ByteInstruction* JumpFalse = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP_IF_FALSE, 0, 0, Data, Memory);

    // Set | Define
    JumpFalse->R1 = static_cast<i64>(-1);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpFalse);
    IfState.PreviousJumpFalse = JumpFalse;

    // Compile 2
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);
    ByteInstruction* JumpEnd = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP, 0, 0, Data, Memory);
    
    // Set | Define
    JumpEnd->R1 = static_cast<i64>(-1);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpEnd);
    IfState.EndJumps.push_back(JumpEnd);
};

// Compile While | Compila While.
void CodeGenerator::CompileWhile(WhileNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Init
    int StartAddress =
        BC.Chunks[State.currChunk]->Instructions.size();

    // Data1
    CompileNode(Node->Cond, State, BC, SARes, Data, Memory);

    ByteInstruction* JumpFalse = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP_IF_FALSE, 0, 0, Data, Memory);

    // Data2
    JumpFalse->R1 = static_cast<i64>(-1);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpFalse);

    // Compile Body | Compila Corpo.
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);

    // Jump Back | Pula de Volta.
    ByteInstruction* JumpBack = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP, 0, 0, Data, Memory);

    JumpBack->R1 = static_cast<i64>(StartAddress);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpBack);

    // Jump End | Pula para o Fim.
    int EndAddress =
        BC.Chunks[State.currChunk]->Instructions.size();

    JumpFalse->R1 = static_cast<i64>(EndAddress);
};

// Compile For | Compila For.
void CodeGenerator::CompileFor(ForNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile | Compila.
    CompileNode(Node->End, State, BC, SARes, Data, Memory);

    // Start Address | Endereço Inicial.
    int StartAddress =
        BC.Chunks[State.currChunk]->Instructions.size();

    // Create Jump For | Cria Salto do For.
    ByteInstruction* Inst = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP_FOR, State.GetLocal(Node->Identifier->Name), -1, Data, Memory);

    // Set | Define.
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);

    // Compile Body | Compila Corpo.
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);

    // Jump Back | Pulo De Volta.
    ByteInstruction* JumpBack = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP, StartAddress, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(JumpBack);

    // End Address | Endereço Final.
    int EndAddress =
        BC.Chunks[State.currChunk]->Instructions.size();
    Inst->R2 =
        EndAddress;
};

// Compile For Each | Compila For Each.
void CodeGenerator::CompileForEach(ForEachNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile For Default| Compila Default For.
void CodeGenerator::CompileForDef(ForDefNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// Compile Return | Compila Retorno.
void CodeGenerator::CompileReturn(ReturnNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (Node->Value)
        CompileNode(Node->Value, State, BC, SARes, Data, Memory);
    
    ByteInstruction* Inst = CodeGenUtils::CreateInst
        (Node, OpCode::RETURN, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);    
};

// Compile Error Statement | Compila Erro de Statement.
void CodeGenerator::CompileErrorStmt(ErrorStmtNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// --- DECLARATIONS --- //

// Compile Variable Declaration | Compila Declaração de Variável.
void CodeGenerator::CompileVarDecl(VarDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Get Slot And Compile | Pega o Espaço e Compila.
    ui32 Slot = State.GetLocal(Node->Name);
    CompileNode(Node->Val, State, BC, SARes, Data, Memory);

    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
        (Node, OpCode::STORE_LOCAL, Slot, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Function Declaration | Compila Declaração de Função.
void CodeGenerator::CompileFnDecl(FnDecl* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Create A New Chunk | Cria uma nova Chunk
    int PreviousC = State.currChunk;
    int FnChunk = 
        CodeGenUtils::CreateChunk(BC, Data, Memory);
    
    BC.Functions[Node->Name] = FnChunk; // Define Fn Chunk | Define a Chunk da Func:
    State.currChunk = FnChunk;

    // Compile Body And Return to the Last Chunk 
    // Compila o Corpo e Retorna para a Ultima Chunk.
    CompileBody(Node->Body, State, BC, SARes, Data, Memory);
    State.currChunk = PreviousC;
};

// Compile Error Declaration | Compila Erro de Declaração.
void CodeGenerator::CompileErrorDecl(ErrorDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// --- EXPRESSIONS --- //

// Compile Literal | Compila Literal.
void CodeGenerator::CompileLiteral(LiteralNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    ByteInstruction* Inst;

    /// Compile Literals | Compila os Literais:
    if (holds_alt<i64>(Node->Value))
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_CONST, std::get<i64>(Node->Value), 0, Data, Memory);
    else if (holds_alt<float>(Node->Value))
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_CONST, std::get<float>(Node->Value), 0, Data, Memory);
    else if (holds_alt<bool>(Node->Value))
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_CONST, std::get<bool>(Node->Value), 0, Data, Memory);
    else if (holds_alt<string>(Node->Value))
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_CONST, std::get<string>(Node->Value), 0, Data, Memory);
    else if (holds_alt<NoneLitVal>(Node->Value))
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_CONST, std::get<NoneLitVal>(Node->Value), 0, Data, Memory);
    else if (holds_alt<NullLitVal>(Node->Value))
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_CONST, std::get<NullLitVal>(Node->Value), 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Identifier | Compila Identificador.
void CodeGenerator::CompileIdentifier(IdentifierNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // FN LOAD
    auto It = BC.Functions.find(Node->Name);
    if (It != BC.Functions.end())
    {
        ByteInstruction* Inst =
            CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_FN, It->second, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
        return;
    }

    // IDENTIFIER LOAD
    ui32 Local = State.GetLocal(Node->Name);
    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_LOCAL, Local, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Unary | Compila Unário.
void CodeGenerator::CompileUnary(UnaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Operand | Compila o Operando.
    CompileNode(Node->Operand, State, BC, SARes, Data, Memory);

    // Generate Instruction | Gera a Instrução.
    ByteInstruction* Inst = CodeGenUtils::
        CreateInst(Node, OpCode::NEG, 0, 0, Data, Memory);
    switch (Node->Operator)
    {
        case Operator::SUB: // NEG:
            Inst->C = OpCode::NEG; break;
        case Operator::NOT: // NOT:
            Inst->C = OpCode::NOT; break;
        default: break;
    }
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Binary | Compila Binário.
void CodeGenerator::CompileBinary(BinaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Right and Left | Compila o Direito e Esquerdo.
    CompileNode(Node->L, State, BC, SARes, Data, Memory);
    CompileNode(Node->R, State, BC, SARes, Data, Memory);

    // Compile Operators | Compila os Operadores
    OpCode Op;
    switch (Node->Op)
    {
        case Operator::ADD:
            Op = OpCode::ADD;
            break;

        case Operator::SUB:
            Op = OpCode::SUB;
            break;

        case Operator::MUL:
            Op = OpCode::MUL;
            break;

        case Operator::DIV:
            Op = OpCode::DIV;
            break;

        case Operator::MOD:
            Op = OpCode::MOD;
            break;

        case Operator::POWER:
            Op = OpCode::POWER;
            break;

        case Operator::EQUAL:
            Op = OpCode::CMP_EQ;
            break;

        case Operator::NOT_EQUAL:
            Op = OpCode::CMP_NE;
            break;

        case Operator::LESS:
            Op = OpCode::CMP_LT;
            break;

        case Operator::LESS_EQUAL:
            Op = OpCode::CMP_LE;
            break;

        case Operator::GREATER:
            Op = OpCode::CMP_GT;
            break;

        case Operator::GREATER_EQUAL:
            Op = OpCode::CMP_GE;
            break;

        default:
            return;
    }

    // Generate Instruction | Gera Instrução.
    ByteInstruction* Inst = CodeGenUtils::
        CreateInst(Node, Op, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Assignment | Compila Atribuição.
void CodeGenerator::CompileAssignment(AssignmentNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    CompileNode(Node->Right, State, BC, SARes, Data, Memory);

    IdentifierNode* Left = static_cast<IdentifierNode*>(Node->Left);

    ui32 Local = State.GetLocal(Left->Name);

    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::STORE_LOCAL, Local, 0, Data, Memory);

    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Member Access | Compila Acesso de Membro.
void CodeGenerator::CompileMemberAccess(MemberAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Object and Member | Compila o Objeto e Membro
    CompileNode(Node->Object, State, BC, SARes, Data, Memory);
    CompileNode(Node->Member, State, BC, SARes, Data, Memory);

    // Generate Instrucion | Gera a Instrução.
    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_MEMBER, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Index Access | Compila Acesso por Índice.
void CodeGenerator::CompileIndexAccess(IndexAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Object and Index | Compila o Objeto e Anexo.
    CompileNode(Node->Object, State, BC, SARes, Data, Memory);
    CompileNode(Node->Index, State, BC, SARes, Data, Memory);

    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
        (Node, OpCode::LOAD_INDEX, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Function Call | Compila Chamada de Função.
void CodeGenerator::CompileFunctionCall(FunctionCall* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Node | Compila os Nós. 
    CompileNode(Node->Callee, State, BC, SARes, Data, Memory);
    for (ExpressionNode* Arg : Node->Args)
        CompileNode(Arg, State, BC, SARes, Data, Memory);

    // Instruction | Instrução.
    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::CALL, int(Node->Args.size()), 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);    
};

// Compile Table Value | Compila Valor de Tabela.
void CodeGenerator::CompileTableValue(TableValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    for (ArrayEntry& Entry : Node->Args)
    {
        CompileNode(Entry.Key, State, BC, SARes, Data, Memory);
        CompileNode(Entry.Value, State, BC, SARes, Data, Memory);
    }

    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::BUILD_TABLE, int(Node->Args.size()), 0, Data, Memory);

    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Array Value | Compila Valor de Array.
void CodeGenerator::CompileArrayValue(ArrayValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Node | Compila os Nós. 
    for (ExpressionNode* Arg : Node->Args)
        CompileNode(Arg, State, BC, SARes, Data, Memory);

    // Instruction | Instrução.
    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::BUILD_ARRAY, int(Node->Args.size()), 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);    

};

// Compile Range | Compila Intervalo.
void CodeGenerator::CompileRange(RangeNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    CompileNode(Node->Begin, State, BC, SARes, Data, Memory);
    CompileNode(Node->End, State, BC, SARes, Data, Memory);

    ByteInstruction* Inst =
        CodeGenUtils::CreateInst
            (Node, OpCode::BUILD_RANGE, 0, 0, Data, Memory);

    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
};

// Compile Error Expression | Compila Erro de Expressão.
void CodeGenerator::CompileErrorExpr(ErrorExprNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{};

// ========= ENTRY-POINT ========= //

// Compile a Node | Compila Um Nó.
void CodeGenerator::CompileNode(
    ASTNode* Node, 
    CodeGenState& State, 
    ByteCode& BC, 
    SAResult& SARes,
    RunTimeData& Data, 
    Arena& Memory
)
{
    switch (Node->Type)
    {
        // PROGRAM
        case NodeType::PROGRAM:
            CompileProgram(
                static_cast<ProgramNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        // CONTROL FLOW
        case NodeType::IF_CONTROL:
            CompileIf(
                static_cast<IfNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::ELIF_CONTROL:
            CompileElif(
                static_cast<ElifNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::ELSE_CONTROL:
            CompileElse(
                static_cast<ElseNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::WHILE:
            CompileWhile(
                static_cast<WhileNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::FOR:
            CompileFor(
                static_cast<ForNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::FOR_EACH:
            CompileForEach(
                static_cast<ForEachNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::FOR_DEF:
            CompileForDef(
                static_cast<ForDefNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::RETURN:
            CompileReturn(
                static_cast<ReturnNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        // DECLARATIONS
        case NodeType::VAR_DECL:
            CompileVarDecl(
                static_cast<VarDeclNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::FN_DECL:
            CompileFnDecl(
                static_cast<FnDecl*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        // EXPRESSIONS
        case NodeType::LITERAL:
            CompileLiteral(
                static_cast<LiteralNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::IDENTIFIER:
            CompileIdentifier(
                static_cast<IdentifierNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::UNARY:
            CompileUnary(
                static_cast<UnaryNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::BINARY:
            CompileBinary(
                static_cast<BinaryNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::ASSIGNMENT:
            CompileAssignment(
                static_cast<AssignmentNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::MEMBER_ACCESS:
            CompileMemberAccess(
                static_cast<MemberAccessNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::INDEX_ACCESS:
            CompileIndexAccess(
                static_cast<IndexAccessNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::FN_CALL:
            CompileFunctionCall(
                static_cast<FunctionCall*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::TABLE_VALUE:
            CompileTableValue(
                static_cast<TableValue*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::ARRAY_VALUE:
            CompileArrayValue(
                static_cast<ArrayValue*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        case NodeType::RANGE:
            CompileRange(
                static_cast<RangeNode*>(Node),
                State,
                BC,
                SARes,
                Data,
                Memory
            );
            break;

        default:
            break;
    }
}

// Convert Byte Value to a String | Converte ByteValue para uma String.
string ByteValueToString(const ByteValue& Value)
{
    return std::visit([](const auto& V) -> string
    {
        using T = std::decay_t<decltype(V)>;

        if constexpr (std::is_same_v<T, string>)
            return V;

        else if constexpr (std::is_same_v<T, bool>)
            return V ? "true" : "false";

        else if constexpr (std::is_same_v<T, NoneLitVal>)
            return "none";

        else if constexpr (std::is_same_v<T, NullLitVal>)
            return "null";

        else
            return std::to_string(V);

    }, Value);
}

// Generate CodeGen Log | Gera o Log do CodeGen.
void GenerateCodeGenLog(ByteCode& BC, RunTimeData& Data)
{
    fstream file(Data.LogDir, std::ios::out | std::ios::app);
    file << "\n\n// =========== CODE-GENERATION ======== //\n\n";

    int ci=0;
    for (Chunk* C : BC.Chunks)
    {
        string txt;

        txt += "CHUNK["+std::to_string(ci)+"]\n";
        txt += "\tINSTRUCTIONS: \n";

        int i=0;
        for (ByteInstruction* Inst : C->Instructions)
        {
            txt += "\t\tByteInst["+std::to_string(i)+"]: \n";
            txt += "\t\t\tOpCode: "+std::to_string(static_cast<int>(Inst->C))+"\n";
            txt += "\t\t\tRegister1: "+ByteValueToString(Inst->R1)+"\n";
            txt += "\t\t\tRegister2: "+ByteValueToString(Inst->R2)+"\n";
            txt += "\t\t\tLocals: 1 - "+ByteValueToString(Inst->L1)+" 2 - "+ByteValueToString(Inst->L2)+"\n";
            
            i++;
        }

        file << txt;
        ci++;
    }

    file << "\n\n// =========== ENDOF: 'CODE-GENERATION' . .. ... ======== //\n\n";
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
        PrintIn("STARTING TASK: Compile ORBIT");
    // Data    
    CodeGenState State;
    ByteCode BC;
    Chunk* Cnk = Memory.New<Chunk>();

    // Set Data
    BC.Chunks.push_back(Cnk); // Compile Program:
    CompileNode(PRes.AST, State, BC, SARes, Data, Memory);

    if (Data.flags.generateLog)
        GenerateCodeGenLog(BC, Data);
    if (Data.flags.debugMode)
    {
        PrintIn("ENDOF TASK: 'Compile ORBIT'. .. ...");
        PrintIn("");
    }
    return BC;
}