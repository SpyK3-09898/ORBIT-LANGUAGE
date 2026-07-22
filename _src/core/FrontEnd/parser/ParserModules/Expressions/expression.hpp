
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

// Current State of Expression Parser | Estado Atual do Parser de Expressões
struct ExprParserState
{
    Instruction* Inst = nullptr;
    ui32 Curr = 0;
};

// MAIN CLASS | CLASSE PRINCIPAL
class ExpressionParser
{
    private:

        // CORE

        Token* Peek(ExprParserState& EState);
        Token* Advance(ExprParserState& EState, ParseState& State);
        bool Match(TokenType Type, ExprParserState& EState, ParseState& State);

        // EXPRESSIONS

        ExpressionNode* Nud(
            ExprParserState& EState,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory
        );

        ExpressionNode* Led(
            ExpressionNode* Left,
            Token* Operator,
            int RightBindingPower,
            ExprParserState& EState,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory
        );

    public:

        // CORE

        pair<int, int> BindingPower(TokenType Type);

        // ENTRY-POINT

        uniq_ptr<ExpressionNode> ParseExpression(
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory,
            int MinBindingPower = 0
        );
};