
// ========== PARSER =========== //
// Parse Token And Generate '_AST'(Abstract Syntax Tree).
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "parser.hpp" // HEADER FILE | CABEÇALHO
#include "AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "ParserModules/Control/control.hpp"
#include "ParserModules/Declaration/declaration.hpp"
#include "ParserModules/Expressions/expression.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <system_error>

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
        if (Tok->Type == TokenType::ENTRY_POINT)
            continue;
        else if (
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
                if (CurrInst.Tokens.size() != 0)
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

// Run AST | Percorre a AST
void DumbNode(ASTNode& Node, fstream& file, RunTimeData& Data, int Depth = 0)
{
    
    string Indent(Depth * 4, ' ');

    file << Indent << "NodeType: " << (int)Node.Type << '\n';
    file << Indent << "Pos(line/collumn): "
         << Node.pos.line << ';' << Node.pos.collumn << '\n';
    switch (Node.Type) {

        // PROGRAM | PROGRAMA.
        case NodeType::PROGRAM:
        {
            auto& N = static_cast<ProgramNode&>(Node);

            file << Indent << "Program\n";
            if (N.Node)
                DumbNode(*N.Node, file, Data, Depth + 1);

            break;
        }

        case NodeType::BODY:
        {
            auto& N = static_cast<BodyNode&>(Node);

            file << Indent << "Body\n";
            file << Indent << "Children: " << N.Data.size() << '\n';

            for (size_t i = 0; i < N.Data.size(); i++)
            {
                if (!N.Data[i])
                {
                    file << Indent
                        << "["
                        << i
                        << "] INVALID POINTER: nullptr\n";
                    continue;
                }

                file << Indent
                    << "["
                    << i
                    << "] Ptr: "
                    << N.Data[i]
                    << " | Type: "
                    << (int)N.Data[i]->Type
                    << '\n';
            }

            for (size_t i = 0; i < N.Data.size(); i++)
            {
                if (!N.Data[i])
                    continue;

                DumbNode(*N.Data[i], file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::VAR_DECL:
        {
            auto& N = static_cast<VarDeclNode&>(Node);

            file << Indent << "VarDecl\n";
            file << Indent << "Name: " << N.Name << '\n';
            file << Indent << "Mutable: " << (int)N.MutType << '\n';
            file << Indent << "InferType: " << (int)N.InferType << '\n';

            if (N.Val)
            {
                file << Indent << "Value:\n";
                DumbNode(*N.Val, file, Data, Depth + 1);
            }

            break;
        }
        
        case NodeType::FN_DECL:
        {
            auto& N = static_cast<FnDecl&>(Node);

            file << Indent << "Function\n";
            file << Indent << "Name: " << N.Name << '\n';
            file << Indent << "Have Return: " << (N.haveReturn ? "true" : "false") << '\n';

            if (!N.Params.empty())
            {
                file << Indent << "Parameters:\n";

                for (auto* Param : N.Params)
                {
                    if (Param)
                        DumbNode(*Param, file, Data, Depth + 1);
                }
            }

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::RETURN:
        {
            auto& N = static_cast<ReturnNode&>(Node);

            file << Indent << "Return\n";

            if (N.Value)
            {
                file << Indent << "Value:\n";
                DumbNode(*N.Value, file, Data, Depth + 1);
            }

            break;
        }

        // CONTROL | CONTROLE.
        case NodeType::IF_CONTROL:
        {
            auto& N = static_cast<IfNode&>(Node);

            file << Indent << "If\n";

            if (N.Cond)
            {
                file << Indent << "Condition:\n";
                DumbNode(*N.Cond, file, Data, Depth + 1);
            }

            if (N.IfBody)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.IfBody, file, Data, Depth + 1);
            }

            if (!N.ElifBodyStack.empty())
            {
                file << Indent << "Elifs: "
                     << N.ElifBodyStack.size() << '\n';

                for (auto* Elif : N.ElifBodyStack)
                {
                    if (Elif)
                        DumbNode(*Elif, file, Data, Depth + 1);
                }
            }

            if (N.ElseBody)
            {
                file << Indent << "Else:\n";
                DumbNode(*N.ElseBody, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::ELIF_CONTROL:
        {
            auto& N = static_cast<ElifNode&>(Node);

            file << Indent << "Elif\n";

            if (N.Cond)
            {
                file << Indent << "Condition:\n";
                DumbNode(*N.Cond, file, Data, Depth + 1);
            }

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::ELSE_CONTROL:
        {
            auto& N = static_cast<ElseNode&>(Node);

            file << Indent << "Else\n";

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }

            case NodeType::WHILE:
        {
            auto& N = static_cast<WhileNode&>(Node);

            file << Indent << "While\n";
            file << Indent << "LoopType: " << (int)N.Type << '\n';

            if (N.Cond)
            {
                file << Indent << "Condition:\n";
                DumbNode(*N.Cond, file, Data, Depth + 1);
            }

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::FOR_EACH:
        {
            auto& N = static_cast<ForEachNode&>(Node);

            file << Indent << "ForEach\n";
            file << Indent << "LoopType: " << (int)N.Type << '\n';

            if (N.Identifier)
            {
                file << Indent << "Identifier:\n";
                DumbNode(*N.Identifier, file, Data, Depth + 1);
            }

            if (N.Index)
            {
                file << Indent << "Index:\n";
                DumbNode(*N.Index, file, Data, Depth + 1);
            }

            if (N.Value)
            {
                file << Indent << "Value:\n";
                DumbNode(*N.Value, file, Data, Depth + 1);
            }

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::FOR_DEF:
        {
            auto& N = static_cast<ForDefNode&>(Node);

            file << Indent << "ForDef\n";
            file << Indent << "LoopType: " << (int)N.Type << '\n';

            if (N.Identifier)
            {
                file << Indent << "Identifier:\n";
                DumbNode(*N.Identifier, file, Data, Depth + 1);
            }

            if (N.Value)
            {
                file << Indent << "Value:\n";
                DumbNode(*N.Value, file, Data, Depth + 1);
            }

            if (N.Cond)
            {
                file << Indent << "Condition:\n";
                DumbNode(*N.Cond, file, Data, Depth + 1);
            }

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::FOR:
        {
            auto& N = static_cast<ForNode&>(Node);

            file << Indent << "For\n";
            file << Indent << "LoopType: " << (int)N.Type << '\n';

            if (N.Identifier)
            {
                file << Indent << "Identifier:\n";
                DumbNode(*N.Identifier, file, Data, Depth + 1);
            }

            if (N.End)
            {
                file << Indent << "Range:\n";
                DumbNode(*N.End, file, Data, Depth + 1);
            }

            if (N.Body)
            {
                file << Indent << "Body:\n";
                DumbNode(*N.Body, file, Data, Depth + 1);
            }

            break;
        }
        
        // EXPRESSIONS | EXPRESSOES.
        case NodeType::LITERAL:
        {
            auto& N = static_cast<LiteralNode&>(Node);

            file << Indent << "Literal\n";
            file << Indent << "Value: ";

            std::visit([&](auto&& Val)
            {
                using T = std::decay_t<decltype(Val)>;

                if constexpr (std::is_same_v<T, NoneLitVal>)
                    file << "None";
                else if constexpr (std::is_same_v<T, NullLitVal>)
                    file << "Null";
                else if constexpr (std::is_same_v<T, nullptr_t>)
                    file << "nullptr";
                else if constexpr (std::is_same_v<T, bool>)
                    file << (Val ? "true" : "false");
                else
                    file << Val;

            }, N.Value);

            file << '\n';
            break;
        }

        case NodeType::IDENTIFIER:
        {
            auto& N = static_cast<IdentifierNode&>(Node);

            file << Indent << "Identifier\n";
            file << Indent << "Name: " << N.Name << '\n';

            break;
        }

        case NodeType::UNARY:
        {
            auto& N = static_cast<UnaryNode&>(Node);

            file << Indent << "Unary\n";
            file << Indent << "Operator: " << (int)N.Operator << '\n';

            if (N.Operand)
                DumbNode(*N.Operand, file, Data, Depth + 1);

            break;
        }

        case NodeType::BINARY:
        {
            auto& N = static_cast<BinaryNode&>(Node);

            file << Indent << "Binary\n";
            file << Indent << "Operator: " << (int)N.Op << '\n';

            if (N.L)
            {
                file << Indent << "Left:\n";
                DumbNode(*N.L, file, Data, Depth + 1);
            }

            if (N.R)
            {
                file << Indent << "Right:\n";
                DumbNode(*N.R, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::ASSIGNMENT:
        {
            auto& N = static_cast<AssignmentNode&>(Node);

            file << Indent << "Assignment\n";
            file << Indent << "Operator: " << (N.Operator ? (int)N.Operator->Type : -1) << '\n';

            if (N.Left)
            {
                file << Indent << "Left:\n";
                DumbNode(*N.Left, file, Data, Depth + 1);
            }

            if (N.Right)
            {
                file << Indent << "Right:\n";
                DumbNode(*N.Right, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::MEMBER_ACCESS:
        {
            auto& N = static_cast<MemberAccessNode&>(Node);
            file << Indent << "MemberAccess\n";

            if (N.Object)
            {
                file << Indent << "Object:\n";
                DumbNode(*N.Object, file, Data, Depth + 1);
            }

            if (N.Member)
            {
                file << Indent << "Member:\n";
                DumbNode(*N.Member, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::INDEX_ACCESS:
        {
            auto& N = static_cast<IndexAccessNode&>(Node);

            file << Indent << "IndexAccess\n";

            if (N.Object)
            {
                file << Indent << "Object:\n";
                DumbNode(*N.Object, file, Data, Depth + 1);
            }

            if (N.Index)
            {
                file << Indent << "Index:\n";
                DumbNode(*N.Index, file, Data, Depth + 1);
            }

            break;
        }
        case NodeType::FN_CALL:
        {
            auto& N = static_cast<FunctionCall&>(Node);

            file << Indent << "FunctionCall\n";

            if (N.Callee)
            {
                file << Indent << "Callee:\n";
                DumbNode(*N.Callee, file, Data, Depth + 1);
            }

            file << Indent << "Arguments: " << N.Args.size() << '\n';

            for (size_t i = 0; i < N.Args.size(); i++)
            {
                file << Indent << "[" << i << "]\n";

                if (N.Args[i])
                    DumbNode(*N.Args[i], file, Data, Depth + 1);
            }

            break;
        }
 
        case NodeType::ARRAY_VALUE:
        {
            auto& N = static_cast<ArrayValue&>(Node);

            file << Indent << "Array\n";
            file << Indent << "Elements: " << N.Args.size() << '\n';

            for (size_t i = 0; i < N.Args.size(); i++)
            {
                file << Indent
                     << "[" << i << "]\n";

                if (N.Args[i])
                    DumbNode(*N.Args[i], file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::TABLE_VALUE:
        {
            auto& N = static_cast<TableValue&>(Node);

            file << Indent << "Table\n";
            file << Indent << "Entries: " << N.Args.size() << '\n';

            for (size_t i = 0; i < N.Args.size(); i++)
            {
                file << Indent
                     << "[" << i << "]\n";

                if (N.Args[i].Key)
                {
                    file << Indent << "Key:\n";
                    DumbNode(
                        *N.Args[i].Key,
                        file,
                        Data,
                        Depth + 1
                    );
                }

                if (N.Args[i].Value)
                {
                    file << Indent << "Value:\n";
                    DumbNode(
                        *N.Args[i].Value,
                        file,
                        Data,
                        Depth + 1
                    );
                }
            }

            break;
        }

        case NodeType::RANGE:
        {
            auto& N = static_cast<RangeNode&>(Node);

            file << Indent << "Range\n";

            if (N.Begin)
            {
                file << Indent << "Begin:\n";
                DumbNode(*N.Begin, file, Data, Depth + 1);
            }

            if (N.End)
            {
                file << Indent << "End:\n";
                DumbNode(*N.End, file, Data, Depth + 1);
            }

            break;
        }

        case NodeType::ERROR:
        {
            file << Indent << "ErrorNode\n";
            break;
        }

        default:
        {
            file << Indent << "Unknown Node\n";
            break;
        }
    }
}

// Generate Log of Parsing Step | Gera Log da Ettapa de Parsing
void GenerateParserLog(ParseResult& Res, RunTimeData& Data)
{
    if (Data.flags.debugMode)
        PrintIn("INITING TASK: Starting Generate ParserLog. .. ...");

    fstream file(Data.LogDir, std::ios::out | std::ios::app);
    if (!file.is_open())
    {
        OrbitLog::Error("parser.cpp", 
            "Cannot Open Log File, Why: "+
            string(std::strerror(errno))
            +" Path: "+Data.LogDir.string(),
            errno
        );
    }

    file << "\n// ========== PARSING ========== //\n\n";

    if (Res.AST)
        DumbNode(*Res.AST, file, Data);

    file << "\n// ========== ENDOF: PARSING ========== //\n";

    file.close();

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: Starting Generate ParserLog. .. ...");
}

// ======== ENTRY-POINT ======= //
// Entry-Point of Parse-Program | Ponto-de-Entrada no Programa de Parsing.
ParseResult Parser::InitP(LexResult& LRes, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: Parsing");
    ParseResult Res;
    ParseState State;
    
    // CREATE INIT POS | CRIA POS INICIAL
    State.Pos = NodePos{
        0, 
        0, 
        0,
        0, 
        0
    };

    // CREATE ENTRY-POINT | CRIA UM PONTO-DE-ENTRADA
    ProgramNode* Program = Memory.New<ProgramNode>(
        State.Pos,
        Memory
    );
    Program->Node->Type = BodyTypes::PROGRAM;
    Program->Node->Father = Program;
    Res.AST = Program;
    State.CurrBody = Program->Node;
    State.Bodys.push_back(Program->Node);

    // Prev Init Parser Modules | Inicia os Modulos do Parser Previamente.
    ExpressionParser ExprParser;
    DeclarationParser DeclParser;
    ControlParser CtrlParser;

    // Take Instructions and Run Then | Pega as Instrução e Percorre/Gera Nodes e Erros
    int I=0;
    InstVec Instructions = SeparateInstructions(LRes, Data);
    for (Instruction& Inst : Instructions) // PARSE ALL INSTRUCTIONS | PARSEIA TODAS AS INTRUÇOES
    {

        State.consumedInst=false;

        // PARSE TOKENS
        ASTNode* Node = nullptr; // CREATE BASE NODE

        Node = CtrlParser.ParseControl(
            Inst,
            State,
            Res,
            Data,
            DeclParser,
            ExprParser,
            Memory
        );
        
        if (Node == nullptr and !State.consumedInst)
            Node = DeclParser.ParseDeclaration(
                Inst,
                State,
                Res,
                Data,
                ExprParser,
                Memory
            );

        if (Node == nullptr and !State.consumedInst)
            Node =
                ExprParser.
                    ParseExpression(Inst, State, Res, Data, Memory);

        // INDENT SYSTEM
        if (I + 1 < Instructions.size())
        {
            size_t InstIndent = !Inst.Tokens.empty()
                ? Inst.Tokens[0]->pos.indent
                : Inst.Modifiers[0]->pos.indent;

            Instruction& NextInst = Instructions[I + 1];

            size_t NextIndent = !NextInst.Tokens.empty()
                ? NextInst.Tokens[0]->pos.indent
                : NextInst.Modifiers[0]->pos.indent;

            if (
                State.Bodys.size() > 1  and
                NextIndent < InstIndent and
                State.CurrBody->Type != BodyTypes::PROGRAM and
                Inst.Tokens.size() > 0 and
                Inst.Tokens[0]->Lexeme(Data) != "End"
            ) ParserUtils::PopBodyStack(State, Data);
        //
        }

        if (!State.consumedInst)
            ParserUtils::AddInst<ASTNode>(Node, State, Res, Memory);
        I++;
    }
    if (Data.flags.generateLog)
        GenerateParserLog(Res, Data);
    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: Parsing. .. ...");
    return Res;
}