
// ========== CONTROL PARSER =========== //
// Parse Control Statement Tinstructions And Generate '_ASTNodes'(Abstract Syntax Tree Members).
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "control.hpp" // HEADER FILE | CABEÇALHO
#include "../../AST/AST.hpp"

#include "../Declaration/declaration.hpp"
#include "../Expressions/expression.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"
#include <filesystem>
#include <variant>

// ======== CORE ======== //

// ======== UTILS ======= //

namespace ControlUtils {

    // Parse If Controls | Parser Ifs
    ControlNode* ParseIfNode(
        Instruction& Inst, 
        ParseState& State, 
        ParseResult& Res, 
        RunTimeData& Data, 
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory
    )
    {
        // INIT
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        vec<Token*> Cond;
        while (true)
        {
            Token* Tok = Inst.Advance();
            ParserUtils::UpdateStatePos(Tok, State);
            if (!Tok)
                break;

            Cond.push_back(Tok);
        }

        // ERROR PREV | PREVENÇÃO DE ERRO.
        if (Cond.empty())
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected Boolean <CONDITION>",
                "Expected <CONDITION> After If Statement",
                "Add a Valid Cond",
                Inst.Tokens[0]->pos.line,
                Inst.Tokens[0]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
        if (Cond[Cond.size()-1]->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <IF_COND>",
                "Expected ':' After If Condition",
                "Finalize If Instruction",
                Cond[Cond.size()-1]->pos.line,
                Cond[Cond.size()-1]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data); 
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
        Cond.pop_back();

        // CREATE PREV.
        IfNode* Ctrl = ParserUtils::MakeNode<IfNode>(State, Res, Memory);
        ParserUtils::AddInst<IfNode>(Ctrl, State, Res, Memory);

        // PARSE CONDITION | FAZ O PARSER DA CONDIÇÃO.
        Instruction ICond{{}, Cond};
        Ctrl->Cond = ExprParser.ParseExpression(
            ICond, 
            State, 
            Res, 
            Data, 
            Memory
        );
        BodyNode* Body = ParserUtils::
            MakeNode<BodyNode>(State, Res, Memory);
        
        // SET BODY | DEFINE BODY.    
        Ctrl->IfBody = Body;
        Body->Father = Ctrl;
        Body->Type = BodyTypes::CONTROL_IF;

        // FINALIZE | FINALIZA.
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;
        
        return Ctrl;
    }

    ControlNode* ParseElifNode(
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory
    )
    {
        // ERROR PREVENTION | PREVENÇÃO DE ERROS.
        if (State.CurrBody->Type != BodyTypes::CONTROL_IF)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <ELIF_CONTROL> Statement",
                "<IF_CONTROL> Statement Expected Before Elif",
                "~",
                State.Pos.line,
                State.Pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }

        // GET CONDITION | OBTÉM A CONDIÇÃO.
        vec<Token*> Cond;
        int i = 1;
        while (true)
        {
            if (Inst.Tokens.size() < i)
                break;
            Token* E = Inst.Advance();
            Cond.push_back(E);
            ParserUtils::UpdateStatePos(E, State);
            i++;
        }
        Cond.pop_back();

        // ERROR PREVENTION | PREVENÇÃO DE ERROS.
        if (Cond[Cond.size() - 1]->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <ELIF_COND>",
                "Expected ':' After Elif Condition",
                "Finalize Elif Instruction",
                Cond[Cond.size() - 1]->pos.line,
                Cond[Cond.size() - 1]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }

        // CREATE PREV.
        ElifNode* Elif = ParserUtils::MakeNode<ElifNode>(State, Res, Memory);
        ParserUtils::AddInst<ElifNode>(Elif, State, Res, Memory);
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);

        // PARSE CONDITION | FAZ O PARSER DA CONDIÇÃO.
        Instruction ICond{{}, Cond};
        Elif->Cond = ExprParser.ParseExpression(
            ICond,
            State,
            Res,
            Data,
            Memory
        );

        // SET BODYS | SETA OS BODYS.
        Elif->Body = Body;

        // UPDATE STACK | ATUALIZA A PILHA.
        ParserUtils::UpdateBodyStack(Body, State, Data);

        // SET IF | DEFINE IF.
        IfNode* PrevIfFather = static_cast<IfNode*>(State.CurrBody->Father);
        PrevIfFather->ElifBodyStack.push_back(Body);

        Body->Father = Elif;
        Body->Type = BodyTypes::CONTROL_IF;

        State.lastIndent = Body->pos.indent;

        // FINALIZE | FINALIZA.
        ParserUtils::PopBodyStack(State, Data);
        return Elif;
    }

    ControlNode* ParseElseNode(
        Instruction& Inst, 
        ParseState& State, 
        ParseResult& Res, 
        RunTimeData& Data, 
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory
    )
    {
        // ERROR PREVENTION | PREVENÇÃO DE ERROS.
        if (State.CurrBody->Type != BodyTypes::CONTROL_IF) 
        { 
            OrbitLog::SyntaxLog::SyntaxError( 
                "Parsing", 
                "Invalid <ELSE_CONTROL> Statement", 
                "<IF_CONTROL> Statement Expected Before Else", 
                "~", 
                State.Pos.line, 
                State.Pos.collumn 
            ); 
            if (!Data.flags.debugMode) 
                OrbitLog::SyntaxLog::ThrowLog(Data); 
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory); 
        }

        // CREATE PREV.
        ElseNode* Else = ParserUtils::MakeNode<ElseNode>(State, Res, Memory);
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);
        ParserUtils::AddInst<ElseNode>(Else, State, Res, Memory);

        // SET BODYS | SETA OS BODYS.
        Else->Body = Body;

        // UPDATE STACK | ATUALIZA A PILH.A
        ParserUtils::UpdateBodyStack(Body, State, Data);

        // SET IF | DEFINE IF.
        IfNode* PrevIfFather = static_cast<IfNode*>(State.CurrBody->Father);
        PrevIfFather->ElseBody = Else;

        // FINALIZE | FINALIZA.
        ParserUtils::PopBodyStack(State, Data);
        return Else;
    }
}

// ======== ENTRY-POINT | PONTO DE ENTRADA ======= //
ControlNode* ControlParser::ParseControl(
    Instruction& Inst, 
    ParseState& State, 
    ParseResult& Res, 
    RunTimeData& Data, 
    DeclarationParser& DeclParser,
    ExpressionParser& ExprParser,
    Arena& Memory
)
{
    Token* Entry = Inst.Tokens[0];
    switch (Entry->Type) {
        
        case TokenType::KEYWORD:
            if (Entry->Lexeme(Data) == "If")
                return ControlUtils::ParseIfNode(Inst, State, Res, Data, DeclParser, ExprParser, Memory);
            else if (Entry->Lexeme(Data) == "Else")
                return ControlUtils::ParseElseNode(Inst, State, Res, Data, DeclParser, ExprParser, Memory)  ;  
            else if (Entry->Lexeme(Data) == "Elif")
                return ControlUtils::ParseElifNode(Inst, State, Res, Data, DeclParser, ExprParser, Memory)  ;  
            else if (Entry->Lexeme(Data) == "End")
                ParserUtils::PopBodyStack(State, Data);
            else
                return nullptr;
        default:
            return nullptr;
    }
}

// EOF
