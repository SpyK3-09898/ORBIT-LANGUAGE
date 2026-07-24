
// ========== EXPRESSION PARSER =========== //
// Parse Expressions And generate AST Members
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "Expression.hpp" // HEADER FILE | CABEÇALHO
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// ======= CORE ======= //

// Peek Current Token | Espia o Token Atual
Token* ExpressionParser::Peek(ExprParserState& EState)
{
    if (
        EState.Inst == nullptr or
        EState.Curr >= EState.Inst->Tokens.size()
    )
        return nullptr;

    return EState.Inst->Tokens[EState.Curr];
}

// Advance Current Token | Avança o Token Atual
Token* ExpressionParser::Advance(
    ExprParserState& EState,
    ParseState& State
)
{
    Token* Tok = Peek(EState);

    if (Tok == nullptr)
        return nullptr;

    State.Pos.indent = Tok->pos.indent;
    State.Pos.start = Tok->pos.start;
    State.Pos.len = Tok->pos.len;
    State.Pos.line = Tok->pos.line;
    State.Pos.collumn = Tok->pos.collumn;

    ++EState.Curr;

    return Tok;
}

// Match Current Token | Verifica e Avança o Token Atual
bool ExpressionParser::Match(
    TokenType Type,
    ExprParserState& EState,
    ParseState& State
)
{
    Token* Tok = Peek(EState);

    if (Tok == nullptr)
        return false;

    if (Tok->Type is_not Type)
        return false;

    Advance(EState, State);

    return true;
}

// Precedence | Precedencia
pair<int, int> ExpressionParser::BindingPower(TokenType Type)
{
    switch (Type)
    {
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

        // Access / Call / Index
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

// Read Operand | Le um Operando
ExpressionNode* ExpressionParser::Nud(
    ExprParserState& EState,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    Token* Tok = Advance(EState, State);

    if (Tok == nullptr)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Expected <EXPRESSION>",
            "Expression Cannot be Empty",
            "Add a Valid Expression",
            State.Pos.line,
            State.Pos.collumn
        );

        return nullptr;
    }

    switch (Tok->Type)
    {
        // LITERALS | LITERAIS

        case TokenType::INTEGER:
        case TokenType::FLOAT:
        case TokenType::STRING:
        case TokenType::TRUE:
        case TokenType::FALSE:
        case TokenType::NONE:
        case TokenType::_NULL:
        {
            LiteralNode* Node =
                Memory.New<LiteralNode>(State.Pos);

            switch (Tok->Type)
            {
                case TokenType::INTEGER:
                    Node->Value = std::stoll(Tok->Lexeme(Data));
                    break;

                case TokenType::FLOAT:
                    Node->Value = std::stof(Tok->Lexeme(Data));
                    break;

                case TokenType::STRING:
                    Node->Value = Tok->Lexeme(Data);
                    break;

                case TokenType::TRUE:
                    Node->Value = true;
                    break;

                case TokenType::FALSE:
                    Node->Value = false;
                    break;

                case TokenType::NONE:
                case TokenType::_NULL:
                    Node->Value = nullptr;
                    break;

                default:
                    break;
            }

            return Node;
        }

        // IDENTIFIER | IDENTIFICADOR

        case TokenType::IDENTIFIER:
        {
            IdentifierNode* Node =
                Memory.New<IdentifierNode>(State.Pos);

            Node->Identifier = Tok;

            return Node;
        }

        // PREFIX OPERATORS | OPERADORES PREFIXOS

        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::INCR:
        case TokenType::DECR:
        {
            UnaryNode* Node =
                Memory.New<UnaryNode>(State.Pos);

            Node->Operator = Tok;

            Node->Operand =
                ParseExpression(
                    *EState.Inst,
                    State,
                    Res,
                    Data,
                    Memory,
                    95
                );

            if (Node->Operand == nullptr)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected <OPERAND>",
                    "Unary Operator Requires an Operand",
                    "Add a Valid Expression After Operator",
                    Tok->pos.line,
                    Tok->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            }

            return Node;
        }

        // GROUP EXPRESSION | EXPRESSÃO AGRUPADA

        case TokenType::LPARENT:
        {
            ExpressionNode* Node =
                ParseExpression(
                    *EState.Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            if (
                Match(
                    TokenType::RPARENT,
                    EState,
                    State
                ) is false
            )
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected <RPARENT>",
                    "Parenthesized Expression was Never Closed",
                    "Add Closing <RPARENT>",
                    Tok->pos.line,
                    Tok->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            }

            return Node;
        }

        default:
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Unexpected <TOKEN>",
                "<TOKEN> Cannot Start an <EXPRESSION>",
                "Replace With a Valid Operand",
                Tok->pos.line,
                Tok->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return nullptr;
        }
    }
}

