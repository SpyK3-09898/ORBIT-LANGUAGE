
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

    // PARSER VAR DECLARATIONS | PARSERIA DECLARAÇOES DE VARIAVEIS.
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
            return 
            ParserUtils::
                MakeNode<ErrorDeclNode>(NodeType::ERROR, State, Res, Memory);
        }

        // Init | Inicio
        Token* NameToken = Inst.Advance();

        ParserUtils::UpdateStatePos(NameToken, State);
        VarDeclNode Decl(State.Pos);
        for (Token* Arg : Inst.Modifiers)
        {
            string argLexeme = Arg->Lexeme(Data);
            if (argLexeme == "const")
                Decl.MutType = MutableTypes::CONST;
            else
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Unexpected <MODIFIER>",
                    "This Mod: "+argLexeme+" Is Not Allowed Here",
                    "Remove",
                    Arg->pos.line,
                    Arg->pos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::MakeNode<ErrorDeclNode>(
                    NodeType::ERROR, 
                    State, 
                    Res, 
                    Memory
                );
            }
        }

        string name = NameToken->Lexeme(Data);
        Decl.Name = name;

        // Take the Value of Var | Pega o Valor da Variavel.
        Token* Next = Inst.Peek();

        if (Next != nullptr)
        {
            if (Next->Type == TokenType::EQUAL)
            {
                Inst.Advance();

                Instruction NewInst{
                    {},
                    vec(
                        Inst.Tokens.begin() + Inst.pos.curr,
                        Inst.Tokens.end()
                    ),
                };

                ExprParser.ParseExpression(
                    NewInst,
                    State,
                    Res,
                    Data,
                    Memory
                );

            } else if (Next->Type == TokenType::COLON) {

                // Infered Type | Tipo Inferido.
                Inst.Advance();

                vec<Token*> SubInfer {
                    Inst.Tokens.begin() + Inst.pos.curr,
                    Inst.Tokens.end()
                };

                pair<LiteralTypes, int> InferedType = 
                    ParserUtils::Comm::InferType(SubInfer, Data);

                for (int i=0; i<InferedType.second; i++)
                    Inst.Advance();

            } else { // Case Unexpected '=' | Case Tenha Um '=' Inesperado.

                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "Expected '='",
                    "'=' Expected After <VAR_DECL> But Found: "+Next->GetType(),
                    "Add ';' To Separate if This is a New Inst",
                    Next->pos.line,
                    Next->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                                return ParserUtils::MakeNode<ErrorDeclNode>(
                    NodeType::ERROR, 
                    State, 
                    Res, 
                    Memory
                );
            }

        } else {

            LiteralNode* ValueNode = Memory.New<LiteralNode>
                (LiteralNode(State.Pos));

            ValueNode->Value = NoneLitVal{};
            Decl.Val = ValueNode;
        }

        return nullptr;
    }
}

// ======= ENTRY-POINT | PONTO-DE-ENTRADA ====== //
DeclarationNode* ParseDeclaration(            
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    ExpressionParser& ExprParser,
    Arena& Memory
)
{
    Token* Entry = Inst.Tokens[1];
    string lexeme = Entry->Lexeme(Data);
    switch (Entry->Type) {
     
        case TokenType::KEYWORD:

            if (lexeme == "var")
                return DeclUtils::ParseVarDecl(
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