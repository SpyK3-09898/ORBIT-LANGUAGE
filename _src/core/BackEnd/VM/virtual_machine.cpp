
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
#include <algorithm>
#include <type_traits>

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
                else if (holds_alt<ByteFn*>(Val)) return "Function";
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

    // Convert Left to String To Debug | Converte Esquerda para a Direita Para Debug.
    string ConvertByteToString(ByteValue& Val)
    {
        if (holds_alt_value<bool>(Val, false)) return "false";
        else if (holds_alt_value<bool>(Val, true)) return "true";
        else if (holds_alt<float>(Val)) 
            return std::to_string(std::get<float>(Val));
        else if (holds_alt<i64>(Val))
            return std::to_string(std::get<i64>(Val));
        else if (holds_alt<string>(Val)) return std::get<string>(Val);
        else if (holds_alt<NoneLitVal>(Val)) return "None";
        else if (holds_alt<NullLitVal>(Val)) return "Null";
        else if (holds_alt<shared_ptr<ByteArray>>(Val)) return "{..}";
        else if (holds_alt<ByteIterator*>(Val)) return "Iterator";
        else if (holds_alt<ByteFn*>(Val)) return "Function";
        else return "Unk";
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
            else if constexpr(std::is_same_v<T, ByteIterator>)
            {
                return false;
            }
            else
            {
                return A == B;
            }

        }, L);
    }
}

// =========== GARBAGE-COLLECTOR ========== //

// Update GC | Atualiza o GC.
void GarbageCollector::Update(ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Take Stack Objects | Pega os Objetos da Stack.
    vec<ObjectDescr*> StackObjs{};
    VM_Frame* CurrFrame = VM->CallStack->GetTop();

    while (CurrFrame)
    {
        for (ByteValue& Val : CurrFrame->Stack)
        {
            if (holds_alt<ByteFn*>(Val))
                StackObjs.push_back(std::get<ByteFn*>(Val)->Descr);
            else if (holds_alt<ByteIterator*>(Val))
                StackObjs.push_back(std::get<ByteIterator*>(Val)->Descr);
        }
        CurrFrame = CurrFrame->Back;
    }

    // Take Descriptions | Pega as Descriçoes.
    for (auto It = Descriptions.begin(); It != Descriptions.end();)
    {
        ObjectDescr* Descr = *It;

        if (!Descr->changed)
        {
            ++It;
            continue;
        }

        Descr->changed = false;

        if (!Descr->marked)
            if (std::find(StackObjs.begin(), StackObjs.end(), Descr) == StackObjs.end())
            {
                It = Descriptions.erase(It);
                Descr->Destroy(Descr->Owner, Memory);
                continue;
            }

        ++It;
    }
}

// Register A New Object | Regista Um Novo Objeto.
ObjectDescr* GarbageCollector::Register(void* Object, void (*Destroy)(void*, Arena&), Arena& Memory)
{
    ObjectDescr* Descr = Memory.New<ObjectDescr>();

    Descr->Owner = Object;
    Descr->Destroy = Destroy;

    Descriptions.push_back(Descr);

    return Descr;
}

// ========== CORE =========== //

// Run Binary Arithmetic Operations | Roda Operações Binarias Aritmeticas.
int VirtualMachine::RunBinary(ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    auto Start = std::chrono::high_resolution_clock::now();

    auto& CurrInst = BC.Chunks[BC.currChunk]->Instructions[IP.Index];

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

        case OpCode::MOD:
        {
            if (holds_alt<float>(L) || holds_alt<float>(R))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "RunTime", 
                    "Cannot Make <MOD> Whit Float Numbers", 
                    "<FLOAT> And <FLOAT> DONT MAKE SENSE", 
                    "Convert to <INT>",
                    BC.Chunks[BC.currChunk]->Instructions[IP.Index]->Pos.line,
                    BC.Chunks[BC.currChunk]->Instructions[IP.Index]->Pos.collumn
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
            }
            else
            {
                ByteValue Left = VM_Utils::ConvertValue
                (L, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                ByteValue Right = VM_Utils::ConvertValue
                (R, TypeKind::NUMBER, SubTypeKind::INT, CurrInst->Pos, Data);

                CallStack->GetTop()->PushBack(
                    static_cast<i64>(
                        std::get<i64>(Left) %
                        std::get<i64>(Right)
                    )
                );
            }
            break;
        }

        default:
            OrbitLog::Error("virtual_machine.cpp", "Trying to Make a Aritm Operation Whit Unknow OpCode: "+std::to_string(static_cast<int>(CurrInst->C)), true, 1);
    }

    if (calcExecTime)
    {
        auto End = std::chrono::high_resolution_clock::now();
        auto Duration = End - Start;
        TimeData["RunFunctions"] += std::chrono::duration<double, std::milli>(Duration).count();
    }

    return 0;
}

