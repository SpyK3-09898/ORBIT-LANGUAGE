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

// ======= PREVS | PREVIAS ====== //

void ParseBasicDeclaration(
    DeclarationNode& Node,
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    ExpressionParser& ExprParser,
    Arena& Memory
);

// ======= UTILS | UTILIDADES ======= //

// Declaration Parser Utils | Utilidades do Parser se Declarações.
namespace DeclUtils 
{

    // PARSE VAR DECLARATIONS | PARSEIA DECLARAÇOES DE VARIAVEIS.
    DeclarationNode* ParseVarDecl(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory,
        bool isList=false   
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
        ParseBasicDeclaration
        (*Decl, Inst, State, Res, Data, ExprParser, Memory); 
        
        for (Token* Arg : Inst.Modifiers)
        {
            string argLexeme = Arg->Lexeme(Data);

            if (argLexeme == "const")
                Decl->MutType = MutableTypes::CONST;
            else if (argLexeme == "export")
                Decl->export_decl=true;
        }

        string name = NameToken->Lexeme(Data);
        Decl->Name = name;
        Decl->InferType = LiteralTypes::MONO_STATE;

        Token* Next = Inst.Peek();

        if (Next != nullptr)
        {
            if (Next->Type == TokenType::EQUAL)
            {
                Token* E = Inst.Advance();
                ParserUtils::UpdateStatePos(E, State);

                Instruction NewInst{
                    Inst.Modifiers,
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
                Next = Inst.Peek();
                if (Next != nullptr && Next->Type == TokenType::EQUAL)
                {
                    Token* E = Inst.Advance();
                    ParserUtils::UpdateStatePos(E, State);

                    Instruction NewInst{
                        Inst.Modifiers,
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

            ValueNode->Value = NullLitVal{};
            Decl->Val = ValueNode;
        }
        if (Decl->Val->Type == NodeType::ARRAY_VALUE or Decl->Val->Type == NodeType::TABLE_VALUE)
        {
            if (!isList)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "<LITERAL_VALUE> Expected, But Got: "+Decl->Val->GetNodeType(),
                    "'var' ONLY Can be Used to Literals(Array, Tables)",
                    "Change 'var' to 'list'",
                    Decl->pos.line, Decl->pos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::MakeNode<ErrorDeclNode>
                    (State, Res, Memory);            
            }
        }
        else 
        {
            if (isList)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing",
                    "<ARRAY_VALUE> Expected, But Got: "+Decl->Val->GetNodeType(),
                    "'list' ONLY Can be Used to Lists(Array, Tables)",
                    "Change 'list' to 'var'",
                    Decl->pos.line, Decl->pos.collumn
                );
                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::MakeNode<ErrorDeclNode>
                    (State, Res, Memory);
            }
        }
        
        return Decl;
    }

    // PARSE FN DECLS | PARSEIA DECLARAÇÕES DE FUNÇÕES.
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

        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        string Name = E->Lexeme(Data);

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
        Cond.pop_back(); // )
        Cond.pop_back(); // :

        FnDecl* Decl = ParserUtils::MakeNode<FnDecl>(State, Res, Memory);
        ParseBasicDeclaration
        (*Decl, Inst, State, Res, Data, ExprParser, Memory); 
        
        if (!Cond.empty())
        {
            int level = 0;
            int start = 0;          // Start of Current Param | Início do Parâmetro Atual.
            int i = 0;

            for (Token* Tok : Cond)
            {
                switch (Tok->Type)
                {
                    case TokenType::LPARENT:
                    case TokenType::LBRACE:
                    case TokenType::LBRACKET:
                        level++;
                        break;

                    case TokenType::RPARENT:
                    case TokenType::RBRACE:
                    case TokenType::RBRACKET:
                        level--;
                        break;

                    case TokenType::COMMA:
                        if (level == 0)
                        {
                            // Take Only The Current Param Slice | Pega Apenas o Pedaço do Parâmetro Atual.
                            Instruction CondInst(
                                Inst.Modifiers,
                                vec<Token*>(Cond.begin() + start, Cond.begin() + i)
                            );

                            Decl->Params.push_back(
                                ExprParser.ParseExpression(CondInst, State, Res, Data, Memory)
                            );

                            start = i + 1;   // Next Param Starts After Comma | Próximo Parâmetro Começa Depois da Vírgula.
                        }
                        break;

                    default:
                        break;
                }
                i++;
            }

            // Last Param (After Last Comma or Single Param) | Último Parâmetro (Depois da Última Vírgula ou Parâmetro Único).
            if (start < (int)Cond.size())
            {
                Instruction CondInst(
                    Inst.Modifiers,
                    vec<Token*>(Cond.begin() + start, Cond.end())
                );

                Decl->Params.push_back(
                    ExprParser.ParseExpression(CondInst, State, Res, Data, Memory)
                );
            }
        }
        else
        {
            Decl->Params = {};
        }

        // CREATE BODY | CRIA O BODY.
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);
        Body->Type = BodyTypes::FUNCTION;
        Body->Father = Decl;

