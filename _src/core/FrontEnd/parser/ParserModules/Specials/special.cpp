
// ========== SPECIAL PARSER ========== //
// Parse Special Statements | Parseia Instruções Especiais.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "specials.hpp" // HEADER FILE | CABEÇALHO

#include "../../AST/AST.hpp"
#include "../Control/control.hpp"
#include "../Declaration/declaration.hpp"
#include "../Expressions/expression.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../../../RunTimeData.hpp"

#include <functional> // LIBRARIES | BIBLIOTECAS:

// ========== UTILS | UTILIDADES ========== //
namespace SpecialUtils
{

    // Parse Import Statements | Parseia Instruções de Imports.
    SpecialNode* ParseImports(
        Instruction& Inst, 
        ParseState& State, 
        ParseResult& Res,
        RunTimeData& Data, 
        ControlParser& CntrlParser,
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory,
        bool from=false
    )
    {
        // Update State Position | Atualiza a Posição do ParseState.
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        // Create Node | Cria o Nó.
        ImportNode* Node = ParserUtils::
            MakeNode<ImportNode>(State, Res, Memory);

        vec<Token*> Path;
        while (true)
        {
            Token* Tok = Inst.Advance();
            if (!Tok)
                break;

            ParserUtils::UpdateStatePos(Tok, State);
            if (Tok->Type == TokenType::CNTXT_KW and Tok->Lexeme(Data) == "as")
            {
                Token* Alias = Inst.Advance();
                if (!Alias || Alias->Type != TokenType::IDENTIFIER)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Expected: <IDENTIFIER>, But Got: "
                            +(Alias ? Alias->GetType() : "<EOF>"),
                        "Imports Need A Valid Alias",
                        "Add A Valid <IDENTIFIER> After <AS>",
                        Alias ? Alias->pos.line : State.Pos.line,
                        Alias ? Alias->pos.collumn : State.Pos.collumn
                    );
                    if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                    return ParserUtils::MakeNode
                        <ErrorSpecialNode>(State, Res, Memory);
                }

                ParserUtils::UpdateStatePos(Alias, State);
                Node->Alias = Alias->Lexeme(Data);
                break;
            }

            Path.push_back(Tok);
        }

        Instruction PathInst({}, Path);
        Node->Path = ExprParser.ParseExpression
            (PathInst, State, Res, Data, Memory);

        // MAIN LAMBDA | LAMBDA PRINCIPAL
        std::function<SpecialNode*(ExpressionNode*, bool)> RunPath
            =[&](ExpressionNode* Curr, bool Base) -> SpecialNode*
        {
            if (Curr->Type == NodeType::MEMBER_ACCESS)
            {
                auto C = static_cast<MemberAccessNode*>(Curr);

                SpecialNode* Err = RunPath(C->Object, Base);
                if (Err) return Err;

                return RunPath(C->Member, false);
            }
            else if (Curr->Type == NodeType::IDENTIFIER) 
            {
                auto O = static_cast<IdentifierNode*>(Curr);

                if (Base)
                {
                    if (from)
                        Node->Origin = O->Name;
                    else
                        Node->Base = Curr;
                }
                else
                {
                    if (from && !Node->Base)
                        Node->Base = Curr;
                    else
                    {
                        Node->Bottom = Curr;
                        Node->Modules.push_back(O->Name);
                    }
                }
            }
            else
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing", 
                    "Expected: <IDENTIFIER>, But Got: "+Curr->GetNodeType(), 
                    "Imports Need A Valid Path To Run",
                    "Add A Valid <MOD-PATH>",
                    Curr->pos.line, Curr->pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::MakeNode
                    <ErrorSpecialNode>(State, Res, Memory);
            }

            return nullptr;
        };

        // Finalize | Finalzia
        auto R = RunPath(Node->Path, true);
        if (R)
            return R;

        Res.ImportRefs.push_back(Node);
        return Node;
    }

    // Parse Library Defines | Parseia Definições de Bibliotecas.
    SpecialNode* ParseLibrary(
        Instruction& Inst, 
        ParseState& State, 
        ParseResult& Res,
        RunTimeData& Data, 
        ControlParser& CntrlParser,
        DeclarationParser& DeclParser,
        ExpressionParser& ExprParser,
        Arena& Memory
    )
    {
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        Token* LibName = Inst.Advance();
        if (!LibName)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <IDENTIFIER>", 
                "Expected <IDENTIFIER> After <LIBRARY>", 
                "Add A Valid Identifier After <LIBRARY>",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorSpecialNode>(State, Res, Memory);            
        }
        ParserUtils::UpdateStatePos(LibName, State);
        if (LibName->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <IDENTIFIER>", 
                "Expected <IDENTIFIER> After <LIBRARY>", 
                "Add A Valid Identifier After <LIBRARY>",
                LibName->pos.line, LibName->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorSpecialNode>(State, Res, Memory);
        }

        string Name = LibName->Lexeme(Data);
        if (Name.size() < 5 ||
            Name.substr(0, 2) != "__" ||
            Name.substr(Name.size() - 2) != "__" ||
            Name.substr(2, Name.size() - 4).find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != string::npos)
        {
            OrbitLog::SyntaxLog::SyntaxWarn(
                "Parsing",
                "Wreid Library Name",
                Name+" Dont Follow Name Convention: __NAME_OF_LIB__",
                "Add A Conventional Name",
                LibName->pos.line, LibName->pos.collumn
            );
        }

        if (Inst.Tokens.size() > 2)
        {
            OrbitLog::SyntaxLog::SyntaxWarn(
                "Parsing",
                "Extra Tokens After <LIB_NAME> Statement", 
                "Unexpected "+Inst.Tokens[3]->Lexeme(Data)+", Ignoring. .. ...",
                "Add ';' or <NEW-LINE> To Spearate",
                Inst.Tokens[3]->pos.line, Inst.Tokens[3]->pos.collumn
            );
        }

        LibraryNode* Node = ParserUtils::
            MakeNode<LibraryNode>(State, Res, Memory);
        Node->Name = Name;
        return Node;
    }
};

// ======= ENTRY-POINT | PONTO-DE-ENTRADA ====== //
// EntryPoint of Declaration Parsing Program
// Ponto-De-Entrada do Programa de Parsing de Declarações.
SpecialNode* SpecialParser::ParseSpecial(
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    ExpressionParser& ExprParser,
    DeclarationParser& DeclParser,
    ControlParser& CntrlParser,
    Arena& Memory
)
{
    Token* Entry = Inst.Tokens[0];
    string Lexeme = Entry->Lexeme(Data);
    switch (Entry->Type) {
     
        case TokenType::KEYWORD:

            if      (Lexeme == "_library")
                return SpecialUtils::ParseLibrary
                    (Inst, State, Res, Data, CntrlParser, DeclParser, ExprParser, Memory);
            else if (Lexeme == "_import")
                return SpecialUtils::ParseImports
                    (Inst, State, Res, Data, CntrlParser, DeclParser, ExprParser, Memory, false);
            else if (Lexeme == "_import_from")
                return SpecialUtils::ParseImports
                    (Inst, State, Res, Data, CntrlParser, DeclParser, ExprParser, Memory, true);
        default:
            return nullptr;
    }    
}