// Compare 2 Values | Compara 2 Valores
int VirtualMachine::RunComp(const ByteValue& L, const ByteValue& R, OpCode Op)
{
    auto Start = std::chrono::high_resolution_clock::now();

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

        case OpCode::CMP_LT:
        {
            std::visit([&](const auto& A, const auto& B) {
                using T1 = std::decay_t<decltype(A)>;
                using T2 = std::decay_t<decltype(B)>;

                if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A < B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, float>
                )
                    Result = A < B;

                else if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, float>
                )
                    Result = static_cast<float>(A) < B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A < static_cast<float>(B);

            }, L, R);

            break;
        }

        case OpCode::CMP_LE:
        {
            std::visit([&](const auto& A, const auto& B) {
                using T1 = std::decay_t<decltype(A)>;
                using T2 = std::decay_t<decltype(B)>;

                if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A <= B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, float>
                )
                    Result = A <= B;

                else if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, float>
                )
                    Result = static_cast<float>(A) <= B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A <= static_cast<float>(B);

            }, L, R);

            break;
        }

        case OpCode::CMP_GT:
        {
            std::visit([&](const auto& A, const auto& B) {
                using T1 = std::decay_t<decltype(A)>;
                using T2 = std::decay_t<decltype(B)>;

                if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A > B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, float>
                )
                    Result = A > B;

                else if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, float>
                )
                    Result = static_cast<float>(A) > B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A > static_cast<float>(B);

            }, L, R);

            break;
        }

        case OpCode::CMP_GE:
        {
            std::visit([&](const auto& A, const auto& B) {
                using T1 = std::decay_t<decltype(A)>;
                using T2 = std::decay_t<decltype(B)>;

                if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A >= B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, float>
                )
                    Result = A >= B;

                else if constexpr (
                    std::is_same_v<T1, i64> &&
                    std::is_same_v<T2, float>
                )
                    Result = static_cast<float>(A) >= B;

                else if constexpr (
                    std::is_same_v<T1, float> &&
                    std::is_same_v<T2, i64>
                )
                    Result = A >= static_cast<float>(B);

            }, L, R);

            break;
        }

        default:
            break;
    }

    Frame->PushBack(Result);
    
    if (calcExecTime)
    {
        auto End = std::chrono::high_resolution_clock::now();
        auto Duration = End - Start;
        TimeData["RunFunctions"] += std::chrono::duration<double, std::milli>(Duration).count();
    }
    return 0;
}

// Set Unary Values | Define Valores Unarios.
int VirtualMachine::RunUnary(ByteValue& O, OpCode Op, ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    auto Start = std::chrono::high_resolution_clock::now();

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

    if (calcExecTime)
    {
        auto End = std::chrono::high_resolution_clock::now();
        auto Duration = End - Start;
        TimeData["RunFunctions"] += std::chrono::duration<double, std::milli>(Duration).count();
    }
    return 0;
}

