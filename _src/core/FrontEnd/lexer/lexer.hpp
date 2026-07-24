
// ============= ORBIT LEXICAL ANALIZER =========== //
// Analize Code and generate Tokens | Analiza o codigo e gera Tokens.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "utils/aliases.hpp"
#include "../../RunTimeData.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
using fstream=std::fstream;

// =========== DEFINITIONS | DEFINIÇOES. =========== //

// CONSTANT EXPRESSIONS | EXPRESSOES CONSTANTES
inline constexpr char EOF_CHAR = '\0';

// RETURN IF IS NUMBER | RETORNA SE E NUMERO
inline bool IS_DIGIT(char C)
{
    return C >= '0' && C <= '9';
}
// RETURN IF IS A NORMAL LETTER | RETORNA SE E UMA LETRA NORMAL
inline bool IS_ALPHA(char C)
{
    return (C >= 'a' && C <= 'z') ||
           (C >= 'A' && C <= 'Z') ||
           C == '_';
}
// RETURN IF IS A NUMBER CONSTRUCTION CHAR | RETORNA S E UM CARACTERE DE CONSTRUÇÃO DE NUMEROS
inline bool IS_NUM(char C)
{
    return IS_DIGIT(C) || C == '.' || C == 'e' || C == '\'';
}
// RETURN IF IS ALPHA OR NUM IN SAME TIME | RETORNA SE E ALFA E NUMERO AO MESMO TEMPO
inline bool IS_ALPHANUM(char C)
{
    return IS_ALPHA(C) || IS_DIGIT(C);
}

inline bool IS_STRING(char C)
{
    return (C is '\'' or C is '"');
}
inline bool IS_COMMENT(char C)
{
    return (C == '\\');
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
    TRUE,
    FALSE,
    NONE,
    _NULL,
    
    // NORMALS
    IDENTIFIER,
    KEYWORD,
    LIT_TYPE,

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
    POT,
    HOLE,

    EQPL,
    EQMIN,
    EQSTAR,
    EQBAR,
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
    AMPERSAND,

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
    NEW_LINE,
    PRAGMA,
    SEMI_PRAGMA,
    PLACE_HOLDER,
    MODIFIER,
    UNKNOWN,
    ENTRY_POINT,
    _EOF
};

// POSITION OF TOKENS | POSIÇÃO DOS TOKENS.
struct TokenPos
{
    int indent = 0;
    ui32 start = 0;
    ui32 len = 0;

    ui32 line = 0;
    ui32 collumn = 0;
};

