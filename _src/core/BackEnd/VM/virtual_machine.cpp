
// ========== VIRTUAL-MACHINE =========== //
// OVM - Orbit Virtual Machine | Maquina Virtual ORBIT.
// Developed By: SpyK3(2026) | License: MIT(GitHub).

// INCLUDE HEADERS 'N DEPENDENCES
#include "virtual_machine.hpp" // HEADER FILE | CABEÇALHO.

#include "../byte_code.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"
#include "../../FrontEnd/SA/semantic_analysis.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <cstddef>
#include <string>
#include <cmath>

// ========== UTILS =========== //

// Utils of O-VM | Utilidades da O-VM.
namespace VM_Utils {

    // Convert Left To Right | Converte Esquerda para a Direita.
    ByteValue ConvertValue(ByteValue& Val, TypeKind K1, SubTypeKind K2, NodePos& Pos, RunTimeData& Data)
    {
        switch (K1) {
        
            // INT & FLOATS
            case TypeKind::NUMBER:
            {
                if (K2 == SubTypeKind::INT)
                {
                    if (holds_alt<i64>(Val))
                        return std::get<i64>(Val);
                    else if (holds_alt<float>(Val))
                        return static_cast<i64>(std::round(std::get<float>(Val)));
                    else if (holds_alt_value<bool>(Val, true))
                        return static_cast<i64>(1);
                    else if (holds_alt_value<bool>(Val, false))
                        return static_cast<i64>(0);
                    else if (holds_alt<NoneLitVal>(Val)) {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <NUMBER>, To: <NONE>", 
                            "Cannot Cast Left And Right", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    }
                    else if (holds_alt<NullLitVal>(Val)) {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <INT>, To: <NULL>", 
                            "Cannot Cast Left And Right", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    } else {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <INT>, To: <UNK>", 
                            "Cannot Cast Left And Right", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    }
                }
                else if (K2 == SubTypeKind::FLOAT)
                {
                    if (holds_alt<float>(Val))
                        return std::get<float>(Val);
                    else if (holds_alt<i64>(Val))
                        return static_cast<float>(std::get<i64>(Val));
                    else if (holds_alt_value<bool>(Val, true))
                        return static_cast<float>(1.0f);
                    else if (holds_alt_value<bool>(Val, false))
                        return static_cast<float>(0.0f);
                    else if (holds_alt<NoneLitVal>(Val)) {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <FLOAT>, To: <NONE>", 
                            "Cannot Cast Left And Right", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    }
                    else if (holds_alt<NullLitVal>(Val)) {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <FLOAT>, To: <NULL>", 
                            "Cannot Cast Left And Right", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    } else {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <FLOAT>, To: <UNK>", 
                            "Cannot Cast Left And Right", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    }
                }
                break;
            }
            // BOOLEAN
            case TypeKind::BOOL:
            {
                if (holds_alt<string>(Val))
                {
                    if (std::get<string>(Val) == "true" or std::get<string>(Val) == "True")
                        return true;
                    else if (std::get<string>(Val) == "false" or std::get<string>(Val) == "False")
                        return false;
                    else {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "RunTime", 
                            "Non Viable Conversion In: <STRING>, To: <BOOLEAN>", 
                            "Cannot Cast Left And Right. Strings ONLY Can be Converted if hes Text is 'True'/'true' or 'False'/'false'.", 
                            "Add A Valid Type",
                            Pos.line, Pos.collumn
                        );
                        OrbitLog::SyntaxLog::ThrowLog(Data);                       
                    }
                } else if (holds_alt<bool>(Val)) {
                    return Val;
                } else {
                    
                    OrbitLog::SyntaxLog::SyntaxError(
                        "RunTime", 
                        "Non Viable Conversion In: <ANY>, To: <BOOLEAN>", 
                        "Cannot Cast Left And Right", 
                        "Add A Valid Type",
                        Pos.line, Pos.collumn
                    );
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                }
            }
            // STRING
            case TypeKind::STRING:
            {
                if (holds_alt<bool>(Val)) 
                {
                    if (std::get<bool>(Val) == true)
                        return "true";
                    else return "false";
                } else if (holds_alt<i64>(Val))
                    return std::to_string(std::get<i64>(Val));
                else if (holds_alt<float>(Val))
                    return std::to_string(std::get<float>(Val));
                else if (holds_alt<string>(Val))
                    return std::get<string>(Val);
                else if (holds_alt<NoneLitVal>(Val))
                    return "<NONE>";
                else if (holds_alt<NullLitVal>(Val)) 
                    return "<NULL>";
                else if (holds_alt<shared_ptr<ByteArray>>(Val)) {
                    shared_ptr<ByteArray> Arr = std::get<shared_ptr<ByteArray>>(Val);
                    string ret = "[";
                    int i=0;
                    for (ByteValue Val : *Arr)
                    {
                        ret += std::get<string>(ConvertValue(Val, TypeKind::STRING, SubTypeKind::NONE, Pos, Data));
                        if (i < static_cast<int>(Arr->size()) - 1)
                            ret += ", ";
                        i++;
                    }
                    ret += "]";
                    return ret;
                }
                else {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "RunTime", 
                        "Non Viable Conversion In: <STRING>, To: <UNK>", 
                        "Cannot Cast Left And Right", 
                        "Add A Valid Type",
                        Pos.line, Pos.collumn
                    );
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                }
                break;
            }
            // NONE
            case TypeKind::NONE:
            {
                if (holds_alt<NoneLitVal>(Val))
                    return NoneLitVal{};
                else {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "RunTime", 
                        "Non Viable Conversion In: <NONE>, To: <ANY>", 
                        "Cannot Cast Left And Right", 
                        "Add A Valid Type",
                        Pos.line, Pos.collumn
                    );
                    OrbitLog::SyntaxLog::ThrowLog(Data);                    
                }
                break;
            }
            // NULL
            case TypeKind::_NULL:
            {
                if (holds_alt<NullLitVal>(Val))
                    return NullLitVal{};
                else {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "RunTime", 
                        "Non Viable Conversion In: <NULL>, To: <ANY>", 
                        "Cannot Cast Left And Right", 
                        "Add A Valid Type",
                        Pos.line, Pos.collumn
                    );
                    OrbitLog::SyntaxLog::ThrowLog(Data);                    
                }
                break;
            }
            // BYTE ARRAY
            case TypeKind::TABLE:
            {
                if (holds_alt<shared_ptr<ByteArray>>(Val))
                    return std::get<shared_ptr<ByteArray>>(Val);
                else {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "RunTime", 
                        "Non Viable Conversion In: <TABLE>, To: <ANY>", 
                        "Cannot Cast Left And Right", 
                        "Add A Valid Type",
                        Pos.line, Pos.collumn
                    );
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                }
                break;
            }

            default: return 0;
        }
        return 0;
    }

    // Compare 2 Values | Compara 2 Valores.
    bool CompareEqual(const ByteValue& L, const ByteValue& R)
    {
        if (L.index() != R.index())
            return false;

        return std::visit([&](const auto& A) -> bool {
            using T = std::decay_t<decltype(A)>;

            const auto& B = std::get<T>(R);

            if constexpr (
                std::is_same_v<T, NoneLitVal> ||
                std::is_same_v<T, NullLitVal>
            )
            {
                return true;
            }
            else if constexpr (std::is_same_v<T, shared_ptr<ByteArray>>)
            {
                if (A == nullptr || B == nullptr)
                    return A == B;

                if (A->size() != B->size())
                    return false;

                for (size_t i = 0; i < A->size(); ++i)
                {
                    if (!CompareEqual((*A)[i], (*B)[i]))
                        return false;
                }

                return true;
            }
            else
            {
                return A == B;
            }

        }, L);
    }
}

