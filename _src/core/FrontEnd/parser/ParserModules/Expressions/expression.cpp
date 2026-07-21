
// ========== STATEMENT PARSER =========== //
// Parse Statement Instructions And generate AST Members
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "Expression.hpp" // HEADER FILE | CABEÇALHO
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// ======= CORE ======= //

// Precedence | Precedencia
pair<int, int> ExpressionParser::BindingPower(TokenType Type) 
{

    switch (Type) {
        // Assignment (Right Associative)
        case TokenType::EQUAL:
        case TokenType::EQPL:
        case TokenType::EQMIN:
        case TokenType::EQSTAR:
        case TokenType::EQBAR:
        case TokenType::EQSL:
        case TokenType::EQPWR:
        case TokenType::EQMOD:
            return {10, 9};

        // Logical
        case TokenType::OR:
        return {20, 21};

        case TokenType::AND:
            return {30, 31};

        // Equality
        case TokenType::EQEQ:
        case TokenType::NOT_EQUAL:
            return {40, 41};

        // Comparison
        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESSEQ:
        case TokenType::GREATEQ:
            return {50, 51};

        // Range
        case TokenType::RANGE:
            return {60, 61};

        // Addition
        case TokenType::PLUS:
        case TokenType::MINUS:
            return {70, 71};

        // Multiplication
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::MOD:
            return {80, 81};

        // Power (Right Associative)
        case TokenType::POWER:
        case TokenType::POT:
            return {90, 89};

        // Access / Call / Index (Left Associative)
        case TokenType::DOT:
        case TokenType::LPARENT:
        case TokenType::LBRACKET:
        case TokenType::INCR:
        case TokenType::DECR:
            return {100, 101};

        default:
            return {0, 0};
    }
}

// Read Operand | Le o Operando


// ====== ENTRY-POINT | PONTO DE ENTRADA ======= //
ASTNode ExpressionParser::ParseExpression(Instruction Inst, ParseState& State, ParseResult& Res, Arena& Memory)
{
    
}