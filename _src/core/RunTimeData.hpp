
// ============ RUNTIME DATA =========== //
// Shared Data in Core
// Developed by: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES | INCLUDE HEADERS E DEPENDENCIAS
#include "utils/aliases.hpp"
#include <cstddef>
#include <filesystem>
#include <vcruntime_typeinfo.h>

#define KB 1'024
#define MB KB*KB
#define _1MB MB
#define _2MBs MB+MB
#define _4MBs _2MBs+_2MBs
#define _8MBs _4MBs+_4MBs
#define _12MBs _8MBs+_8MBs

// DATA

struct RunTimeArg
{
    string name;
    variant<
        int,
        bool,
        string
    > value;
};

struct RunTimeData
{
    vec<RunTimeArg> Args;
    string source;
    std::filesystem::path LogDir;
    struct {
        bool debugMode=false;
        bool generateLog=false;
    } flags;
};

// =========== ARENA & MEMORY ============ //

// ======== ARENA BLOCK ======== //

struct ArenaBlock
{
    ArenaBlock* Next;
    byte* memory;

    size_t Size;
    size_t Used;
};

// ======== ARENA ========= //
class Arena
{

    private:

    struct CheckPointMarker
    {
        ArenaBlock* Block;
        size_t Used;
    };

    public:

    Arena(size_t size = _1MB)
    {

    };

    void* Alloc(size_t size, size_t allign)
    {
        
    }

    template<typename T> T* New(T)
    {

    };

    template<typename T> T* NewArray(T)
    {

    }

    CheckPointMarker SaveCheckPoint();

    void RestoreCheckPoint(CheckPointMarker Marker);

    void Reset();

    void Finalize();
};
// EOF