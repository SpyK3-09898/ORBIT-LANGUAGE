
// ========== EXPRESSION PARSER =========== //
// PrattParse Expressions And generate AST Members
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "Expression.hpp" // HEADER FILE | CABEÇALHO
#include "../../AST/AST.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"
#include <vector>

// ======= UTILS ======= //

namespace ExprUtils {

    // UTILS

    // Return Op From Token | Retorna o Operador do Token.
    Operator GetOperator(TokenType Type)
    {
        switch(Type)
        {
            case TokenType::PLUS:
                return Operator::ADD;

            case TokenType::MINUS:
                return Operator::SUB;

            case TokenType::NOT:
                return Operator::NOT;

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

    // PARSE FUNCTIONS

    // Parse Member Access | Parseia Acessos de Membro.
    ExpressionNode* ParseAcess(
        ExpressionNode* L,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        Arena& Memory        
    )
    {
        MemberAccessNode* Access =
            ParserUtils::MakeNode<MemberAccessNode>(
                State,
                Res,
                Memory
            );

        // Set Object | Define o Objeto
        Access->Object = L;

        Token* Member = Inst.Peek();

        // Check if Have Member | Olha se tem o Membro
        if (!Member)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <MEMBER_ACCESS>",
                "Expected Identifier After '.'",
                "Add A Member Name After '.'",
                State.Pos.line,
                State.Pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }

        // Check Identifier | Verifica Identifier
        if (Member->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <MEMBER_ACCESS>",
                "Expected Identifier After '.', But Got: "+Member->GetType(),
                "Use A Valid Identifier As Member Name",
                Member->pos.line,
                Member->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }

        // Create Identifier Node | Cria o Nó Identifier
        IdentifierNode* Id =
            ParserUtils::MakeNode<IdentifierNode>(
                State,
                Res,
                Memory
            );

        Id->Name = str_view(
            Data.source.data() + Member->pos.start,
            Member->pos.len
        );

        Access->Member = Id;

        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        return Access;
    }

    // Parse Index Access | Parseia Acesso por Índice
    ExpressionNode* ParseIndex(
        ExpressionNode* L,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        Arena& Memory
    )
    {
        IndexAccessNode* Index =
            ParserUtils::MakeNode<IndexAccessNode>(
                State,
                Res,
                Memory
            );

        Index->Object = L;

        ExpressionNode* Expr =
            ExpressionParser::ParseExpression(
                Inst,
                State,
                Res,
                Data,
                Memory
            );

        if (!Expr)
            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );

        Index->Index = Expr;

        Token* Current = Inst.Peek();

        if (!Current)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <INDEX_ACCESS>",
                "Unexpected <EOF> While Parsing Index",
                "Add ']' To Close Index Access",
                State.Pos.line,
                State.Pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }

