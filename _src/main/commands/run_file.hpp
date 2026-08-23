
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
#include "../../core/FrontEnd/parser/parser.hpp"
#include "../../core/FrontEnd/SA/semantic_analysis.hpp"

#include "../../core/BackEnd/byte_code.hpp"
#include "../../core/BackEnd/codegen/codegen.hpp"
#include "../../core/BackEnd/VM/virtual_machine.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <filesystem>
using fstream=std::fstream;
namespace fs=std::filesystem;

#ifdef _WIN32
#define byte win_byte
#include <windows.h>
#undef byte
#endif

inline bool HasUnicodeSupport()
{
#ifdef _WIN32

    HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (Handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD Mode;

    if (!GetConsoleMode(Handle, &Mode))
        return false;

    return GetConsoleOutputCP() == CP_UTF8;

#else

    const char* Lang = getenv("LANG");

    if (!Lang)
        return false;

    string Value = Lang;

    return Value.find("UTF-8") != string::npos ||
           Value.find("utf8") != string::npos;

#endif
}

// ENTRY POINT
inline int RunOrbit(string filePath, RunTimeData& Data)
{
    Data.flags.UnicodeSupport = HasUnicodeSupport();
    if (Data.flags.debugMode)
        Print("[DRIVER] STARTING TASK: Build Orbit");

    if (!fs::exists(filePath))
        throw runt_err("\nInvalid Path: " + filePath);

    fs::path Path(filePath);

    if (Path.extension() is_not ".ORBIT")
        throw runt_err(
            "\nInvalid Extension: " +
            Path.extension().string() +
            " Expected '.ORBIT'"
        );

    fstream file(Path);
    if (fs::file_size(Path) is 0)
        throw runt_err("Empty File Recived");

    Arena Memory(_16MB);

    // --- FRONT-END --- //

    // INSTANCES
    Lexer L;
    Tokenizer T;
    Parser P;
    SemanticAnalizer AS;

    // FUNCTIONS
    LexResult LRes = L.InitL(file, Data, Memory); // LEXER
    LRes = T.InitT(LRes, Data, Memory); // TOK...
    ParseResult PRes = P.InitP(LRes, Data, Memory); // PARSER
    SAResult SARes = AS.InitSA(PRes, Data, Memory); // SEMANTIC

    // For Debug | Para Debug
    if (Data.flags.debugMode)
        PrintIn("[DRIVER] ENDOF TASK: Build ORBIT. .. ..."); 

    // Finalize | Fializa: 
    OrbitLog::SyntaxLog::ThrowLog(Data); // Throw | Chamada.

    // --- BACK-END --- //

    // INSTANCES
    CodeGenerator CG;
    VirtualMachine VM;

    // FUNCTIONS
    ByteCode BC = CG.InitCG(PRes, SARes, Data, Memory);
    VM.InitVM(BC, SARes, Data, Memory);

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

    return 0;
}

// EOF