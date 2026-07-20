
// ============= ORBIT LEXICAL ANALIZER =========== //
// Analize Code and generate Tokens | Analiza o codigo e gera Tokens.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "lexer.hpp" // HEADER FILE

#include "utils/aliases.hpp" // HEADERS
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

#include <cstddef> // LIBRARIES | BIBLIOTECAS
#include <cstdint> 
#include <cerrno>
#include <cstring>
#include <string>
#include <fstream>
#include <filesystem>
using fstream=std::fstream;
namespace fs=std::filesystem;

// =========== CORE ========== //

// Skip the Current Line for Debug
void Lexer::SkipLine(LexState& State, RunTimeData& Data)
{
    char N = LexUtils::Peek(State, Data);
    while (N is_not '\n' or N == EOF_CHAR)
        { LexUtils::Advance(State, Data); N = LexUtils::Peek(State, Data); }
    State.currPos.indent = 0;
}

// Make a Token and PushBack | Cria um Novo Token e Empilha
void MakeToken(LexResult& Res, LexState& State, RunTimeData& Data, TokenType Type, Arena& Memory)
{
    Token* Tok = Memory.New<Token>(
        Type,
        State.currPos
    );

    Res.Tokens.push_back(Tok);
}

// SCANNER TO READ A NUMBER | SCANNER PARA LER UM NUMERO
void Lexer::Scanners::ReadNumber(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, Arena& Memory)
{
    bool hasDot = false;
    bool hasE = false;

    State.currPos.len = 1;

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
        hasDot or hasE ? TokenType::FLOAT : TokenType::INTEGER,
        Memory
    );
}

// SCANNER TO READ A STRING | SCANNER PARA LER UMA STRING
void Lexer::Scanners::ReadString(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, char N, Arena& Memory) {
    enum class StringType
    {
        SINGLE_QUOTE,
        DOUBLE_QUOTE,
        MULTI_LINE
    };

    StringType SType;

    State.currPos.len = 1;

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

    MakeToken(Lexer.Res, State, Data, TokenType::STRING, Memory);
}

// SCANNER TO READ A COMMENT | SCANNER PARA LER UM COMENTÁRIO
void Lexer::Scanners::ReadComment(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, char N, Arena& Memory)
{
    enum class CommentType
    {
        INLINE,
        MULTI_LINE
    };

    CommentType CType;

    if (C is '\\')
    {
        if (N is '*')
        {
            CType = CommentType::MULTI_LINE;
            LexUtils::Advance(State, Data);
        }
        else
            CType = CommentType::INLINE;
    }

    LexUtils::Advance(State, Data);

    while (true)
    {
        C = LexUtils::Peek(State, Data);
        N = LexUtils::PeekNext(State, Data, 1);

        if (C is EOF_CHAR)
        {
            if (CType is CommentType::MULTI_LINE)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing",
                    "Unterminated <COMMENT>",
                    "Expected Closing Delimiter Before <EOF>",
                    "Close The Comment",
                    State.currPos.line,
                    State.currPos.collumn
                );
            }

            return;
        }


        if (CType is CommentType::INLINE)
        {
            if (C is '\n')
                return;

            if (C is '\\' and LexUtils::Peek(State, Data) is '\\')
            {
                LexUtils::Advance(State, Data);
                LexUtils::Advance(State, Data);
                return;
            }
        }
        else
        {
            if (C is '*' and N is '\\')
            {
                LexUtils::Advance(State, Data);
                LexUtils::Advance(State, Data);
                return;
            }
        }

        LexUtils::Advance(State, Data);
    }
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

        fileName = "__" + id + "__.txt";
    }
    fs::path dir = Data.LogDir;
    if (!fs::exists(dir))
        fs::create_directories(dir);

    fs::path p = dir / fileName;
    Data.LogDir = p;
    fstream file(p, std::ios::out | std::ios::trunc); 
    if (not file.is_open())
        { 
            PrintOut("");
            OrbitLog::Error(
                "Lexer.cpp",
                "Cannot Open File! Why: "+string(std::strerror(errno)), 
                true,
                errno
            ); 
        }

    string text = 
        "\n// ========== LEXER ============ // \n\n"
        "TOKEN COUNT: " + std::to_string(Res.Tokens.size())+"\n"
        "TOKENS: \n\n";
    int i=0;
    for (Token* Tok : Res.Tokens)
    {
        text +=
            "Token"+std::to_string(i)+": "
            "TypeId: "+std::to_string(static_cast<int>(Tok->Type))+"\n"
            "Lexeme: "+Tok->Lexeme(Data)+"\n"
            "Pos(line/index): "+std::to_string(Tok->pos.line)+";"+std::to_string(Tok->pos.collumn)+"\n\n";
        i++;
    }
    text += "\n// ============ ENDOF: LEXER =========== // ";
    file << text;
}