        if (Current->Type != TokenType::RBRACKET)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <INDEX_ACCESS>",
                "Expected ']' After Index Expression",
                "Close The Index Access With ']'",
                Current->pos.line,
                Current->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }

        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        return Index;
    }

    // Parse Table | Parseia Tabelas.
    ExpressionNode* ParseTable(
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        Arena& Memory
    )
    {
        // Create Table | Cria a Tabela.
        TableValue* Table =
            ParserUtils::MakeNode<TableValue>(
                State,
                Res,
                Memory
            );

        while (true)
        {
            // Take Current Token | Pega o Token Atual.
            Token* Current = Inst.Peek();

            // Empty Table | Tabela Vazia.
            if (Current && Current->Type == TokenType::RBRACE)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);

                return Table;
            }

            // Parse Key | Parseia a Chave.
            ExpressionNode* Key =
                ExpressionParser::ParseExpression(
                    Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            Current = Inst.Peek();

            // EOF
            if (!Current)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <TABLE>",
                    "Unexpected <EOF> While Parsing Table",
                    "Add '}' To Close Table",
                    State.Pos.line,
                    State.Pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }

            // Expected ':'
            if (Current->Type != TokenType::COLON)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <TABLE>",
                    "Expected ':' After Table Key",
                    "Add ':' Between Key And Value",
                    Current->pos.line,
                    Current->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }

            // Consume ':'
            Token* E = Inst.Advance();
            ParserUtils::UpdateStatePos(E, State);

            // Parse Value | Parseia o Valor.
            ExpressionNode* Value =
                ExpressionParser::ParseExpression(
                    Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            // Add Entry | Adiciona a Entrada.
            Table->Args.push_back({
                Key,
                Value
            });

            Current = Inst.Peek();

            // EOF
            if (!Current)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <TABLE>",
                    "Unexpected <EOF> While Parsing Table",
                    "Add '}' To Close Table",
                    State.Pos.line,
                    State.Pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }

            // Next Entry | Próxima Entrada.
            if (Current->Type == TokenType::COMMA)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);
                continue;
            }

            // Closing | Fechamento.
            if (Current->Type == TokenType::RBRACE)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);;
                return Table;
            }

            // Missing Comma | Falta Virgula.
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <TABLE>",
                "Expected ',' Or '}' After Table Entry",
                "Separate Table Entries With ','",
                Current->pos.line,
                Current->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return ParserUtils::MakeNode<ErrorExprNode>(
            State,
            Res,
            Memory
        );
    }
    
    // Parse Array | Parseia Arrays.
    ExpressionNode* ParseArray(
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        Arena& Memory
    )
    {
        // Create Array | Cria o Array.
        ArrayValue* Array =
            ParserUtils::MakeNode<ArrayValue>(
                State,
                Res,
                Memory
            );

        while (true)
        {
            // Take Current Token | Pega o Token Atual.
            Token* Current = Inst.Peek();

            // Empty Array | Array Vazio.
            if (Current && Current->Type == TokenType::RBRACKET)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);
                return Array;
            }

            // Parse Value | Parseia o Valor.
            ExpressionNode* Value =
                ExpressionParser::ParseExpression(
                    Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            Array->Args.push_back(Value);

            // Update Current | Atualiza o Token Atual.
            Current = Inst.Peek();

            // EOF
            if (!Current)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <ARRAY>",
                    "Unexpected <EOF> While Parsing Array",
                    "Add ']' To Close Array",
                    State.Pos.line,
                    State.Pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }

            // Next Element | Próximo Elemento.
            if (Current->Type == TokenType::COMMA)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);
                continue;
            }

            // Closing | Fechamento.
            if (Current->Type == TokenType::RBRACKET)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);
                return Array;
            }

            // Invalid Separator | Separador Inválido.
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <ARRAY>",
                "Expected ',' Or ']' After Array Element",
                "Separate Array Elements With ','",
                Current->pos.line,
                Current->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }
    }

    // Parse Fn Calls | Parseia Chamadas de Função
    ExpressionNode* ParseFOO(
        ExpressionNode* L,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        Arena& Memory
    )
    {
        // Call Base
        FunctionCall* Call = ParserUtils::MakeNode<FunctionCall>(State, Res, Memory);
        Call->Callee = L; // Set Callee | Define Callee 
        while (true) {
        
            // Take Current Token.
            Token* Current = Inst.Peek();
            if (Current && Current->Type == TokenType::RPARENT)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);;
                
                return Call;
            }
            // Create a New Arg
            ExpressionNode* Arg =
                ExpressionParser::ParseExpression(
                    Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );
            Current = Inst.Peek();

            // Set a New Arg | Define um Novo Arg.
            Call->Args.push_back(Arg);
            
            // Separate By Commas | Separa por Virgulas.
            if (!Current)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <FUNCTION_CALL>",
                    "Unexpected <EOF> While Parsing Arguments",
                    "Add ')' To Close Function Call",
                    State.Pos.line,
                    State.Pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }
            if (Current->Type == TokenType::COMMA)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);
                continue;
            }

            // Closing
            if (Current->Type == TokenType::RPARENT)
            {
                Inst.Advance();
                return Call;
                break;
            }

            // FOO(1 2)/WhitOut Comma | FOO(1 2)/Sem Virgula.
            else

            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <FUNCTION_CALL>",
                    "Expected ',' Or ')' After Function Argument",
                    "Separate Arguments With ','",
                    Current->pos.line,
                    Current->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }
        }

        return ParserUtils::MakeNode<ErrorExprNode>(
            State,
            Res,
            Memory
        );
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
            return {10, 9};

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

        case TokenType::RANGE:
            return {55,56};

        case TokenType::PLUS:
        case TokenType::MINUS:
            return {60, 61};

        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::MOD:
            return {70, 71};

        case TokenType::POWER:
        case TokenType::POT:
            return {80, 79};

        case TokenType::DOT:
        case TokenType::LPARENT:
        case TokenType::LBRACKET:
            return {90, 91};

        default:
            return {-1, -1};
    }
}

