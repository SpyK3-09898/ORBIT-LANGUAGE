
// ============= ORBIT LEXICAL ANALIZER =========== //
// Analize Code and generate Tokens | Analiza o codigo e gera Tokens.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "lexer.hpp" // HEADER FILE

#include "utils/aliases.hpp" // HEADERS
#include "tools/console.hpp"
#include "../RunTimeData.hpp"

#include <cstddef>
#include <cstdint> // LIBRARIES | BIBLIOTECAS
#include <fstream>
#include <string>
#include <filesystem>
using fstream=std::fstream;
namespace fs=std::filesystem;

// =========== CORE ========== //

void Lexer::SkipLine(LexState& State, RunTimeData& Data)
{
    while (LexUtils::Peek(State, Data) is_not '\n')
        LexUtils::Advance(State, Data);
}

// Make a Token and PushBack | Cria um Novo Token e Empilha
void MakeToken(LexResult& Res, LexState& State, RunTimeData& Data, TokenType Type)
{
    Token Tok(Type, State.currPos);
    Res.Tokens.push_back(std::move(Tok));
}

// SCANNER TO READ A NUMBER | SCANNER PARA LER UM NUMERO
void Lexer::Scanners::ReadNumber(Lexer& Lexer, RunTimeData& Data, LexState& State, char C)
{
    bool hasDot = false;
    bool hasE = false;

    State.currPos.len = 0;

    while (true)
    {
        C = LexUtils::Peek(State, Data);

        if (IS_DIGIT(C))
        {
            ++State.currPos.len;
            LexUtils::Advance(State, Data);
            continue;
        }

        if (C is '.')
        {
            if (LexUtils::PeekNext(State, Data, 1) is '.')
                break;

            if (hasDot)
            {
                SkipLine(State, Data);
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing",
                    "Invalid <DOT>",
                    "Number Already Contains <DOT>",
                    "Remove <DOT>",
                    State.currPos.line,
                    State.currPos.collumn
                );
                return;
            }

            if (hasE)
            {
                SkipLine(State, Data);
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing",
                    "Invalid <DOT>",
                    "Try to Assign <DOT> After <E-NOT>",
                    "Remove <DOT>",
                    State.currPos.line,
                    State.currPos.collumn
                );
                return;
            }

            hasDot = true;

            ++State.currPos.len;
            LexUtils::Advance(State, Data);

            if (IS_DIGIT(LexUtils::Peek(State, Data)) is false)
            {
                SkipLine(State, Data);
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing",
                    "Invalid <NUMBER>",
                    "<DIGIT> Expected After <DOT>",
                    "Add Digits After <DOT>",
                    State.currPos.line,
                    State.currPos.collumn
                );
                return;
            }

            continue;
        }

        if (C is 'e' or C is 'E')
        {
            if (hasE)
            {
                SkipLine(State, Data);
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing",
                    "Invalid <E-NOT>",
                    "Number Already Contains <E-NOT>",
                    "Remove <E-NOT>",
                    State.currPos.line,
                    State.currPos.collumn
                );
                return;
            }

            hasE = true;

            ++State.currPos.len;
            LexUtils::Advance(State, Data);

            C = LexUtils::Peek(State, Data);

            if (C is '+' or C is '-')
            {
                ++State.currPos.len;
                LexUtils::Advance(State, Data);
            }

            if (IS_DIGIT(LexUtils::Peek(State, Data)) is false)
            {
                SkipLine(State, Data);
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing",
                    "Invalid <NUMBER>",
                    "<DIGIT> Expected After <E-NOT>",
                    "Add Digits After <E-NOT>",
                    State.currPos.line,
                    State.currPos.collumn
                );
                return;
            }

            continue;
        }

        break;
    }

    if (IS_ALPHA(LexUtils::Peek(State, Data)))
    {
        SkipLine(State, Data);
        OrbitLog::SyntaxLog::SyntaxError(
            "Lexing",
            "Invalid <NUMBER>",
            "<SPACE> Expected Between Number And Identifier",
            "Add <SPACE>",
            State.currPos.line,
            State.currPos.collumn
        );
        return;
    }

    MakeToken(
        Lexer.Res,
        State,
        Data,
        hasDot or hasE ? TokenType::FLOAT : TokenType::INTEGER
    );
}

