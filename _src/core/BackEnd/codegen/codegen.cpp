
// ========== CODE-GENNERATOR =========== //
// Parse '_AST' And Generate ByteCodes.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "codegen.hpp" // HEADER FILE | CABEÇALHO

#include "../byte_code.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"
#include "../../FrontEnd/SA/semantic_analysis.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp" // LIBRARIES | BIBLIOTECAS:
#include <algorithm>
#include <cstddef>
#include <string>
#include <cerrno>
#include <cstring>

// ============ UTILS =========== //

// Utils of Code Generator | Utilidades do Gerador de Codigo.
namespace CodeGenUtils {

    // Create A New Instruction | Cria uma Nova Instrução.
    ByteInstruction* CreateInst(
        opt<ASTNode*> Node, 
        OpCode C, 
        const ByteValue& R1, 
        const ByteValue& R2, 
        RunTimeData& Data, 
        Arena& Memory
    ) 
    {
        ByteInstruction* Inst = Memory.New<ByteInstruction>();

        Inst->C = C;
        Inst->R1 = R1;
        Inst->R2 = R2;

        if (Node.has_value() && *Node != nullptr) {
            Inst->Pos = (*Node)->pos;
        } else {
            Inst->Pos.line    = 0;
            Inst->Pos.collumn = -1;
        }

        return Inst;
    }

    // Return ByteValue Version of LiteralTypes via std::visit
    // Retorna Uma Versão ByteValue De Um LiteralTypes usando std::visit.
    ByteValue LiteralToByteValue(const LiteralValue& V)
    {
        return std::visit([](auto&& arg) -> ByteValue {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return NullLitVal{};
            }
            else if constexpr (std::is_same_v<T, string>)
            {
                string Value;

                for (size_t i = 0; i < arg.size(); i++)
                {
                    if (arg[i] == '\\' && i + 1 < arg.size())
                    {
                        ++i;

                        switch (arg[i])
                        {
                            case 'n':
                                Value += '\n';

                                if (i + 1 < arg.size() && arg[i + 1] == ' ')
                                    ++i;

                                break;

                            case 't':
                                Value += '\t';

                                if (i + 1 < arg.size() && arg[i + 1] == ' ')
                                    ++i;

                                break;

                            case 'r':
                                Value += '\r';
                                break;

                            case '0':
                                Value += "<NULL-CHAR>";
                                break;

                            case '\\':
                                Value += '\\';
                                break;

                            case '\'':
                                Value += '\'';
                                break;

                            case '"':
                                Value += '"';
                                break;

                            default:
                                Value += '\\';
                                Value += arg[i];
                                break;
                        }

                        continue;
                    }

                    Value += arg[i];
                }

                return Value;
            }
            else
            {
                return arg;
            }
        }, V);
    }
}

// ========== CORE =========== //

// PROGRAM