// Nud Denot | Denotação Nula.
ExpressionNode* ExpressionParser::Nud(
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    // Take Current Token | Pega o Token Atual
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
    ParserUtils::UpdateStatePos(Entry, State);

    // RUN TYPES | PERCORRE O TIPO.
    switch (Entry->Type) {
        
        // ===== UNARYS ===== //
        case TokenType::MINUS:
        case TokenType::NOT:
        case TokenType::PLUS:
        {
            // Take the Operand | Pega o Operando.
            ExpressionNode* Operand =
                ParseExpression(
                Inst,
                State,
                Res,
                Data,
                Memory,
                80
                ); 
            // Create Unary Node | Cria o Nó Unario.
            UnaryNode* U =
                ParserUtils::MakeNode<UnaryNode>(
                    State,
                    Res,
                    Memory
                );
            // SEY UNARY | DEFINE A DATA DO NÓ UNARIO.
            U->Operator = ExprUtils::GetOperator(Entry->Type);
            U->Operand  = Operand;

            return U;
        }

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
            Node->Value = Entry->Lexeme(Data);
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
        // ===== BLOCKS ===== //
        case TokenType::LPARENT:
        {
            // Create the Node | Cria o No
            ExpressionNode* Node =
                ParseExpression(
                    Inst,
                    State,
                    Res,
                    Data,
                    Memory
                );
            // Take Closing | Pega o Fechamento
            Token* C = Inst.Peek();
            if (!C or C->Type is_not TokenType::RPARENT)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Invalid <EXPRESSION>",
                    "Expected ')' After Expression",
                    "Close The Parenthesis",
                    Entry->pos.line,
                    Entry->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return ParserUtils::MakeNode<ErrorExprNode>(
                    State,
                    Res,
                    Memory
                );
            }
            Token* E = Inst.Advance();
            ParserUtils::UpdateStatePos(E, State);;
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

        // ===== TABLES & ARRAYS/CONTAINER ==== //

        case TokenType::LBRACE:
            return ExprUtils::ParseTable(
                Inst,
                State,
                Res,
                Data,
                Memory
            );
        case TokenType::LBRACKET:
            return ExprUtils::ParseArray(
                Inst,
                State,
                Res,
                Data,
                Memory               
            );

        // ===== DEF ===== //
        default:
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <EXPRESSION>",
                "Expected Init of Expression, But Got: "+Entry->GetType()+" Lexeme: "+Entry->Lexeme(Data),
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
    // RANGE
    if (OperatorToken->Type == TokenType::RANGE)
    {
        RangeNode* Node =
            ParserUtils::MakeNode<RangeNode>(
                State,
                Res,
                Memory
            );

        Node->Begin = L;

        Node->End =
            ParseExpression(
                Inst,
                State,
                Res,
                Data,
                Memory,
                RightBindingPower
            );

        if (!Node->End)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <RANGE>",
                "Expected Expression After '..'",
                "Add A Valid Range End",
                State.Pos.line,
                State.Pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorExprNode>(
                State,
                Res,
                Memory
            );
        }

        return Node;
    }
    // ===== POST-FIX!!! ===== //
    if (OperatorToken->Type == TokenType::LPARENT)
        return ExprUtils::ParseFOO(L, Inst, State, Res, Data, Memory);
    if (OperatorToken->Type == TokenType::LBRACKET)
        return ExprUtils::ParseIndex(
            L,
            Inst,
            State,
            Res,
            Data,
            Memory
        );
    if (OperatorToken->Type == TokenType::DOT)
        return ExprUtils::ParseAcess(
            L,
            Inst,
            State,
            Res,
            Data,
            Memory
        );
    // CONTINUE | CONTINUA.
    // Take the Curr Token and Op | Pega o Token Atual e o Operador.
    Operator Op = ExprUtils::GetOperator(OperatorToken->Type);
    if (Op == Operator::NONE)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Invalid <OPERATOR>",
            "Expected A Valid <OPERATOR>, But Got: "+OperatorToken->GetType(),
            "Add a Valid <OPERATOR> To Continue Expression",
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

    // ASSIGNS | ASSIGNS(MEMA COISA).
    if (
        OperatorToken->Type == TokenType::EQUAL ||
        OperatorToken->Type == TokenType::EQPL ||
        OperatorToken->Type == TokenType::EQMIN ||
        OperatorToken->Type == TokenType::EQSTAR ||
        OperatorToken->Type == TokenType::EQSL ||
        OperatorToken->Type == TokenType::EQMOD ||
        OperatorToken->Type == TokenType::EQPWR
    )
    {
        // ASSIGN | NÓ DE DEFINIÇÃO
        AssignmentNode* Node =
            ParserUtils::MakeNode<AssignmentNode>(
                State,
                Res,
                Memory
            );

        // SET ASSIGN | DEFIEN O ASSIGN
        Node->Operator = OperatorToken;
        Node->Left = L;
        Node->Right = R;

        // FINALIZE
        return Node;
    }

    // BINARY | NÓ DE OPERAÇÕES BINARIAS
    BinaryNode* Node =
        ParserUtils::MakeNode<BinaryNode>(
            State,
            Res,
            Memory
        );

    // SET BINARY | DEFINE O BINARIO.
    Node->L = L;
    Node->R = R;
    Node->Op = Op;
    
    // FINALIZE | FINALIZA.
    return Node;
}

