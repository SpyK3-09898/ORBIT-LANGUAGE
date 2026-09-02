
// ============ RUNTIME DATA =========== //
// Shared Data in Core | Data Compartilhada do Core
// Developed by: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES | INCLUDE HEADERS E DEPENDENCIAS
#include "utils/aliases.hpp"
#include "Arena/Arena.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <streambuf>
using fstream=std::fstream;
namespace fs=std::filesystem;

// PACKKAGES | PACOTES

// Typeof Pack | Tipos de Pacotes.
enum class PACKAGE_TYPES
{
    LIBRARY,
    EXTENSION,
};

// Config File Section | Seção do Arquivo de Configuração.
struct ConfigSection
{
    string Name;
    vec<pair<string, variant<
        bool,
        int,
        string,
        ConfigSection*
    >>> Values;
};

// Package Repr | Representação dos Pacotes.
struct OrbitPackage
{
    // Data | Dados
    string Name;
    string Version;
    vec<ConfigSection> Sections;
    vec<pair<string, variant<
        bool,
        int,
        string,
        ConfigSection*
    >>> Globals;
    PACKAGE_TYPES Type;
    fs::path ConfigPath;
    fs::path MainFile;

    // Constructor | Construtor
    OrbitPackage(PACKAGE_TYPES T)
        : Type(T) {};
    // Parse Orbit Config | Parseia Arquivos de configuração ORBIT:
    inline void ParseConfig(fs::path ConfigPath)
    {
        // Data | Dados,
        fstream file(ConfigPath);
        string currLine;
        ConfigSection* CurrentSection = nullptr;
        bool inMultiComment = false;

        // Error Prevention
        if (!file)
        {
            if (!fs::exists(ConfigPath))
                throw runt_err("Cannot Find: "+ConfigPath.string());
            else if (ConfigPath.extension() is_not ".cfg")
                throw runt_err("Expected '.cfg' File, But Got: "+ConfigPath.string());
            else
                throw runt_err("Cannot Open File: "+ConfigPath.string());
        }

        // Run File | Percorre os Arquivos.
        this->ConfigPath = ConfigPath;
        while (std::getline(file, currLine))
        {
            // Run Line | Percorre a Linha.
            string Line;
            for (int i = 0; i < currLine.size(); i++)
            {
                if (inMultiComment)
                {
                    if (i + 1 < currLine.size() && currLine[i] == '#' && currLine[i + 1] == '#')
                    {
                        inMultiComment = false;
                        i++;
                    }

                    continue;
                }

                if (i + 1 < currLine.size() && currLine[i] == '#' && currLine[i + 1] == '#')
                {
                    inMultiComment = true;
                    i++;
                    continue;
                }

                if (currLine[i] == '#')
                    break;

                Line += currLine[i];
            }

            // 1
            while (!Line.empty() && std::isspace(static_cast<unsigned char>(Line.front())))
                Line.erase(Line.begin());
            while (!Line.empty() && std::isspace(static_cast<unsigned char>(Line.back())))
                Line.pop_back();
            if (Line.empty())
                continue;
            if (Line == "<<")
            {
                if (CurrentSection == nullptr)
                    throw runt_err("Unexpected '<<' In Config File: "+ConfigPath.string());

                CurrentSection = nullptr;
                continue;
            }
            if (Line.front() == '[')
            {
                if (CurrentSection != nullptr)
                    throw runt_err("Expected '<<' Before New Section In Config File: "+ConfigPath.string());

                if (Line.back() != ']')
                    throw runt_err("Unclosed Section In Config File: "+ConfigPath.string());

                string SectionName = Line.substr(1, Line.size() - 2);

                if (SectionName.empty())
                    throw runt_err("Empty Section Name In Config File: "+ConfigPath.string());

                Sections.push_back({});
                CurrentSection = &Sections.back();
                CurrentSection->Name = SectionName;

                continue;
            }

            // 2
            string Key;
            string Value;
            bool inKey = true;
            bool foundEquals = false;

            // 3
            for (int i = 0; i < Line.size(); i++)
            {
                char C = Line[i];

                if (C == '=' && inKey)
                {
                    inKey = false;
                    foundEquals = true;
                    continue;
                }

                if (C == ',' && !inKey)
                    break;

                if (inKey)
                    Key += C;
                else
                    Value += C;
            }

            // 4
            while (!Key.empty() && std::isspace(static_cast<unsigned char>(Key.front())))
                Key.erase(Key.begin());
            while (!Key.empty() && std::isspace(static_cast<unsigned char>(Key.back())))
                Key.pop_back();
            while (!Value.empty() && std::isspace(static_cast<unsigned char>(Value.front())))
                Value.erase(Value.begin());
            while (!Value.empty() && std::isspace(static_cast<unsigned char>(Value.back())))
                Value.pop_back();
            if (!foundEquals)
                throw runt_err("Expected '=' In Config File: "+ConfigPath.string());
            if (Key.empty())
                throw runt_err("Empty Key In Config File: "+ConfigPath.string());
            if (Value.empty())
                throw runt_err("Empty Value In Config File: "+ConfigPath.string());
            if (Value == "true")
            {
                if (CurrentSection != nullptr)
                    CurrentSection->Values.push_back({Key, true});
                else
                    Globals.push_back({Key, true});
            }
            else if (Value == "false")
            {
                if (CurrentSection != nullptr)
                    CurrentSection->Values.push_back({Key, false});
                else
                    Globals.push_back({Key, false});
            }
            else if (Value.front() == '"' && Value.back() == '"')
            {
                string StringValue = Value.substr(1, Value.size() - 2);

                if (CurrentSection != nullptr)
                    CurrentSection->Values.push_back({Key, StringValue});
                else
                    Globals.push_back({Key, StringValue});
            }
            else
            {
                bool isInt = true;
                int Start = 0;

                if (Value.front() == '-' || Value.front() == '+')
                    Start = 1;

                if (Start == Value.size())
                    isInt = false;

                for (int i = Start; i < Value.size(); i++)
                {
                    if (!std::isdigit(static_cast<unsigned char>(Value[i])))
                    {
                        isInt = false;
                        break;
                    }
                }

                if (!isInt)
                    throw runt_err("Invalid Value In Config File: "+ConfigPath.string());

                int IntValue = std::stoi(Value);

                if (CurrentSection != nullptr)
                    CurrentSection->Values.push_back({Key, IntValue});
                else
                    Globals.push_back({Key, IntValue});
            }
        }

        // Error Prev | Prevenção de Erros.
        if (inMultiComment)
            throw runt_err("Unclosed Multi-Line Comment In Config File: "+ConfigPath.string());
        if (CurrentSection != nullptr)
            throw runt_err("Unclosed Section In Config File: "+ConfigPath.string());
        
        using ConfigValue = 
            pair<string, variant<
                bool,
                int,
                string,
                ConfigSection*
            >>;
        for (ConfigValue Val : Globals)
        {

            if (Val.first == "Type" ) {
                
                if (!holds_alt<string>(Val.second))
                    throw runt_err("STRING EXPECTED IN: "+ConfigPath.string()+" <TYPE> Assign");
                string T = std::get<string>(Val.second);
                if (T == "EXT")
                    Type = PACKAGE_TYPES::EXTENSION;
                else if (T == "LIB")
                    Type = PACKAGE_TYPES::LIBRARY;
                else {
                    throw runt_err("UNKNOW TYPE FOR: "+T+" In: "+ConfigPath.string());
                }
            }
            else if (Val.first == "Version")
            {
                if (!holds_alt<string>(Val.second))
                    throw runt_err("STRING EXPECTED IN: "+ConfigPath.string()+" <VERSION> Assign");
                Version = std::get<string>(Val.second);
            }
        }
        for (ConfigSection& Sect : Sections)
        {
            if (Sect.Name != "ORBIT_PACKAGE")
                throw runt_err("Invalid Config Section In: "+ConfigPath.string());
        }
    }
};
using ConfigValue = 
    pair<string, variant<
        bool,
        int,
        string,
        ConfigSection*
    >>;

