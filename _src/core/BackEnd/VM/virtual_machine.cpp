
// =========== VM ========== //
// Virtual Machine ORBIT Runner | CPU-Virtual Para Rodar Codigo ORBIT.
// Developed By SpyK3(2026) | License: GitHub(MIT). 
// INCLUDE HEADERS 'N DEPENDENCES

#include "virtual_machine.hpp" // HEADER FILE | CABEÇALHO
#include "../byte_code.hpp"

#include "../../FrontEnd/SA/semantic_analysis.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <string>
#include <variant>

// ========== ENTRY-POINT | PONTO DE ENTRADA ========= //

// Run A Instruction || Roda Uma Instrução.
void VirtualMachine::Run()
{
    // MAIN LOOP | LOOP PRINCIPAL.
    while (running) {
    
        // Data
        VM_Frame& Frame = Calls.Top();
        ByteInstruction* Inst =
            Frame.C->Instructions[Frame.IP.Index];
            
        // Main Switch | Switch Principal.
        switch(Inst->C)
        {

            default:
                running=false;
                        break;
        }
        Frame.IP.Index++;
    }
}

// Entry-Point of Run Program || Ponto-De-Entrada do Programa de Execução.
void VirtualMachine::InitVM(ByteCode& BC, RunTimeData& Data, Arena& Memory)
{
    // DATA
    VM_Frame       Frame;
    Code           = &BC;
    Frame.C        = BC.Chunks[0];
    Frame.IP.Index = 0;

    // Set | Define.
    Calls.Push(Frame);
    running = true;

    // Run Orbit | Roda A Orbit.
    Run();
}