        // SET BODY | DEFINE O BODY;
        Decl->Body = Body;
        Decl->Name = Name;
        for (Token* Mod : Inst.Modifiers)
        {
            if (Mod->Lexeme(Data) == "export")
                Decl->export_decl=true;
        }

        // UPDATE STACK | ATUALIZA A PILHA
        State.consumedInst = true;
        ParserUtils::AddInst<FnDecl>(Decl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        // FINALIZE | FINALIZA.
        return Decl;
    }

    // PARSE NAMESPACE DECLARATIONS | PARSEIA DECLARAÇÃO DE NAMESPACES
    DeclarationNode* ParseNamespace(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory    
    )
    {
        // ADVANCE | AVANÇA
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);

        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State); // ERROR PREV | PREVENÇÃO DE ERROS:
        if (!E or E->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <NAMESPACE> Name", 
                "Namespace Need a Name to Be Created", 
                "Add A Valid Name After 'namespace'",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorDeclNode>(State, Res, Memory);
        }

        // CREATE NAMESPACCE
        NameSpaceDecl* Decl = ParserUtils::MakeNode<NameSpaceDecl>(State, Res, Memory);
        Decl->Name = E->Lexeme(Data);
        ParseBasicDeclaration
        (*Decl, Inst, State, Res, Data, ExprParser, Memory); 

        // ERROR PREVENTION | PREVENÇÃO DE ERROS
        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        if (!E or E->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected ':'", 
                "Expected ':' After <NAMESPACE> Name", 
                "Add A Valid Symbol After 'namespace'",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::
                MakeNode<ErrorDeclNode>(State, Res, Memory);
        }

        // CREATE BODY | CRIA O BODY.
        BodyNode* Body = ParserUtils::MakeNode<BodyNode>(State, Res, Memory);
        Body->Type = BodyTypes::NAMESPACE;
        Body->Father = Decl;

        // SET BODY | DEFINE O BODY;
        Decl->Body = Body;

        // UPDATE STACK | ATUALIZA A PILHA
        for (int i=0; i<State.Bodys.size(); i++)
            if (State.Bodys[i]->Type == BodyTypes::NAMESPACE)
                Decl->isNested=true;
        for (Token* Mod : Inst.Modifiers)
            if (Mod->Lexeme(Data) == "export")
                Decl->export_decl=true;

        State.consumedInst = true;
        ParserUtils::AddInst<NameSpaceDecl>(Decl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;        

        // FINALIZE | FINALIZA:
        return Decl;
    }

    // PARSE STRUCT/CLASS DECL | PARSEIA DECLARAÇÕES DE ESTRUTURAS E CLASSES.
    DeclarationNode* ParseStruct(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory    
    )
    {
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        Token* Name = Inst.Advance();
        ParserUtils::UpdateStatePos(Name, State);

        // Error Prev | Prevenção de Erros.
        if (!Name)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <IDENTIFIER> After 'struct' UDT", 
                "<STRUCT>'s Need A Name To Be Builded", 
                "Add A Valid Name to Struct",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }
        else if (Name->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <IDENTIFIER> After 'struct' UDT, But Got: "+Name->Lexeme(Data), 
                "<STRUCT>'s Need A Name To Be Builded", 
                "Add A Valid Name to Struct",
                Name->pos.line, Name->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }

        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        StructDeclNode* Decl = ParserUtils::MakeNode<StructDeclNode>
            (State, Res, Memory);
        ParseBasicDeclaration
        (*Decl, Inst, State, Res, Data, ExprParser, Memory); 
        Decl->isUDT=true;

        // Error Prevention | Prevenção de Erros.
        if (!E)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <COLON> After 'struct Name' UDT", 
                "<STRUCT>'s Need A ':' For Semantic", 
                "Add A Valid <COLON> After <IDENTIFIER>",
                Name->pos.line, Name->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }

        if (E->Type == TokenType::CNTXT_KW)
        {
            if (E->Lexeme(Data) == "from")
            {
                vec<Token*> Cond;
                bool FoundColon = false;

                while (true)
                {
                    Token* Tok = Inst.Advance();

                    if (!Tok)
                        break;

                    ParserUtils::UpdateStatePos(Tok, State);

                    if (Tok->Type == TokenType::COLON)
                    {
                        E = Tok;
                        FoundColon = true;
                        break;
                    }

                    Cond.push_back(Tok);
                }

                // ERROR PREV | PREVENÇÃO DE ERRO.
                if (Cond.size() == 0)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Expected <PATH> After 'from'",
                        "Expected <CONDITION> After If Statement",
                        "Add a Valid Cond",
                        Inst.Tokens[0]->pos.line,
                        Inst.Tokens[0]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
                }
                else if (!FoundColon)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Invalid <STRUCT>",
                        "Expected ':' After Struct Path",
                        "Finalize Struct Instruction",
                        Cond.back()->pos.line,
                        Cond.back()->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data); 
                    return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
                }

                Instruction ICond{Inst.Modifiers, Cond};
                Decl->Extend = ExprParser.ParseExpression(
                    ICond, 
                    State, 
                    Res, 
                    Data, 
                    Memory
                );
            }
            else
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing", 
                    "Expected <COLON> After 'struct Name' UDT", 
                    "<STRUCT>'s Need A ':' For Semantic", 
                    "Add A Valid <COLON> After <IDENTIFIER>",
                    E->pos.line, E->pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::MakeNode<ErrorDeclNode>
                    (State, Res, Memory);
            }
        }
        else if (E->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <COLON> After 'struct Name' UDT", 
                "<STRUCT>'s Need A ':' For Semantic", 
                "Add A Valid <COLON> After <IDENTIFIER>",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }

        // CREATE A NEW BODY | CRIA UM NOVO BODY
        BodyNode* Body = ParserUtils::
            MakeNode<BodyNode>(State, Res, Memory);

        // SET BODY
        Body->Father = Decl;
        Body->Type = BodyTypes::STRUCT;
        Decl->Body = Body;

        // UPDATE STACK | ATUALIZA O PILHA.
        State.consumedInst = true;
        ParserUtils::AddInst<StructDeclNode>(Decl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        return Decl;
    }
    DeclarationNode* ParseClass(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory    
    )
    {
        Token* E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        Token* Name = Inst.Advance();
        ParserUtils::UpdateStatePos(Name, State);

        // Error Prev | Prevenção de Erros.
        if (!Name)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <IDENTIFIER> After 'class' UDT", 
                "<CLASS>'s Need A Name To Be Builded", 
                "Add A Valid Name to Class",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }
        else if (Name->Type != TokenType::IDENTIFIER)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <IDENTIFIER> After 'class' UDT, But Got: "+Name->Lexeme(Data), 
                "<CLASS>'s Need A Name To Be Builded", 
                "Add A Valid Name to Class",
                Name->pos.line, Name->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }

        E = Inst.Advance();
        ParserUtils::UpdateStatePos(E, State);
        
        ClassDeclNode* Decl = ParserUtils::MakeNode<ClassDeclNode>
            (State, Res, Memory);
        ParseBasicDeclaration
        (*Decl, Inst, State, Res, Data, ExprParser, Memory); 
        Decl->isUDT=true;

        // Error Prevention | Prevenção de Erros.
        if (!E)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <COLON> After 'class Name' UDT", 
                "<CLASS>'s Need A ':' For Semantic", 
                "Add A Valid <COLON> After <IDENTIFIER>",
                Name->pos.line, Name->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }

        if (E->Type == TokenType::CNTXT_KW)
        {
            if (E->Lexeme(Data) == "from")
            {
                vec<Token*> Cond;
                bool FoundColon = false;

                while (true)
                {
                    Token* Tok = Inst.Advance();

                    if (!Tok)
                        break;

                    ParserUtils::UpdateStatePos(Tok, State);

                    if (Tok->Type == TokenType::COLON)
                    {
                        E = Tok;
                        FoundColon = true;
                        break;
                    }

                    Cond.push_back(Tok);
                }

                // ERROR PREV | PREVENÇÃO DE ERRO.
                if (Cond.size() == 0)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Expected <PATH> After 'from'",
                        "Expected <CONDITION> After If Statement",
                        "Add a Valid Cond",
                        Inst.Tokens[0]->pos.line,
                        Inst.Tokens[0]->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);
                    return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
                }
                else if (!FoundColon)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Parsing",
                        "Invalid <CLASS>",
                        "Expected ':' After Class Path",
                        "Finalize Class Instruction",
                        Cond.back()->pos.line,
                        Cond.back()->pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data); 
                    return ParserUtils::MakeNode<ErrorDeclNode>(State, Res, Memory);
                }

                Instruction ICond{Inst.Modifiers, Cond};
                Decl->Extend = ExprParser.ParseExpression(
                    ICond, 
                    State, 
                    Res, 
                    Data, 
                    Memory
                );
            }
            else
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Parsing", 
                    "Expected <COLON> After 'class Name' UDT", 
                    "<CLASS>'s Need A ':' For Semantic", 
                    "Add A Valid <COLON> After <IDENTIFIER>",
                    E->pos.line, E->pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                return ParserUtils::MakeNode<ErrorDeclNode>
                    (State, Res, Memory);
            }
        }
        else if (E->Type != TokenType::COLON)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Parsing", 
                "Expected <COLON> After 'class Name' UDT", 
                "<CLASS>'s Need A ':' For Semantic", 
                "Add A Valid <COLON> After <IDENTIFIER>",
                E->pos.line, E->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return ParserUtils::MakeNode<ErrorDeclNode>
                (State, Res, Memory);
        }

        // CREATE A NEW BODY | CRIA UM NOVO BODY
        BodyNode* Body = ParserUtils::
            MakeNode<BodyNode>(State, Res, Memory);

        // SET BODY
        Body->Father = Decl;
        Body->Type = BodyTypes::CLASS;
        Decl->Body = Body;

        // UPDATE STACK | ATUALIZA O PILHA.
        State.consumedInst = true;
        ParserUtils::AddInst<ClassDeclNode>(Decl, State, Res, Memory);
        ParserUtils::UpdateBodyStack(Body, State, Data);
        State.lastIndent = Body->pos.indent;

        return Decl;
    }

    // PARSE ALIAS DECL | PARSEIA DECLARÇÕES DE APELIDOS.
    DeclarationNode* ParseTypeDef(
        Token* Entry,
        Instruction& Inst,
        ParseState& State,
        ParseResult& Res,
        RunTimeData& Data,
        ExpressionParser& ExprParser,
        Arena& Memory    
    );
}