// Library Repr Pack | Representação de Bibliotecas.
struct OrbitLibrary : OrbitPackage
{
    // LIB META-DATA | META-DADOS DA BIBLIOTECA.
    string DeclName;
    string Creator;

    // CONSTRUCTOR | CONSTRUTOR
    OrbitLibrary(fs::path ConfigPath, bool prev=false)
        : OrbitPackage(PACKAGE_TYPES::LIBRARY)
        {
            if (prev) return;
            ParseConfig(ConfigPath);

            for (ConfigSection& Sect : Sections)
            {
                if (Sect.Name != "ORBIT_PACKAGE")
                    throw runt_err("Unknow <SECTION>: "+Sect.Name);        
                for (ConfigValue& Val : Sect.Values)
                {
                    if (Val.first == "Creator") {

                        if (!holds_alt<string>(Val.second))
                            throw runt_err("Expected <STRING> In <CREATOR> On: "+ConfigPath.string());
                        Creator = std::get<string>(Val.second);
                    }                
                    else if (Val.first == "MainFile")
                    {
                        if (!holds_alt<string>(Val.second))
                            throw runt_err("<STRING> Expected In <MAIN-FILE>");
                        else if (!fs::exists(ConfigPath.parent_path() / std::get<string>(Val.second)))
                            throw runt_err("LIB MAIN FILE DONT EXISTS IN: "+fs::path(ConfigPath.parent_path() / std::get<string>(Val.second)).string());
                        MainFile = ConfigPath.parent_path() / std::get<string>(Val.second);
                    }
                }
            }
        };
};

// DATA | DADOS

// Position Pointer in Code | Ponteiro de Posição No Codigo.
struct CodePosition
{
    int indent = 0;
    ui32 start = 0;
    ui32 len = 0;

    ui32 line = 0;
    ui32 collumn = 0;
};

// Runtime Flag | Flag de Runtime.
struct RunTimeArg
{
    string name;
    variant<
        int,
        bool,
        string
    > value;
};

// Shared Exec Data | Data de Execução compartilhada.
struct RunTimeData
{
    vec<RunTimeArg> Args{};
    vec<OrbitLibrary*> Librarys{};
    vec<string> ImportStack{};
    std::filesystem::path LogDir;
    string source;
    struct {
        bool debugMode=false;
        bool buildMode=false;
        bool generateLog=false;
        bool vmConsoleDebug=false;
        bool UnicodeSupport=true;
    } flags;
    char** argv;
    ui_max fileSize=0;
};