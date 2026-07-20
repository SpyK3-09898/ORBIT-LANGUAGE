
// ============ TOKENIZER =========== //
// Token Pos-Processor(don't Ask Me About Tokenizer Name... DONT CHANGE!!!!!!!!!!) 
// Pos Processador de Tokens
// Developed By: SpyK3(2026) | License: GitHub(MIT).


// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "tokenizer.hpp"
#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "../../RunTimeData.hpp"

// ENTRY POINT | PONTO DE ENTRADA
// Entry-Point of Tokenizing Program | Ponto-de-Entrada do Programa de Tokenizing.
LexResult& Tokenizer::InitT(LexResult& Res, RunTimeData& Data, Arena& Memory)
{

    vec<string> KeyWords
    {
        
    };
    for (Token* Tok : Res.Tokens)
    {

    }

    return Res;
}