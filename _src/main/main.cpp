
// =========== ORBIT ENTRY-POINT ========== //
// Entry-Point, 'main' fn | Ponto De Entrada da ORBIT
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES | INCLUDE HEADERS E DEPENDENCIAS
#include "utils/aliases.hpp"
#include "utils/file.hpp"
#include "commands/run_file.hpp"
#include "../core/RunTimeData.hpp"

#include <chrono>
#include <thread>
#include <filesystem>
#include <numeric>
#include <limits>
#include <string>

// PARSE ARGUMENTS | PARSEIA ARGUMENTOS
void ParseRunTimeArgs(const vec<string>& args, RunTimeData& Data)
{

    for (const auto& arg : args)
    {
        if (!arg.starts_with("--"))
            continue;

        auto eqPos = arg.find('=');

        if (eqPos == string::npos)
            continue;

        RunTimeArg runtimeArg;
        runtimeArg.name = arg.substr(2, eqPos - 2);
        string value = arg.substr(eqPos + 1);

        if (value == "ON")
        {
            runtimeArg.value = true;
        }
        else if (value == "OFF")
        {
            runtimeArg.value = false;
        }
        else if (
            value.size() >= 2 &&
            (
                (value.front() == '\'' && value.back() == '\'') ||
                (value.front() == '"' && value.back() == '"')
            )
        )
        {
            runtimeArg.value = value.substr(1, value.size() - 2);
        }
        else
        {
            bool isInt = !value.empty();

            for (char c : value)
            {
                if (!std::isdigit(static_cast<unsigned char>(c)))
                {
                    isInt = false;
                    break;
                }
            }

            if (isInt)
                runtimeArg.value = std::stoi(value);
            else
                runtimeArg.value = value;
        }

        Data.Args.push_back(std::move(runtimeArg));
    }
}

void Await(int Time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(Time));
}

