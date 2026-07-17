
// ============ RUNTIME DATA =========== //
// Shared Data in Core
// Developed by: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES | INCLUDE HEADERS E DEPENDENCIAS
#include "utils/aliases.hpp"

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
    struct {
        bool debugMode=false;
        bool generateLog=false;
    } flags;
};

// EOF