// ========== CORE =========== //

// Run Binary Arithmetic Operations | Roda Operações Binarias Aritmeticas.
int VirtualMachine::RunBinary(ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    auto& CurrInst = BC.Chunks[0]->Instructions[IP.Index];

    ByteValue R = CallStack->GetTop()->Pop();
    ByteValue L = CallStack->GetTop()->Pop();

    switch (CurrInst->C) {

        case OpCode::ADD:
        {
            if (holds_alt<string>(L) || holds_alt<string>(R))
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::STRING, SubTypeKind::NONE, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::STRING, SubTypeKind::NONE, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<string>(Left) + std::get<string>(Right)
                );
            }
            else if (holds_alt<float>(L) || holds_alt<float>(R))
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<float>(Left) + std::get<float>(Right)
                );
            }
            else
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<i64>(Left) + std::get<i64>(Right)
                );
            }
            break;
        }

        case OpCode::SUB:
        {
            if (holds_alt<float>(L) || holds_alt<float>(R))
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<float>(Left) - std::get<float>(Right)
                );
            }
            else
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<i64>(Left) - std::get<i64>(Right)
                );
            }
            break;
        }

        case OpCode::MUL:
        {
            if (holds_alt<string>(L) && holds_alt<i64>(R))
            {
                string Str = std::get<string>(L);
                i64 Count = std::get<i64>(R);

                string Result;

                for (i64 i = 0; i < Count; i++)
                    Result += Str;

                CallStack->GetTop()->PushBack(Result);
            }
            else if (holds_alt<string>(R) && holds_alt<i64>(L))
            {
                string Str = std::get<string>(R);
                i64 Count = std::get<i64>(L);

                string Result;

                for (i64 i = 0; i < Count; i++)
                    Result += Str;

                CallStack->GetTop()->PushBack(Result);
            }
            else if (holds_alt<float>(L) || holds_alt<float>(R))
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<float>(Left) * std::get<float>(Right)
                );
            }
            else
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::get<i64>(Left) * std::get<i64>(Right)
                );
            }
            break;
        }

        case OpCode::DIV:
        {
            ByteValue Left = VM_Utils::ConvertValue
            (L, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

            ByteValue Right = VM_Utils::ConvertValue
            (R, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

            CallStack->GetTop()->PushBack(
                std::get<float>(Left) / std::get<float>(Right)
            );
            break;
        }

        case OpCode::POWER:
        {
            if (holds_alt<float>(L) || holds_alt<float>(R))
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::FLOAT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    std::pow(
                        std::get<float>(Left),
                        std::get<float>(Right)
                    )
                );
            }
            else
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    static_cast<i64>(
                        std::pow(
                            std::get<i64>(Left),
                            std::get<i64>(Right)
                        )
                    )
                );
            }
            break;
        }

        default:
            OrbitLog::Error("virtual_machine.cpp", "Trying to Make a Aritm Operation Whit Unknow OpCode", true, 1);
    }

    return 0;
}

