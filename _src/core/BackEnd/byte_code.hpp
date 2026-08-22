
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

// Description of Object.
struct ObjectDescr
{
    // Data
    int references=0;
    bool marked=true;
    bool changed=true;

    // Reference Data | Data de Referencias.
    vec<ObjectDescr*> References;
    vec<ObjectDescr*> ReferencedBy;

    // Object | Objeto.
    void* Owner;
	void (*Destroy)(void*, Arena& Memory);

    // UTILS

    // Connect a New Object | Conecta Um Novo Objeto.
    void Reference(ObjectDescr* Obj)
    {
        References.push_back(Obj);
        Obj->ReferencedBy.push_back(this);

        references = ReferencedBy.size();
        marked=true;
        changed=true;
    }

    // Disconnect A Obj | Desconecta Um Obj.
    void Unreference(ObjectDescr* Obj)
    {
        auto It = std::find(References.begin(), References.end(), Obj);

        if (It == References.end())
            return;

        References.erase(It);

        auto RefIt = std::find(Obj->ReferencedBy.begin(), Obj->ReferencedBy.end(), this);

        if (RefIt != Obj->ReferencedBy.end())
            Obj->ReferencedBy.erase(RefIt);

        Obj->references--;
        if (references == 0)
            marked=false;
        changed = true;
    }
};

// Iterator | Iterador.
struct ByteIterator
{
    // DATA
    ui32 Start; size_t End; i32 Step; int Curr;
    ObjectDescr* Descr;

    // CONSTRUCTOR
    ByteIterator(ui32 St, size_t E, i32 S)
        : Start(St), Step(S), End(E), Curr(St) {}; // GC | CB:
    ~ByteIterator() = default;
    static void Destroy(void* Ptr, Arena& Memory)
    {
        ByteIterator* It = static_cast<ByteIterator*>(Ptr);
        It->~ByteIterator();
    }

    // UTILS
    bool InEnd() // Return if In End of It WHITOUT Step | Retorna se Esta no Fim do It DESCONSIDERANDO O Passo:
    {
        if (Step > 0) return Curr > End;
        else          return Curr < End;
    }
    bool HasNext() // Return if In End of It | Retorna se Esta no Fim do it:
    {
        return !InEnd();   
    }
    void Advance() // Advance it | Avança o Iterador:
    {
        if (!InEnd()) Curr+=Step;
    }
};

// Function Repr | Representação de Função.
struct ByteFn
{
    ui16 ID;
    ui8 ParamCount=0;
    ObjectDescr* Descr;

    static void Destroy(void* Ptr, Arena& Memory)
    {
        ByteFn* Fn = static_cast<ByteFn*>(Ptr);
        Memory.Delete(Fn);
    }
};

struct ByteArray; // Array Repr
struct ByteTable; // Table Repr
struct ByteFn; // Functions Repr
using  ByteValue = variant<

    bool,
    float,
    i64,
    string,
    NoneLitVal,
    NullLitVal,
    shared_ptr<ByteArray>,
    shared_ptr<ByteTable>,
    ByteFn*,
    ByteIterator*
>;
struct ByteArray : vec<ByteValue>
{
    using vec<ByteValue>::vector;
};
struct ByteTable : vec<pair<string, ByteValue>>
{
    using vec<pair<string, ByteValue>>::vector;
};

// ENUMS | ENUMERAÇÕES

/* OPCODE-TABLE:

    NOP            / -> '0' 

    POP            / -> '1' - X
    PUSH           / -> '2' - X

    ADD            / -> '3' - X
    SUB            / -> '4' - X
    MUL            / -> '5' - X
    DIV            / -> '6' - X
    MOD            / -> '7' - X
    POWER          / -> '8' - X

    NEG            / -> '9' - X
    NOT            / -> '10' - X

    CMP_EQ         / -> '11' - X
    CMP_NE         / -> '12' - X
    CMP_LT         / -> '13' - X
    CMP_LE         / -> '14' - X
    CMP_GT         / -> '15' - X
    CMP_GE         / -> '16' - X

    LOAD_LOCAL     / -> '17' - X
    STORE_LOCAL    / -> '18' - X
    LOAD_CONST     / -> '19' - X
    STORE_CONST    / -> '20' - X

    GET_MEMBER     / -> '21'
    LOAD_MEMBER    / -> '22'
    STORE_MEMBER   / -> '23'

    GET_INDEX      / -> '24'
    LOAD_INDEX     / -> '25'
    STORE_INDEX    / -> '26'

    LOAD_FN        / -> '27' - X

    BUILD_ARRAY    / -> '28'
    BUILD_TABLE    / -> '29'
    BUILD_RANGE    / -> '30' - X

    SET_TKEY       / -> '31'

    ITER_NEXT      / -> '32' - X
    ITER_HAS_NEXT  / -> '33' - X

    JUMP           / -> '34' - X
    JUMP_IF_FALSE  / -> '35' - X
    JUMP_IF_TRUE   / -> '36'

    ECHO           / -> '37' - X
    CALL           / -> '38' - X
    RETURN         / -> '39' - X

    OR             / -> '40'
    AND            / -> '41'
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

    // SETS
    SET_TKEY,

    // ITERS
    ITER_NEXT,
    ITER_HAS_NEXT,

    // CONTROL-FLOW
    JUMP,
    JUMP_IF_FALSE,
    JUMP_IF_TRUE,

    // OTHERS
    ECHO,
    CALL,
    RETURN,

    AND,
    OR
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
    int ParamCount;
};

// ByteCode | ByteCode.
struct ByteCode
{
    int currChunk=0;
    vec<Chunk*> Chunks;
    unord_map<string, ui32> Functions;
};
