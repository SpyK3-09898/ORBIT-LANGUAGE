
// ============= ORBIT ARENA ALLOCATOR ============ //
// Fast Linear Memory Allocator | Alocador Linear de Memória.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

#pragma once

// INCLUDE HEADERS 'N DEPENDENCES

#include "utils/aliases.hpp" // HEADERS

#include <complex>
#include <cstddef> // LIBRARIES | BIBLIOTECAS
#include <cstdint>
#include <utility>
#include <algorithm>
#include <new>

size_t constexpr _1KB = 1024;
size_t constexpr _1MB = _1KB * _1KB;
size_t constexpr _2MB = _1MB*2;
size_t constexpr _4MB = _2MB*2;
size_t constexpr _8MB = _4MB*2;
size_t constexpr _16MB = _8MB*2;

// =========== CORE ========== //

// Base-Block for Memory | Bloco Base Para Memoria
struct ArenaBlock
{
    ArenaBlock* Next;

    byte* Begin;
    byte* Current;
    byte* End;

    size_t Size;
};


// =========== ARENA ========= //
// Linear Memory Allocator | Alocador Linear de Memoria
class Arena
{

    private:

    size_t DefBlockSize; // Default Block Size | Tamanho Padrao Do Bloco
    size_t MaxSize; // Maximum Arena Size | Tamanho Maximo Da Arena
    size_t Reserved; // Current Reserved Memory | Memoria Reservada Atual
    size_t Blocks; // Block Count | Quantidade De Blocos

    // First Allocated Block | Primeiro Bloco Alocado
    ArenaBlock* FirstBlock;


    // Current Active Block | Bloco Atualmente Usado
    ArenaBlock* CurrBlock;


    // Freed Memory Block | Bloco de Memoria Liberado
    struct FreeBlock
    {
        byte* Begin;
        size_t Size;
    };


    vec<FreeBlock> FreeBlocks;


    // CheckPoint For Temporary Allocations
    // Ponto De Controle Para Alocacoes Temporarias
    struct CheckPointMarker
    {
        ArenaBlock* Block;
        byte* Current;
    };


    // CREATE NEW MEMORY BLOCK | CRIA NOVO BLOCO DE MEMORIA
    ArenaBlock* NewBlock(size_t Size);


    // ALIGN MEMORY ADDRESS | ALINHA ENDERECO DE MEMORIA
    byte* AlignForward(byte* Address, size_t Alignment);


    public:


    // CONSTRUCTOR | CONSTRUTOR
    Arena(size_t size = _1MB);


    // DESTRUCTOR | DESTRUTOR
    ~Arena();


    // ALLOC RAW MEMORY | ALOCA MEMORIA BRUTA
    void* Alloc(size_t Size, size_t Alignment = alignof(std::max_align_t));


    // ALLOC AND CONSTRUCT OBJECT | ALOCA E CONSTROI OBJETO
    template<typename T, typename... Args>
    T* New(Args&&... args)
    {
        for (auto It = FreeBlocks.begin(); It != FreeBlocks.end(); ++It)
        {
            byte* Address = AlignForward(
                It->Begin,
                alignof(T)
            );

            size_t Offset =
                static_cast<size_t>(Address - It->Begin);

            if (Offset + sizeof(T) <= It->Size)
            {
                void* Memory = Address;

                if (Offset == 0 && sizeof(T) == It->Size)
                {
                    FreeBlocks.erase(It);
                }
                else if (Offset == 0)
                {
                    It->Begin += sizeof(T);
                    It->Size -= sizeof(T);
                }
                else
                {
                    size_t Remaining =
                        It->Size - Offset - sizeof(T);

                    byte* OldBegin = It->Begin;

                    It->Begin = Address + sizeof(T);
                    It->Size = Remaining;

                    if (Offset > 0)
                    {
                        FreeBlocks.push_back({
                            OldBegin,
                            Offset
                        });
                    }
                }

                return new(Memory) T(
                    std::forward<Args>(args)...
                );
            }
        }

        void* Memory = Alloc(
            sizeof(T),
            alignof(T)
        );

        return new(Memory) T(
            std::forward<Args>(args)...
        );
    }

    // STORE A OBJECT ALREADY EXISTS | GUARDA UM OBJETO QUE JA EXISTE.
    template<typename T>
    T* Store(T&& Obj)
        { return New<std::remove_reference_t<T>>(std::forward<T>(Obj)); }

    // DELETE A OBJECT | DELETA UM OBJETO.
    template<typename T>
    void Delete(T* Obj)
    {
        if (!Obj)
            return;

        Obj->~T();

        FreeBlocks.push_back({
            reinterpret_cast<byte*>(Obj),
            sizeof(T)
        });
    };


    // ALLOC ARRAY | ALOCA ARRAY
    template<typename T>
    T* NewArray(size_t Count)
    {
        void* Memory = Alloc(
            sizeof(T) * Count,
            alignof(T)
        );

        T* Array = static_cast<T*>(Memory);

        for (size_t i = 0; i < Count; i++)
            new(&Array[i]) T();

        return Array;
    }


    // SAVE CURRENT STATE | SALVA ESTADO ATUAL
    CheckPointMarker SaveCheckPoint()
    {
        CheckPointMarker Marker;

        Marker.Block = CurrBlock;
        Marker.Current = CurrBlock->Current;

        return Marker;
    }


    // RESTORE PREVIOUS STATE | RESTAURA ESTADO ANTERIOR
    void RestoreCheckPoint(CheckPointMarker Marker)
    {
        CurrBlock = Marker.Block;
        CurrBlock->Current = Marker.Current;

        ArenaBlock* Block = CurrBlock->Next;

        while (Block)
        {
            Block->Current = Block->Begin;
            Block = Block->Next;
        }
    }


    // RESET ALL MEMORY | RESETAR TODA MEMORIA
    void Reset()
    {
        ArenaBlock* Block = FirstBlock;

        while (Block)
        {
            Block->Current = Block->Begin;
            Block = Block->Next;
        }

        CurrBlock = FirstBlock;
        FreeBlocks.clear();
    }


    // FREE ALL MEMORY | LIBERAR TODA MEMORIA
    void Finalize();


    // GET USED MEMORY | PEGAR MEMORIA USADA
    size_t UsedMemory() const
    {
        size_t Used = 0;

        ArenaBlock* Block = FirstBlock;

        while (Block)
        {
            Used += static_cast<size_t>(
                Block->Current - Block->Begin
            );

            Block = Block->Next;
        }

        return Used;
    }


    // GET RESERVED MEMORY | PEGAR MEMORIA RESERVADA
    size_t ReservedMemory() const
    {
        size_t Total = 0;

        ArenaBlock* Block = FirstBlock;

        while (Block)
        {
            Total += Block->Size;

            Block = Block->Next;
        }

        return Total;
    }


    // GET BLOCK COUNT | PEGAR QUANTIDADE DE BLOCOS
    size_t BlockCount() const
    {
        size_t Count = 0;

        ArenaBlock* Block = FirstBlock;

        while (Block)
        {
            Count++;

            Block = Block->Next;
        }

        return Count;
    }


    // GET CURRENT BLOCK | PEGAR BLOCO ATUAL
    ArenaBlock* CurrentBlock() const
    {
        return CurrBlock;
    }


    // GET FIRST BLOCK | PEGAR PRIMEIRO BLOCO
    ArenaBlock* First() const
    {
        return FirstBlock;
    }


    // GET DEFAULT BLOCK SIZE | PEGAR TAMANHO PADRAO DO BLOCO
    size_t DefaultSize() const
    {
        return DefBlockSize;
    }

};


// EOF