// Compare 2 Values | Compara 2 Valores
int VirtualMachine::RunComp(const ByteValue& L, const ByteValue& R, OpCode Op)
{
    VM_Frame* Frame = CallStack->GetTop();

    bool Equal = VM_Utils::CompareEqual(L, R);
    bool Result = false;

    switch (Op)
    {
        case OpCode::CMP_EQ:
            Result = Equal;
            break;

        case OpCode::CMP_NE:
            Result = !Equal;
            break;

        default:
            break;
    }

    Frame->PushBack(Result);
    return 0;
}

// Set Unary Values | Define Valores Unarios.
int VirtualMachine::RunUnary(ByteValue& O, OpCode Op, ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Main Switch | Switch Principal.
    switch (Op) {
    
        case OpCode::NEG:
            if (holds_alt<i64>(O))
                CallStack->GetTop()->PushBack(-std::get<i64>(O));
            else if (holds_alt<float>(O))
                std::get<float>(O) = -std::get<float>(O);
            break;        
        case OpCode::NOT:

            CallStack->GetTop()->PushBack(!std::get<bool>(VM_Utils::ConvertValue
            (O, TypeKind::BOOL, SubTypeKind::NONE, BC.Chunks[BC.currChunk]->Instructions[IP.Index]->Pos, Data)));
            break;
        default: CallStack->GetTop()->PushBack(0);
    }

    return 0;
}

