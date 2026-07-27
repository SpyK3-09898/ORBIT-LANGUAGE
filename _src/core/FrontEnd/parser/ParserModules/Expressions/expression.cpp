
// ========== EXPRESSION PARSER =========== //
// PrattParse Expressions And generate AST Members
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "Expression.hpp" // HEADER FILE | CABEÇALHO
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// ======= UTILS ======= //

namespace ExprUtils {

    // Return Op From Token | Retorna o Operador do Token.
    Operator GetOperator(TokenType Type)
    {
        switch(Type)
        {
            case TokenType::PLUS:
                return Operator::ADD;

            case TokenType::MINUS:
                return Operator::SUB;

            case TokenType::STAR:
                return Operator::MUL;

            case TokenType::SLASH:
                return Operator::DIV;

            case TokenType::MOD:
                return Operator::MOD;

            case TokenType::POWER:
            case TokenType::POT:
                return Operator::POWER;

            case TokenType::EQEQ:
                return Operator::EQUAL;

            case TokenType::NOT_EQUAL:
                return Operator::NOT_EQUAL;

            case TokenType::LESS:
                return Operator::LESS;

            case TokenType::GREATER:
                return Operator::GREATER;

            case TokenType::LESSEQ:
                return Operator::LESS_EQUAL;

            case TokenType::GREATEQ:
                return Operator::GREATER_EQUAL;

            case TokenType::AND:
                return Operator::AND;

            case TokenType::OR:
                return Operator::OR;

            case TokenType::EQUAL:
                return Operator::ASSIGN;

            case TokenType::EQPL:
                return Operator::ADD_ASSIGN;

            case TokenType::EQMIN:
                return Operator::SUB_ASSIGN;

            case TokenType::EQSTAR:
                return Operator::MUL_ASSIGN;

            case TokenType::EQSL:
                return Operator::DIV_ASSIGN;

            case TokenType::EQMOD:
                return Operator::MOD_ASSIGN;

            case TokenType::EQPWR:
                return Operator::POWER_ASSIGN;

            default:
                return Operator::NONE;
        }
    }
}

// ======= CORE ======= //

// Return Bind Power for the Token | Retorna o Poder de Mesclagen do Token.
pair<int, int> ExpressionParser::BindingPower(TokenType Type)
{
    switch (Type)
    {
        case TokenType::EQUAL:
        case TokenType::EQPL:
        case TokenType::EQMIN:
        case TokenType::EQSTAR:
        case TokenType::EQBAR:
        case TokenType::EQSL:
        case TokenType::EQPWR:
        case TokenType::EQMOD:
            return {10, 10};

        case TokenType::OR:
            return {20, 21};

        case TokenType::AND:
            return {30, 31};

        case TokenType::EQEQ:
        case TokenType::NOT_EQUAL:
            return {40, 41};

        case TokenType::LESS:
        case TokenType::GREATER:
        case TokenType::LESSEQ:
        case TokenType::GREATEQ:
            return {50, 51};

        case TokenType::PLUS:
        case TokenType::MINUS:
            return {60, 61};

        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::MOD:
            return {70, 71};

        case TokenType::POWER:
        case TokenType::POT:
            return {80, 80};

        case TokenType::DOT:
        case TokenType::LPARENT:
        case TokenType::LBRACKET:
            return {90, 91};

        default:
            return {-1, -1};
    }
}

