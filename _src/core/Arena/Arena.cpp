
// ============= ORBIT ARENA ALLOCATOR =========== //
// Fast Linear Memory Allocator | Alocador Linear de Memoria.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "Arena.hpp" // HEADER FILE

#include <cstdlib> // LIBRARIES | BIBLIOTECAS
#include <algorithm>
#include <new>


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
    DefBlockSize = Size;


    FirstBlock = NewBlock(
        DefBlockSize
    );


    CurrBlock = FirstBlock;
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


        ArenaBlock* New =
            NewBlock(
                BlockSize
            );


        CurrBlock->Next = New;

        CurrBlock = New;


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
    ArenaBlock* Block = FirstBlock;


    while (Block)
    {
        ArenaBlock* Next =
            Block->Next;


        std::free(Block->Begin);

        std::free(Block);


        Block = Next;
    }


    FirstBlock = nullptr;

    CurrBlock = nullptr;
}


// EOF