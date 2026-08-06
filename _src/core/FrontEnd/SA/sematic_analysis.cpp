
// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o COdigo em Busca de Erros Semanticos.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "semantic_analysis.hpp" // HEADER FILE | CABEÇALHO.

#include "../parser/parser.hpp"
#include "../parser/AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <algorithm>
#include <string>

// ===== UTILS ===== //

namespace SAUtils {

    // Create A New Scope | Cria Um Novo Escopo.
    Scope* EntryScope(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
    {
        // Create Scope | Cria o Escopo
        Scope* S = Memory.New<Scope>();
        S->Owner = &Node;
        
        // Set Scope | Define o Escopo.
        State.CurrScope = S;
        State.ScopeStack.back()->Next = S;
        S->Parent = State.ScopeStack.back();
        State.ScopeStack.push_back(S);

        // Return.
        return S;
    }

    // Remove The Last Scope | Remove o Ultimo Escopo.
    Scope* LeaveScope(SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
    {

        // Take Last | Pega o Ultimo.
        Scope* S = State.CurrScope;
        if (S->Owner->Type == NodeType::PROGRAM)
        {
            OrbitLog::Warn("semantic_analysis.cpp", "Try To Close the <GLOBAL> Scope");
            return S;
        }
        S->Next = nullptr;

        // Updt Stack | Atualiza a Stack.
        State.ScopeStack.pop_back();
        State.CurrScope = State.ScopeStack.back();

        // Return.
        return State.CurrScope;
    }

    // Create A New Symbol | Cria um Novo Simbolo.
    Symbol* CreateSymbol(string Name, SymbolTypes Type, ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
    {
        // Create the Symbol | Cria o Simbolo.
        Symbol* S = Memory.New<Symbol>();
    
        // Define Sym | Define o Simbolo.
        S->Pos = Node.pos;
        S->DeclaredScope = State.CurrScope;
        S->Type = Type;

        // Finalize | Finaliza
        State.CurrScope->Symbols[Name] = S;
        Res.SymbolTable[Name] = S;
        return S;
    }

    // Get IValue Names | Pega o Nome de IValues
    string GetIValueName(ExpressionNode* Node)
    {
        if (Node->Type == NodeType::IDENTIFIER)
        {
            IdentifierNode* N = static_cast<IdentifierNode*>(Node);
            return N->Name;
        } else {
            return "UNKNOW";
        }
        return "UNKNOW";
    }
}

// ===== NODES ====== //

// --- PROGRAM --- //

// LookUp Any Node Type | Checa Qualquer Tipo de Nó.
void SemanticAnalizer::LookUpNode(
    ASTNode& Node, 
    SAState& State, 
    SAResult& Res,
    RunTimeData& Data,
    Arena& Memory    
)
{
    switch (Node.Type) {

        // PROGRAM
        case NodeType::PROGRAM:
            LookUpProgram(static_cast<ProgramNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::BODY:
            LookUpBody(static_cast<BodyNode&>(Node), State, Res, Data, Memory);
            break;

        // DECLARATIONS
        case NodeType::VAR_DECL:
            LookUpVarDecl(static_cast<VarDeclNode&>(Node), State, Res, Data, Memory);
            break;

        // EXPRESSIONS
        case NodeType::IDENTIFIER:
            LookUpIdentifier(static_cast<IdentifierNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::ASSIGNMENT:
            LookUpAssignment(static_cast<AssignmentNode&>(Node), State, Res, Data, Memory);
            break;

        default:
            break;
    }
}

// LookUp Entry Point | Checa o Ponto de Entrada.
void SemanticAnalizer::LookUpProgram(
    ProgramNode& Node, 
    SAState& State, 
    SAResult& Res,
    RunTimeData& Data,
    Arena& Memory
)
{
    // Create Global Scope | Cria o Escopo Global.
    Res.GlobalScope = Memory.New<Scope>();
    Res.GlobalScope->Owner = &Node; // Define State | Define o Estado:
    State.ScopeStack.push_back(Res.GlobalScope);
    State.CurrScope = Res.GlobalScope;

    // LookUp AST
    for (ASTNode* N : Node.Node->Data)
        LookUpNode(*N, State, Res, Data, Memory);
}

// --- EXPRESSION --- //

// Returns Expression Types | Retorna Tipo das Expressoes.
TypeInfo GetExpressionType(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data)
{
    // Cache
    if (Res.ExpressionTypes.contains(&Node))
        return Res.ExpressionTypes[&Node];

    // Main Switch | Switch Principal.
    TypeInfo TInfo;
    switch (Node.Type) {
        
        case NodeType::IDENTIFIER:
        {
            Symbol* Sym = 
                State.CurrScope->FindSym(static_cast<IdentifierNode&>(Node).Name);
            if (!Sym)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Used of Undeclared Identifier",
                    "Identifier: "+static_cast<IdentifierNode&>(Node).Name+" Dont Exists",
                    "Create the Identifier",
                    Node.pos.line, Node.pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                
                TInfo.Kind = TypeKind::UNK;
                break;
            }

            TInfo.Kind = Sym->TInfo->Kind;
            break;
        }
    
        case NodeType::LITERAL:
        {
            LiteralValue& Val = static_cast<LiteralNode&>(Node).Value;

            if (holds_alt<i64>(Val))
                TInfo.Kind = TypeKind::INT;
            else if (holds_alt<float>(Val))
                TInfo.Kind = TypeKind::FLOAT;
            else if (holds_alt<str_view>(Val))
                TInfo.Kind = TypeKind::STRING;
            else if (holds_alt<bool>(Val))
                TInfo.Kind = TypeKind::BOOL;
            else if (holds_alt<NoneLitVal>(Val))
                TInfo.Kind = TypeKind::NONE;
            else if (holds_alt<NullLitVal>(Val))
                TInfo.Kind = TypeKind::_NULL;
            else
                TInfo.Kind = TypeKind::UNK;

            break;
        }

        case NodeType::BINARY:
        {
            BinaryNode& B = static_cast<BinaryNode&>(Node);

            TypeInfo L = GetExpressionType(*B.L, State, Res, Data);
            TypeInfo R = GetExpressionType(*B.R, State,Res, Data);

            switch (B.Op)
            {
                case Operator::ADD:
                {
                    if (L.Kind == TypeKind::BOOL || R.Kind == TypeKind::BOOL)
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Binary Operation",
                            "Cannot use BOOL in arithmetic expressions",
                            "Remove the BOOL operand",
                            Node.pos.line, Node.pos.collumn
                        );
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        TInfo.Kind = TypeKind::UNK;
                        break;
                    }

                    if (L.Kind == TypeKind::NONE || L.Kind == TypeKind::_NULL ||
                        R.Kind == TypeKind::NONE || R.Kind == TypeKind::_NULL)
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Binary Operation",
                            "NONE and NULL cannot be used in arithmetic expressions",
                            "Compare them instead",
                            Node.pos.line, Node.pos.collumn
                        );
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        TInfo.Kind = TypeKind::UNK;
                        break;
                    }

                    if (L.Kind == TypeKind::STRING || R.Kind == TypeKind::STRING)
                        TInfo.Kind = TypeKind::STRING;
                    else if (L.Kind == TypeKind::FLOAT || R.Kind == TypeKind::FLOAT)
                        TInfo.Kind = TypeKind::FLOAT;
                    else if (L.Kind == TypeKind::INT && R.Kind == TypeKind::INT)
                        TInfo.Kind = TypeKind::INT;
                    else
                        TInfo.Kind = TypeKind::UNK;

                    break;
                }

                case Operator::SUB:
                case Operator::MUL:
                case Operator::DIV:
                case Operator::MOD:
                case Operator::POWER:
                {
                    if ((L.Kind == TypeKind::INT || L.Kind == TypeKind::FLOAT) &&
                        (R.Kind == TypeKind::INT || R.Kind == TypeKind::FLOAT))
                    {
                        if (L.Kind == TypeKind::FLOAT || R.Kind == TypeKind::FLOAT)
                            TInfo.Kind = TypeKind::FLOAT;
                        else
                            TInfo.Kind = TypeKind::INT;
                    }
                    else
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Binary Operation",
                            "Arithmetic operators require numeric operands",
                            "Use INT or FLOAT values",
                            Node.pos.line, Node.pos.collumn
                        );
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        TInfo.Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::EQUAL:
                case Operator::NOT_EQUAL:
                {
                    if ((L.Kind == TypeKind::NONE && R.Kind == TypeKind::_NULL) ||
                        (L.Kind == TypeKind::_NULL && R.Kind == TypeKind::NONE))
                    {
                        TInfo.Kind = TypeKind::BOOL;
                    }
                    else if (L.Kind == R.Kind)
                    {
                        TInfo.Kind = TypeKind::BOOL;
                    }
                    else if ((L.Kind == TypeKind::INT && R.Kind == TypeKind::FLOAT) ||
                             (L.Kind == TypeKind::FLOAT && R.Kind == TypeKind::INT))
                    {
                        TInfo.Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Comparison",
                            "Cannot compare these types",
                            "Compare compatible types",
                            Node.pos.line, Node.pos.collumn
                        );
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        TInfo.Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::LESS:
                case Operator::GREATER:
                case Operator::LESS_EQUAL:
                case Operator::GREATER_EQUAL:
                {
                    if ((L.Kind == TypeKind::INT || L.Kind == TypeKind::FLOAT) &&
                        (R.Kind == TypeKind::INT || R.Kind == TypeKind::FLOAT))
                    {
                        TInfo.Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Comparison",
                            "Comparison operators require numeric operands",
                            "Use INT or FLOAT values",
                            Node.pos.line, Node.pos.collumn
                        );
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        TInfo.Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::AND:
                case Operator::OR:
                {
                    if (L.Kind == TypeKind::BOOL && R.Kind == TypeKind::BOOL)
                    {
                        TInfo.Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Logical Operation",
                            "Logical operators require BOOL operands",
                            "Use BOOL values",
                            Node.pos.line, Node.pos.collumn
                        );
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        TInfo.Kind = TypeKind::UNK;
                    }

                    break;
                }

                default:
                    TInfo.Kind = TypeKind::UNK;
                    break;
            }

            break;
        }

        case NodeType::UNARY:
        {
            UnaryNode& U = static_cast<UnaryNode&>(Node);
            
            switch (U.Operator) {
            
                case Operator::NOT:
                    
                    TInfo = GetExpressionType(*U.Operand, State, Res, Data);
                    if (TInfo.Kind != TypeKind::BOOL) //!10(Ex) Error | !10(Ex) Erro:
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid <UNARY>",
                            "<NOT> Operator ONLY Accept Bool Expressions",
                            "Convert to Bool if Possible",
                            U.pos.line, U.pos.collumn
                        );
                        TInfo.Kind = TypeKind::UNK;
                        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                        return TInfo;
                    }
                default:
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Unknow <UNARY_OPERATOR>",
                        "Operator is not supported to Unary Operations",
                        "Add a Valid Operator Before LITERAL",
                        U.pos.line, U.pos.collumn
                    );
                    if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                    TInfo.Kind = TypeKind::UNK;
                    return TInfo;
            }

