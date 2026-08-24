
// =========== LIBRARY-MANAGER =========== //
// Main of Library System | Gerencuadir do Sistema de Bibliotecas.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS N' DEPENDENCES
#include "utils/aliases.hpp"
#include "utils/file.hpp"
#include "tools/console.hpp"

#include "../RunTimeData.hpp"

#include <fstream>
#include <filesystem>
#include <string>
using fstream=std::fstream;
namespace fs=std::filesystem;

// Pack Repr | Representação de Pacotes.
struct OrbitPackage
{
    string type;
    string file_name;
    string lib_name;
    string version;
    string creator;
    fs::path path;
};

// Config Section Repr | Representação de uma Sessão de Arquivos de Configuração
using ConfigValue = variant<
    bool,
    string
>;
struct ConfigSect
{
    string Name;
    vec<pair<string, ConfigValue>> Values;
};

// Config File Repr | Representação de um Arquivo de Configuração.
struct OrbitConfig
{
    vec<ConfigSect> Sections;
};

// MAIN CLASS | CLASSE PRINCIPAL.
class LibSystem
{

    private:

        // Generate Orbit Package | Gera Pacotes ORBIT
        OrbitPackage GeneratePack(OrbitConfig Conf)
        {
            OrbitPackage Pack;
            for (ConfigSect& Sect : Conf.Sections)
            {
                if (Sect.Name == "ORBIT_PACKAGE")
                {
                    for (pair<string, ConfigValue> Val : Sect.Values)
                    {
                        if (Val.first == "Type")
                        {
                            if (
                                holds_alt_value<string>(Val.second, "EXT")
                                or
                                holds_alt_value<string>(Val.second, "LIB")
                            ) Pack.type = std::get<string>(Val.second);
                            else {
                                OrbitLog::Error("lib_manager.hpp", "Unknow VALUE In Config File", true, 404);
                            }
                        } 
                        else if (Val.first == "Version")
                        {
                            if (!holds_alt<string>(Val.second))
                                OrbitLog::Error("lib_manager.hpp", "Unknow VALUE In Config File", true, 404);
                            Pack.version = std::get<string>(Val.second);
                        }
                        else if (Val.first == "Creator")
                        {
                            if (!holds_alt<string>(Val.second))
                                OrbitLog::Error("lib_manager.hpp", "Unknow VALUE In Config File", true, 404);
                            Pack.creator = std::get<string>(Val.second);
                        }
                        else {
                            OrbitLog::Error("lib_manager.hpp", "Unknow KEY In Config File", true, 404);
                        }
                    }
                } else {
                    OrbitLog::Error("lib_manager.hpp", "Invalid SECTION In Config File", true, 404);
                }
            }
            return Pack;
        }
    public:

        // Parse Config File | Parseia Arquivos de Configuração.
        OrbitConfig ParseConfigFile(fstream& configFile)
        {
            int indent = 0;
            string L;
            OrbitConfig Config;
            ConfigSect* CurrentSection = nullptr;

            while (std::getline(configFile, L))
            {
                if (L.empty())
                    continue;

                if (L[0] == '[')
                {
                    string Name;

                    for (int i = 1; i < L.size(); i++)
                    {
                        if (L[i] == ']')
                            break;

                        Name += L[i];
                    }

                    Config.Sections.push_back({Name, {}});
                    CurrentSection = &Config.Sections.back();

                    continue;
                }

                if (L == "<<")
                {
                    CurrentSection = nullptr;
                    continue;
                }

                if (CurrentSection == nullptr)
                    continue;

                string Key;
                string Value;
                bool inKey = true;
                bool inString = false;

                for (int i = 0; i < L.size(); i++)
                {
                    char C = L[i];

                    if (C == '"')
                    {
                        inString = !inString;
                        continue;
                    }

                    if (C == '=' && inKey)
                    {
                        inKey = false;
                        continue;
                    }

                    if (inKey)
                        Key += C;
                    else
                        Value += C;
                }

                if (!Key.empty())
                {
                    if (Value == "true")
                        CurrentSection->Values.push_back({Key, true});
                    else if (Value == "false")
                        CurrentSection->Values.push_back({Key, false});
                    else
                        CurrentSection->Values.push_back({Key, Value});
                }
            }

            return Config;
        }

        // Load A Lib | Carrega Uma Biblioteca.
        OrbitPackage LoadLib(string Name, string Origin, RunTimeData& Data)
        {
            if (!fs::exists(GetOrbitOrigin(&Data.argv[0]) / "_lib/" / Origin))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "LibManager", "Cannot Find Origin Of: "+Origin, 
                    "Origin Does Not Exists, Aborting. .. ...", 
                    "Add A Valid Origin"
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
            }
            else if (!fs::exists(GetOrbitOrigin(&Data.argv[0]) / "_lib/" / Origin / Name))
            if (!fs::exists(GetOrbitOrigin(&Data.argv[0]) / "_lib/" / Origin))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "LibManager", "Cannot Find Name Of: "+Name, 
                    "Name Does Not Exists, Aborting. .. ...", 
                    "Add A Valid Origin"
                );
                OrbitLog::SyntaxLog::ThrowLog(Data);
            }
            
            fs::path libPath = GetOrbitOrigin(&Data.argv[0]) / "_lib/" / Origin / Name;
            if (!fs::exists(libPath / "ORBIT_LIB.cfg"))
                OrbitLog::Error("lib_manager.hpp", "Cannot Find Config File Of: "+Name, true, 404);
            fstream file(libPath);
            
            OrbitConfig Config = ParseConfigFile(file);
            OrbitPackage Pack = GeneratePack(Config);
            Pack.path      = libPath;
            Pack.file_name = libPath.filename().string();

            return Pack;
        }
};