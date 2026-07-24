
// ========== DECLARATION PARSER =========== //
// Parse Token And Generate '_AST'(Abstract Syntax Tree).
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// PRAGMATIC INFOS | INFORMAÇOES PRAGMATICAS
#pragma once

// INCLUDE HEADERS 'N DEPENDENCES
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// MAIN CLASS | CLASSE PRINCIPAL
class DeclarationParser
{
    
    public:

        DeclarationNode* ParseDeclaration(            
            Instruction& Inst,
            ParseState& State,
            ParseResult& Res,
            RunTimeData& Data,
            Arena& Memory
        );
    private:
};