            break;
        }

        default: {};
    }

    // Finalize | Finaliza.
    Res.ExpressionTypes[&Node] = TInfo; // Set Cache | Define Cache.
    return TInfo;
}

// Get Kindof Literal Values Type | Pega o Kind do Tipo Literal.
TypeKind GetLiteralTypeKind(LiteralTypes Type)
{
    switch (Type) {
    
        case LiteralTypes::INT: return TypeKind::INT;
        case LiteralTypes::FLOAT: return TypeKind::FLOAT;
        case LiteralTypes::BOOL: return TypeKind::BOOL;
        case LiteralTypes::STRING: return TypeKind::STRING;
        case LiteralTypes::NONE: return TypeKind::NONE;
        case LiteralTypes::_NULL: return TypeKind::_NULL;
        case LiteralTypes::MONO_STATE: return TypeKind::MONO_STATE;
        default: return TypeKind::UNK;
    }
}

// Get String of Kind | Pega A <STRING> Com Base No Tipo.
string GetStringOfKind(TypeKind K)
{
    switch (K) {
    
        case TypeKind::INT: return "<INT>";
        case TypeKind::FLOAT: return "<FLOAT>";
        case TypeKind::BOOL: return "<BOOL>";
        case TypeKind::STRING: return "<STRING>";
        case TypeKind::NONE: return "<NONE>";
        case TypeKind::_NULL: return "<_NULL>";
        case TypeKind::MONO_STATE: return "<UND>";
        default: return "<UNK>";
    }
}