// Write Operand | Escreve um Operando
ExpressionNode* ExpressionParser::Led(
    ExpressionNode* Left,
    Token* Operator,
    int RightBindingPower,
    ExprParserState& EState,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    switch (Operator->Type)
    {
        // ASSIGNMENT | ATRIBUIÇÃO

        case TokenType::EQUAL:
        case TokenType::EQPL:
        case TokenType::EQMIN:
        case TokenType::EQSTAR:
        case TokenType::EQBAR:
        case TokenType::EQSL:
        case TokenType::EQPWR:
        case TokenType::EQMOD:
        {
            AssignmentNode* Node =
                Memory.New<AssignmentNode>(State.Pos);

            Node->Operator = Operator;
            Node->Left = Left;

            Node->Right =
                ParseExpression(
                    *EState.Inst,
                    State,
                    Res,
                    Data,
                    Memory,
                    RightBindingPower
                );

            if (Node->Right == nullptr)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected <EXPRESSION>",
                    "Assignment Requires a Right Expression",
                    "Add a Valid Expression After Operator",
                    Operator->pos.line,
                    Operator->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            }

            return Node;
        }


        // BINARY OPERATORS | OPERADORES BINÁRIOS

        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::MOD:

        case TokenType::POWER:
        case TokenType::POT:

        case TokenType::EQEQ:
        case TokenType::NOT_EQUAL:

        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESSEQ:
        case TokenType::GREATEQ:

        case TokenType::AND:
        case TokenType::OR:

        case TokenType::RANGE:
        {
            BinaryNode* Node =
                Memory.New<BinaryNode>(State.Pos);

            Node->Operator = Operator;
            Node->Left = Left;

            Node->Right =
                ParseExpression(
                    *EState.Inst,
                    State,
                    Res,
                    Data,
                    Memory,
                    RightBindingPower
                );

            if (Node->Right == nullptr)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected <OPERAND>",
                    "Binary Operator Requires a Right Operand",
                    "Add a Valid Expression After Operator",
                    Operator->pos.line,
                    Operator->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            }

            return Node;
        }


        // MEMBER ACCESS | ACESSO DE MEMBRO

        case TokenType::DOT:
        {
            Token* Member = Advance(
                EState,
                State
            );

            if (
                Member == nullptr or
                Member->Type is_not TokenType::IDENTIFIER
            )
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected <IDENTIFIER>",
                    "Member Access Requires an Identifier",
                    "Add a Valid Identifier After <DOT>",
                    Operator->pos.line,
                    Operator->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return nullptr;
            }

            MemberAccessNode* Node =
                Memory.New<MemberAccessNode>(State.Pos);

            Node->Object = Left;
            Node->Member = Member;

            return Node;
        }


        // INDEX ACCESS | ACESSO POR ÍNDICE

        case TokenType::LBRACKET:
        {
            IndexAccessNode* Node =
                Memory.New<IndexAccessNode>(State.Pos);

            Node->Object = Left;

            Node->Index =
                ParseExpression(
                    *EState.Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            if (
                Match(
                    TokenType::RBRACKET,
                    EState,
                    State
                ) is false
            )
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected <RBRACKET>",
                    "Index Access was Never Closed",
                    "Add Closing <RBRACKET>",
                    Operator->pos.line,
                    Operator->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            }

            return Node;
        }


        // POSTFIX OPERATORS | OPERADORES PÓS-FIXOS

        case TokenType::INCR:
        case TokenType::DECR:
        {
            UnaryNode* Node =
                Memory.New<UnaryNode>(State.Pos);

            Node->Operator = Operator;
            Node->Operand = Left;

            return Node;
        }


        default:
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Unexpected <OPERATOR>",
                "Operator is Invalid in Current Expression Context",
                "Remove or Replace the Operator",
                Operator->pos.line,
                Operator->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return Left;
        }
    }
}

// ========== ENTRY-POINT | PONTO DE ENTRADA ========== //
// Parse Expression Entry Point | Ponto de Entrada do Parser de Expressões
ASTNode* ExpressionParser::ParseExpression(
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory,
    int MinBindingPower
)
{
    ExprParserState EState;

    EState.Inst = &Inst;
    EState.Curr = 0;


    ExpressionNode* Left =
        Nud(
            EState,
            State,
            Res,
            Data,
            Memory
        );


    if (Left == nullptr)
        return nullptr;


    while (true)
    {
        Token* Tok = Peek(EState);

        if (Tok == nullptr)
            break;


        auto [LeftBindingPower, RightBindingPower] =
            BindingPower(Tok->Type);


        if (LeftBindingPower <= MinBindingPower)
            break;


        Advance(
            EState,
            State
        );


        Left =
            Led(
                Left,
                Tok,
                RightBindingPower,
                EState,
                State,
                Res,
                Data,
                Memory
            );


        if (Left == nullptr)
            return nullptr;
    }


    return Left;
}

// _EOF