// BASE TOKEN | TOKEN BASE.
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
    string GetType()
    {
        switch (Type)
        {
            // LITERAIS
            case TokenType::INTEGER:      return "<INTEGER>";
            case TokenType::FLOAT:        return "<FLOAT>";
            case TokenType::STRING:       return "<STRING>";
            case TokenType::TRUE:         return "<TRUE>";
            case TokenType::FALSE:        return "<FALSE>";
            case TokenType::NONE:         return "<NONE>";
            case TokenType::_NULL:        return "<NULL>";

            // NORMALS
            case TokenType::IDENTIFIER:   return "<IDENTIFIER>";
            case TokenType::KEYWORD:      return "<KEYWORD>";

            // MATH
            case TokenType::EQUAL:        return "<EQUAL>";
            case TokenType::PLUS:         return "<PLUS>";
            case TokenType::MINUS:        return "<MINUS>";
            case TokenType::STAR:         return "<STAR>";
            case TokenType::SLASH:        return "<SLASH>";
            case TokenType::BAR:          return "<BAR>";
            case TokenType::PIPE:         return "<PIPE>";
            case TokenType::POWER:        return "<POWER>";
            case TokenType::MOD:          return "<MOD>";
            case TokenType::POT:          return "<POT>";
            case TokenType::HOLE:         return "<HOLE>";

            case TokenType::EQPL:         return "<EQPL>";
            case TokenType::EQMIN:        return "<EQMIN>";
            case TokenType::EQSTAR:       return "<EQSTAR>";
            case TokenType::EQBAR:        return "<EQBAR>";
            case TokenType::EQSL:         return "<EQSL>";
            case TokenType::EQPWR:        return "<EQPWR>";
            case TokenType::EQMOD:        return "<EQMOD>";

            case TokenType::EQEQ:         return "<EQEQ>";
            case TokenType::NOT_EQUAL:    return "<NOT_EQUAL>";
            case TokenType::LESS:         return "<LESS>";
            case TokenType::GREATER:      return "<GREATER>";
            case TokenType::LESSEQ:       return "<LESS_EQUAL>";
            case TokenType::GREATEQ:      return "<GREATER_EQUAL>";

            case TokenType::AND:          return "<AND>";
            case TokenType::NOT:          return "<NOT>";
            case TokenType::OR:           return "<OR>";

            case TokenType::DOT:          return "<DOT>";
            case TokenType::COMMA:        return "<COMMA>";
            case TokenType::RANGE:        return "<RANGE>";
            case TokenType::AMPERSAND:    return "<AMPERSAND>";

            // BLOCKS
            case TokenType::LPARENT:      return "<LEFT_PARENT>";
            case TokenType::LBRACE:       return "<LEFT_BRACE>";
            case TokenType::LBRACKET:     return "<LEFT_BRACKET>";
            case TokenType::RBRACKET:     return "<RIGHT_BRACKET>";
            case TokenType::RBRACE:       return "<RIGHT_BRACE>";
            case TokenType::RPARENT:      return "<RIGHT_PARENT>";

            case TokenType::INCR:         return "<INCREMENT>";
            case TokenType::DECR:         return "<DECREMENT>";
            case TokenType::COLON:        return "<COLON>";
            case TokenType::SEMI_COLON:   return "<SEMI_COLON>";

            // SPECIALS
            case TokenType::NEW_LINE:     return "<NEW_LINE>";
            case TokenType::PRAGMA:       return "<PRAGMA>";
            case TokenType::SEMI_PRAGMA:  return "<SEMI_PRAGMA>";
            case TokenType::PLACE_HOLDER: return "<PLACE_HOLDER>";
            case TokenType::MODIFIER:     return "<MODIFIER>";
            case TokenType::UNKNOWN:      return "<UNKNOWN>";
            case TokenType::ENTRY_POINT:  return "<ENTRY_POINT>";
            case TokenType::_EOF:         return "<EOF>";
        }

        return "<UNKNOWN>";
    }
};

// LEXER

struct LexState
{
    TokenPos currPos; // Current Position of Lexer | Posição Atual dos Tokens.
    ui32 index = 0; // Current Global Position Of Lexer | Posição Atual Global dos Tokens
};

struct LexResult
{
    vec<Token*> Tokens; // Main TOken Stack | Pilha De Tokens Principal.
};

// MAIN CLASS | CLASSE PRINCIPAL.
class Lexer
{

public:

    LexResult InitL(fstream& file, RunTimeData& Data, Arena& Memory);
    static void SkipLine(LexState& State, RunTimeData& Data);

    // UTILS SCANNERS | SCANNERS UTILITARIOS
    class Scanners
    {
    public:
        void ReadNumber(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, Arena& memory);
        void ReadString(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, char N, Arena& memory);
        void ReadComment(Lexer& Lexer, RunTimeData& Data, LexState& State, char C, char N, Arena& memory);
    };
    Scanners Scanners;
private:

    // UTILS OF LEXER | UTILIDADES DO LEXER
    struct LexUtils
    {
        // RETURN NEXT CHAR AND CONSUMES | RETORNA O PROXIMO CHAR E CONSOME
        static inline char Advance(LexState& State, RunTimeData& Data)
        {
            if (State.index >= Data.source.size())
                return EOF_CHAR;

            char C = Data.source[State.index];

            if (C is '\n')
            {
                State.currPos.line++;
                State.currPos.collumn = 0;
            }
            else
            {
                State.currPos.collumn++;
            }
            
            State.index++;
            return C;
        }

        // RETURN NEXT CHAR AND NOT CONSUMES | RETORNA O PROXIMO CHAR E NAO CONSUME
        static inline char Peek(LexState& State, RunTimeData& Data)
        {
            if (State.index >= Data.source.size())
                return EOF_CHAR;

            return Data.source[State.index];
        }

        // RETURN NEXT CHAR WHIT OFFSET | RETORNA O PROXIMO CHAR COM OFFSET
        static inline char PeekNext(LexState& State, RunTimeData& Data, size_t offset = 0)
        {
            if (State.index + offset >= Data.source.size())
                return EOF_CHAR;

            return Data.source[State.index + offset];
        }
    };

    // DATA 
    LexResult Res;
    LexState State;  
};

// EOF