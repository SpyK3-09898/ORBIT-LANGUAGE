
// =========== ORBIT ENTRY-POINT ========== //
// Entry-Point, 'main' fn | Ponto De Entrada da ORBIT
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES | INCLUDE HEADERS E DEPENDENCIAS
#include "utils/aliases.hpp"
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
                Data.LogDir = fs::absolute(argv[0]).parent_path().parent_path() / "_tests/logs";
                RunOrbit(argv[2], Data);
            }
        } else if (Entry == "--benchmark") {

            int times = 100;
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
                        Arg.name == "GenerateLog"
                        && holds_alt_value<bool>(Arg.value, true)
                    ) Data.flags.generateLog = true;

                    if (
                        Arg.name == "Times"
                        && holds_alt<int>(Arg.value)
                    ) times = std::get<int>(Arg.value);
                }

                Data.LogDir = fs::absolute(argv[0]).parent_path().parent_path() / "_tests/logs";

                for (int i = 0; i < 10; i++)
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
        } else if (Entry == "--version") {

            fs::path p(argv[0]); p = p.parent_path().parent_path();
            PrintLn("ORBIT - version: ", p.filename());
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