// Compile a Random Node | Compila um Nó Aleatorio
void CodeGenerator::CompileNode(ASTNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (!Node) return;

    switch (Node->Type)
    {
        // PROGRAM
        case NodeType::PROGRAM:
            CompileProgram(static_cast<ProgramNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::BODY:
            CompileBody(static_cast<BodyNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        // EXPRESSIONS
        case NodeType::LITERAL:
            CompileLiteral(static_cast<LiteralNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::IDENTIFIER:
            CompileIdentifier(static_cast<IdentifierNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::UNARY:
            CompileUnary(static_cast<UnaryNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::BINARY:
            CompileBinary(static_cast<BinaryNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::ASSIGNMENT:
            CompileAssignment(static_cast<AssignmentNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::MEMBER_ACCESS:
            CompileMemberAccess(static_cast<MemberAccessNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::INDEX_ACCESS:
            CompileIndexAccess(static_cast<IndexAccessNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::FN_CALL:
            CompileFunctionCall(static_cast<FunctionCall*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::TABLE_VALUE:
            CompileTableValue(static_cast<TableValue*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::ARRAY_VALUE:
            CompileArrayValue(static_cast<ArrayValue*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::RANGE:
            CompileRange(static_cast<RangeNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::ERROR:
            CompileErrorExpr(static_cast<ErrorExprNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        // DECLARATIONS
        case NodeType::VAR_DECL:
            CompileVarDecl(static_cast<VarDeclNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::FN_DECL:
            CompileFnDecl(static_cast<FnDecl*>(Node), State, BC, SARes, Data, Memory);
            break;

        // CONTROL FLOW
        case NodeType::IF_CONTROL:
            CompileIf(static_cast<IfNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::ELSE_CONTROL:
            CompileElse(static_cast<ElseNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::ELIF_CONTROL:
            CompileElif(static_cast<ElifNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::WHILE:
            CompileWhile(static_cast<WhileNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::FOR:
            CompileFor(static_cast<ForNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::FOR_EACH:
            CompileForEach(static_cast<ForEachNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::FOR_DEF:
            CompileForDef(static_cast<ForDefNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::RETURN:
            CompileReturn(static_cast<ReturnNode*>(Node), State, BC, SARes, Data, Memory);
            break;

        case NodeType::ECHO:
            CompileEcho(static_cast<EchoNode*>(Node), State, BC, SARes, Data, Memory);
            break;

    }
}

// Compile Program Root Node | Compila o Nó Raiz do Programa
void CodeGenerator::CompileProgram(ProgramNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (!Node) return;
    CompileNode(Node->Node, State, BC, SARes, Data, Memory);
}

// Compile Code Body Block | Compila um Bloco de Corpo de Código
void CodeGenerator::CompileBody(BodyNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (!Node) return;

    for (ASTNode* N : Node->Data)
    {
        CompileNode(N, State, BC, SARes, Data, Memory);
    }
}

// EXPRESSION

// Compile Literal Values | Compila Valores Literais
void CodeGenerator::CompileLiteral(LiteralNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Create Inst | Cria a Instrução.
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst
            (Node, OpCode::PUSH, CodeGenUtils::LiteralToByteValue(Node->Value), 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Identifier Access | Compila Acesso a Identificadores
void CodeGenerator::CompileIdentifier(IdentifierNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Error Prevention | Prevenção de Erros:
    auto It = SARes.SymbolTable.find(Node->Name);
    if (It == SARes.SymbolTable.end())
        OrbitLog::Error("codegen.cpp", "Cannot Find: "+Node->Name+" in SymbolTable", true, 404);

    // Resolve Type | Resolve o tipo:
    auto& S = It->second->Type;
    
    ByteInstruction* Inst;
    if (S == SymbolTypes::IDENTIFIER or S == SymbolTypes::VAR or S == SymbolTypes::PARAM)
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_LOCAL, State.GetLocal(Node->Name), 0, Data, Memory);
    else if (S == SymbolTypes::FN)
    {
        i64 id = BC.Functions.at(Node->Name);
        Inst = CodeGenUtils::CreateInst
            (Node, OpCode::LOAD_FN, id, BC.Chunks[id]->ParamCount, Data, Memory);
    } else
    {
        OrbitLog::Error
        ("codegen.cpp", "Invalid Symbol Type for: '"+Node->Name+"', Type: "+std::to_string(static_cast<int>(S)), true, 400);
        return;
    }
    // Set | Define:
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Unary Operation | Compila Operação Unária
void CodeGenerator::CompileUnary(UnaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile
    CompileNode(Node->Operand, State, BC, SARes, Data, Memory);

    // Main Switch | Switch Principal
    OpCode op = OpCode::NEG;
    switch (Node->Operator) {
        case Operator::SUB:
            op = OpCode::NEG; 
            break;

        case Operator::NOT:
            op = OpCode::NOT; 
            break;

        default:
            if (Data.flags.generateLog)
                OrbitLog::Warn("codegen.cpp", "Unknown Unary Operand cannot be Compiled, Using 'NEG' Instead. Run with '--generateLog=ON' to see Logs.");
            else
                OrbitLog::Warn("codegen.cpp", "Unknown Unary Operand cannot be Compiled, Using 'NEG' Instead. Check <LOG_FILE>");
            break;
    }

    // Create Inst | Cria a Instrução.
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst
            (Node, op, 0, 0, Data, Memory);

    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Binary Operation | Compila Operação Binária
void CodeGenerator::CompileBinary(BinaryNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (Node->Op == Operator::AND)
    {
        CompileNode(Node->L, State, BC, SARes, Data, Memory);

        ByteInstruction* Jump = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP_IF_FALSE, -1, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(Jump);

        ByteInstruction* Pop = CodeGenUtils::CreateInst
            (Node, OpCode::POP, 0, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(Pop);

        CompileNode(Node->R, State, BC, SARes, Data, Memory);

        Jump->R1 = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());

        return;
    }
    else if (Node->Op == Operator::OR)
    {
        CompileNode(Node->L, State, BC, SARes, Data, Memory);

        ByteInstruction* Jump = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP_IF_TRUE, -1, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(Jump);

        ByteInstruction* Pop = CodeGenUtils::CreateInst
            (Node, OpCode::POP, 0, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(Pop);

        CompileNode(Node->R, State, BC, SARes, Data, Memory);
        Jump->R1 = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());

        return;
    }

    CompileNode(Node->L, State, BC, SARes, Data, Memory);
    CompileNode(Node->R, State, BC, SARes, Data, Memory);

    // Main Switch | Switch Principal.
    OpCode op = OpCode::ADD;
    switch (Node->Op) {
        
        // ARITMETIC | ARITMETICOS.
        case Operator::ADD:           op = OpCode::ADD;       break;
        case Operator::SUB:           op = OpCode::SUB;       break;
        case Operator::MUL:           op = OpCode::MUL;       break;
        case Operator::DIV:           op = OpCode::DIV;       break;
        case Operator::MOD:           op = OpCode::MOD;       break;
        case Operator::POWER:         op = OpCode::POWER;     break;

        // COMP | COMPARAÇOES.
        case Operator::EQUAL:         op = OpCode::CMP_EQ;    break;
        case Operator::NOT_EQUAL:     op = OpCode::CMP_NE;    break;
        case Operator::LESS:          op = OpCode::CMP_LT;    break;
        case Operator::GREATER:       op = OpCode::CMP_GT;    break;
        case Operator::LESS_EQUAL:    op = OpCode::CMP_LE;    break;
        case Operator::GREATER_EQUAL: op = OpCode::CMP_GE;    break;

        // LOGICAL | LOGICOS.
        case Operator::AND:           op = OpCode::AND;       break;
        case Operator::OR:            op = OpCode::OR;        break;

        default:
            if (Data.flags.generateLog)
                OrbitLog::Warn("codegen.cpp", "Unknow Operator: "+std::to_string(static_cast<int>(Node->Op))+" cannot be Compiled, Using 'ADD' Insted, Run Whit '--generateLog=ON' to see Logs. .. ..");
            else
                OrbitLog::Warn("codegen.cpp", "Unknow Operator: "+std::to_string(static_cast<int>(Node->Op))+" cannot be Compiled, Using 'ADD' Insted. check <LOG_FILE>");
    }

    // Create Inst | Cria a Instrução.
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst
            (Node, op, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Variable Assignment | Compila Atribuição de Variável
void CodeGenerator::CompileAssignment(AssignmentNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile | Compila:
    CompileNode(Node->Right, State, BC, SARes, Data, Memory);
    CompileLValue(Node->Left, State, BC, SARes, Data, Memory, true);
}

// Compile Object Member Access | Compila Acesso a Membro de Objeto
void CodeGenerator::CompileMemberAccess(MemberAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Object | Compila o Objeto.
    CompileNode(Node->Object, State, BC, SARes, Data, Memory);

    // Generate Inst | Gera a Instrução.
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst(Node, OpCode::GET_MEMBER, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Array/Table Index Access | Compila Acesso a Índice de Array/Tabela
void CodeGenerator::CompileIndexAccess(IndexAccessNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile | Compila:
    CompileNode(Node->Object, State, BC, SARes, Data, Memory);
    CompileNode(Node->Index, State, BC, SARes, Data, Memory);

    // Generate Instruction | Gera uma Instrução.
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst(Node, OpCode::GET_INDEX, 0, 0, Data, Memory);
    Inst->LX1 = Node->fallback;    
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Function Call | Compila Chamada de Função
void CodeGenerator::CompileFunctionCall(FunctionCall* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile | Compila
    CompileNode(Node->Callee, State, BC, SARes, Data, Memory);
    for (ExpressionNode* Arg : Node->Args)
        CompileNode(Arg, State, BC, SARes, Data, Memory);
    
    // Generate Inst | Gera a Instrução.
    ByteValue argCount = static_cast<i64>(Node->Args.size());
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst(Node, OpCode::CALL, argCount, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Table Literal Value | Compila Valor Literal de Tabela/Dicionário
void CodeGenerator::CompileTableValue(TableValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    
    // Instantiate a Hole Table | Instancia uma Tabela Vazia.
    ByteInstruction* CreateInst = 
        CodeGenUtils::CreateInst(Node, OpCode::BUILD_TABLE, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(CreateInst);

    // Compile Entrys | Compila as Entradas.
    for (ArrayEntry& E : Node->Args)
    {
        CompileNode(E.Key, State, BC, SARes, Data, Memory);
        CompileNode(E.Value, State, BC, SARes, Data, Memory);

        ByteInstruction* SetInst = 
            CodeGenUtils::CreateInst(Node, OpCode::SET_TKEY, 0, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(SetInst);
    }
}

// Compile Array Literal Value | Compila Valor Literal de Array
void CodeGenerator::CompileArrayValue(ArrayValue* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile | Compila:
    for (ExpressionNode* N : Node->Args)
        CompileNode(N, State, BC, SARes, Data, Memory);

    // Generate Instruction
    int S = Node->Args.size();
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst(Node, OpCode::BUILD_ARRAY, S, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Range Expression | Compila Expressão de Intervalo (Range)
void CodeGenerator::CompileRange(RangeNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile | Compila.
    CompileNode(Node->Begin, State, BC, SARes, Data, Memory);
    CompileNode(Node->End, State, BC, SARes, Data, Memory);

    // Generate Inst | Gera a Instrução.
    ByteInstruction* Inst = 
        CodeGenUtils::CreateInst(Node, OpCode::BUILD_RANGE, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Handle/Compile Expression Errors | Manipula/Compila Erros em Expressões
void CodeGenerator::CompileErrorExpr(ErrorExprNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{

}

// Compile L-Values | Compila Valores Esquedos
void CodeGenerator::CompileLValue(ExpressionNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory, bool base)
{
    switch (Node->Type) {
    
        case NodeType::IDENTIFIER:
        {
            IdentifierNode* N = static_cast<IdentifierNode*>(Node);
            OpCode op = base ? OpCode::STORE_LOCAL : OpCode::LOAD_LOCAL;
            ByteInstruction* Inst = CodeGenUtils::
                CreateInst(Node, op, State.GetLocal(N->Name), 0, Data, Memory);
            BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
            break;
        }
        case NodeType::MEMBER_ACCESS:
        {
            MemberAccessNode* N = static_cast<MemberAccessNode*>(Node);

            CompileLValue(N->Object, State, BC, SARes, Data, Memory, false);
            CompileNode(N->Member, State, BC, SARes, Data, Memory);

            OpCode op = base ? OpCode::STORE_MEMBER : OpCode::GET_MEMBER;
            ByteInstruction* Inst = CodeGenUtils::
                CreateInst(Node, op, 0, 0, Data, Memory);
            BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
            break;            
        }
        case NodeType::INDEX_ACCESS:
        {
            IndexAccessNode* N = static_cast<IndexAccessNode*>(Node);

            CompileLValue(N->Object, State, BC, SARes, Data, Memory, false);
            CompileNode(N->Index, State, BC, SARes, Data, Memory);

            OpCode op = base ? OpCode::STORE_INDEX : OpCode::GET_INDEX;
            ByteInstruction* Inst = CodeGenUtils::
                CreateInst(Node, op, 0, 0, Data, Memory);
            BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
            break;
        }
        default:
            OrbitLog::Error("codegen.cpp", "Trying to Assign a non 'L-Value'", true, 1);
    }
}

// DECLARATION

// Compile Variable Declaration | Compila Declaração de Variável
void CodeGenerator::CompileVarDecl(VarDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (SARes.SymbolTable[Node->Name]->read_count == 0 and SARes.SymbolTable[Node->Name]->write_count == 0)
        return;

    // Compile | Compila:
    CompileNode(Node->Val, State, BC, SARes, Data, Memory);
    ui32 ID = State.CreateLocal(Node->Name);

    // Gen Inst | Gera a Instrução.
    ByteInstruction* Inst = CodeGenUtils::
        CreateInst(Node, OpCode::STORE_LOCAL, ID, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Function Declaration | Compila Declaração de Função
void CodeGenerator::CompileFnDecl(FnDecl* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Data
    int PrevC = State.currChunk;
    int FnC = BC.Chunks.size();

    // Create Chunk | Cria a Chunk
    BC.Chunks.push_back(Memory.New<Chunk>());
    BC.Chunks.back()->ParamCount = Node->Params.size();
    BC.Functions[Node->Name] = FnC;
    State.currChunk = FnC;

    for (ExpressionNode* Param : Node->Params)
    {
        IdentifierNode* P = static_cast<IdentifierNode*>(Param);
        State.CreateLocal(P->Name);
    }

    // Compile | Compila
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);

    // Gen Insts | Gera as Instruções.
    ByteInstruction* RetValueInst =
        CodeGenUtils::CreateInst(Node, OpCode::PUSH, NullLitVal{}, 0, Data, Memory);
    ByteInstruction* ReturnInst =
        CodeGenUtils::CreateInst(Node, OpCode::RETURN, 0, 0, Data, Memory);
    
    BC.Chunks[State.currChunk]->Instructions.push_back(RetValueInst);
    BC.Chunks[State.currChunk]->Instructions.push_back(ReturnInst);
    
    State.currChunk = PrevC;
}

// Handle/Compile Declaration Errors | Manipula/Compila Erros em Declarações
void CodeGenerator::CompileErrorDecl(ErrorDeclNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{

}

// CONTROL

// Compile Conditional 'if' Statement | Compila Instrução Condicional 'if'
void CodeGenerator::CompileIf(IfNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    State.IfStates.push_back({});

    // IF
    if (!Node->alwaysTrue) // Condition Is Always True:
    {
        CompileNode(Node->Cond, State, BC, SARes, Data, Memory);
        ByteInstruction* JFALSE_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP_IF_FALSE, -1, 0, Data, Memory);
        ByteInstruction* POP_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::POP, -1, 0, Data, Memory);
         
        BC.Chunks[State.currChunk]->Instructions.push_back(JFALSE_Inst);
        BC.Chunks[State.currChunk]->Instructions.push_back(POP_Inst);

        CompileNode(Node->IfBody, State, BC, SARes, Data, Memory);

        ByteInstruction* JEND_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP, -1, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(JEND_Inst);
        State.IfStates.back().EndJumps.push_back(JEND_Inst);

        JFALSE_Inst->R1 = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());
        ByteInstruction* POP_False_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::POP, -1, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(POP_False_Inst);
    }
    else
    {
        CompileNode(Node->IfBody, State, BC, SARes, Data, Memory);
    }

    // ELSES
    for (ElifNode* N : Node->ElifBodyStack)
    {
        CompileNode(N, State, BC, SARes, Data, Memory);
    } 
    if (Node->ElseBody)
    {
        CompileNode(Node->ElseBody, State, BC, SARes, Data, Memory);
    }

    i64 EndIp = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());
    for (ByteInstruction* J : State.IfStates.back().EndJumps)
        J->R1 = EndIp;

    State.IfStates.pop_back();
}

// Compile Conditional 'else' Block | Compila Bloco Condicional 'else'
void CodeGenerator::CompileElse(ElseNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);
}

// Compile Conditional 'elif' Block | Compila Bloco Condicional 'elif'
void CodeGenerator::CompileElif(ElifNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Inst1
    CompileNode(Node->Cond, State, BC, SARes, Data, Memory);

    ByteInstruction* JFALSE_Inst = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP_IF_FALSE, static_cast<i64>(-1), 0, Data, Memory);
    ByteInstruction* POP_True_Inst = CodeGenUtils::CreateInst
        (Node, OpCode::POP, -1, 0, Data, Memory);

    BC.Chunks[State.currChunk]->Instructions.push_back(JFALSE_Inst);
    BC.Chunks[State.currChunk]->Instructions.push_back(POP_True_Inst);

    CompileNode(Node->Body, State, BC, SARes, Data, Memory);

    // Inst2
    ByteInstruction* JUMP_Inst = CodeGenUtils::CreateInst
        (Node, OpCode::JUMP, static_cast<i64>(-1), 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(JUMP_Inst);

    State.IfStates.back().EndJumps.push_back(JUMP_Inst);

    JFALSE_Inst->R1 = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());

    ByteInstruction* POP_False_Inst = CodeGenUtils::CreateInst
        (Node, OpCode::POP, -1, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(POP_False_Inst);
}

// Compile 'while' Loop | Compila Laço de Repetição 'while'
void CodeGenerator::CompileWhile(WhileNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (Node->alwaysExecuteFirst)
    {
        // Take Curr Ip | Pega o IP Atual.
        int ip = BC.Chunks[State.currChunk]->Instructions.size();

        // Compile | Compila.
        CompileNode(Node->Body, State, BC, SARes, Data, Memory);
        CompileNode(Node->Cond, State, BC, SARes, Data, Memory);

        // Generate Insts | Gera a Instrução.
        ByteInstruction* JFALSE_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP_IF_FALSE, -1, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(JFALSE_Inst);

        ByteInstruction* J_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP, ip, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(J_Inst);

        // Set | Define.
        JFALSE_Inst->R1 = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());
    }
    else
    {
        // Take Curr Ip | Pega o IP Atual.
        int ip = BC.Chunks[State.currChunk]->Instructions.size();

        // Compile | Compila.
        CompileNode(Node->Cond, State, BC, SARes, Data, Memory);

        // Generate Insts | Gera a Instrução.
        ByteInstruction* JFALSE_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP_IF_FALSE, -1, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(JFALSE_Inst);

        // Compile | Compila.
        CompileNode(Node->Body, State, BC, SARes, Data, Memory);

        // Generate Insts | Gera a Instrução.
        ByteInstruction* J_Inst = CodeGenUtils::CreateInst
            (Node, OpCode::JUMP, ip, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(J_Inst);

        // Set | Define.
        JFALSE_Inst->R1 = static_cast<i64>(BC.Chunks[State.currChunk]->Instructions.size());
    }
}

// Compile Standard 'for' Loop | Compila Laço de Repetição 'for' Padrão
void CodeGenerator::CompileFor(ForNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile And Take Insts | Compila e Pega a Instrução:
    auto& Insts = BC.Chunks[State.currChunk]->Instructions;
    CompileNode(Node->End, State, BC, SARes, Data, Memory);

    // Take The Loop Start Pos | Pega a Posição do Inicio do Loop.
    size_t LoopStart = Insts.size();

    // Instructions | Instruções:
    ByteInstruction* HasNext = CodeGenUtils::
        CreateInst(Node, OpCode::ITER_HAS_NEXT, 0, 0, Data, Memory);
    Insts.push_back(HasNext);
    ByteInstruction* ExitJump = CodeGenUtils::
        CreateInst(Node, OpCode::JUMP_IF_FALSE, 0, 0, Data, Memory);
    Insts.push_back(ExitJump);
    ByteInstruction* PopCond = CodeGenUtils::
        CreateInst(Node, OpCode::POP, 0, 0, Data, Memory);
    Insts.push_back(PopCond);
    ByteInstruction* Next = CodeGenUtils::
        CreateInst(Node, OpCode::ITER_NEXT, 0, 0, Data, Memory);
    Insts.push_back(Next);

    int slot = State.CreateLocal(Node->Identifier->Name);
    ByteInstruction* Store = CodeGenUtils::
        CreateInst(Node, OpCode::STORE_LOCAL, slot, 0, Data, Memory);
    Insts.push_back(Store);

    // Compile Body of Loop | Compila o Corpo do Loop.
    CompileNode(Node->Body, State, BC, SARes, Data, Memory);

    // Come Back | Volta Para o inicio.
    ByteInstruction* LoopJump = CodeGenUtils::
        CreateInst(Node, OpCode::JUMP, static_cast<i64>(LoopStart), 0, Data, Memory);
    Insts.push_back(LoopJump);

    size_t LoopEnd = Insts.size();
    ExitJump->R1 = static_cast<i64>(LoopEnd);

    // Remove Iter | Remove o Iterador.
    ByteInstruction* PopCondEnd = CodeGenUtils::
        CreateInst(Node, OpCode::POP, 0, 0, Data, Memory);
    Insts.push_back(PopCondEnd);
    ByteInstruction* Pop = CodeGenUtils::
        CreateInst(Node, OpCode::POP, 0, 0, Data, Memory);
    Insts.push_back(Pop);
}

// Compile 'foreach' Iteration Loop | Compila Laço de Repetição 'foreach'
void CodeGenerator::CompileForEach(ForEachNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{}

// Compile 'for' Loop Default Node | Compila Nó de do Laço de Repetição 'for' Padrão.
void CodeGenerator::CompileForDef(ForDefNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{}

// Compile Return Statement | Compila Instrução de Retorno ('return')
void CodeGenerator::CompileReturn(ReturnNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Expr | Compila a Expressão:
    if (Node->Value)
        CompileNode(Node->Value, State, BC, SARes, Data, Memory);
    else
    {
        ByteInstruction* NullInst = CodeGenUtils::
            CreateInst(Node, OpCode::PUSH, NullLitVal{}, 0, Data, Memory);
        BC.Chunks[State.currChunk]->Instructions.push_back(NullInst);
    }

    // Gen Inst | Gera a Instrução.
    ByteInstruction* Inst = CodeGenUtils::
        CreateInst(Node, OpCode::RETURN, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Compile Echo/Print Statement | Compila Instrução 'echo' (Impressão)
void CodeGenerator::CompileEcho(EchoNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    // Compile Expr | Compila a Expressão:
    CompileNode(Node->Value, State, BC, SARes, Data, Memory);
    
    // Gen Inst | Gera a Instrução.
    ByteInstruction* Inst = CodeGenUtils::
        CreateInst(Node, OpCode::ECHO, 0, 0, Data, Memory);
    BC.Chunks[State.currChunk]->Instructions.push_back(Inst);
}

// Handle/Compile Statement Errors | Manipula/Compila Erros em Instruções (Statements)
void CodeGenerator::CompileErrorStmt(ErrorStmtNode* Node, CodeGenState& State, ByteCode& BC, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{

}


// ========== ENTRY-POINT =========== //

// Generate CodeGen Log | Gera o Log de CodeGen.
void GenerateCodeGenLog(ByteCode& BC, RunTimeData& Data)
{
    fstream file(Data.LogDir, std::ios::out | std::ios::app);
    if (!file.is_open())
    {
        OrbitLog::Error("parser.cpp", 
            "Cannot Open Log File, Why: "+
            string(std::strerror(errno))
            +" Path: "+Data.LogDir.string(),
            errno
        );
    }
    file << "\n// ============ CODE-GENERATION =========== //\n\n";
    int ic=0;
    for (Chunk* C : BC.Chunks)
    {
        file << "CHUNK["+std::to_string(ic)+"]: \n";
        int i=0;
        for (ByteInstruction* Inst : C->Instructions)
        {
            if (i == C->Instructions.size())
                file << "\tOpCode: ["+std::to_string(i)+"]: "+std::to_string(static_cast<int>(Inst->C))+",\n";
            else file << "\tOpCode: ["+std::to_string(i)+"]: "+std::to_string(static_cast<int>(Inst->C))+"\n";
            i++;
        }
        ic++;
    }
    file << "\n\n// ============ ENDOF: 'CODE-GENERATION'. .. ... =========== //\n\n";
}

// Entry Point of CodeGenerator Utils
// Ponto de Entrada do Programa de geração de Codigo.
ByteCode CodeGenerator::InitCG(ParseResult& PRes, SAResult& SARes, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: Compile ORBIT");

    ByteCode BC;
    BC.Chunks.push_back(Memory.New<Chunk>());
    CodeGenState State; 
    
    CompileNode(PRes.AST, State, BC, SARes, Data, Memory);

    if (Data.flags.generateLog)
        GenerateCodeGenLog(BC, Data);
    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: 'Compile ORBIT'. .. ...");
    return BC;
}

// EOF