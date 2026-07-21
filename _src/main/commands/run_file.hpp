
// =========== ORBIT RUN FILE COMMAND ============ //
// Entry-Point OrbitCore PipeLine | Ponto de Entrada do PipeLine Do Core da Orbit
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFO'S | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "utils/aliases.hpp"
#include "../../core/RunTimeData.hpp"
#include "../../../_include/tools/console.hpp"

#include "../../core/FrontEnd/lexer/lexer.hpp"
#include "../../core/FrontEnd/tokenizer/tokenizer.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
using fstream=std::fstream;
namespace fs=std::filesystem;

// ENTRY POINT
inline int RunOrbit(string filePath, RunTimeData& Data)
{
    if (Data.flags.debugMode)
        Print("[DRIVER] STARTING TASK: Build Orbit");

    if (!fs::exists(filePath))
        throw runt_err("Invalid Path: " + filePath);

    fs::path Path(filePath);

    if (Path.extension() is_not ".ORBIT")
        throw runt_err(
            "Invalid Extension: " +
            Path.extension().string() +
            " Expected '.ORBIT'"
        );

    fstream file(Path);
    if (fs::file_size(Path) is 0)
        throw runt_err("Empty File Recived");

    Arena Memory;

    Lexer L;
    Tokenizer T;
    LexResult LRes = L.InitL(file, Data, Memory);
    LRes = T.InitT(LRes, Data, Memory);
    
    if (Data.flags.debugMode)
    {
        PrintInLn("[DRIVER] ENDOF TASK: Build ORBIT. .. ..."); 
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        PrintOut("");
    }
    // GENERATE MEMORY LOG | GERA LOG DE MEMORIA
    if (Data.flags.generateLog)
    {
        fstream log_file(
        Data.LogDir,
        std::ios::out | std::ios::app
        );
        string text =
            "\n// ============ MEMORY & DATA =========== //"
            "\n\nLIMIT: "+std::to_string(Memory.ReservedMemory())
            +"\nUSED: "+std::to_string(Memory.UsedMemory())
            +"\nAVALIABLE: "+std::to_string(Memory.ReservedMemory() - Memory.UsedMemory())
            +"\nSPACES/BLOCKS USED ACCOUNT: "+std::to_string(Memory.BlockCount())
            +"\n\nFinishing Arena. .. ..."
            "\n\n"
            "\\ ============ ENDOF: 'ARENA' ==========\n\n";
        log_file << text;
    }

    Memory.Finalize();
    OrbitLog::SyntaxLog::ThrowLog(Data);

    return 0;
}

// EOF