// ======= CORE | NUCLEO ======= //

// Basic Declararion Parsing | Declaração Basica de Parsing.
void ParseBasicDeclaration(
    DeclarationNode& Node,
    Instruction& Inst,
    ParseState& State,
    ParseResult& Res,
    RunTimeData& Data,
    ExpressionParser& ExprParser,
    Arena& Memory
)
{
    bool acessSeted=false;
    if (State.AcessStack.size() > 0)
    {
        Node.AcessType=State.AcessStack.back();
        acessSeted=true;
    }
    // Modifiers
    int i=0;
    for (Token* Mod : Inst.Modifiers)
    {
        string Lexeme = Mod->Lexeme(Data);
        if (Lexeme == "public")
        {
            // OverWriting | SobreScrita
            if (!acessSeted and (State.AcessStack.size() > 0 and State.AcessStack.back() != AcessTypes::PUBLIC))
            {
                OrbitLog::SyntaxLog::SyntaxWarn(
                    "Parsing",
                    "Acess OverWriting", 
                    "'public' has been overwrited by: <PRIVATE> In Upper Scope", 
                    "Move: "+Node.GetNodeType()+" To A Public Region",
                    Node.pos.line, Node.pos.collumn
                );
                continue;
            }
            Node.AcessType = AcessTypes::PUBLIC;
        }
        else if (Lexeme == "private")
        {
            // OverWriting | Sobrescrita
            if (!acessSeted and (State.AcessStack.size() > 0 and State.AcessStack.back() != AcessTypes::PRIVATE))
            {
                OrbitLog::SyntaxLog::SyntaxWarn(
                    "Parsing",
                    "Acess OverWriting", 
                    "'private' has been overwrited by: <PUBLIC> In Upper Scope", 
                    "Move: "+Node.GetNodeType()+" To A Public Region",
                    Node.pos.line, Node.pos.collumn
                );
                continue;
            }
            Node.AcessType = AcessTypes::PRIVATE;
        }
        Inst.Modifiers.erase(Inst.Modifiers.begin()+i, Inst.Modifiers.end());
        i++;
    }
}

// ======= ENTRY-POINT | PONTO-DE-ENTRADA ====== //
// EntryPoint of Declaration Parsing Program
// Ponto-De-Entrada do Programa de Parsing de Declarações.
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
            if (Lexeme == "list")
                return DeclUtils::ParseVarDecl(
                    Entry, 
                    Inst, 
                    State, 
                    Res,
                    Data, 
                    ExprParser,
                    Memory,
                    true
                );
            else if (Lexeme == "func" or Lexeme == "fn")
                return DeclUtils::ParseFnDecl(
                    Entry, 
                    Inst, 
                    State, 
                    Res, 
                    Data, 
                    ExprParser,
                    Memory
                );
            else if (Lexeme == "namespace")
                return DeclUtils::ParseNamespace(
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