// LookUp Body Nodes | Checa os BodyNodes.
void SemanticAnalizer::LookUpBody(
    BodyNode& Node, 
    SAState& State, 
    SAResult& Res,
    RunTimeData& Data,
    Arena& Memory    
)
{
    SAUtils::EntryScope(Node,State, Res, Data, Memory); 
    for (ASTNode* Node : Node.Data)
        LookUpNode(*Node, State, Res, Data, Memory);
    SAUtils::LeaveScope(State, Res, Data, Memory);
}

// LookUp Identifiers | Checa Identificadores.
void SemanticAnalizer::LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (!State.CurrScope->FindSym(Node.Name)) 
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Unknown <IDENTIFIER>",
            "Used of Undeclared <IDENTIFIER>: "+Node.Name,
            "Create Or Change Identifier Name",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
}

// LookUp I-Values | Checa os I-Values.
void SemanticAnalizer::LookUpIValue(ExpressionNode* Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory
)
{
    if (Node->Type == NodeType::IDENTIFIER)
    {
        IdentifierNode* N = static_cast<IdentifierNode*>(Node);
        LookUpIdentifier(*N, State, Res, Data, Memory);
    } else {

        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <I-VALUE>",
            "Expected <I-VALUE>, But Got: "+Node->GetNodeType(),
            "~",
            Node->pos.line, Node->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
}

// LooKupAssignments | Checa As Atribuições. 
void SemanticAnalizer::LookUpAssignment(AssignmentNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Data
    LookUpIValue(Node.Left, State, Res, Data, Memory);
    string n = SAUtils::GetIValueName(Node.Left);
    
    // Error Prev | Prevenção de Erros.
    if (!State.CurrScope->FindSym(n))
        return; // Data:
    Symbol* Sym = State.CurrScope->FindSym(n);
    Sym->inited = true;

    // Error Prev | Prevenção de Erros
    if (Sym->Mut == MutableTypes::CONST)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Try to Assign a CONST <I-VALUE>",
            string(Sym->Name)+" Is Declarated as <CONST>",
            "~",
            Sym->Pos.line, Sym->Pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
    if (
        Sym->TInfo->Kind != TypeKind::MONO_STATE && 
        Sym->TInfo->Kind != GetExpressionType
        (*Node.Right, State, Res, Data).Kind
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Infer Type is Diff of Expression Type",
            GetStringOfKind
            (GetExpressionType
            (*Node.Right, State, Res, Data).Kind)+" Is Diff of: "+GetStringOfKind(Sym->TInfo->Kind),
            "Change Type or Convert",
            Node.Right->pos.line, Node.Right->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
    
    // Finalize
    Sym->write_count++;
}

// --- DECLARATION --- //

// LookUp Var Declaration Nodes | Checa os varDecl.
void SemanticAnalizer::LookUpVarDecl(
    VarDeclNode& Node, 
    SAState& State, 
    SAResult& Res, 
    RunTimeData& Data, 
    Arena& Memory
)
{
    // Error Prev | Prevenção De Erros.
    if (State.CurrScope->FindSymLocal(Node.Name))
    {
        char c = Node.Name.back();

        if (std::isdigit(c))
        {
            int last = c - '0';

            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Try to Create A New Var Whit A Invalid Name",
                "Identifier: "+Node.Name+" Already Exists, Shadowing NOT Is Allowed In Same Scope",
                "Add a Diff Name, Ex: "+Node.Name.substr(0, Node.Name.length() - 1)+std::to_string(last + 1),
                Node.pos.line, Node.pos.collumn
            );
        }
        else
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Try to Create A New Var Whit A Invalid Name",
                "Identifier: "+Node.Name+" Already Exists, Shadowing NOT Is Allowed In Same Scope",
                "Add a Diff Name, Ex: "+Node.Name+"2",
                Node.pos.line, Node.pos.collumn
            );
        }
    }

    // Take Data | Pega os Dados.
    TypeKind InfType = GetLiteralTypeKind(Node.InferType);
    Symbol* Sym = // Take Symbol | Pega o Simbolo.
        SAUtils::CreateSymbol(Node.Name, SymbolTypes::VAR, Node, State, Res, Data, Memory);
    
    // Error Prev | Prevenção de Erros.
    if (InfType != TypeKind::MONO_STATE and GetExpressionType(*Node.Val, State, Res, Data).Kind != InfType)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            GetStringOfKind(InfType)+" Expected, But Got: "+GetStringOfKind(GetExpressionType(*Node.Val, State, Res, Data).Kind),
            "Infered Type Does Not Match Whit Recived Type",
            "Change To A Valid Type or Convert",
            Node.Val->pos.line, Node.Val->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
    }

    LookUpNode(*Node.Val, State, Res, Data, Memory);
}

