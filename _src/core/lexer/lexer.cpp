
// ============= ORBIT LEXICAL ANALIZER =========== //
// Analize Code and generate Tokens | Analiza o codigo e gera Tokens.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "lexer.hpp" // HEADER FILE

#include "utils/aliases.hpp" // HEADERS
#include "../RunTimeData.hpp"

#include <cstdint> // LIBRARIES | BIBLIOTECAS
#include <fstream>
using fstream=std::fstream;

// =========== CORE ========== //

// =========== ENTRY-POINT | PONTO DE ENTRADA ========== //
LexResult Lexer::InitL(fstream& file, RunTimeData& Data)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: LEXING");

    LexResult Res;
    Data.source = string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );

    // MAIN LOOP | LOOP PRINCIPAL
    while (LexUtils::Peek(State, Data) != EOF_CHAR)
    {
        char C = LexUtils::Advance(State, Data);
        char N = LexUtils::Peek(State, Data);

        if (C is '\n')
        {
            
        }
    }

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: LEXING. .. ...");
    return std::move(Res);
}
