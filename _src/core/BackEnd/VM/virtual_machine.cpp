
// ========== VIRTUAL-MACHINE =========== //
// OVM - Orbit Virtual Machine | Maquina Virtual ORBIT.
// Developed By: SpyK3(2026) | License: MIT(GitHub).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMANTICAS.
#pragma once

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
}
// ========== CORE =========== //

// Main Function, Run ORBIT | Função Principal, Roda a ORBIT.
int VirtualMachine::Run(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    InstructionPointer IP = {0};
    size_t codeSize = BC.Chunks[0]->Instructions.size();
    while (IP.Index < codeSize) // Main Loop | Loop principal:
    {
        // Take OpCode | Pega o Codigo de Operação:
        OpCode OP = BC.Chunks[0]->Instructions[IP.Index]->C;
        Chunk* CurrChunk = BC.Chunks[0];
        auto& CurrInst = BC.Chunks[0]->Instructions[IP.Index];
        auto& Insts = CurrChunk->Instructions;

        // Main Switch | Switch Principal:
        switch (OP) {

            // STACK-CONTROL:
            case OpCode::PUSH:
                CallStack->GetTop()->Stack.push_back(CurrInst->R1);
                break;
            
            // OTHERS:
            case OpCode::ECHO:
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