// --- CONTROL --- //

void SemanticAnalizer::LookUpIf(
    IfNode& Node, 
    SAState& State, 
    SAResult& Res, 
    RunTimeData& Data, 
    Arena& Memory
)
{
    if (
        GetExpressionType(*Node.Cond, State, Res, Data).Kind 
        != 
        TypeKind::BOOL
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <BOOLEAN> Condition",
            "Ifs need a <BOOLEAN> Condition, But Got: "+GetStringOfKind(GetExpressionType(*Node.Cond, State, Res, Data).Kind ),
            "Convert to <BOOLEAN> If Possible",
            Node.Cond->pos.line, Node.Cond->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
    }
    LookUpBody(*Node.IfBody, State, Res, Data, Memory);
    LookUpElse(*Node.ElseBody, State, Res, Data, Memory);
    for (ElifNode* Elif : Node.ElifBodyStack)
        LookUpElif(*Elif, State, Res, Data, Memory);
}

// ======= ENTRY-POINT | PONTO-DE-ENTRADA ======= //
SAResult SemanticAnalizer::InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: SemanticAnalysis");

    SAResult Res;
    SAState State;
    
    LookUpNode(*PRes.AST, State, Res, Data, Memory);

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: SemanticAnalysis. .. ...");
    
    return Res;
}