// ===== MAIN FN ====== //
int main(int argc, char* argv[])
{
    // Try Init ORBIT
    try {

        if (argc == 1)
            throw runt_err("Non commands provided");
        
        RunTimeData Data;
        Data.argv = argv;
        string Entry = argv[1];

        // PROJECTS | PROJETOS
        if (Entry == "--new") {

            if (argc == 2)
                throw runt_err("Expected Instace to Create. .. ...");
            string inst=argv[2] ;
            if (inst == "project") {
                
                if (argc == 3)
                    throw runt_err("Expected Project-Path to Create. .. ...");

                fs::path ProjectDir(argv[3]);
                string ProjectName = "_project";

                ParseRunTimeArgs(vec<string>(argv + 4, argv + argc), Data);
                for (RunTimeArg& Arg : Data.Args)
                {
                    if (
                        Arg.name == "Name"
                        && holds_alt<string>(Arg.value)
                    )
                    {
                        ProjectName = std::get<string>(Arg.value);
                        break;
                    }
                }

                fs::path filePath = ProjectDir / ProjectName;

                if (fs::exists(filePath))
                    throw runt_err("Project Already Exists");

                fs::create_directories(ProjectDir);
                fs::path TemplatePath =
                    fs::absolute(argv[0]).parent_path().parent_path() /
                    "_templates/_project";

                if (!fs::exists(TemplatePath))
                    throw runt_err("Project Template Not Found");   
            
                PrintLn("\nSTARTING TASK: Copy Project Template");
                Await(250);
                PrintIn("[########----------] 50%");

                fs::copy(
                    TemplatePath,
                    filePath,
                    fs::copy_options::recursive
                );
                
                Await(250);
                PrintIn("[##################] 100%");
                Await(250);
                PrintInLn("ENDOF TASK: 'Copy Project Template'. .. ...");
                Await(250);
                PrintInLn("");  
                

            } else if (inst == "script")  {

                if (argc == 3)
                    throw runt_err("Expected Script-Path to Create. .. ...");
 
                fs::path filePath(argv[3]);
                if (filePath.extension() != ".ORBIT")
                    throw runt_err("Try Create a Non-.ORBIT File");
                if (fs::exists(filePath))
                    throw runt_err("File Already Exists");
                
                fs::create_directories(filePath.parent_path());
                std::ofstream file(filePath);
                if (!file)
                    throw runt_err("Cannot Write Temporary Data");
                file << 
                    "\n_extends ORBIT;\n"
                    "\n_import stdlib=standart;"
                    "\n_typedef using std=standart.*;"
                    "\n\n_method In;\n\n";

                file.close();
            }
        }

        // RUN ORBIT | RODANDO A ORBIT
        else if (Entry == "--run") {
            if (argc < 3)
                throw runt_err("File Expected after commandd '--run'");
            else {
                ParseRunTimeArgs(vec<string>(argv +  2, argv + argc), Data);
                for (RunTimeArg Arg : Data.Args)
                {
                    if (
                        Arg.name == "DebugMode" 
                        && holds_alt_value<bool>(Arg.value, true)
                    ) Data.flags.debugMode = true;
                    if (
                        Arg.name == "GenerateLog" 
                        && holds_alt_value<bool>(Arg.value, true)
                    ) Data.flags.generateLog=true;
                }
                Data.LogDir = GetOrbitOrigin(argv) / "_tests/logs";
                RunOrbit(argv[2], Data);
            }
        } else if (Entry == "--build") {
            
            if (argc < 3)
                throw runt_err("File Expected after commandd '--run'");
            else {
                ParseRunTimeArgs(vec<string>(argv +  2, argv + argc), Data);
                for (RunTimeArg Arg : Data.Args)
                {
                    if (
                        Arg.name == "DebugMode" 
                        && holds_alt_value<bool>(Arg.value, true)
                    ) Data.flags.debugMode = true;
                    if (
                        Arg.name == "GenerateLog" 
                        && holds_alt_value<bool>(Arg.value, true)
                    ) Data.flags.generateLog=true;
                }
                Data.LogDir = GetOrbitOrigin(argv) / "_tests/logs";
                Data.flags.buildMode=true;
                RunOrbit(argv[2], Data);
            }
        } else if (Entry == "--benchmark") {

            int times = 100;
            int warm_up = 10;
            if (argc < 3)
                throw runt_err("File Expected after commandd '--benchmark'");
            else {
                ParseRunTimeArgs(vec<string>(argv + 2, argv + argc), Data);

                for (RunTimeArg Arg : Data.Args)
                {
                    if (
                        Arg.name == "DebugMode"
                        && holds_alt_value<bool>(Arg.value, true)
                    ) Data.flags.debugMode = true;  

                    if (
                        Arg.name == "Times"
                        && holds_alt<int>(Arg.value)
                    ) times = std::get<int>(Arg.value);

                    if (
                        Arg.name == "WarmUp"
                        && holds_alt<int>(Arg.value)
                    ) warm_up = std::get<int>(Arg.value);

                    if (
                        Arg.name == "GenerateLog"
                        && holds_alt_value<bool>(Arg.value, true)
                    ) throw runt_err("Can ONLY Generate Log Of 1 Exec, But Got: "+std::to_string(times));
                }

                Data.LogDir = GetOrbitOrigin(argv) / "_tests/logs";

                for (int i = 0; i < warm_up; i++)
                    RunOrbit(argv[2], Data);

                std::vector<double> Times;
                Times.reserve(times);

                for (int i = 0; i < times; i++)
                {
                    auto Start = std::chrono::steady_clock::now();

                    RunOrbit(argv[2], Data);

                    auto End = std::chrono::steady_clock::now();

                    double Time =
                        std::chrono::duration<double, std::milli>
                        (End - Start).count();

                    Times.push_back(Time);
                }

                double Sum = 0.0;

                for (double Time : Times)
                    Sum += Time;

                double Average = Sum / Times.size();
                double Min = *std::min_element(Times.begin(), Times.end());
                double Max = *std::max_element(Times.begin(), Times.end());

                PrintLn("\n\nBenchmark:");
                PrintLn("Executions: ", Times.size());
                PrintLn("Average: ", Average, " ms");
                PrintLn("----------------");
                PrintLn("Min: "+std::to_string(Min));
                PrintLn("Max: "+std::to_string(Max));
            }
        } else if (Entry == "--test") {

            PrintLn("[##########] 0%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            int errCount=0;
            int succCount=0;

            PrintLn("Checking core library. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            if (
                !fs::exists
                (GetOrbitOrigin(argv) / "_lib" / "libs" / "ORBIT") 
            ) {
                errCount++;
                PrintLn("\tLooking for ORBIT library root. .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                PrintLn("\t\tCannot find: ", fs::path(GetOrbitOrigin(argv) / "_lib" / "libs" / "ORBIT").string(), ". .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } else {
                succCount++;
                PrintLn("\tLooking for ORBIT library root. .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                PrintLn("\t\tFound: ", fs::path(GetOrbitOrigin(argv) / "_lib" / "libs" / "ORBIT").string(), ". .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            PrintLn("\tLooking for stdlib. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            if (
                !fs::exists
                (GetOrbitOrigin(argv) / "_lib" / "libs" / "ORBIT" / "stdlib") 
            ) {
                errCount++;
                PrintLn("\t\tCannot find: ", fs::path(GetOrbitOrigin(argv) / "_lib" / "libs" / "ORBIT" / "stdlib").string(), ". .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            } else {
                succCount++;
                PrintLn("\t\tFound: ", fs::path(GetOrbitOrigin(argv) / "_lib" / "libs" / "ORBIT" / "stdlib").string(), ". .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            PrintLn("Checking PATH environment. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            bool InPath = [&]()
            {
                const char* Path = std::getenv("PATH");

                if (!Path)
                    return false;

            #ifdef _WIN32
                const char Separator = ';';
            #else
                const char Separator = ':';
            #endif

                std::string Paths = Path;

                for (size_t Start = 0; Start < Paths.size();)
                {
                    size_t End = Paths.find(Separator, Start);

                    if (End == std::string::npos)
                        End = Paths.size();

            #ifdef _WIN32
                    std::string Executable = Paths.substr(Start, End - Start) + "\\orbit.exe";
            #else
                    std::string Executable = Paths.substr(Start, End - Start) + "/orbit";
            #endif

                    if (std::filesystem::exists(Executable))
                        return true;

                    Start = End + 1;
                }

                return false;
            }();
            if (!InPath)
                { 
                    PrintLn("\t\tCannot find ORBIT in <PATH>. .. ..."); 
                    errCount++; 
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            else { 
                PrintLn("\t\tORBIT found in <PATH>. .. ...");   
                succCount++; 
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            PrintLn("[---#######] 30%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));

            PrintLn("Checking essential source files. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            fs::path W_Path = GetOrbitOrigin(argv);
            vec<fs::path> EssRunTimeFiles{
                W_Path / "_src/core/FrontEnd/lexer/lexer.cpp",
                W_Path / "_src/core/FrontEnd/tokenizer/tokenizer.cpp",
                W_Path / "_src/core/FrontEnd/parser/parser.cpp",
                W_Path / "_src/core/FrontEnd/parser/ParserModules/Specials/special.cpp",
                W_Path / "_src/core/FrontEnd/parser/ParserModules/Control/control.cpp",
                W_Path / "_src/core/FrontEnd/parser/ParserModules/Declaration/declaration.cpp",
                W_Path / "_src/core/FrontEnd/parser/ParserModules/Expressions/expression.cpp",
                W_Path / "_src/core/BackEnd/codegen/codegen.cpp",
                W_Path / "_src/core/Arena/Arena.cpp"
            };
            for (fs::path P : EssRunTimeFiles)
            {
                PrintLn("\tChecking file. .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (fs::exists(P))
                    { 
                        PrintLn("\t\tFound: ", P.string(), ". .. ...");     
                        succCount++; 
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                else { 
                    PrintLn("\t\tCannot find: ", P.string(), ". .. ..."); 
                    errCount++; 
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            PrintLn("[------####] 60%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));

            PrintLn("Checking essential folders. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            vec<fs::path> EsseFolderPath{
                W_Path / "build",
                W_Path / "_bin",
                W_Path / "_cache",
                W_Path / "_dev",
                W_Path / "_dist",
                W_Path / "_github",
                W_Path / "_include",
                W_Path / "_lib",
                W_Path / "_others",
                W_Path / "_src",
                W_Path / "_templates",
                W_Path / "_templates" / "_project",
                W_Path / "_templates" / "_scripts",
                W_Path / "_tests",
                W_Path / "_tests" / "logs",
                W_Path / "_tests" / "scripts",
                W_Path / "_tests" / "test"
            };
            for (fs::path P : EsseFolderPath)
            {
                PrintLn("\tChecking folder. .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (fs::exists(P))
                     { 
                        PrintLn("\t\tFound: ", P.string(), ". .. ..."); 
                        succCount++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                     }
                else { 
                    PrintLn("\t\tCannot find: ", P.string(), ". .. ..."); 
                    errCount++; 
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }

            PrintLn("[---------#] 90%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));

            PrintLn("Checking origin files. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            vec<fs::path> EssOriginFiles{
                W_Path / "CMakeLists.txt",
                W_Path / "version.hpp",
            };
            for (fs::path P : EssOriginFiles)
            {
                PrintLn("\tChecking file. .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (fs::exists(P))
                     { 
                        PrintLn("\t\tFound: ", P.string(), ". .. ...");   
                        succCount++; 
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                     }
                else { 
                    PrintLn("\t\tCannot find: ", P.string(), ". .. ..."); 
                    errCount++;  
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
            
            PrintLn("[----------] 100%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));

            if (errCount == 0)
                PrintLn("ORBIT DOWNLOADED SUCCESSFULLY!. .. ...");
            else if (errCount > succCount)
                PrintLn("ORBIT DOWNLOAD IS CORRUPTED, PLEASE RE-STORE( --restore ) AND CHECK AGAIN. .. ...");    
            else {
                PrintLn("Errors found: ", errCount, ", Success: ", succCount, ". .. ...");
            }
        } else if (Entry == "--restore") {

            fs::path W_Path = GetOrbitOrigin(argv);
            vec<fs::path> RestorableFolders{
                W_Path / "_others",
                W_Path / "_tests",
                W_Path / "_tests" / "logs",
                W_Path / "_tests" / "scripts",
                W_Path / "_tests" / "test",
                W_Path / "_github",
            };

            PrintLn("Restoring ORBIT. .. ...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            PrintLn("[##########] 0%. .. ..."); 
            std::this_thread::sleep_for(std::chrono::seconds(1));

            bool needRestore=false;
            for (fs::path P : RestorableFolders){
                if (!fs::exists(P))
                    needRestore=true;
            };

            PrintLn("[-----#####] 50%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            if (needRestore) 
            {
                PrintLn("\tCreating missing folders. .. ...");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                for (fs::path P : RestorableFolders)
                    if (!fs::exists(P)) {
                        fs::create_directories(P);
                        PrintLn("\t\tCreated: ", P.string(), ". .. ...");
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
            } else {
                PrintLn("[----------] 100%. .. ...");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                PrintLn("NOTHING TO RESTORE. .. ...");
                return 0;
            }
            PrintLn("[----------] 100%. .. ...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            PrintLn("ORBIT RESTORED, CHECK WITH '--test'. .. ...");
        } else if (Entry == "--version") {
            fs::path p(argv[0]); p = p.parent_path().parent_path();
            PrintLn(" ~ ORBIT - version: ", p.filename(), ", Developed By: SpyK3(2026) ;). .. ...");
        } else {
            throw runt_err("Invalid command: "+Entry);
        }
        return 0;
    } catch(const runt_err& e) {
        Print("[ERROR] " + string(e.what()));
        return 1;
    }
}

// EOF