// Main Function, Run ORBIT | Função Principal, Roda a ORBIT.
int VirtualMachine::Run(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    fstream file;
    if (Data.flags.generateLog)
    {
        file = fstream(Data.LogDir, std::ios::out | std::ios::app);
        file << "\n//=========== VIRTUAL-MACHINE ========== //\n\n";
    }
    BC.currChunk=0;
    InstructionPointer IP = {0};
    size_t codeSize = BC.Chunks[BC.currChunk]->Instructions.size();
    while (IP.Index < codeSize) // Main Loop | Loop principal:
    {
        // Take OpCode | Pega o Codigo de Operação:
        OpCode OP = BC.Chunks[BC.currChunk]->Instructions[IP.Index]->C;
        Chunk* CurrChunk = BC.Chunks[BC.currChunk];
        auto& CurrInst = BC.Chunks[BC.currChunk]->Instructions[IP.Index];
        auto& Insts = CurrChunk->Instructions;
        if (Data.flags.generateLog)
        {
            file <<
                "RUNNING: "+
                std::to_string(static_cast<int>(CurrInst->C))+ 
                " | REG1: "+
                VM_Utils::ConvertByteToString(CurrInst->R1)+
                ", REG2: "+
                VM_Utils::ConvertByteToString(CurrInst->R2)+
                "\nIP: "+std::to_string(IP.Index)
                +"\nSTACK: ";
            
            int i=0;
            for (ByteValue& Val : CallStack->GetTop()->Stack)
            {
                if (i != CallStack->GetTop()->Stack.size())
                    file << "  ["+std::to_string(i)+"]: " + VM_Utils::ConvertByteToString(Val) + ",\n";
                else file << "  ["+std::to_string(i)+"]: " + VM_Utils::ConvertByteToString(Val) + "\n";
                i++;
            }
            file << "\n";
        }
        if (Data.flags.vmConsoleDebug)
        {
            PrintLn(
                "RUNNING: "+
                std::to_string(static_cast<int>(CurrInst->C)), 
                " | REG1: "+
                VM_Utils::ConvertByteToString(CurrInst->R1)+
                ", REG2: "+
                VM_Utils::ConvertByteToString(CurrInst->R2)+
                "\nIP: "+std::to_string(IP.Index)
                +"\nSTACK: "
            );
            int i=0;
            for (ByteValue& Val : CallStack->GetTop()->Stack)
            {
                if (i != CallStack->GetTop()->Stack.size())
                    PrintLn("  ["+std::to_string(i)+"]: ", VM_Utils::ConvertByteToString(Val), ",\n");
                else PrintLn("  ["+std::to_string(i)+"]: ", VM_Utils::ConvertByteToString(Val), "\n");
            }
        }

        // Main Switch | Switch Principal:
        switch (OP) 
        {

            // STACK-CONTROL:
            case OpCode::PUSH: // Push A New Value to Sack | Coloca um Novo Valor na Pilha:
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
            case OpCode::MOD:
                { RunBinary(BC, IP, Res, Data, Memory); break; }

            
            // Comp | Comparaçoes:
            case OpCode::CMP_EQ:
            case OpCode::CMP_NE:
            case OpCode::CMP_LT:
            case OpCode::CMP_LE:
            case OpCode::CMP_GT:
            case OpCode::CMP_GE:
            {
                ByteValue R = CallStack->GetTop()->Pop();
                ByteValue L = CallStack->GetTop()->Pop();

                RunComp(L, R, CurrInst->C);
                break;
            }
                
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
            case OpCode::STORE_CONST: // Store 'Pop' In A Constant | Guarda o Valor de 'Pop' Em Uma Constante.
            {
                
            }
            // BUILDS:
            // Build a Range Iterator | Constroi um Iterador de Intervalo.
            case OpCode::BUILD_RANGE: 
            {
                i64 end   = std::get<i64>(CallStack->GetTop()->Pop());
                i64 start = std::get<i64>(CallStack->GetTop()->Pop());

                ByteIterator* It = Memory.New<ByteIterator>(
                    static_cast<ui32>(start),
                    static_cast<size_t>(end),
                    1
                );
                It->Descr = GC.Register(It, ByteIterator::Destroy, Memory);

                CallStack->GetTop()->PushBack(It);
                break;
            }

            // ITERS:
            // Add '.InEnd()' Result in Stack | Adiciona o Resultado a Função: '.InEnd()' Na Stack.
            case OpCode::ITER_HAS_NEXT:
            {
                ByteValue& Val = CallStack->GetTop()->Stack.back();

                if (!holds_alt<ByteIterator*>(Val))
                    OrbitLog::Error(
                        "virtual_machine", 
                        "Trying to Get a Non-Iterador Object",
                        true,
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );
                ByteIterator* It = std::get<ByteIterator*>(Val);
                CallStack->GetTop()->PushBack(!It->InEnd());
                break;
            }
            case OpCode::ITER_NEXT: // Advance Iterator | Avança o Iterador.
            {
                ByteValue& Val = CallStack->GetTop()->Stack.back();

                if (!holds_alt<ByteIterator*>(Val))
                    OrbitLog::Error(
                        "virtual_machine", 
                        "Trying to Get a Non-Iterador Object",
                        true,
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );
                ByteIterator* It = std::get<ByteIterator*>(Val);

                i64 current = It->Curr;
                It->Advance();

                CallStack->GetTop()->PushBack(current);
                break;    
            }

            // CONTROL-FLOW
            case OpCode::JUMP: // Jump to Another Point in Code | Pula pra Outro Ponto no Codigo
            {
                if (std::get<i64>(CurrInst->R1) > BC.Chunks[BC.currChunk]->Instructions.size())
                {
                    OrbitLog::Error(
                        "virtual_machine.cpp", 
                        "Trying to Acess a Instruction OUT_OF_RANGE: "+std::to_string(std::get<i64>(CurrInst->R1)), 
                        true, 
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );
                }
                IP.Index = std::get<i64>(CurrInst->R1);
                continue;
            }
            case OpCode::JUMP_IF_FALSE: // Jump to Another Point in Code If Cond is 'False' | Pula para Outro Ponto no Codigo se a Condição for FALSA.
            {
                ByteValue Cond = CallStack->GetTop()->Pop();

                if (!holds_alt<bool>(Cond))
                    OrbitLog::Error(
                        "virtual_machine.cpp",
                        "Expected Boolean Condition In Stack. But Got: " +
                        std::get<string>(
                            VM_Utils::ConvertValue(
                                Cond,
                                TypeKind::STRING,
                                SubTypeKind::NONE,
                                CurrInst->Pos,
                                Data
                            )
                        ),
                        true,
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );

                if (std::get<i64>(CurrInst->R1) > BC.Chunks[BC.currChunk]->Instructions.size())
                {
                    OrbitLog::Error(
                        "virtual_machine.cpp",
                        "Trying to Acess a Instruction OUT_OF_RANGE: " +
                        std::to_string(std::get<i64>(CurrInst->R1)),
                        true,
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );
                }
                if (std::get<bool>(Cond) == false)
                {
                    IP.Index = std::get<i64>(CurrInst->R1);
                    continue;
                }

                break;
            }
            case OpCode::JUMP_IF_TRUE: // Jump to Another Point in Code If Cond is 'True' | Pula para Outro Ponto no Codigo se a Condição for VERDADEIRA.
            {
                ByteValue Cond = CallStack->GetTop()->Pop();

                if (!holds_alt<bool>(Cond))
                    OrbitLog::Error(
                        "virtual_machine.cpp",
                        "Expected Boolean Condition In Stack. But Got: " +
                        std::get<string>(
                            VM_Utils::ConvertValue(
                                Cond,
                                TypeKind::STRING,
                                SubTypeKind::NONE,
                                CurrInst->Pos,
                                Data
                            )
                        ),
                        true,
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );

                if (std::get<i64>(CurrInst->R1) > BC.Chunks[BC.currChunk]->Instructions.size())
                {
                    OrbitLog::Error(
                        "virtual_machine.cpp",
                        "Trying to Acess a Instruction OUT_OF_RANGE: " +
                        std::to_string(std::get<i64>(CurrInst->R1)),
                        true,
                        ORBIT_ERRORS_CODE::RUNTIME_ERROR
                    );
                }
                if (std::get<bool>(Cond) == true)
                {
                    IP.Index = std::get<i64>(CurrInst->R1);
                    continue;
                }

                break;                
            }

            // OTHERS:
            case OpCode::LOAD_FN: // Load a Function | Carrega Uma Função.
            {
                ByteFn* Fn = Memory.New<ByteFn>();

                Fn->ID =std::get<i64>(CurrInst->R1);
                Fn->ParamCount = std::get<i64>(CurrInst->R2);

                Fn->Descr = GC.Register(Fn, ByteFn::Destroy, Memory);
                CallStack->GetTop()->PushBack(Fn);
                break;
            }
            case OpCode::CALL: // Call a Function | Chama uma Função.
            {
                // Take Data | Pega a Data.
                i64 argCount = std::get<i64>(CurrInst->R1);
                i64 fnPos = static_cast<i64>(CallStack->GetTop()->Stack.size()) - argCount - 1;
                
                ByteFn* B_Fn = std::get<ByteFn*>(CallStack->GetTop()->Stack[fnPos]);
                
                // Create A New Frame | Cria um Novo Quadro.
                VM_Frame* New = Memory.New<VM_Frame>();
                New->Back = CallStack->GetTop();
                New->retChunk = BC.currChunk;

                // Define Args | Define os Argumentos.
                for (int i = 0; i < B_Fn->ParamCount; i++)
                {
                    if (i < argCount)
                        New->Locals[i] = CallStack->GetTop()->Stack[fnPos + i + 1];
                    else
                        New->Locals[i] = NullLitVal{};
                }

                // Remove Args | Remove os Argumentos:
                for (int i = 0; i < argCount; i++)
                    CallStack->GetTop()->Pop();

                // Remove a função da stack também
                CallStack->GetTop()->Pop();

                // Finalize | Finalize:
                BC.currChunk = B_Fn->ID;
                CallStack->Push(New, &IP);

                IP.Index = 0; // começa do início da função
                codeSize = BC.Chunks[BC.currChunk]->Instructions.size();

                continue;
            }
            case OpCode::RETURN: // Exit a Function And Return A Value | Sai da Func e Retorna um Valor.
            {
                // Take Data | Pega a Data.
                VM_Frame* Curr = CallStack->GetTop();
                VM_Frame* Caller = Curr->Back;

                // Set Return Value | Define o Valor de Retorno.
                if (!Curr->Stack.empty())
                    Caller->PushBack(Curr->Pop());

                // Come Back to Original Chunk | Volta Para o Chunk Original.
                BC.currChunk = Curr->retChunk;

                // Restore Caller IP | Restaura o IP do Caller.
                IP = CallStack->Pop();
                codeSize = BC.Chunks[BC.currChunk]->Instructions.size();
                break;
            }
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
        if (GC.curr_ipdt == GC.updt_rate)
        {
            auto Start = std::chrono::high_resolution_clock::now();

            GC.curr_ipdt = 0;
            GC.Update(BC, IP, Res, Data, Memory);

            if (calcExecTime)
            {
                auto End = std::chrono::high_resolution_clock::now();
                auto Duration = End - Start;
                TimeData["GC_Update"] += std::chrono::duration<double, std::milli>(Duration).count();
            }
        } else GC.curr_ipdt++;
        IP.Index++;
    }
    if (Data.flags.generateLog)
    {
        file << "\n//=========== ENDOF: 'VIRTUAL-MACHINE'. .. ... ========== //\n";
        file.close();
    }
    return 0;
}

// =========== ENTRY-POINT ============ //
// Entry-Point Of Virtual-Machine | Ponto-de-Entrada da Maquina-Virtual.
void VirtualMachine::InitVM(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    for (RunTimeArg Arg : Data.Args)
    {
        if (Arg.name == "VM_ConsoleLog" && holds_alt_value<bool>(Arg.value, true))
            Data.flags.vmConsoleDebug=true;
        if (Arg.name == "VM_ExecLog" && holds_alt_value<bool>(Arg.value, true))
            calcExecTime=true;
    }     
    
    // Create Call Stack | Cria a Call-Stack
    this->CallStack.emplace(Memory);

    // Create Global Frame | Cria um Quadro-Global.
    VM_Frame* MainF = Memory.New<VM_Frame>();
    MainF->Back = nullptr;

    // Set Entry | Define a Entrada.
    InstructionPointer EntryIP = {0}; 
    this->CallStack->Push(MainF, &EntryIP);
    GC.VM = this;
    GC.updt_rate = 10000;

    // Time Calc | Calculo de Tempo de Execução.
    if (calcExecTime)
    {
        TimeData["RunFunctions"] = 0.0;
        TimeData["GC_Update"] = 0.0;

        auto Start = std::chrono::high_resolution_clock::now();

        int Return = Run(BC, Res, Data, Memory);

        auto End = std::chrono::high_resolution_clock::now();

        auto Duration = End - Start;
        double Seconds = std::chrono::duration<double>(Duration).count();
        double Milliseconds = std::chrono::duration<double, std::milli>(Duration).count();

        PrintLn(
            "\n\n// ----- TIME-LOG ----- //\n\n",
            "\n// --- RUNTIME --- //\n",
            "\nTotal VM Execution Time: ",
            std::fixed, std::setprecision(6),
            Seconds, " s | ",
            Milliseconds, " ms\n",
            "\nRuntimeFuncs: " + std::to_string(TimeData["RunFunctions"]),
            "\n\n// ----- GARBAGE-COLLECTOR --- //\n",
            "\nGC_Update: " + std::to_string(TimeData["GC_Update"]),
            "\n\n// ----- ENDOF: 'TIME-LOG'. .. ... ----- //\n\n"
        );
    }
    else
    {
        auto Return = Run(BC, Res, Data, Memory);
    }

};

// EOF