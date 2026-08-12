
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

            if (!Tok)
                break;

            ParserUtils::UpdateStatePos(Tok, State);
            Cond.push_back(Tok);
        }

        // ERROR PREV | PREVENÇÃO DE ERRO.
        if (Cond.size()-1 == 0)
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
        else if (Cond[Cond.size()-1]->Type != TokenType::COLON)
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

        // UPDATE STACK | ATUALIZA A PILHA.
        State.consumedInst = true;
        ParserUtils::AddInst<IfNode>(Ctrl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;
        
        // FINALIZE | FINALIZA.
        return Ctrl;
    }

    // Elif  Statements | Instruções de Elif
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

        IfNode* IfFather = static_cast<IfNode*>(State.CurrBody->Father);

        if (IfFather->ElseBody != nullptr)
        {
            OrbitLog::SyntaxLog::SyntaxWarn(
                "Parsing",
                "'Elif' Block After <ELSE>",
                "<ELIF> After <ELSE> Block",
                "Elif Must be After If or Others Elifs",
                IfFather->pos.line,
                IfFather->pos.collumn
            );
        }

        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        // GET CONDITION | PEGA A COND.
        vec<Token*> Cond;
        while (true)
        {
            Token* Tok = Inst.Advance();
            if (!Tok) break;
            ParserUtils::UpdateStatePos(Tok, State);
            Cond.push_back(Tok);
        }

        // ERROR PREV | PREVENÇÃO DE ERROS.
        if (Cond.empty() || Cond.size() == 1)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected Boolean <CONDITION>",
                "Expected <CONDITION> After Elif Statement",
                "Add a Valid Cond",
                Inst.Tokens[0]->pos.line,
                Inst.Tokens[0]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
        else if (Cond.back()->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <ELIF_COND>",
                "Expected ':' After Elif Condition",
                "Finalize Elif Instruction",
                Cond.back()->pos.line,
                Cond.back()->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
        Cond.pop_back();

        // CREATE NODES | CRIA OS NOS
        ElifNode* Elif = ParserUtils::MakeNode<ElifNode>(State, Res, Memory);
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);

        // PARSE CONDITION | PARSEIA A COND
        Instruction ICond{{}, Cond};
        Elif->Cond = ExprParser.ParseExpression(ICond, State, Res, Data, Memory);

        // SET BODY | DEFINE O BODY
        Elif->Body = Body;
        Body->Father = Elif;
        Body->Type = BodyTypes::CONTROL_IF;
        IfFather->ElifBodyStack.push_back(Elif);

        // UPDATE STACK | ATUALIZA A STACK
        State.consumedInst = true;
        ParserUtils::PopBodyStack(State, Data);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        return Elif;
    }

    // Else  Statements | Instruções de Else
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

        // SET BODYS | SETA OS BODYS.
        Else->Body = Body;
        Body->Father = Else;
        Body->Type = BodyTypes::CONTROL_IF;

        // SET IF | DEFINE IF.
        IfNode* PrevIfFather = static_cast<IfNode*>(State.CurrBody->Father);
        PrevIfFather->ElseBody = Else;

        // UPDATE STACK | ATUALIZA A PILHA.
        State.consumedInst = true;
        ParserUtils::PopBodyStack(State, Data);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        // FINALIZE | FINALIZA.
        return Else;
    }

    // While Loop Controls | Loops de Controle 'While'
    ControlNode* ParseWhileLoop(
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

            if (!Tok)
                break;

            ParserUtils::UpdateStatePos(Tok, State);
            Cond.push_back(Tok);
        }
        // ERROR PREVENTION | PREVENÇÃO DE ERROS
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
        WhileNode* Cntrl = ParserUtils::MakeNode<WhileNode>(State, Res, Memory);
        Cntrl->Type = LoopTypes::WHILE;
        Instruction PrevInst{{}, Cond};
        Cntrl->Cond = ExprParser.ParseExpression(
            PrevInst,
            State,
            Res,
            Data,
            Memory
        );
        
        // MODIFIERS | MODIFICADORES.
        for (Token* Arg : Inst.Modifiers)
        {
            if (Arg->Lexeme(Data) == "do")
                Cntrl->alwaysExecuteFirst=true;
            else {

                string argLexeme = Arg->Lexeme(Data);
                OrbitLog::SyntaxLog::SyntaxWarn(
                    "Parsing",
                    "Unexpected <MODIFIER>",
                    "This Mod: " + argLexeme + " Is Not Allowed Here",
                    "Remove",
                    Arg->pos.line,
                    Arg->pos.collumn
                );
            }
        }

        // CREATE A NEW BODY | CRIA UM NOVO BODY
        BodyNode* Body = ParserUtils::
            MakeNode<BodyNode>(State, Res, Memory);

        // SET BODY
        Body->Father = Cntrl;
        Body->Type = BodyTypes::LOOP_WHILE;
        Cntrl->Body = Body;

        // UPDATE STACK | ATUALIZA A PILHA.
        State.consumedInst = true;
        ParserUtils::AddInst<WhileNode>(Cntrl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        return Cntrl;
    }

    // For Loop Controls | Controle de Loop For
    ControlNode* ParseForLoop(
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
        
        // ERROR PREVENTIONS | PREVEÇÕES DE ERROS
        if (!Inst.Peek())
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected <IDENTIFIER> After 'For' Statement",
                "For Need A <IDENTIFIER>",
                "Add a Valid <IDENTIFIER> After 'For'",
                E->pos.line,
                E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);
        } else if (Inst.Peek()->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected <IDENTIFIER> After 'For' Statement, But Got: "+E->GetType(),
                "For Need A <IDENTIFIER>",
                "Add a Valid <IDENTIFIER> After 'For'",
                E->pos.line,
                E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
       
        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        // ERROR PREVENTION | PREVENÇÃO DE ERROS
        if (!Inst.Peek())
        {
           OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected 'In' After '"+E->Lexeme(Data)+"' Statement",
                "For Need A 'In'",
                "Add a Valid 'In' After 'IDENTIFIER'",
                E->pos.line,
                E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);            
        }
        else if (Inst.Peek()->Type != TokenType::CNTXT_KW or Inst.Peek()->Lexeme(Data) != "in")
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected 'in' After '"+E->Lexeme(Data)+"' Statement",
                "For Need A 'in', But Got: "+Inst.Peek()->Lexeme(Data),
                "Add a Valid 'in' After 'IDENTIFIER'",
                E->pos.line,
                E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);    
        }

        ForNode* Cntrl = ParserUtils::MakeNode<ForNode>(State, Res, Memory);
        Cntrl->Identifier = ParserUtils::MakeNode<IdentifierNode>
            (State, Res, Memory);

        // IND
        Cntrl->Identifier->Name = E->Lexeme(Data);
        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        // COND LOOP | LOOP DE CONDIÇÃO
        vec<Token*> Cond;
        while (true)
        {
            Token* Tok = Inst.Advance();

            if (!Tok)
                break;

            ParserUtils::UpdateStatePos(Tok, State);
            Cond.push_back(Tok);
        }

        // ERROR PREV | PREVENÇÃO DE ERRO.
        if (Cond.size() <= 1)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected <RANGE>",
                "Expected <CONDITION> After For Statement",
                "Add a Valid Range",
                Inst.Tokens[0]->pos.line,
                Inst.Tokens[0]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
        else if (Cond[Cond.size()-1]->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <FOR_RANGE>",
                "Expected ':' After For Range",
                "Finalize Range Instruction",
                Cond[Cond.size()-1]->pos.line,
                Cond[Cond.size()-1]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data); 
            return ParserUtils::MakeNode<ErrorStmtNode>(State, Res, Memory);
        }
        Cond.pop_back();

        Instruction RangeInst{{}, Cond};
        ExpressionNode* Range = ExprParser.ParseExpression(RangeInst, State, Res, Data, Memory);
        
        // ERROR PREV | PREVENÇÃO DE ERROS.
        if (Range->Type != NodeType::RANGE && Range->Type != NodeType::ERROR)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "<RANGE> Expected",
                "Expected <RANGE> After <FOR> Statement, But Got: ",
                "Add a Valid <RANGE> After <FOR> Statement",
                Range->pos.line,
                Range->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);  
        }
        RangeNode* R = static_cast<RangeNode*>(Range);
    
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);

        // SET FOR | DEFINE O FOR 
        Body->Father = Cntrl;
        Body->Type = BodyTypes::LOOP_FOR;
        Cntrl->End = R;
        Cntrl->Body = Body;
        Cntrl->Type = LoopTypes::FOR;

        // UPDATE STACK | ATUALIZA A PILHA.
        State.consumedInst = true;
        ParserUtils::AddInst<ForNode>(Cntrl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        return Cntrl;
    }

    // Parse Return Statements | Parseia Instruções de Retorno.
    ControlNode* ParseReturn(
        Instruction& Inst, 
        ParseState& State, 
        ParseResult& Res,
        RunTimeData& Data, 
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory,
        bool IsIf=false 
    )
    {
        // ERROR PREV | PREVENÇÃO DE ERROS.
        if (!State.HaveTypeInBodyStack(BodyTypes::FUNCTION))
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "<RETURN> Not In <FUNCTION> Body",
                "<RETURN> Can ONLY Stay in <FUNCTION> Bodys",
                "Move Or Remove <RETURN>",
                State.Pos.line,
                State.Pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);              
        }

        // RETURN
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        // TAKE COND
        vec<Token*> Cond;
        while(true)
        {
            Token* Tok = Inst.Advance();

            if (!Tok)
                break;

            ParserUtils::UpdateStatePos(Tok, State);
            Cond.push_back(Tok);
        }

        // INSTRUCTION COND
        Instruction ICond{{}, Cond};
        ExpressionNode* CondNode = ExprParser.
            ParseExpression(ICond, State,Res, Data, Memory);
        
        FnDecl* F = static_cast<FnDecl*>(State.Bodys[State.HaveTypeInBodyStack(BodyTypes::FUNCTION)]->Father);
        F->haveReturn=true;

        // SET NODE | DEFINE O NODE
        ReturnNode* Cntrl = ParserUtils::
            MakeNode<ReturnNode>(State, Res, Memory);
        Cntrl->Value = CondNode;
        Cntrl->isIf = IsIf;

        return Cntrl;
    }

    // Parse Echo Statements | Parseia Instruções de Echos.
    ControlNode* ParseEcho(
        Instruction& Inst, 
        ParseState& State, 
        ParseResult& Res,
        RunTimeData& Data, 
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory
    )
    {
        // RETURN
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        // Error Prevention | Prevenção de Erros.
        if (Inst.Peek() and Inst.Peek()->Type != TokenType::CNTXT_KW)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <CONTEXTUAL_KW>",
                "Expected CNTX... After <ECHO> Statement, Ex: 'cout', 'cerr, 'callert'. Bug Got: <NULL>",
                "Add a Valid Mode For Echo",
                Inst.Peek()->pos.line, Inst.Peek()->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorStmtNode>(State, Res, Memory);
        }

        // Set Node | Define o Nó
        EchoNode* Cntrl = ParserUtils::MakeNode<EchoNode>
            (State, Res, Memory);

        // Set Mode | Define o Modo.
        string L = Inst.Peek()->Lexeme(Data);
        if (L == "cout")
            Cntrl->Type = EchoTypes::COUT;
        else if (L == "cwarn")
            Cntrl->Type = EchoTypes::CWARN;
        else if (L == "cerr")
            Cntrl->Type = EchoTypes::CERR; 
        else {
            OrbitLog::SyntaxLog::SyntaxWarn("Parsing", 
                "Ignoring <ECHO> Node", 
                "Mode: "+L+" Is Invalid, Ignoring. .. ...", 
                "Add a Valid Mode",
                Inst.Peek()->pos.line, Inst.Peek()->pos.collumn
            ); 
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::
                    MakeNode<ErrorStmtNode>(State, Res, Memory);           
        }

        // Advance
        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        // TAKE COND
        vec<Token*> Cond;
        while(true)
        {
            Token* Tok = Inst.Advance();

            if (!Tok)
                break;

            ParserUtils::UpdateStatePos(Tok, State);
            Cond.push_back(Tok);
        }

        // INSTRUCTION COND
        Instruction ICond{{}, Cond};
        ExpressionNode* CondNode = ExprParser.
            ParseExpression(ICond, State,Res, Data, Memory);
        Cntrl->Value = CondNode;

        return Cntrl;
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
    string Lexeme = Entry->Lexeme(Data);
    switch (Entry->Type) {
        
        case TokenType::KEYWORD:
            if (Lexeme == "if")
                return ControlUtils::ParseIfNode(Inst, State, Res, Data, DeclParser, ExprParser, Memory);
            else if (Lexeme == "else")
                return ControlUtils::ParseElseNode(Inst, State, Res, Data, DeclParser, ExprParser, Memory);  
            else if (Lexeme == "elif")
                return ControlUtils::ParseElifNode(Inst, State, Res, Data, DeclParser, ExprParser, Memory);  
            else if (Lexeme == "while")
                return ControlUtils::ParseWhileLoop(Inst, State, Res, Data, DeclParser, ExprParser, Memory);
            else if (Lexeme == "for") 
                return ControlUtils::ParseForLoop(Inst, State, Res, Data, DeclParser, ExprParser, Memory);
            else if (Lexeme == "return")
                return ControlUtils::ParseReturn(Inst, State, Res, Data, DeclParser, ExprParser, Memory, false);
            else if (Lexeme == "return_if")
                return ControlUtils::ParseReturn(Inst, State, Res, Data, DeclParser, ExprParser, Memory, true);
    
            else if (Lexeme == "end")
                {
                    ParserUtils::PopBodyStack(State, Data); 
                    State.consumedInst=true;  
                    return nullptr;
                }
            else
                return nullptr;
        default:
            return nullptr;
    }
}

// EOF
