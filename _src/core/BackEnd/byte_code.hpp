
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
#include <cstddef>
#include <cstdint>

// FORWARDS
struct Chunk;
struct ByteCode;
struct ByteInstruction;

struct ByteArray;    // Array Repr     | Representação de Matrizes.
struct ByteTable;    // Table Repr     | Representação de Tabelas.
struct ByteFn;       // Functions Repr | Representação de Funções
struct ByteIterator; // Iterador Repr  | Rerpesentação de Iteradores
struct BytePackage;  // Package Repr   | Representação de Pacotes.
using  ByteValue = variant< // RunTime Value | Valor de RunTime.
    bool,
    float,
    i64,
    string,
    NoneLitVal,
    NullLitVal,
    shared_ptr<ByteArray>,
    shared_ptr<ByteTable>,
    ByteFn*,
    ByteIterator*,
    BytePackage*,
    nullptr_t
>;

// Description of Object | Descrição de Um Objeto.
struct ObjectDescr
{
    // Data
    int references = 0;
    bool marked    = true;
    bool changed   = true;

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

// Base Object Repr | Representação Basica De Um Objeto.
struct ByteObject
{

    // Object Description | Descrição do Objeto.
    public: ObjectDescr* Descr;
    bool acessible=false;

    virtual ByteValue Acess
    (
        ByteValue& Val,
        ByteInstruction& CurrInst,
        ByteCode* BC,
        RunTimeData& Data
    );
};

// Iterator | Iterador.
struct ByteIterator : ByteObject
{
    // DATA
    i64 Curr;
    i64 End;
    i32 Step;

    // CONSTRUCTOR
    ByteIterator(i64 start, i64 end, i32 step)
        : Curr(start - step), End(end), Step(step) {}

    ~ByteIterator() = default;
    static void Destroy(void* Ptr, Arena& Memory)
    {
        ByteIterator* It = static_cast<ByteIterator*>(Ptr);
        It->~ByteIterator();
    }

    // UTILS
    bool HasNext() const // Return if In End of It | Retorna se Esta no Fim do it:
    {
        if (Step > 0)
            return Curr + Step <= End;
        else
            return Curr + Step >= End;
    }

    void Advance() // Advance it | Avança o Iterador:
    {
        Curr += Step;
    }
};

// RunTime Orbit Package Repr | Representação de Pacotes Orbti em RunTime.
struct BytePackage : ByteObject
{
    // DATA | DADOS
    Chunk* Chunk;
    ui8 chunkId=0;
    ui32 SymbolCount=0;
    unord_map<ui16, ByteValue> Members;
    
    // FUNCTIONS | FUNÇÕES
    BytePackage() { acessible=true; };

    // Member Acess | Acesso de Membros.
    ByteValue Acess
        (ByteValue& Val, ByteInstruction& CurrInst, ByteCode* BC, RunTimeData& Data) 
        override;

    // GC | CB
    ~BytePackage() = default;
    static void Destroy(void* Ptr, Arena& Memory)
    {
        BytePackage* Pack = static_cast<BytePackage*>(Ptr);
        Pack->~BytePackage();
    }
};

// Function Repr | Representação de Função.
struct ByteFn : ByteObject
{
    ui16 ID;
    ui8 ParamCount=0;

    static void Destroy(void* Ptr, Arena& Memory)
    {
        ByteFn* Fn = static_cast<ByteFn*>(Ptr);
        Memory.Delete(Fn);
    }
};

struct ByteArray : vec<ByteValue>
{
    using vec<ByteValue>::vector;
};
struct ByteTable : vec<pair<string, ByteValue>>
{
    using vec<pair<string, ByteValue>>::vector;
};

// ENUMS | ENUMERAÇÕES

// OpCodes | OpCodes.
enum class OpCode: uint8_t
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
    LOAD_PACK,

    // BUILDS | CONSTRUÇÕES.
    BUILD_ARRAY,
    BUILD_TABLE,
    BUILD_RANGE,
    BUILD_PACKAGE,

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
    unord_map<string, ui8> Contexts;
    unord_map<ui8, unord_map<ui16, ui32>> ExportSlots;
    unord_map<ui8, unord_map<string, i64>> Functions;
};