// Nud Denot | Denotação Nula
ExpressionNode* ExpressionParser::Nud(
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    // Take Current Token
    // Pega o Token Atual
    Token* Entry = Inst.Advance();
    if (!Entry)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Invalid <EXPRESSION>",
            "Expected Expression",
            "Add a Valid Expression",
            State.Pos.line,
            State.Pos.collumn
        );
        if (!Data.flags.debugMode) {
            OrbitLog::SyntaxLog::ThrowLog(Data);
        }
        return ParserUtils::
        MakeNode<ErrorExprNode>(State, Res, Memory);
    }

    // RUN TYPES | PERCORRE O TIPO.
    switch (Entry->Type) {
        
        // ===== LITERALS | LITERAIS ===== //
        case TokenType::INTEGER:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = std::stoi(Entry->Lexeme(Data));
            return Node;
        } 
        case TokenType::FLOAT:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = std::stof(Entry->Lexeme(Data));
            return Node;
        }
        case TokenType::STRING:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = Data.source.substr(Entry->pos.start, Entry->pos.len);
            return Node;
        }
        case TokenType::TRUE:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = true;
            return Node;
        }
        case TokenType::FALSE:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = false;
            return Node;
        }
        case TokenType::NONE:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = NoneLitVal{};
            return Node;
        }
        case TokenType::_NULL:
        {
            LiteralNode* Node =
                ParserUtils::MakeNode<LiteralNode>
                    (State, Res, Memory);
            Node->Value = NullLitVal{};
            return Node;
        }

        // ===== IDENTIFIERS ===== //
        case TokenType::IDENTIFIER:
        {
            IdentifierNode* Node = 
                ParserUtils::MakeNode<IdentifierNode>
                    (State, Res, Memory);
            Node->Name = str_view(
                Data.source.data() + Entry->pos.start,
                Entry->pos.len
            );
            return Node;
        }

        // ===== DEF ===== //
        default:
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <EXPRESSION>",
                "Expected Init of Expression, But Got:"+Entry->GetType(),
                "Add a Valid <NUMBER> or <IDENTIFIER> Before Operator",
                State.Pos.line,
                State.Pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
            MakeNode<ErrorExprNode>(State, Res, Memory);
    }

    return nullptr;
}

// Led Denot | Denotação Esquerda
ExpressionNode* ExpressionParser::Led(
    ExpressionNode* L,
    Token* OperatorToken,
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory,
    int RightBindingPower
)
{
    // Take the Curr Token and Op | Pega o Token Atual e o Operador.
    Operator Op = ExprUtils::GetOperator(OperatorToken->Type);
    if (Op == Operator::NONE)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Invalid <OPERATOR>",
            "Expected A Valid <OPERATOR>",
            "Add a Valid <OPERATOR>",
            State.Pos.line,
            State.Pos.collumn
        );
        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);
        return ParserUtils::
        MakeNode<ErrorExprNode>(State, Res, Memory);
    }

    ExpressionNode* R = ParseExpression(
        Inst,
        State,
        Res,
        Data,
        Memory,
        RightBindingPower
    );

    if (!R)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Invalid <EXPRESSION>",
            "Expected <EXPRESSION> After <OPERATOR>",
            "Add a Valid Expression After <OPERATOR>",
            State.Pos.line,
            State.Pos.collumn
        );
        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);
        return ParserUtils::
            MakeNode<ErrorExprNode>(State, Res, Memory);
    }

    BinaryNode* Node =
        ParserUtils::MakeNode<BinaryNode>(
            State,
            Res,
            Memory
        );

    Node->L = L;
    Node->R = R;
    Node->Op = Op;

    return Node;
}

// ======= ENTRY-POINT ======= //

ExpressionNode* ExpressionParser::ParseExpression(
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory,
    int MinBindingPower
)
{
    ExpressionNode* L = ExpressionParser::Nud(
        Inst,
        State,
        Res,
        Data,
        Memory
    );
    if (!L)
        return nullptr;

    
    int i=0;
    while (true)
    {
        if (i > Inst.Tokens.size()+5)
        {
            OrbitLog::Error("expression.cpp", "Infinite Loop Detected(Ind out_of_range of Expression) in Expression: "+std::to_string(i), true);
        }
        // Take Operator | Pega o Operador.
        Token* OpToken = Inst.Peek();
        if (!OpToken)
            break;
        auto [LP, RP] = 
            BindingPower(OpToken->Type);
        if (LP < MinBindingPower)
            break;

        // CONSUMES OPERATOR | CONSOME O OPERADOR.
        Inst.Advance();

        L = Led(
            L,
            OpToken,
            Inst,
            State,
            Res,
            Data,
            Memory,
            RP
        );
        if (!L)
            return nullptr;
        i++;
    }
    return L;
}
// _EOF