
// ========= BYTE-CODEs ========== //
// Assembly-Style Code to VM | Codigo Estilo Ass para VM's.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../FrontEnd/parser/AST/AST.hpp"

#include "../FrontEnd/lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../RunTimeData.hpp"

// ENUMS | ENUMERAÇÕES

// OpCodes | OpCodes.
enum class OpCode
{
    NOP,

    LOAD_CONST,
    LOAD_NULL,

    POP,

    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    NEG,
    NOT,

    CMP_EQ,
    CMP_NE,
    CMP_LT,
    CMP_LE,
    CMP_GT,
    CMP_GE,

    LOAD_LOCAL,
    STORE_LOCAL,

    JUMP,
    JUMP_IF_FALSE,

    CALL,
    RETURN
};

// STRUCTS

// Instruction | Instrução
struct ByteInstruction
{
    OpCode C; // Operation Code | Codigo Operacional.

    // REGISTERS | REGISTRADORES:
    ui32 R1; ui32 R2;      // Operation Principal Registeres | Registradores Principais da Operação.
    ui32* L1; ui32 L2;    //  Locals .

    ui8 RX1; ui8 RX2;   // Extras.
    ui8 LX1; ui8 LX2;  //  Extras.
};

// ByteCode Chunks | Chunks de ByteCode
struct Chunk
{
    vec<ByteInstruction*> Instructions;
};

// ByteCode | ByteCode.
struct ByteCode
{
    vec<Chunk*> Chunks;
};
