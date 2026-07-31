// ========== DECLARATION PARSER =========== //
// Parse Token And Generate '_AST'(Abstract Syntax Tree).
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "declaration.hpp" // HEADER FILE | CABEÇALHO

#include "../../AST/AST.hpp"
#include "../Expressions/expression.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

// ======= UTILS | UTILIDADES ======= //

namespace DeclUtils {

    // PARSER VAR DECLARATIONS | PARSEIA DECLARAÇOES DE VARIAVEIS.
    DeclarationNode* ParseVarDecl(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory    
    )
    {
        // Check if Have Identifier | Verifica se um Identificador foi Passado.
        if (Inst.Tokens.size() == 1)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Invalid <VARR_DECL>",
                "Expected Identifier After <VAR_DECL>",
                "Add a Valid Identifier After 'var' Command",
                Entry->pos.line,
                Entry->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return ParserUtils::MakeNode<ErrorDeclNode>(
                State,
                Res,
                Memory
            );
        }

        // Init | Inicio
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        Token* NameToken = Inst.Advance();
        ParserUtils::UpdateStatePos(NameToken, State);

        VarDeclNode* Decl = ParserUtils::MakeNode<VarDeclNode>(State, Res,Memory);

        for (Token* Arg : Inst.Modifiers)
        {
            string argLexeme = Arg->Lexeme(Data);

            if (argLexeme == "const")
                Decl->MutType = MutableTypes::CONST;
            else
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Unexpected <MODIFIER>",
                    "This Mod: " + argLexeme + " Is Not Allowed Here",
                    "Remove",
                    Arg->pos.line,
                    Arg->pos.collumn
                );

