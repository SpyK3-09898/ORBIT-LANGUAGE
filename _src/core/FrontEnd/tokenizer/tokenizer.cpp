
// ============ TOKENIZER =========== //
// Token Pos-Processor(don't Ask Me About Tokenizer Name... DONT CHANGE!!!!!!!!!!) 
// Pos Processador de Tokens
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "tokenizer.hpp"
#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "../../RunTimeData.hpp"

#include <fstream>
using fstream=std::fstream;

// ENTRY POINT | PONTO DE ENTRADA
// Entry-Point of Tokenizing Program | Ponto-de-Entrada do Programa de Tokenizing.
LexResult& Tokenizer::InitT(LexResult& Res, RunTimeData& Data, Arena& Memory)
{
    string text;
    fstream log_file;
    TokenType LastType;
    if (Data.flags.debugMode)
    {
        log_file.open(
            Data.LogDir,
            std::ios::out | std::ios::app
        );
        text = 
            "\n\n// =========== TOKENIZING ========== //\n"
        ;
    }
    vec<string> KeyWords
    {
        "var", 
    };
    vec<string> Modifiers
    {
        "const"
    };
    unord_map<string, TokenType> Others
    {
        {"true", TokenType::TRUE}, {"True", TokenType::TRUE},
        {"false", TokenType::FALSE}, {"False", TokenType::FALSE},
        {"and", TokenType::AND}, {"or", TokenType::OR}, {"not", TokenType::NOT},
        {"is", TokenType::EQUAL}, {"is_not", TokenType::NOT_EQUAL}
    };
    int i=0;
    for (Token* Tok : Res.Tokens)
    {
        bool changed=false;
        string Lexeme = Tok->Lexeme(Data);
        if (Tok->Type == TokenType::IDENTIFIER) {
            if (contains_at(KeyWords, Lexeme))
                { LastType = Tok->Type; Tok->Type = TokenType::KEYWORD; changed=true; }
            else if (contains_at(Modifiers, Lexeme))
                { LastType = Tok->Type; Tok->Type = TokenType::MODIFIER; changed=true; }
            auto It = Others.find(Lexeme);
            if (It != Others.end())
                { LastType = Tok->Type; Tok->Type = It->second; changed=true; }
        } 
        if (changed and Data.flags.debugMode)
            text += "\nToken"+std::to_string(i)+
            ": LastType: "
            +std::to_string(static_cast<int>(LastType))
            +" -> "
            +"Type: "+std::to_string(static_cast<int>(Tok->Type))
            +"\n";
        i++;
    }
    if (Data.flags.debugMode)
    {
        text += "\n\n// =========== ENDOF: TOKENIZING ========== //\n";
        log_file << text;
    }

    return Res;
}