// =========== ENTRY-POINT | PONTO DE ENTRADA ========== //
// Entry Point OF Lex Program | Ponto de Partida Do Programa 
LexResult Lexer::InitL(fstream& file, RunTimeData& Data, Arena& Memory)
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
        State.currPos.start = State.index;
        State.currPos.len = 0;

        char C = LexUtils::Advance(State, Data);
        char N = LexUtils::Peek(State, Data);

        // SPECIAL CHARACTERS | CARACTERES ESPECIAIS.
        if (C == '\\')
        {
            if (N == '\n')
            {
                LexUtils::Advance(State, Data);
                continue;
            }
        }
        else if (C is '\n')
        {
            State.currPos.indent = 0;
            MakeToken(Res, State, Data, TokenType::NEW_LINE, Memory);
            continue;
        }
        else if (C is '\t') {
            State.currPos.indent++;
        } else if (C is '\r') continue;
        else if (C == '.')
            if (N == '.')
                {
                    LexUtils::Advance(State, Data);
                    MakeToken(Res, State, Data, TokenType::RANGE, Memory);
                    continue;
                }
        // DEF CHARS
        if (IS_ALPHA(C))
        {
            State.currPos.len++;

            while (IS_ALPHANUM(LexUtils::Peek(State, Data)))
            { 
                LexUtils::Advance(State, Data); 
                State.currPos.len++;
            }

            MakeToken(Res, State, Data, TokenType::IDENTIFIER, Memory);
            continue;
        } else if (IS_DIGIT(C)) Scanners.ReadNumber(*this, Data, State, C, Memory);
        else if (IS_STRING(C)) Scanners.ReadString(*this, Data, State, C, N, Memory);
        else if (IS_COMMENT(C) and (IS_COMMENT(N) or N == '*'))
            Scanners.ReadComment(*this, Data, State, C, N, Memory);
        else if (C == ' ')
            continue;
        else

        // OPERATORS | OPERADORES
        switch (C) {
            
            case '=':
                if (N == '=')
                    { 
                        MakeToken(Res, State, Data, TokenType::EQEQ, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else MakeToken(Res, State, Data, TokenType::EQUAL, Memory);
                continue;
            case '+':
                if (N == '=')
                    { 
                        MakeToken(Res, State, Data, TokenType::EQPL, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else MakeToken(Res, State, Data, TokenType::PLUS, Memory);
                continue;

            case '-':
                if (N == '=')
                    { 
                        MakeToken(Res, State, Data, TokenType::EQMIN, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else MakeToken(Res, State, Data, TokenType::MINUS, Memory);
                continue;

            case '*':
                if (N == '=')
                    {
                        MakeToken(Res, State, Data, TokenType::EQSTAR, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else if (N == '*')
                    {
                        MakeToken(Res, State, Data, TokenType::POT, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else MakeToken(Res, State, Data, TokenType::STAR, Memory);
                continue;

            case '/':
                if (N == '=')
                    { 
                        MakeToken(Res, State, Data, TokenType::EQBAR, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else MakeToken(Res, State, Data, TokenType::BAR, Memory);
                continue;
            case '%':
                if (N == '=')
                    { 
                        MakeToken(Res, State, Data, TokenType::EQMOD, Memory);
                        LexUtils::Advance(State, Data);
                    }
                else MakeToken(Res, State, Data, TokenType::MOD, Memory);
                continue;
            
            case '(':
                MakeToken(Res, State, Data, TokenType::LPARENT, Memory);      
                continue;

            case '[':
                MakeToken(Res, State, Data, TokenType::LBRACKET, Memory);      
                continue;            

            case '{':
                MakeToken(Res, State, Data, TokenType::LBRACE, Memory);      
                continue;
                
            case '}':
                MakeToken(Res, State, Data, TokenType::RBRACE, Memory);      
                continue; 

            case ']':
                MakeToken(Res, State, Data, TokenType::RBRACKET, Memory);      
                continue; 

            case ')':
                MakeToken(Res, State, Data, TokenType::RPARENT, Memory);      
                continue; 

            
            default:

                OrbitLog::SyntaxLog::SyntaxError(
                    "Lexing", 
                    "Unknow <CHAR>",
                    "Char not Supported or Invalid in Context. Ignoring",
                    "Change to a Valid Char",
                    State.currPos.line,
                    State.currPos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                MakeToken(Res, State, Data, TokenType::UNKNOWN, Memory);
                SkipLine(State, Data);
                continue;
        }
    }

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: LEXING. .. ...");
    if (Data.flags.generateLog)
        GenerateLexerLog(Res , Data);
    return std::move(Res);
}

// EOF