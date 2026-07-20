
// ============= ORBIT ARENA ALLOCATOR =========== //
// Fast Linear Memory Allocator | Alocador Linear de Memoria.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "Arena.hpp" // HEADER FILE
#include "tools/console.hpp"

#include <cstdlib> // LIBRARIES | BIBLIOTECAS
#include <algorithm>
#include <new>
#include <string>


// =========== CREATE BLOCK =========== //
// Create a New Memory Block | Cria Um Novo Bloco De Memoria
ArenaBlock* Arena::NewBlock(size_t Size)
{
    ArenaBlock* Block = 
        static_cast<ArenaBlock*>(std::malloc(sizeof(ArenaBlock)));

    Block->Begin =
        static_cast<byte*>(std::malloc(Size));


    Block->Current = Block->Begin;

    Block->End =
        Block->Begin + Size;


    Block->Size = Size;

    Block->Next = nullptr;


    return Block;
}


// =========== ALIGN MEMORY =========== //
// Align Memory Address | Alinha Endereco De Memoria
byte* Arena::AlignForward(byte* Address, size_t Alignment)
{
    uintptr_t Ptr =
        reinterpret_cast<uintptr_t>(Address);


    uintptr_t Aligned =
        (Ptr + Alignment - 1)
        &
        ~(Alignment - 1);


    return reinterpret_cast<byte*>(Aligned);
}


// =========== CONSTRUCTOR =========== //
// Initialize Arena | Inicializa Arena
Arena::Arena(size_t Size)
{
    MaxSize = Size;


    // Each Block Uses 1/16 Of Total Memory
    // Cada Bloco Usa 1/16 Da Memoria Total
    DefBlockSize = MaxSize / 16;


    if (DefBlockSize == 0)
        DefBlockSize = MaxSize;


    Reserved = 0;
    Blocks = 0;


    FirstBlock = NewBlock(
        DefBlockSize
    );


    CurrBlock = FirstBlock;


    Reserved += DefBlockSize;
    Blocks++;
}


// =========== DESTRUCTOR =========== //
// Destroy Arena | Destruir Arena
Arena::~Arena()
{
    Finalize();
}


// =========== ALLOC =========== //
// Allocate Raw Memory | Aloca Memoria Bruta
void* Arena::Alloc(size_t Size, size_t Alignment)
{
    byte* Ptr =
        AlignForward(
            CurrBlock->Current,
            Alignment
        );


    // CHECK IF MEMORY FITS
    // VERIFICA SE A MEMORIA CABE

    if (Ptr + Size > CurrBlock->End)
    {
        size_t BlockSize =
            std::max(
                DefBlockSize,
                Size
            );


        if (Reserved + BlockSize > MaxSize)
        {
            size_t Overflow =
                (Reserved + BlockSize) - MaxSize;

            OrbitLog::Error(
                "Arena.cpp", 
                "Arena 'OutOfRange', Memory limit Exceeded(Bad Alloc): Limit: "+
                std::to_string(MaxSize)
                +" | Used: "+std::to_string(Reserved)
                +" | OverFlow: "=std::to_string(Overflow),
                true,
                MEMORY_ERROR
            );
        }


        ArenaBlock* New =
            NewBlock(
                BlockSize
            );


        CurrBlock->Next = New;

        CurrBlock = New;


        Reserved += BlockSize;
        Blocks++;


        Ptr =
            AlignForward(
                CurrBlock->Current,
                Alignment
            );
    }


    CurrBlock->Current =
        Ptr + Size;


    return Ptr;
}


// =========== FINALIZE =========== //
// Free All Memory | Libera Toda Memoria
void Arena::Finalize()
{
    // TAKE FIRST BLOCK | PEGA O 1 BLOCO
    ArenaBlock* Block = FirstBlock;

    // FINALIZE BLOCKS | FINALIZA OS BLOCOS
    while (Block) // WHILE NEXT BLOCK NOT IS NULL | ENQUANTO EXISTIR PROXIMO BLOCO.
    {
        ArenaBlock* Next =
            Block->Next;


        std::free(Block->Begin);

        std::free(Block);


        Block = Next;
    }

    // FINALIZE DATA
    FirstBlock = nullptr;
    CurrBlock = nullptr;
    Reserved = 0;
    Blocks = 0;
}


// EOF