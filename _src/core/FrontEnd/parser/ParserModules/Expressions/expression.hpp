
// ========== STATEMENT PARSER =========== //
// Parse Statement Instructions And generate AST Members
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// MAIN CLASS | CLASSE PRINCIPAL
class ExpressionParser
{

    private:
    
    public:

        ASTNode ParseExpression(Instruction Inst, ParseState& State, ParseResult& Res, Arena& Memory);
        pair<int, int> BindingPower(TokenType Type);
};