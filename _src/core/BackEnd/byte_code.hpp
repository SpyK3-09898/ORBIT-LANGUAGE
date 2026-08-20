
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

// Iterator | Iterador.
struct ByteIterator
{
    // DATA
    ui32 Start; size_t End; i32 Step; int Curr;

    // CONSTRUCTOR
    ByteIterator(ui32 St, size_t E, i32 S)
        : Start(St), Step(S), End(E), Curr(St) {};

    // UTILS
    bool InEnd() // Return if In End of It | Retorna se Esta no Fim do it:
    {
        if (Curr+Step > End)
            return true;
        else return false;
    }
    void Advance() // Advance it | Avança o Iterador:
    {
        if (!InEnd()) Curr++;
    }
};

struct ByteArray;
struct ByteFn;
using ByteValue = variant<

    bool,
    float,
    i64,
    string,
    NoneLitVal,
    NullLitVal,
    shared_ptr<ByteArray>,
    ByteFn*,
    ByteIterator
>;
struct ByteArray : vec<ByteValue>
{
    using vec<ByteValue>::vector;
};

// ENUMS | ENUMERAÇÕES

/* OPCODE-TABLE
    NOP            / -> '0'

    POP            / -> '1'
    PUSH           / -> '2'

    ADD            / -> '3'
    SUB            / -> '4'
    MUL            / -> '5'
    DIV            / -> '6'
    MOD            / -> '7'
    POWER          / -> '8'

    NEG            / -> '9'
    NOT            / -> '10'

    CMP_EQ         / -> '11'
    CMP_NE         / -> '12'
    CMP_LT         / -> '13'
    CMP_LE         / -> '14'
    CMP_GT         / -> '15'
    CMP_GE         / -> '16'

    LOAD_LOCAL     / -> '17'
    STORE_LOCAL    / -> '18'
    LOAD_CONST     / -> '19'
    STORE_CONST    / -> '20'

    GET_MEMBER     / -> '21'
    LOAD_MEMBER    / -> '22'
    STORE_MEMBER   / -> '23'

    GET_INDEX      / -> '24'
    LOAD_INDEX     / -> '25'
    STORE_INDEX    / -> '26'

    LOAD_FN        / -> '27'

    BUILD_ARRAY    / -> '28'
    BUILD_TABLE    / -> '29'
    BUILD_RANGE    / -> '30'

    SET_TKEY       / -> '31'

    ITER_NEXT      / -> '32'
    ITER_HAS_NEXT  / -> '33'

    JUMP           / -> '34'
    JUMP_IF_FALSE  / -> '35'
    JUMP_FOR       / -> '36'

    ECHO           / -> '37'
    CALL           / -> '38'
    RETURN         / -> '39'
*/

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
    STORE_MEMBER,

    GET_INDEX,
    LOAD_INDEX,
    STORE_INDEX,

    LOAD_FN,

    // BUILDS | CONSTRUÇÕES.
    BUILD_ARRAY,
    BUILD_TABLE,
    BUILD_RANGE,

    SET_TKEY,

    // ITERS
    ITER_NEXT,
    ITER_HAS_NEXT,

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

// Function Repr | Representação de Função.
struct ByteFn
{
    ui16 ID;
    ui8 ParamCount=0;
};

// ByteCode Chunks | Chunks de ByteCode
struct Chunk
{
    vec<ByteInstruction*> Instructions;
    int ParamCount;
};

// ByteCode | ByteCode.
struct ByteCode
{
    int currChunk=0;
    vec<Chunk*> Chunks;
    unord_map<string, ui32> Functions;
};
