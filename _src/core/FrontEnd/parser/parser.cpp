
// ========== PARSER =========== //
// Parse Token And Generate '_AST'(Abstract Syntax Tree).
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "parser.hpp" // HEADER FILE | CABEÇALHO
#include "AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

// ======== CORE ======== //

// Separate The Tokens in Instructions | Separa Token em Instruçoes
InstVec SeparateInstructions(LexResult& LRes, RunTimeData& Data)
{
    InstVec Instructions;
    Instruction CurrInst;
    bool inStartofInst = true;
    int i = 0;

    for (Token* Tok : LRes.Tokens)
    {
        if (
            Tok->Type is TokenType::SEMI_COLON
            or Tok->Type is TokenType::NEW_LINE
            or Tok->Type is TokenType::_EOF
        )
        {
            if (
                Tok->Type is TokenType::SEMI_COLON and
                i + 1 < LRes.Tokens.size() and
                LRes.Tokens[i + 1]->Type == TokenType::NEW_LINE
            )
            {
                OrbitLog::SyntaxLog::SyntaxType(
                    "Parsing",
                    "Unused ';'",
                    "<SEMI_COLON>'s Before <NEW_LINE> May be Irrelevant",
                    "Remove or Ignore this Type",
                    Tok->pos.line,
                    Tok->pos.collumn
                );
            }

            if (
                !CurrInst.Tokens.empty() or
                !CurrInst.Modifiers.empty()
            )
            {
                Instructions.push_back(CurrInst);
                CurrInst = {};
            }

            inStartofInst = true;
        }
        else if (Tok->Type is TokenType::MODIFIER)
        {
            if (inStartofInst)
            {
                CurrInst.Modifiers.push_back(Tok);
            }
            else
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <MODIFIER>",
                    "Modifiers only can be in START of <INSTRUCTION>",
                    "Add the Modifier in START of <INSTRUCTION>",
                    Tok->pos.line,
                    Tok->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            }
        }
        else
        {
            inStartofInst = false;
            CurrInst.Tokens.push_back(Tok);
        }

        i++;
    }

    return std::move(Instructions);
}

// ======== ENTRY-POINT ======= //
// Entry-Point of Parse-Program | Ponto-de-Entrada no Programa de Parsing.
ParseResult Parser::InitP(LexResult& LRes, RunTimeData& Data, Arena& Memory)
{
    ParseResult Res;

    InstVec Instructions = SeparateInstructions(LRes, Data);
    for (Instruction Inst : Instructions)
    {

    }

    return Res;
}