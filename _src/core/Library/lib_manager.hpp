
// =========== LIBRARY-MANAGER =========== //
// Main of Library System | Gerencuadir do Sistema de Bibliotecas.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS N' DEPENDENCES
#pragma once

#include "utils/aliases.hpp"
#include "utils/file.hpp"
#include "tools/console.hpp"

#include "../RunTimeData.hpp"

#include <fstream>
#include <filesystem>
#include <string>
using fstream=std::fstream;
namespace fs=std::filesystem;

class LibManager
{
    public:

        // Load a Library | Carrega Uma Biblioteca.
        OrbitLibrary LoadLib(string Name, string Origin, RunTimeData& Data)
        {
            // Error Prev | Prevenção de Erros:
            if (!fs::exists(GetOrbitOrigin(Data.argv) / "_lib/libs" / Origin / Name))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Module", 
                    "Cannot Find Origin: "+Origin, 
                    "Cannot Find This 'Origin-Pack' In: "+GetOrbitOrigin(Data.argv).string(),
                    "Add A Valid Origin"
                    -1,-1
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
            } else if (!fs::exists(GetOrbitOrigin(Data.argv) / "_lib/libs" / Origin))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Module", 
                    "Cannot Find Lib: "+Name, 
                    "Cannot Find This 'Library' In: "+GetOrbitOrigin(Data.argv).string(),
                    "Add A Valid Name"
                    -1,-1
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
            } else if (!fs::exists(GetOrbitOrigin(Data.argv) / "_lib/libs" / Origin / Name / "ORBIT.cfg")) 
                OrbitLog::Error("lib_manager.hpp", "Cannot Find Confg File In: "+Name, true, 404);

            // Data | Dados.
            OrbitLibrary Pack;
            Pack.ParseConfig(GetOrbitOrigin(Data.argv) / "_lib/libs" / Origin / Name / "ORBIT.cfg");
            Pack.Name = Name;
            
            return Pack;
        }
};