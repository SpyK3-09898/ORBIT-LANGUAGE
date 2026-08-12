
// =========== VM ========== //
// Virtual Machine ORBIT Runner | CPU-Virtual Para Rodar Codigo ORBIT.
// Developed By SpyK3(2026) | License: GitHub(MIT). 

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

// Bytecode Instruction-Pointer | Ponteiro do Bytecode.
struct InstructionPointer
{
    ui32 Index;
};

// Frame Context of VM | Contexto do Quadro da VM.
struct VM_Frame
{
    Chunk* C;
    InstructionPointer IP;
    vec<ByteValue> Locals;
};

// Context of Calls | Contexto de Chamada.
struct CallStack
{
    // DATA:
    vec<VM_Frame> Frames;

    // UTILS | UTILIDADES:

    // Return Top of Frames | Retorna o Topo dos Frames.
    VM_Frame& Top()
    {
        return Frames.back();
    }

    // Add a New Frame | Adiciona Um Novo Frame.
    void Push(VM_Frame& Frame)
    {
        Frames.push_back(Frame);
    }

    // Remove A Frame | Remove um Frame.
    void Pop()
    {
        Frames.pop_back();
    }
};

// Stack of VM | Pilha da VM.
struct Stack
{
    // DATA
    vec<ByteValue> Data;
    int Top;

    // UTILS | UTILIDADES
    // Remove 1 Value of Stack | Remove Um Valor na Stack.
    ByteValue Pop()
    { 
        ByteValue V = Data.back();
        Data.pop_back(); 
        Top = Data.empty() ? -1 : Data.size() - 1;
        return V;
    }
    void Push(ByteValue Val)
    {
        Data.push_back(std::move(Val));
        Top = Data.size() - 1;
    }
};

// MAIN CLASS | CLASSE PRINCIPAL.
class VirtualMachine
{
    private:
    
        ByteCode* Code;
        Stack St;
        CallStack Calls;
        fstream logFile;
        bool generate_log=false;
        bool running=false;
    public:

        void Run(RunTimeData& Data, Arena& Memory);
        void InitVM(ByteCode& BC, RunTimeData& Data, Arena& Memory);
};

// EOF.