// Implicit Multiply | Multiplicação Implicita.
ExpressionNode* ExpressionParser::ParseImplMulti(
    ExpressionNode* L,
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    // Consumes '(' | Consome o '('.
    Token* E = Inst.Advance();
    ParserUtils::UpdateStatePos(E, State);

    ExpressionNode* R = ParseExpression(
        Inst,
        State,
        Res,
        Data,
        Memory
    );
    // Take Final | Pega o Final.
    if (!R)
        return nullptr;
    if (!Inst.Peek() || Inst.Peek()->Type != TokenType::RPARENT)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Invalid <PARENTHESES>",
            "Expected ')' After Expression",
            "Close The Parentheses",
            State.Pos.line,
            State.Pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return ParserUtils::MakeNode<ErrorExprNode>(
            State,
            Res,
            Memory
        );
    }
     
    E = Inst.Advance(); // Consumes ')' | Consome ')'.
    ParserUtils::UpdateStatePos(E, State);
    // Create Binary Node to Expression in '(...)'
    // Cria o No Binario Para a Expressao Entre '(...)'.
    BinaryNode* N =
        ParserUtils::MakeNode<BinaryNode>(
            State, Res, Memory
        );
    
    N->R  = R;
    N->L  = L;
    N->Op = Operator::MUL;

    return N;
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

        // Implicit Multiply | Multiplicação Implicita.
        if (OpToken->Type == TokenType::LPARENT && L->Type == NodeType::LITERAL)
            L = ParseImplMulti(L, Inst, State, Res, Data, Memory);

        // LEFT AND RIGHT BINDING POWER | BP ESQUEDO E DIREITO.
        auto [LP, RP] = 
            BindingPower(OpToken->Type);
        if (LP < MinBindingPower)
            break;

        // CONSUMES OPERATOR | CONSOME O OPERADOR.
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

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