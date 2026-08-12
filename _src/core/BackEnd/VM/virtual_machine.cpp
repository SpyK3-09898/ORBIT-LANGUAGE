
// =========== VM ========== //
// Virtual Machine ORBIT Runner | CPU-Virtual Para Rodar Codigo ORBIT.
// Developed By SpyK3(2026) | License: GitHub(MIT). 
// INCLUDE HEADERS 'N DEPENDENCES

#include "virtual_machine.hpp" // HEADER FILE | CABEÇALHO
#include "../codegen/codegen.hpp"
#include "../byte_code.hpp"

#include "../../FrontEnd/SA/semantic_analysis.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <cmath>
#include <string>

// ========= UTILS || UTILIDADES =========== //

// Utils of VM || Utilidades da Maquina Virtual.
namespace VM_Utils {

    // Execute Binary Operations || Executa Operações Binarias.
    ByteValue RunBinary(OpCode Op, ByteValue L, ByteValue R, NodePos& P, RunTimeData& Data)
    {
        auto Err = [&](string n)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "RunTime",
                "Cannot Convert",
                n+" Cannot be Casted Whit Other Side",
                "Add A Valid Type Or Convert",
                P.line, P.collumn
            );
            OrbitLog::SyntaxLog::ThrowLog(Data);
        };

        switch (Op) {
        
            // '+'.
            case OpCode::ADD:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<i64>(L) + std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<i64>(L) + std::get<float>(R);
                    else if (holds_alt<string>(R))
                        return 
                            std::to_string(std::get<i64>(L)) 
                            + std::get<string>(R);
                    else {
                        Err("<INT>");
                    }
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<float>(L) + std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<float>(L) + std::get<float>(R);
                    else if (holds_alt<string>(R))
                        return 
                            std::to_string(std::get<float>(L)) 
                            + std::get<string>(R);
                    else {
                        Err("<FLOAT>");
                    }
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return 
                            std::get<string>(L) + std::get<string>(R);
                    else if (holds_alt<i64>(R))
                        return 
                            std::get<string>(L) 
                            + std::to_string(std::get<i64>(R));
                    else if (holds_alt<float>(R))
                        return 
                            std::get<string>(L) 
                            + std::to_string(std::get<float>(R));
                    else {
                        Err("<STRING>");
                    }
                }

                break;
            }

            // '-'.
            case OpCode::SUB:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<i64>(L) - std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<i64>(L) - std::get<float>(R);
                    else {
                        Err("<INT>");
                    }
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<float>(L) - std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<float>(L) - std::get<float>(R);
                    else {
                        Err("<FLOAT>");
                    }
                }

                break;
            }

            // '*'.
            case OpCode::MUL:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<i64>(L) * std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<i64>(L) * std::get<float>(R);
                    else {
                        Err("<INT>");
                    }
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<float>(L) * std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<float>(L) * std::get<float>(R);
                    else {
                        Err("<FLOAT>");
                    }
                }

                break;
            }

            // '/'.
            case OpCode::DIV:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            static_cast<float>(std::get<i64>(L)) 
                            / std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<i64>(L) 
                            / std::get<float>(R);
                    else {
                        Err("<INT>");
                    }
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<float>(L) 
                            / std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return 
                            std::get<float>(L) 
                            / std::get<float>(R);
                    else {
                        Err("<FLOAT>");
                    }
                }

                break;
            }

            // '%'.
            case OpCode::MOD:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            std::get<i64>(L) % std::get<i64>(R);
                    else {
                        Err("<INT>");
                    }
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            float(std::fmod(
                                std::get<float>(L),
                                std::get<i64>(R)
                            ));
                    else if (holds_alt<float>(R))
                        return 
                            std::fmod(
                                std::get<float>(L),
                                std::get<float>(R)
                            );
                    else {
                        Err("<FLOAT>");
                    }
                }

                break;
            }

            // '^'.
            case OpCode::POWER:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            float(std::pow(
                                std::get<i64>(L),
                                std::get<i64>(R)
                            ));
                    else if (holds_alt<float>(R))
                        return 
                            float(std::pow(
                                std::get<i64>(L),
                                std::get<float>(R)
                            ));
                    else {
                        Err("<INT>");
                    }
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return 
                            float(std::pow(
                                std::get<float>(L),
                                std::get<i64>(R)
                            ));
                    else if (holds_alt<float>(R))
                        return 
                            std::pow(
                                std::get<float>(L),
                                std::get<float>(R)
                            );
                    else {
                        Err("<FLOAT>");
                    }
                }

                break;
            }

            default:
                return 0;
        }

        return 0;
    }

    // Get Member of M-Acess | Pega o Membro de Acessos.
    ByteValue RunMemberAcess(ByteValue Object, ByteValue Member, NodePos& P, RunTimeData& Data)
    {
        if (false);
        else {
            OrbitLog::SyntaxLog::SyntaxError(
                "RunTime",
                "Trying To Acess A Non-Object",
                "This Object Dont Have Member-Acess",
                "~", P.line, P.collumn
            );
            OrbitLog::SyntaxLog::ThrowLog(Data);
            return false;
        }
    }

    // Get Value of I-Acess | Pega o Membro de Indices.
    ByteValue RunIndexAcess(ByteValue Object, ByteValue Member, NodePos& P, RunTimeData& Data)
    {
        if (holds_alt<shared_ptr<ByteArray>>(Object))
        {
            
        }

        return Object;
    }

    // Execute Comparisons || Executa Comparações.
    bool RunComparision(OpCode Op, ByteValue L, ByteValue R, NodePos& P, RunTimeData& Data)
    {
        auto Err = [&](string n)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "RunTime",
                "Cannot Compare",
                n+" Cannot be Compared Whit Other Side",
                "Add A Valid Type Or Convert",
                P.line, P.collumn
            );
            OrbitLog::SyntaxLog::ThrowLog(Data);
        };

        switch (Op) {

            // '=='.
            case OpCode::CMP_EQ:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<i64>(L) == std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<i64>(L) == std::get<float>(R);
                    else
                        Err("<INT>");
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<float>(L) == std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<float>(L) == std::get<float>(R);
                    else
                        Err("<FLOAT>");
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return std::get<string>(L) == std::get<string>(R);
                    else
                        Err("<STRING>");
                }
                else if (holds_alt<bool>(L))
                {
                    if (holds_alt<bool>(R))
                        return std::get<bool>(L) == std::get<bool>(R);
                    else
                        Err("<BOOL>");
                }
                else if (holds_alt<NoneLitVal>(L))
                {
                    if (holds_alt<NoneLitVal>(R))
                        return true;
                    else
                        return false;
                }
                else if (holds_alt<NullLitVal>(L))
                {
                    if (holds_alt<NullLitVal>(R))
                        return true;
                    else
                        return false;
                }

                break;
            }

            // '!='.
            case OpCode::CMP_NE:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<i64>(L) != std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<i64>(L) != std::get<float>(R);
                    else
                        Err("<INT>");
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<float>(L) != std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<float>(L) != std::get<float>(R);
                    else
                        Err("<FLOAT>");
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return std::get<string>(L) != std::get<string>(R);
                    else
                        Err("<STRING>");
                }
                else if (holds_alt<bool>(L))
                {
                    if (holds_alt<bool>(R))
                        return std::get<bool>(L) != std::get<bool>(R);
                    else
                        Err("<BOOL>");
                }
                else if (holds_alt<NoneLitVal>(L))
                {
                    if (holds_alt<NoneLitVal>(R))
                        return false;
                    else
                        return true;
                }
                else if (holds_alt<NullLitVal>(L))
                {
                    if (holds_alt<NullLitVal>(R))
                        return false;
                    else
                        return true;
                }

                break;
            }

            // '<'.
            case OpCode::CMP_LT:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<i64>(L) < std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<i64>(L) < std::get<float>(R);
                    else
                        Err("<INT>");
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<float>(L) < std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<float>(L) < std::get<float>(R);
                    else
                        Err("<FLOAT>");
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return std::get<string>(L) < std::get<string>(R);
                    else
                        Err("<STRING>");
                }
                else
                {
                    Err("Value");
                }

                break;
            }

            // '<='.
            case OpCode::CMP_LE:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<i64>(L) <= std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<i64>(L) <= std::get<float>(R);
                    else
                        Err("<INT>");
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<float>(L) <= std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<float>(L) <= std::get<float>(R);
                    else
                        Err("<FLOAT>");
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return std::get<string>(L) <= std::get<string>(R);
                    else
                        Err("<STRING>");
                }
                else
                {
                    Err("Value");
                }

                break;
            }

            // '>'.
            case OpCode::CMP_GT:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<i64>(L) > std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<i64>(L) > std::get<float>(R);
                    else
                        Err("<INT>");
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<float>(L) > std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<float>(L) > std::get<float>(R);
                    else
                        Err("<FLOAT>");
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return std::get<string>(L) > std::get<string>(R);
                    else
                        Err("<STRING>");
                }
                else
                {
                    Err("Value");
                }

                break;
            }

            // '>='.
            case OpCode::CMP_GE:
            {
                if (holds_alt<i64>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<i64>(L) >= std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<i64>(L) >= std::get<float>(R);
                    else
                        Err("<INT>");
                }
                else if (holds_alt<float>(L))
                {
                    if (holds_alt<i64>(R))
                        return std::get<float>(L) >= std::get<i64>(R);
                    else if (holds_alt<float>(R))
                        return std::get<float>(L) >= std::get<float>(R);
                    else
                        Err("<FLOAT>");
                }
                else if (holds_alt<string>(L))
                {
                    if (holds_alt<string>(R))
                        return std::get<string>(L) >= std::get<string>(R);
                    else
                        Err("<STRING>");
                }
                else
                {
                    Err("Value");
                }

                break;
            }

            default:
                return false;
        }

        return false;
    }
}

