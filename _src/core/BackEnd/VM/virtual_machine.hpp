
// ========== VIRTUAL-MACHINE =========== //
// OVM - Orbit Virtual Machine | Maquina Virtual ORBIT.
// Developed By: SpyK3(2026) | License: MIT(GitHub).

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
#include <cstddef>
class VirtualMachine;

// STRUCTS

// Pointer of Instruction | Ponteiro de Instruções.
struct InstructionPointer
{
    size_t Index;
};

// Execution Frame | Quadro de Execução.
struct VM_Frame
{
    VM_Frame* Back;
    InstructionPointer ReturnIP;

    unord_map<ui32, ByteValue> Locals;
    vec<ByteValue> Stack;
    int retChunk;

    void PushBack(ByteValue Val)
    {
        Stack.push_back(Val);
    }
    ByteValue Pop()
    {
        if (Stack.size() == 0)
            OrbitLog::Error("virtual_machine.hpp", "Trying to Make a 'Pop' In Empty Stack", true, ORBIT_ERRORS_CODE::MEMORY_ERROR);
        ByteValue Val = Stack.back();
        Stack.pop_back();
        return Val;
    }
    ByteValue Top()
    {
        if (Stack.size() == 0)
            OrbitLog::Error("virtual_machine.hpp", "Trying to Get a 'Top' In Empty Stack", true, ORBIT_ERRORS_CODE::MEMORY_ERROR);
        return Stack.back();
    }
};

// Stack of Calls | Pilha de Chamadas.
class StackCall
{
    private:

        VM_Frame* Top = nullptr;
        Arena& Memory;
        size_t Depth=0;
        const size_t MAX_DEPTH = 2048;
    public:

        StackCall(Arena& M) : Top(nullptr), Depth(0), Memory(M) {};

        // Return Top of Stack | Retorna o Topo da Pilha.
        VM_Frame* GetTop()
        { return Top; }

        // Add a New Frame To O-VM | Adiciona um Novo Quadro pra O-VM.
        void Push(VM_Frame* New, InstructionPointer* Curr)
        {
            if (Depth >= MAX_DEPTH)
                OrbitLog::Error("virtual_machine.hpp", "Stack OverFlow!!! Call Limit Exceded", true, 1);
            New->ReturnIP = *Curr;
            New->Back = Top;

            Top = New;
            Depth++;
        }

        // Remove the Last Frame of O-VM | Remove o Ultimo Quadro da O-VM.
        InstructionPointer Pop()
        {
            if (Top == nullptr)
                OrbitLog::Error("virtual_machine.hpp", "Trying to Pop Empty Call-Stack");
            
            InstructionPointer JumpBackTo = Top->ReturnIP;

            VM_Frame* D = Top;
            Top = D->Back;
            Depth--;

            Memory.Delete(D);
            return JumpBackTo;
        }

        // Return if is Empty | Retorna se Esta Vazio.
        bool IsEmpty() const 
        {
            return Top == nullptr;
        }
};

// Garbage Collector | Coletor de Bagagem.
class GarbageCollector
{
    private:

        vec<ObjectDescr*> Descriptions;
    public:

        size_t updt_rate=0;
        size_t curr_ipdt=0;

        VirtualMachine* VM;
        void Update(ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory);
        ObjectDescr* Register(void* Object, void (*Destroy)(void*, Arena&), Arena& Memory);
}; 

// MAIN CLASS | CLASSE PRINCIPAL
class VirtualMachine
{
    friend class GarbageCollector;
    private:

        opt<StackCall> CallStack{};
        unord_map<string, double> TimeData{};
        GarbageCollector GC;
        bool calcExecTime=false;
        
        int Run(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory);

        int RunBinary(ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory);
        int RunComp(const ByteValue& L, const ByteValue& R, OpCode Op);
        int RunUnary(ByteValue& O, OpCode Op, ByteCode& BC, InstructionPointer& IP, SAResult& Res, RunTimeData& Data, Arena& Memory);
    public:

        void InitVM(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory);        
};

// EOF
