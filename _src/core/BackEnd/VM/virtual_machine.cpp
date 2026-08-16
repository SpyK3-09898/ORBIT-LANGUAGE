
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

// ========== CORE =========== //

int VirtualMachine::Run(ByteCode& BC, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    InstructionPointer IP = {0};
    size_t codeSize = BC.Chunks[0]->Instructions.size();
    while (IP.Index < codeSize) // Main Loop | Loop principal:
    {

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
};

// EOF
