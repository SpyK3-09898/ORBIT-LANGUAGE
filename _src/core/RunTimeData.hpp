
// ============ RUNTIME DATA =========== //
// Shared Data in Core | Data Compartilhada do Core
// Developed by: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES | INCLUDE HEADERS E DEPENDENCIAS
#include "utils/aliases.hpp"
#include "Arena/Arena.hpp"

#include <cstddef>
#include <filesystem>

// DATA

struct CodePosition
{
    int indent = 0;
    ui32 start = 0;
    ui32 len = 0;

    ui32 line = 0;
    ui32 collumn = 0;
};

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
        bool buildMode=false;
        bool generateLog=false;
        bool vmConsoleDebug=false;
        bool UnicodeSupport=true;
    } flags;
    char** argv;
};
