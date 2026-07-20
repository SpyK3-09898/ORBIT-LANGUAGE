
// ============ TOKENIZER =========== //
// Token Pos-Processor(don't Ask Me About Tokenizer Name... DONT CHANGE!!!!!!!!!!) 
// Pos Processador de Tokens
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "../../RunTimeData.hpp"

// MAIN CLASS | CLASSE PRINCIPAL
class Tokenizer
{
    public:

    LexResult& InitT(LexResult& Res, RunTimeData& Data, Arena& Memory);
};