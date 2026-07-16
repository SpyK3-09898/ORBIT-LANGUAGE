
// ============= ORBIT LEXICAL ANALIZER =========== //
// Analize Code and generate Tokens | Analiza o codigo e gera Tokens.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "utils/aliases.hpp"
#include "../RunTimeData.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
using fstream=std::fstream;

// =========== DEFINITIONS | DEFINIÇOES =========== //

inline constexpr char EOF_CHAR = '\0';

inline bool IS_DIGIT(char C)
{
    return C >= '0' && C <= '9';
}

inline bool IS_ALPHA(char C)
{
    return (C >= 'a' && C <= 'z') ||
           (C >= 'A' && C <= 'Z') ||
           C == '_';
}

inline bool IS_NUM(char C)
{
    return IS_DIGIT(C) || C == '.';
}

inline bool IS_ALPHANUM(char C)
{
    return IS_ALPHA(C) || IS_DIGIT(C);
}

// =========== CORE =========== //

// TOKEN

// TOKEN TYPES | TIPO DE TOKENS(OBVIOUS/OBVIO).
enum class TokenType: uint8_t
{
    // LITERAIS
    INTEGER,
    FLOAT,
    STRING,
    BOOL,
    NONE,
    _NULL,
    
    // NORMALS
    IDENTIFIER,
    KEYWORD,

    // SIGNALS

    // MAT 
    EQUAL,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    BAR,
    PIPE,
    POWER,
    MOD,
    HOLE,

    EQPL,
    EQMIN,
    EQSTAR,
    EQSL,
    EQPWR,
    EQMOD,
    
    EQEQ,
    NOT_EQUAL,
    LESS,
    GREATER,
    LESSEQ,
    GREATEQ,

    AND,
    NOT,
    OR,

    DOT,
    COMMA,
    RANGE,

    // BLOCKS
    LPARENT,
    LBRACE,
    LBRACKET,
    RBRACKET,
    RBRACE,
    RPARENT,

    INCR,
    DECR,
    COLON,  
    SEMI_COLON,

    // SPECIALS
    PLACE_HOLDER,
    PRAGMA,
    SEMI_PRAGMA,
    NEW_LINE,
    ENTRY_POINT,
    _EOF
};

// POSITION OF TOKENS | POSIÇÃO DOS TOKENS.
struct TokenPos
{
    ui32 start;
    ui32 len;

    ui32 line;
    ui32 collumn;
};

// BASE TOKEN | TOKEN BASE
struct Token 
{
    TokenType Type;
    TokenPos pos;

    Token(TokenType T, TokenPos P) 
        : Type(T), pos(P) {}
    string Lexeme(const RunTimeData& Data) const
    {
        return Data.source.substr(pos.start, pos.len);
    }
};

// LEXER

struct LexState
{
    TokenPos currPos;
};

struct LexResult
{
    
};

// MAIN CLASS | CLASSE PRINCIPAL
class Lexer
{

public:

    LexResult InitL(fstream& file, RunTimeData& Data);
private:

    // UTILS OF LEXER | UTILIDADES DO LEXER
    struct LexUtils
    {
        // RETURN NEXT CHAR AND CONSUMES | RETORNA O PROXIMO CHAR E CONSOME
        static inline char Advance(LexState& State, RunTimeData& Data)
        {
            if (State.currPos.start >= Data.source.size())
                return EOF_CHAR;

            char C = Data.source[State.currPos.start];

            if (C is '\n')
            {
                State.currPos.line++;
                State.currPos.collumn = 0;
            }
            else
            {
                State.currPos.collumn++;
            }

            State.currPos.start++;

            return C;
        }

        // RETURN NEXT CHAR AND NOT CONSUMES | RETORNA O PROXIMO CHAR E NAO CONSUME
        static inline char Peek(LexState& State, RunTimeData& Data)
        {
            if (State.currPos.start >= Data.source.size())
                return EOF_CHAR;

            return Data.source[State.currPos.start];
        }

        // RETURN NEXT CHAR WHIT OFFSET | RETORNA O PROXIMO CHAR COM OFFSET
        static inline char PeekNext(LexState& State, RunTimeData& Data, size_t offset = 0)
        {
            if (State.currPos.start + offset >= Data.source.size())
                return EOF_CHAR;

            return Data.source[State.currPos.start + offset];
        }
    };

    // DATA 
    LexResult Res;
    LexState State;  
};