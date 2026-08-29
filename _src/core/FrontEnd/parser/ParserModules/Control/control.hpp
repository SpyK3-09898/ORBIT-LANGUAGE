
// ========== CONTROL PARSER =========== //
// Parse Control Statement Tinstructions And Generate '_ASTNodes'(Abstract Syntax Tree Members).
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../../AST/AST.hpp"

#include "../Declaration/declaration.hpp"
#include "../Expressions/expression.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// MAIN CLASS | CLASSE PRINCIPAL
class ControlParser 
{
    private:

    public:

        static ControlNode* ParseControl(
            Instruction &Inst, 
            ParseState &State, 
            ParseResult &Res, 
            RunTimeData &Data, 
            DeclarationParser& DeclParser,
            ExpressionParser& ExprParser,
            Arena& Memory
        );
};