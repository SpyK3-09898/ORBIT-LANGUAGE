
// ========== EXPRESSION PARSER =========== //
// Parse Expressions And generate AST Members
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// ======= STATE ======= //

// MAIN CLASS | CLASSE PRINCIPAL
class ExpressionParser
{
    private:

        // EXPRESSIONS

        static ExpressionNode* Nud(
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory
        );

        static ExpressionNode* Led(
            ExpressionNode* L,
            Token* OperatorToken,
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory,
            int RightBindingPower
        );

        static ExpressionNode* ParseImplMulti(
            ExpressionNode* L,
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory
        );

    public:

        // CORE
        static pair<int, int> BindingPower(TokenType Type);

        // ENTRY-POINT
        static ExpressionNode* ParseExpression(
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory,
            int MinBindingPower = 0
        );
};