
// =========== ORBIT RUN FILE COMMAND ============ //
// Entry-Point OrbitCore PipeLine | Ponto de Entrada do PipeLine Do Core da Orbit
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "utils/aliases.hpp"
#include "../../core/RunTimeData.hpp"

#include "../../core/lexer/lexer.hpp"

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

    if (Path.extension() != ".ORBIT")
        throw runt_err(
            "Invalid Extension: " +
            Path.extension().string() +
            " Expected '.ORBIT'"
        );

    if (fs::file_size(Path) == 0)
        throw runt_err("Empty File Recived");
    if (Data.flags.debugMode)
        PrintInLn("[DRIVER] ENDOF TASK: Build ORBIT. .. ...");
    
    fstream file(Path);
    Lexer L;

    LexResult LRes = L.InitL(file);

    return 0;
}

// EOF