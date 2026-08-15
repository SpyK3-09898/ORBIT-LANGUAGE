
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

struct ByteArray;
using ByteValue = variant<

    bool,
    float,
    i64,
    string,
    NoneLitVal,
    NullLitVal,
    shared_ptr<ByteArray>
>;
struct ByteArray : vec<ByteValue>
{
    using vec<ByteValue>::vector;
};

// ENUMS | ENUMERAÇÕES

// OpCodes | OpCodes.
enum class OpCode
{
    // OT
    NOP,

    // STACK CONTROL | CONTROLE DE PILHA:
    POP,
    PUSH,

    // OPERATIONS | OPERAÇOES:
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    POWER,
    LESS,
    GREATER,
    LESSEQ,
    GREATEREQ,

    // OPS
    NEG,
    NOT,

    CMP_EQ,
    CMP_NE,
    CMP_LT,
    CMP_LE,
    CMP_GT,
    CMP_GE,

    // LOAD & GETS:
    LOAD_LOCAL,
    STORE_LOCAL,
    LOAD_CONST,
    STORE_CONST,

    GET_MEMBER,
    LOAD_MEMBER,

    GET_INDEX,
    LOAD_INDEX,

    LOAD_FN,

    // BUILDS | CONTRUÇÕES.
    BUILD_ARRAY,
    BUILD_TABLE,
    BUILD_RANGE,

    SET_TKEY,

    // CONTROL-FLOW
    JUMP,
    JUMP_IF_FALSE,
    JUMP_FOR,

    // OTHERS
    ECHO,
    CALL,
    RETURN
};

// STRUCTS

// Instruction | Instrução
struct ByteInstruction
{
    OpCode C; // Operation Code | Codigo Operacional.
    NodePos Pos;

    // REGISTERS | REGISTRADORES:
    ByteValue R1; ByteValue R2;      // Operation Principal Registeres | Registradores Principais da Operação.
    ByteValue L1; ByteValue L2;    //  Locals .

    ByteValue RX1; ByteValue RX2;   // Extras.
    ByteValue LX1; ByteValue LX2;  //  Extras.
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
    unord_map<string, ui32> Functions;
};
