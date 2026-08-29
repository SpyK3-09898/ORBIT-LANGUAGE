
// ========== SPECIAL PARSER ========== //
// Parse Special Statements | Parseia Instruções Especiais.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../../AST/AST.hpp"

#include "../Control/control.hpp"
#include "../Declaration/declaration.hpp"
#include "../Expressions/expression.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// MAIN CLASS | CLASSE PRINCIPAL
class SpecialParser
{
    public:
        SpecialNode* ParseSpecial(
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            ExpressionParser& ExprParser,
            DeclarationParser& DeclParser,
            ControlParser& CntrlParser,
            Arena& Memory
        );
};