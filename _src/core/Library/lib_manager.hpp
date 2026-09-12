
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
        OrbitLibrary* LoadLib(vec<string> Path, string Origin, RunTimeData& Data, Arena& Memory)
        {
            // Error Prev | Prevenção de Erros:
            fs::path LibPath = GetOrbitOrigin(Data.argv) / "_lib" / "libs" / Origin;

            if (!fs::exists(LibPath))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Module", 
                    "Cannot Find Origin: "+Origin, 
                    "Cannot Find This 'Origin-Pack' In: "+GetOrbitOrigin(Data.argv).string(),
                    "Add A Valid Origin",
                    -1,
                    -1
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
                return nullptr;
            }

            if (Path.empty())
                return nullptr;

            // Data | Dados.
            for (size_t i = 0; i < Path.size(); ++i)
            {
                LibPath /= Path[i];
            }

            // Error Prev | Prevenção de Erros:
            if (!fs::exists(LibPath))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Module", 
                    "Cannot Find Library Path",
                    "Cannot Find: "+LibPath.string(),
                    "Add A Valid Library Path",
                    -1,
                    -1
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
                return nullptr;
            }

            if (!fs::is_regular_file(LibPath))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Module",
                    "Library Path Is Not A File",
                    "Expected A File At: "+LibPath.string(),
                    "Make The Last Import Path Element A File",
                    -1,
                    -1
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
                return nullptr;
            }

            // Data | Dados.
            OrbitLibrary* Pack = Memory.New<OrbitLibrary>(LibPath);

            Pack->Name = Path.back();
            Pack->MainFile = LibPath.string();

            return Pack;
        }
};