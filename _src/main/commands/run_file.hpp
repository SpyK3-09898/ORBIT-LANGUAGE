
// =========== ORBIT RUN FILE COMMAND ============ //
// Entry-Point OrbitCore PipeLine | Ponto de Entrada do PipeLine Do Core da Orbit
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "utils/aliases.hpp"
#include "../../core/RunTimeData.hpp"
#include "../../../_include/tools/console.hpp"

#include "../../core/lexer/lexer.hpp"

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

    Lexer L;
    LexResult LRes = L.InitL(file, Data);
    if (Data.flags.debugMode)
    { 
        PrintInLn("[DRIVER] ENDOF TASK: Build ORBIT. .. ..."); 
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        PrintInLn("");
    }
    OrbitLog::SyntaxLog::ThrowLog(Data);

    return 0;
}

// EOF