// Main Function, Run ORBIT | Função Principal, Roda a ORBIT.
int VirtualMachine::Run(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    InstructionPointer IP = {0};
    size_t codeSize = BC.Chunks[BC.currChunk]->Instructions.size();
    while (IP.Index < codeSize) // Main Loop | Loop principal:
    {
        // Take OpCode | Pega o Codigo de Operação:
        OpCode OP = BC.Chunks[BC.currChunk]->Instructions[IP.Index]->C;
        Chunk* CurrChunk = BC.Chunks[BC.currChunk];
        auto& CurrInst = BC.Chunks[BC.currChunk]->Instructions[IP.Index];
        auto& Insts = CurrChunk->Instructions;

        // Main Switch | Switch Principal:
        switch (OP) {

            // STACK-CONTROL:
            case OpCode::PUSH: // Push A New Value to Sack | Coloca Um Novo Valor na Pilha:
                CallStack->GetTop()->PushBack(CurrInst->R1);
                break;

            case OpCode::POP: // Remove Last Value of Stack | Remove o Ultimo Valor da Pilha:
                CallStack->GetTop()->Pop();
                break;

            // OPERATIONS:

            // Aritm | Aritmeticos:
            case OpCode::ADD:
            case OpCode::SUB:
            case OpCode::MUL:
            case OpCode::DIV:
            case OpCode::POWER:
                { RunBinary(BC, IP, Res, Data, Memory); break; }

            // Comp | Comparaçoes:
            case OpCode::CMP_EQ:
            case OpCode::CMP_NE:
            case OpCode::CMP_LT:
            case OpCode::CMP_LE:
            case OpCode::CMP_GT:
            case OpCode::CMP_GE:
                RunComp
                (CallStack->GetTop()->Pop(), CallStack->GetTop()->Pop(), CurrInst->C);
                break;
                
            // Unary | Unarios
            case OpCode::NEG:
            case OpCode::NOT:
            {
                ByteValue V = CallStack->GetTop()->Pop();
                RunUnary(V, CurrInst->C, BC, IP, Res, Data, Memory);
                break;
            }
            
            // LOADS & GETS:
            case OpCode::LOAD_LOCAL: // Load A Local | Carrega Um Local:
                CallStack->GetTop()->PushBack(CallStack->GetTop()->Locals[std::get<i64>(CurrInst->R1)]);
                break;

            case OpCode::STORE_LOCAL: // Store A New Local | Guarda Um Novo Local:
            {
                ByteValue Val = CallStack->GetTop()->Pop();
                CallStack->GetTop()->Locals[std::get<i64>(CurrInst->R1)] = Val;
                break;
            }
            
            // CONTROL-FLOW

            // OTHERS:
            case OpCode::ECHO: // Log in Console | Informa no Console:
            {
                ByteValue Val = CallStack->GetTop()->Pop();
                ByteValue Converted = VM_Utils::ConvertValue
                (Val, TypeKind::STRING, SubTypeKind::NONE, CurrInst->Pos, Data);
                
                std::cout << std::get<string>(Converted);
                break;
            }
            default:
                OrbitLog::Error("virtual_machine", "unknow OPCODE: "+std::to_string(static_cast<int>(OP)), false, 404);
                return 1;
        }
        IP.Index++;
    }
    return 0;
}

// ========== ENTRY-POINT =========== //
// Entry-Point Of Virtual-Machine | Ponto-de-Entrada da Maquina-Virtual.
void VirtualMachine::InitVM(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Create Call Stack | Cria a Call-Stack
    this->CallStack.emplace(Memory);

    // Create Global Frame | Cria um Quadro-Global.
    VM_Frame* MainF = Memory.New<VM_Frame>();
    MainF->Back = nullptr;

    // Set Entry | Define a Entrada.
    InstructionPointer EntryIP = {0}; 
    this->CallStack->Push(MainF, &EntryIP);

    auto Return = Run(BC, Res, Data, Memory);

};

// EOF