                if (!Data.flags.debugMode)
                {
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                }
                return ParserUtils::MakeNode<ErrorDeclNode>(
                    State,
                    Res,
                    Memory
                );
            }
        }

        string name = NameToken->Lexeme(Data);
        Decl->Name = name;

        Token* Next = Inst.Peek();

        if (Next != nullptr)
        {
            if (Next->Type == TokenType::EQUAL)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);

                Instruction NewInst{
                    {},
                    vec<Token*>(
                        Inst.Tokens.begin() + Inst.pos.curr,
                        Inst.Tokens.end()
                    ),
                };

                Decl->Val = ExprParser.ParseExpression(
                    NewInst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            } else if (Next->Type == TokenType::COLON) {

                // INIT 
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);

                vec<Token*> SubInfer {
                    Inst.Tokens.begin() + Inst.pos.curr,
                    Inst.Tokens.end()
                };

                pair<LiteralTypes, int> InferedType =
                    ParserUtils::Comm::InferType(SubInfer, Data);

                // ADVANCE THE INFER COMP-SIZE | AVANÇA O TAMANHO DO TIPO INFERIDO.
                for (int i = 0; i < InferedType.second; i++)
                { Token* E = Inst.Advance(); ParserUtils::UpdateStatePos(E, State); }
                Decl->InferType = InferedType.first;

                // EQUAL | IGUAL
                if (Next->Type == TokenType::EQUAL)
                {
                    Token* E = Inst.Advance();
                    ParserUtils::UpdateStatePos(E, State);

                    Instruction NewInst{
                        {},
                        vec<Token*>(
                            Inst.Tokens.begin() + Inst.pos.curr,
                            Inst.Tokens.end()
                        ),
                    };

                    Decl->Val = ExprParser.ParseExpression(
                        NewInst,
                        State,
                        Res,
                        Data,
                        Memory
                    );

                }
            } else {

                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected '='",
                    "'=' Expected After <VAR_DECL> But Found: " + Next->GetType(),
                    "Add ';' To Separate if This is a New Inst",
                    Next->pos.line,
                    Next->pos.collumn
                );

                if (!Data.flags.debugMode)
                {
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                }
                return ParserUtils::MakeNode<ErrorDeclNode>(
                    State,
                    Res,
                    Memory
                );
            }

        } else {

            LiteralNode* ValueNode = Memory.New<LiteralNode>(
                LiteralNode(State.Pos)
            );

            ValueNode->Value = NoneLitVal{};
            Decl->Val = ValueNode;
        }

        return Decl;
    }

    DeclarationNode* ParseFnDecl(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory    
    )
    {
        // ERROR PREVENTIONS | PREVENÇÃO DE ERROS.
        if (Inst.Tokens.size() == 1)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected <IDENTIFIER> After 'func'",
                "Functions Need A Name to Call After",
                "Complete <FUNCTION> Statement",
                Inst.Tokens[0]->pos.line,
                Inst.Tokens[0]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils
                ::MakeNode<ErrorDeclNode>(State, Res, Memory);
        }

        // consome "func"
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        // ERROR PREVENTIONS | PREVENÇÃO DE ERROS.
        if (!Inst.Peek() || Inst.Peek()->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected <IDENTIFIER> After 'func'",
                "Functions Need A Name to Call After",
                "Complete <FUNCTION> Statement",
                Inst.Tokens[0]->pos.line,
                Inst.Tokens[0]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils
                ::MakeNode<ErrorDeclNode>(State, Res, Memory);
        }

        // nome da função
        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        string Name = E->Lexeme(Data);

        // espera '('
        if (!Inst.Peek() || Inst.Peek()->Type != TokenType::LPARENT)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected '(' After Function Name",
                "Functions Need '(' After The Name",
                "Complete <FUNCTION> Statement",
                E->pos.line,
                E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils
                ::MakeNode<ErrorDeclNode>(State, Res, Memory);
        }

        // consome '('
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
        if (Cond.size() < 2)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected '):'",
                "Expected ')' Followed By ':' After Function Arguments",
                "Finalize Function Declaration",
                Inst.Tokens.back()->pos.line,
                Inst.Tokens.back()->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
        }
        if (Cond[Cond.size() - 2]->Type != TokenType::RPARENT)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected ')'",
                "Expected ')' Before ':', But Got: "+Cond[Cond.size() - 2]->GetType(),
                "Close Function Arguments",
                Cond[Cond.size() - 2]->pos.line,
                Cond[Cond.size() - 2]->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
        }
        if (Cond.back()->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing",
                "Expected ':'",
                "Expected ':' After ')', But Got: "+Cond.back()->GetType(),
                "Finalize Function Declaration",
                Cond.back()->pos.line,
                Cond.back()->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
        }

        // REMOVE THE '):' | REMOVE O '):'.
        Cond.pop_back(); // :
        Cond.pop_back(); // )

        FnDecl* Decl = ParserUtils::MakeNode<FnDecl>(State, Res, Memory);

        // args vazios são válidos (func foo():)
        if (!Cond.empty())
        {
            Instruction CondInst{{}, Cond};
            Decl->Args = 
                ExprParser.ParseExpression(CondInst, State, Res, Data, Memory);
        }
        else
        {
            Decl->Args = nullptr; // ou um nó vazio, conforme sua AST
        }

        // CREATE BODY | CRIA O BODY.
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);
        Body->Type = BodyTypes::FUNCTION;
        Body->Father = Decl;

        // SET BODY | DEFINE O BODY;
        Decl->Body = Body;
        Decl->Name = Name;

        // UPDATE STACK | ATUALIZA A PILHA
        State.consumedInst = true;
        ParserUtils::AddInst<FnDecl>(Decl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        // FINALIZE | FINALIZA.
        return Decl;
    }
}

// ======= ENTRY-POINT | PONTO-DE-ENTRADA ====== //
DeclarationNode* DeclarationParser::ParseDeclaration(
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    ExpressionParser& ExprParser,
    Arena& Memory
)
{
    Token* Entry = Inst.Tokens[0];
    string Lexeme = Entry->Lexeme(Data);
    switch (Entry->Type) {
     
        case TokenType::KEYWORD:

            if (Lexeme == "var")
                return DeclUtils::ParseVarDecl(
                    Entry, 
                    Inst, 
                    State, 
                    Res,
                    Data, 
                    ExprParser,
                    Memory
                );
            else if (Lexeme == "func")
                return DeclUtils::ParseFnDecl(
                    Entry, 
                    Inst, 
                    State, 
                    Res, 
                    Data, 
                    ExprParser,
                    Memory
                );
        default:
            return nullptr;
    }
    return nullptr;
}

// EOF