// ========== ENTRY-POINT || PONTO DE ENTRADA ========= //

// Run A Instruction || Roda Uma Instrução.
void VirtualMachine::Run(RunTimeData& Data, Arena& Memory)
{
    // MAIN LOOP | LOOP PRINCIPAL.
    while (running) {
    

        // Data
        VM_Frame& Frame = Calls.Top(); // Error prev | Prevenções de Erros:
        if (Frame.IP.Index >= Frame.C->Instructions.size())
            { running = false; break; }
        ByteInstruction* Inst =
            Frame.C->Instructions[Frame.IP.Index++];
    
        // Main Switch | Switch Principal.
        switch(Inst->C)
        {
            // LOADS | CARREGAMENTOS.
            case OpCode::LOAD_CONST:
                if (generate_log)
                    logFile << "LOADING CONSTANT, IP: "+std::to_string(Frame.IP.Index)+"\n";
                St.Push(Inst->R1);
                break;


            // OPERATIONS | OPERAÇÕES:
            case OpCode::ADD:
            case OpCode::SUB:
            case OpCode::MUL:
            case OpCode::DIV:
            case OpCode::POWER:
            case OpCode::MOD:
            {
                if (generate_log)
                    logFile << "POPING, POPING, ADDING, IP: "+std::to_string(Frame.IP.Index)+"\n";
                
                auto R = St.Pop();
                auto L = St.Pop();

                St.Push
                    (VM_Utils::RunBinary(Inst->C, L, R, Inst->Pos, Data));
                break;
            }

            // COMPARE | COMPARAÇÕES:
            case OpCode::CMP_EQ:
            case OpCode::CMP_NE:
            case OpCode::CMP_LT:
            case OpCode::CMP_LE:
            case OpCode::CMP_GE:
            {
                if (generate_log)
                    logFile << "POPING, POPING, COMPARING, IP: "+std::to_string(Frame.IP.Index)+"\n";
                
                auto R = St.Pop();
                auto L = St.Pop();

                St.Push
                    (VM_Utils::RunComparision(Inst->C, L, R, Inst->Pos, Data));
                break;   
            }


            // LOADS & GETS:
            case OpCode::LOAD_LOCAL: 
            {
                if (generate_log)
                    logFile << "LOADING LOCAL, IP: "+std::to_string(Frame.IP.Index)+"\n";
                
                i64 slotIndex = std::get<i64>(Inst->R1);
                if (slotIndex < 0 || slotIndex >= static_cast<i64>(Calls.Top().Locals.size()))
                {
                    OrbitLog::Error("virtual_machine.cpp", "Slot Index OutOfRange: "+std::to_string(slotIndex)+", Aborting. .. ...", true, ORBIT_ERRORS_CODE::RUNTIME_ERROR);
                }
                St.Push(Calls.Top().Locals[std::get<i64>(Inst->R1)]);
                break;
            }

            case OpCode::STORE_LOCAL:
            {
                if (generate_log)
                    logFile << "STORING LOCAL, IP: " << Frame.IP.Index << "\n";
                i64 slotIndex = std::get<i64>(Inst->R1);
                if (slotIndex >= static_cast<i64>(Calls.Top().Locals.size()))
                    Calls.Top().Locals.resize(static_cast<size_t>(slotIndex + 1));
                Calls.Top().Locals[slotIndex] = St.Pop();
                break;
            }
                
            case OpCode::LOAD_INDEX:
            {
                if (generate_log)
                    logFile << "LOADING INDEX, IP: "+std::to_string(Frame.IP.Index)+"\n";

                ByteValue Index = St.Pop();
                ByteValue Object = St.Pop();

                St.Push(
                    VM_Utils::RunIndexAcess(
                        Object,
                        Index,
                        Inst->Pos,
                        Data
                    )
                );

                break;
            }

            case OpCode::LOAD_MEMBER:
            {
                break;
            }

            // BUILDS | CONSTRUÇÕES:
            case OpCode::BUILD_ARRAY:
            {
                ui32 Count = std::get<i64>(Inst->R1);
                ByteArray Array;
                Array.resize(Count);
                for (i64 i = Count - 1; i >= 0; i--)
                    Array[i] = St.Pop();

                St.Push(std::make_shared<ByteArray>(Array));
                break;
            }

            // CONTROL-FLOWs
            case OpCode::JUMP:
            {
                logFile << "JUMPING, IP:  "+std::to_string(Frame.IP.Index)+"\n";
                
                if (std::get<i64>(Inst->R1) < Frame.C->Instructions.size())
                {
                    OrbitLog::Error("virtual_machine.cpp", "Trying To Jump a OUT-OF-RANGE Ip", true);
                }
                Frame.IP.Index = std::get<i64>(Inst->R1);
                break;
            }
            case OpCode::JUMP_IF_FALSE:
            {
                if (std::get<i64>(Inst->R1) < Frame.C->Instructions.size())
                {
                    OrbitLog::Error("virtual_machine.cpp", "Trying To Jump a OUT-OF-RANGE Ip", true);
                }

                logFile << "COMPARING, FALSE : TRUE ? JUMP, IP: "+std::to_string(Frame.IP.Index)+"\n";
                
                ByteValue Cond = St.Pop();
                if (!std::get<bool>(Cond))
                    Frame.IP.Index = std::get<i64>(Inst->R1);
                break;
            }

            // OTHERS
            case OpCode::ECHO:
            {
                if (generate_log)
                    logFile << "PRINTING, IP: "+std::to_string(Frame.IP.Index)+"\n";;
                ByteValue Value = St.Pop();
                std::cout << ByteValueToString(Value);
                break;
            }

            // ERRORS / NOT IMPLEMENTED || ERROS / NAO IMPLEMENTADOS:
            default:
                logFile << "UNKNOW! EXITING. .. ... IP: "+std::to_string(Frame.IP.Index)+"\n";
                running=false;
                        break;
        }
    }
}

// Entry-Point of Run Program || Ponto-De-Entrada do Programa de Execução.
void VirtualMachine::InitVM(ByteCode& BC, RunTimeData& Data, Arena& Memory)
{
    // DEBUG
    if (Data.flags.generateLog)
    {
        logFile = fstream(Data.LogDir, std::ios::out | std::ios::app);
        logFile << "\n\n// =========== VIRTUAL-MACHINE ========== //\n\n";
        generate_log = Data.flags.generateLog;
    }

    // DATA
    VM_Frame       Frame;
    Code           = &BC;
    Frame.C        = BC.Chunks[1];
    Frame.IP.Index = 0;

    // Set | Define.
    Calls.Push(Frame);
    running = true;

    // Run Orbit | Roda A Orbit.
    Run(Data, Memory);
    if (Data.flags.generateLog) // Logs:
        logFile << "\n// =========== ENDOF: 'VIRTUAL-MACHINE EXEC'. .. ... ========== //\n\n";
    logFile.flush();
    logFile.close();
}

// EOF