// SCANNER TO READ A STRING | SCANNER PARA LER UMA STRING
void Lexer::Scanners::ReadString(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, char N)
{
    enum class StringType
    {
        SINGLE_QUOTE,
        DOUBLE_QUOTE,
        MULTI_LINE
    };

    StringType SType;

    State.currPos.len = 0;

    if (C is '\'')
        SType = StringType::SINGLE_QUOTE;
    else if (C is '"')
    {
        if (N is '\'')
        {
            SType = StringType::MULTI_LINE;
            ++State.currPos.len;
            LexUtils::Advance(State, Data);
        }
        else
            SType = StringType::DOUBLE_QUOTE;
    }

    while (true)
    {
        C = LexUtils::Peek(State, Data);
        N = LexUtils::PeekNext(State, Data);

        if (C is EOF_CHAR)
        {
            SkipLine(State, Data);
            OrbitLog::SyntaxLog::SyntaxError(
                "Lexing",
                "Unterminated <STRING>",
                "Expected Closing Delimiter Before <EOF>",
                "Close The String",
                State.currPos.line,
                State.currPos.collumn
            );
            return;
        }

        if (SType is_not StringType::MULTI_LINE and C is '\n')
        {
            SkipLine(State, Data);
            OrbitLog::SyntaxLog::SyntaxError(
                "Lexing",
                "Unterminated <STRING>",
                "Unexpected End Of Line",
                "Close The String Before The End Of Line",
                State.currPos.line,
                State.currPos.collumn
            );
            return;
        }

        if (SType is StringType::SINGLE_QUOTE)
        {
            if (C is '\'')
            {
                LexUtils::Advance(State, Data);
                break;
            }
        }
        else if (SType is StringType::DOUBLE_QUOTE)
        {
            if (C is '"')
            {
                LexUtils::Advance(State, Data);
                break;
            }
        }
        else
        {
            if (C is '\'' and N is '"')
            {
                LexUtils::Advance(State, Data);
                LexUtils::Advance(State, Data);
                break;
            }
        }

        ++State.currPos.len;
        LexUtils::Advance(State, Data);
    }

    MakeToken(Lexer.Res, State, Data, TokenType::STRING);
}

// GENERATE LOG OF LEXER | GERA LOG DO LEXER
void GenerateLexerLog(LexResult& Res, RunTimeData& Data)
{
    PrintIn("Starting Generate LexLog. .. ...");

    string fileName;
    if (true) {
        std::hash<string> Hasher;
        size_t Hash = Hasher(Data.source);

        constexpr char Alphabet[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        string id;

        while (Hash)
        {
            id += Alphabet[Hash % 62];
            Hash /= 62;
        }

        std::reverse(id.begin(), id.end());

        if (id.empty())
            id = "0";

        if (id.size() > 15)
            id.resize(15);

        fileName = "__" + id + "__";
    }
    fs::path dir = "../../../_tests/log/";
    if (!fs::exists(dir))
        fs::create_directories(dir);

    fs::path p = dir / fileName;
    fstream file(p, std::ios::out | std::ios::trunc); 
        
    string text = 
        "\n // ========== LEXER ============ // \n"
        "TOKENS: \n";
    int i=0;
    for (Token& Tok : Res.Tokens)
    {
        text +=
            "Token"+std::to_string(i)+": "
            "TypeId: "+std::to_string(static_cast<int>(Tok.Type))+"\n"
            "Lexeme: "+Tok.Lexeme(Data)+"\n"
            "Pos(line/index): "+std::to_string(Tok.pos.line)+";"+std::to_string(Tok.pos.collumn)+"\n\n";
    }
    text += " // ============ ENDOF: LEXER =========== // ";
}

// =========== ENTRY-POINT | PONTO DE ENTRADA ========== //
// Entry Point OF Lex Program | Ponto de Partida Do Programa 
LexResult Lexer::InitL(fstream& file, RunTimeData& Data)
{
    // INIT | INICIO
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: LEXING");

    LexResult NewRes;
    LexState NewState;
    Res = NewRes;
    State = NewState;
    Data.source = string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );

    // MAIN LOOP | LOOP PRINCIPAL
    while (LexUtils::Peek(State, Data) != EOF_CHAR)
    {
        char C = LexUtils::Advance(State, Data);
        char N = LexUtils::Peek(State, Data);

        // SPECIAL CHARACTERS | CARACTERES ESPECIAIS.
        if (C is '\n')
        {
            State.currPos.indent = 0;
            MakeToken(Res, State, Data, TokenType::NEW_LINE);
            continue;
        }
        else if (C is '\t') {
            State.currPos.indent++;
        } else if (C is '\r') continue;
        else if (C == '.')
            if (N == '.')
                {
                    LexUtils::Advance(State, Data);
                    MakeToken(Res, State, Data, TokenType::RANGE);
                }
        // DEF CHARS
        if (IS_ALPHA(C))
        {
            while (IS_ALPHANUM(LexUtils::Peek(State, Data)))
            { LexUtils::Advance(State, Data); State.currPos.start++; }
            MakeToken(Res, State, Data, TokenType::IDENTIFIER);
        } else if (IS_DIGIT(C)) Scanners.ReadNumber(*this, Data, State, C);
        else if (IS_STRING(C)) Scanners.ReadString(*this, Data, State, C, N);

        // OPERATORS | OPERADORES
        switch (C) {
        
            default:
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing", 
                    "Unknow <CHAR>",
                    "Char not Supported or Invalid in Context",
                    "Change to a Valid Char",
                    State.currPos.line,
                    State.currPos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                MakeToken(Res, State, Data, TokenType::UNKNOWN);
                SkipLine(State, Data);
                
        }
    }

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: LEXING. .. ...");
    if (Data.flags.generateLog)
        GenerateLexerLog(Res , Data);
    return std::move(Res);
}
