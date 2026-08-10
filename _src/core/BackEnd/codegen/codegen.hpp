
// ========== CODE-GEN =========== //
// Parse '_AST' And Generate ByteCodes.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../byte_code.hpp"
#include "../../FrontEnd/parser/AST/AST.hpp"

#include "../../FrontEnd/lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

// MAIN CLASS | CLASSE PRINCIPAL
class CodeGenerator
{
    public:
        ByteCode InitCG(ParseResult& PRes, RunTimeData& data, Arena& Memory);
};