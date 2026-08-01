
// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o COdigo em Busca de Erros Semanticos.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "semantic_analysis.hpp"

#include "../parser/parser.hpp"
#include "../parser/AST/AST.hpp"

#include "../lexer/lexer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <type_traits>

// ======= CORE ======= //

// ===== UTILS ===== //

TypeInfo* SemanticAnalizer::GetExpressionType(ExpressionNode* Node)
{
    if (!Node)
        return nullptr;

    switch (Node->Type)
    {
        case NodeType::LITERAL:
        {
            LiteralNode* Lit = static_cast<LiteralNode*>(Node);

            TypeInfo* Type = M_Memory->New<TypeInfo>();

            if (holds_alternative<i64>(Lit->Value))
                Type->Kind = TypeKind::INT;

            else if (holds_alternative<float>(Lit->Value))
                Type->Kind = TypeKind::FLOAT;

            else if (holds_alternative<str_view>(Lit->Value))
                Type->Kind = TypeKind::STRING;

            else if (holds_alternative<bool>(Lit->Value))
                Type->Kind = TypeKind::BOOL;

            else if (holds_alternative<NoneLitVal>(Lit->Value))
                Type->Kind = TypeKind::NONE;

            else if (holds_alternative<NullLitVal>(Lit->Value))
                Type->Kind = TypeKind::NULLVAL;

            return Type;
        }


        case NodeType::IDENTIFIER:
        {
            IdentifierNode* Id = static_cast<IdentifierNode*>(Node);

            Symbol* Sym = SAState.CurrScope->LookUp(Id->Name);

            if (!Sym)
                return nullptr;

            return Sym->Type;
        }


        case NodeType::ARRAY_VALUE:
        {
            TypeInfo* Type = M_Memory->New<TypeInfo>();

            Type->Kind = TypeKind::ARRAY;
            Type->ElementType = nullptr;
            
            return Type;
        }


        case NodeType::TABLE_VALUE:
        {
            TypeInfo* Type = M_Memory->New<TypeInfo>();

            Type->Kind = TypeKind::TABLE;
            Type->ElementType = nullptr;

            return Type;
        }


        case NodeType::MEMBER_ACCESS:
        {
            MemberAccessNode* Access = static_cast<MemberAccessNode*>(Node);

            TypeInfo* Object = GetExpressionType(Access->Object);

            if (!Object)
                return nullptr;

            if (Object->Kind != TypeKind::STRUCT &&
            Object->Kind != TypeKind::CLASS)
                return nullptr;


            IdentifierNode* Member =
                static_cast<IdentifierNode*>(Access->Member);


            auto It = Object->Members.find(Member->Name);

            if (It == Object->Members.end())
                return nullptr;


            return It->second;
        }

        case NodeType::INDEX_ACCESS:
        {
            IndexAccessNode* Access = static_cast<IndexAccessNode*>(Node);
            TypeInfo* ObjType = GetExpressionType(Access->Object);
            if (!ObjType) return nullptr;

            if (ObjType->Kind == TypeKind::ARRAY)
                return ObjType->ElementType;
            if (ObjType->Kind == TypeKind::TABLE)
                return ObjType->ValueType;

            return nullptr;
        }

        default:
            return nullptr;
    }
}

TypeInfo* SemanticAnalizer::ConvertLiteralTypeInfo(LiteralTypes Type)
{
    TypeInfo* Info = M_Memory->New<TypeInfo>();

    switch (Type)
    {
        case LiteralTypes::INT:
            Info->Kind = TypeKind::INT;
            break;

        case LiteralTypes::FLOAT:
            Info->Kind = TypeKind::FLOAT;
            break;

        case LiteralTypes::STRING:
            Info->Kind = TypeKind::STRING;
            break;

        case LiteralTypes::BOOL:
            Info->Kind = TypeKind::BOOL;
            break;

        case LiteralTypes::NONE:
            Info->Kind = TypeKind::NONE;
            break;

        case LiteralTypes::_NULL:
            Info->Kind = TypeKind::NULLVAL;
            break;

        case LiteralTypes::ARRAY_INT:
            Info->Kind = TypeKind::ARRAY;
            Info->ElementType = M_Memory->New<TypeInfo>();
            Info->ElementType->Kind = TypeKind::INT;
            break;

        case LiteralTypes::ARRAY_FLOAT:
            Info->Kind = TypeKind::ARRAY;
            Info->ElementType = M_Memory->New<TypeInfo>();
            Info->ElementType->Kind = TypeKind::FLOAT;
            break;

        case LiteralTypes::ARRAY_STRING:
            Info->Kind = TypeKind::ARRAY;
            Info->ElementType = M_Memory->New<TypeInfo>();
            Info->ElementType->Kind = TypeKind::STRING;
            break;

        case LiteralTypes::ARRAY_BOOL:
            Info->Kind = TypeKind::ARRAY;
            Info->ElementType = M_Memory->New<TypeInfo>();
            Info->ElementType->Kind = TypeKind::BOOL;
            break;

        case LiteralTypes::ARRAY_NONE:
            Info->Kind = TypeKind::ARRAY;
            Info->ElementType = M_Memory->New<TypeInfo>();
            Info->ElementType->Kind = TypeKind::NONE;
            break;

        case LiteralTypes::ARRAY_NULL:
            Info->Kind = TypeKind::ARRAY;
            Info->ElementType = M_Memory->New<TypeInfo>();
            Info->ElementType->Kind = TypeKind::NULLVAL;
            break;

        case LiteralTypes::TABLE_INT:
            Info->Kind = TypeKind::TABLE;
            Info->ValueType = M_Memory->New<TypeInfo>();
            Info->ValueType->Kind = TypeKind::INT;
            break;

        case LiteralTypes::TABLE_FLOAT:
            Info->Kind = TypeKind::TABLE;
            Info->ValueType = M_Memory->New<TypeInfo>();
            Info->ValueType->Kind = TypeKind::FLOAT;
            break;

        case LiteralTypes::TABLE_STRING:
            Info->Kind = TypeKind::TABLE;
            Info->ValueType = M_Memory->New<TypeInfo>();
            Info->ValueType->Kind = TypeKind::STRING;
            break;

        case LiteralTypes::TABLE_BOOL:
            Info->Kind = TypeKind::TABLE;
            Info->ValueType = M_Memory->New<TypeInfo>();
            Info->ValueType->Kind = TypeKind::BOOL;
            break;

        case LiteralTypes::TABLE_NONE:
            Info->Kind = TypeKind::TABLE;
            Info->ValueType = M_Memory->New<TypeInfo>();
            Info->ValueType->Kind = TypeKind::NONE;
            break;

        case LiteralTypes::TABLE_NULL:
            Info->Kind = TypeKind::TABLE;
            Info->ValueType = M_Memory->New<TypeInfo>();
            Info->ValueType->Kind = TypeKind::NULLVAL;
            break;

        default:
            return nullptr;
    }

    return Info;
}

// Compare Two Types | Compara Dois Tipos.
bool SemanticAnalizer::TypesEqual(TypeInfo* A, TypeInfo* B)
{
    // ERROR PREV | PREVENÇÃO DE ERROS
    if (!A || !B)
        return false;

    // BASIC CHECK | CHECAGEM BASICA
    if (A->Kind != B->Kind)
        return false;

// COMPOUND TYPES | TIPOS COMPOSTOS
    switch (A->Kind)
    {
        case TypeKind::ARRAY:
            // nullptr = "qualquer tipo de elemento"
            if (!A->ElementType || !B->ElementType)
                return true;
            return TypesEqual(A->ElementType, B->ElementType);

        case TypeKind::TABLE:
            // nullptr = "qualquer tipo de valor"
            if (!A->ValueType || !B->ValueType)
                return true;
            return TypesEqual(A->ValueType, B->ValueType);

        // SIMPLE TYPES | TIPOS SIMPLES
        default:
            return true;
    }
}

string SemanticAnalizer::TypeToString(TypeKind Type)
{
    switch (Type)
    {
        case TypeKind::UNKNOWN: return "<UNKNOWN>";

        case TypeKind::INT:     return "<INT>";
        case TypeKind::FLOAT:   return "<FLOAT>";
        case TypeKind::STRING:  return "<STRING>";
        case TypeKind::BOOL:    return "<BOOL>";

        case TypeKind::NONE:    return "<NONE>";
        case TypeKind::NULLVAL: return "<NULL>";

        case TypeKind::ARRAY:   return "<ARRAY>";
        case TypeKind::TABLE:   return "<TABLE>";

        case TypeKind::STRUCT:  return "<STRUCT>";
        case TypeKind::CLASS:   return "<CLASS>";

        default:
            return "<UNKNOWN>";
    }
}

// ===== SCOPES ===== //

// Analize the Node | Analisa o Nó.
void SemanticAnalizer::Visit(ASTNode* Node)
{
    if (Node == nullptr)
        return;

    switch (Node->Type)
    {
        // PROGRAM
        case NodeType::PROGRAM:
            VisitProgram(static_cast<ProgramNode*>(Node));
            break;

        case NodeType::BODY:
            VisitBody(static_cast<BodyNode*>(Node));
            break;

        // DECLARATIONS
        case NodeType::VAR_DECL:
            VisitVarDecl(static_cast<VarDeclNode*>(Node));
            break;

        // case NodeType::FN_DECL:
        //     VisitFnDecl(static_cast<FnDecl*>(Node));
        //     break;

        // CONTROL
        // case NodeType::IF_CONTROL:
        //     VisitIf(static_cast<IfNode*>(Node));
        //     break;

        // case NodeType::ELSE_CONTROL:
        //     VisitElse(static_cast<ElseNode*>(Node));
        //     break;

        // case NodeType::ELIF_CONTROL:
        //     VisitElif(static_cast<ElifNode*>(Node));
        //     break;

        // case NodeType::WHILE:
        //     VisitWhile(static_cast<WhileNode*>(Node));
        //     break;

        // case NodeType::FOR:
        //     VisitFor(static_cast<ForNode*>(Node));
        //     break;

        // case NodeType::FOR_EACH:
        //     VisitForEach(static_cast<ForEachNode*>(Node));
        //     break;

        // case NodeType::FOR_DEF:
        //     VisitForDef(static_cast<ForDefNode*>(Node));
        //     break;

        // case NodeType::RETURN:
        //     VisitReturn(static_cast<ReturnNode*>(Node));
        //     break;

        // EXPRESSIONS
        case NodeType::LITERAL:
            VisitLiteral(static_cast<LiteralNode*>(Node));
            break;

        case NodeType::IDENTIFIER:
            VisitIdentifier(static_cast<IdentifierNode*>(Node));
            break;

        // case NodeType::UNARY:
        //     VisitUnary(static_cast<UnaryNode*>(Node));
        //     break;

        // case NodeType::BINARY:
        //     VisitBinary(static_cast<BinaryNode*>(Node));
        //     break;

        case NodeType::ASSIGNMENT:
            VisitAssignment(static_cast<AssignmentNode*>(Node));
            break;

        // case NodeType::MEMBER_ACCESS:
        //     VisitMemberAccess(static_cast<MemberAccessNode*>(Node));
        //     break;

        // case NodeType::INDEX_ACCESS:
        //     VisitIndexAccess(static_cast<IndexAccessNode*>(Node));
        //     break;

        // case NodeType::FN_CALL:
        //     VisitFunctionCall(static_cast<FunctionCall*>(Node));
        //     break;

        // case NodeType::TABLE_VALUE:
        //     VisitTable(static_cast<TableValue*>(Node));
        //     break;

        // case NodeType::ARRAY_VALUE:
        //     VisitArray(static_cast<ArrayValue*>(Node));
        //     break;

        // case NodeType::RANGE:
        //     VisitRange(static_cast<RangeNode*>(Node));
        //     break;

        // ERRORS
        // case NodeType::ERROR:
        //     break;

        default: {};
    }
}

// Entry in a New Scope | Entra em Um Novo Escopo.
void SemanticAnalizer::EnterScope(BodyTypes Kind)
{
    Scope* New = M_Memory->New<Scope>();

    New->Parent = SAState.CurrScope;
    New->Kind   = Kind;

    SAState.CurrScope = New;
    SAState.currScopeLvl++;
}

// Back to a Father Scope | Volta Para o Escopo pai.
void SemanticAnalizer::LeaveScope(BodyTypes Kind)
{
    Scope* Curr = SAState.CurrScope;
    if (!Curr)
        OrbitLog::Error("semantic_analysis.cpp", "Try to Back to a Null Scope", true);
    SAState.CurrScope = SAState.CurrScope->Parent;
    SAState.currScopeLvl--;
}

// ===== VISITS ===== //

// --- PROGRAM --- //

// PROGRAM NODE | NO DE PROGRAMAS 
void SemanticAnalizer::VisitProgram(ProgramNode* Node)
{
    for (ASTNode* Child : Node->Node->Data)
    {
        SemanticAnalizer::Visit(Child);
    }
}

// BODY NODE | NO DE BODYS
void SemanticAnalizer::VisitBody(BodyNode* Node)
{
    for(ASTNode* Child : Node->Data)
    {
        Visit(Child);
    }
}

// --- DECLARATION --- //

// Visit Var Declarations | Visita Criações de Variaveis.
void SemanticAnalizer::VisitVarDecl(VarDeclNode* Node)
{
    // Error Prev | Prevenção de Erros
    if(SAState.CurrScope->LookUp(Node->Name))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "<IDENTIFIER> Already Exists",
            "Ident: "+Node->Name+" Already Exists, First Declareated Here: "
            +std::to_string(SAState.CurrScope->LookUp(Node->Name)->Pos.line)
            +":"+std::to_string(SAState.CurrScope->LookUp(Node->Name)->Pos.collumn),
            "Add a Diferent Name, Ex: "+Node->Name+"2",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return ;
    }

    // Validate Expression and Create Symbol | Valida Expressoes e Cria o Simbolo.
    bool inited=false;
    if (Node->Val)
        { VisitExpression(Node->Val); inited=true; }
    Symbol* S = M_Memory->New<Symbol>();
    S->initialized = inited;

    // Set Symbol | Define o Simbolo.
    S->Mut  = Node->MutType;
    S->Pos  = Node->pos;
    S->name = Node->Name;


    TypeInfo* Expected =  ConvertLiteralTypeInfo(Node->InferType);
    TypeInfo* Actual   =  GetExpressionType(Node->Val);  
    if (Node->InferType != LiteralTypes::MONO_STATE && !TypesEqual(Expected, Actual))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
        string(TypeToString(Expected ? Expected->Kind : TypeKind::UNKNOWN)) +
            "    Expected, But Got: " +
            string(TypeToString(Actual ? Actual->Kind : TypeKind::UNKNOWN)),
           "<INFER_TYPE> is DIFFERENT to typeof Value",
            "Add a Valid Type or Convert",
            S->Pos.line, S->Pos.collumn
        );

        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);

        return;
    }

    S->Type = Actual;

    // Finalize | Finaliza.
    SAState.CurrScope->Symbols[S->name] = S;
}

// Visit Identifiers Using | Visita Uso de Indentificadores.
void SemanticAnalizer::VisitIdentifier(IdentifierNode* Node)
{
    Symbol* Sym = SAState.CurrScope->LookUp(Node->Name);
    if (!Sym)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Use a Undefined <IDENTIFIER>",
            "Ident: "+Node->Name+" Non Exists",
            "Create or Change the Symbol",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    Sym->read_count++;
}

// Visit Member Acess Using | Visita Acesso de membros.
void SemanticAnalizer::VisitMemberAccess(MemberAccessNode* Node, bool isBase)
{
    // ERROR PREV | PREVENÇÃO DE ERROS
    if (!IsIValue(Node->Member))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Acess Using A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Member->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    if (!IsIValue(Node->Object))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Acess A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Member->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    // SET | DEFINE
    if (Node->Member->Type == NodeType::MEMBER_ACCESS)
    {
        MemberAccessNode* Mb = static_cast<MemberAccessNode*>(Node->Member);
        VisitMemberAccess(Mb, false);
    }
    if (isBase)
    {
        IdentifierNode* Id = static_cast<IdentifierNode*>(Node->Object);
        VisitIdentifier(Id);
    }
}

// Visit Index Acess Using | Visita o Acesso Per Indice
void SemanticAnalizer::VisitIndexAccess(IndexAccessNode* Node, bool isAssign)
{
    // ERROR PREV | PREVENÇÃO DE ERROS.
    if(!Node) return;
    if (
        !IsIValue(Node->Object) &&
        Node->Object->Type != NodeType::ARRAY_VALUE &&
        Node->Object->Type != NodeType::TABLE_VALUE
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Assign A Non <I_VALUE>",
            "Expected <IVALUE> But Got: "+Node->Object->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    
    // Visit Childs | Visita os Filhos.
    Visit(Node->Object);
    Visit(Node->Index);

    // Take Typeof Objects and Index | Pega o Tipo Do Objeto E Indice. 
    TypeInfo* ObjType = GetExpressionType(Node->Object);
    TypeInfo* IdxType = GetExpressionType(Node->Index);

    // ERROR PREV | PREVENÇÃO DE ERROS.
    if (!ObjType)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Cannot determine type of indexed object",
            "Object type is unknown",
            "Initialize the variable first",
            Node->pos.line, Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }
    if (ObjType->Kind != TypeKind::ARRAY && ObjType->Kind != TypeKind::TABLE)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Index access on non-indexable type",
            "Expected ARRAY or TABLE",
            "Use an array or table variable",
            Node->pos.line, Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }

    // RULES | REGRAS.
    if(ObjType->Kind == TypeKind::ARRAY) // ARRAY | LISTAS. 
    {
        if(!IdxType || IdxType->Kind != TypeKind::INT)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Array index must be integer",
                "Expected INT index for ARRAY",
                "Use a numeric index (ex: arr[0])",
                Node->pos.line, Node->pos.collumn
            );
            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);
            return;
        }   
        if (isAssign)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Cannot assign to array index (append not allowed)",
                "Arrays only support get by index, not set/append via []",
                "Use a TABLE if you need dynamic keys",
                Node->pos.line, Node->pos.collumn
            );
            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);
            return;
        }     
    }
    else // TABLE | TABELAS.
    {
        if (!IdxType ||
            (IdxType->Kind != TypeKind::STRING && IdxType->Kind != TypeKind::INT))
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Table key must be string or integer",
                "Expected STRING or INT key for TABLE",
                "Use table[\"key\"] or table[0]",
                Node->pos.line, Node->pos.collumn
            );
            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);
            return;    
        }

        // Assign: só permite se a chave já existir.
        // NÃO cria chave nova (nem com número nem com string).
        // A existência real da chave é checada no runtime.
        // SA apenas valida tipo e mutabilidade.
    }

    // MUTABILITY | MUTABILIDADE
    if (isAssign)
    {
        // CREATE BASE | Cria a Base.
        ExpressionNode* Base = Node->Object;

        // TAKE BASE | Pega a Base.
        while (Base->Type == NodeType::INDEX_ACCESS)
            Base = static_cast<IndexAccessNode*>(Base)->Object;
        while (Base->Type == NodeType::MEMBER_ACCESS)
            Base = static_cast<MemberAccessNode*>(Base)->Object;

        // Case Identifier
        if (Base->Type == NodeType::IDENTIFIER)
        {
            IdentifierNode* Id = static_cast<IdentifierNode*>(Base);
            Symbol* Sym = SAState.CurrScope->LookUp(Id->Name);

            if (Sym)
            {
                if (Sym->Mut == MutableTypes::CONST)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Cannot assign to index of const value",
                        "Ident '" + Id->Name + "' is const",
                        "Remove const or use a mut variable",
                        Node->pos.line, Node->pos.collumn
                    );
                    if (!M_Data->flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(*M_Data);
                    return;
                }

                Sym->write_count++;
                Sym->initialized = true;
            }
        }
    }
}

// Visit Assign Nodes | Visita Nós de Atribuição.
void SemanticAnalizer::VisitAssignment(AssignmentNode* Node)
{
    // ERROR PREV | PREVENÇÃO DE ERROS.
    if (!IsIValue(Node->Left))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Assign A Non <I_VALUE>",
            "Expected <IVALUE> But Got: " + Node->Left->GetNodeType(),
            "Try To Assign In A Valid <IVALUE>",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }

    // ANALIZE | ANALIZA
    if (Node->Left->Type == NodeType::MEMBER_ACCESS)
        VisitMemberAccess(static_cast<MemberAccessNode*>(Node->Left));
    else if (Node->Left->Type == NodeType::INDEX_ACCESS)
        VisitIndexAccess(static_cast<IndexAccessNode*>(Node->Left), true);
    else
    {
        Symbol* Sym = SAState.CurrScope->LookUp(
            static_cast<IdentifierNode*>(Node->Left)->Name
        );

        if (Sym->Mut == MutableTypes::CONST)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Try to Assign a <CONSTANT> IValue",
                Sym->name + " Is Declareated a Constant",
                "Remove 'const' or Dont Assign",
                Sym->Pos.line,
                Sym->Pos.collumn
            );

            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);

            return;
        }

        Sym->write_count++;
        Sym->initialized = true;
    }

    VisitExpression(Node->Right);

    TypeInfo* LeftType = GetExpressionType(Node->Left);
    TypeInfo* RightType = GetExpressionType(Node->Right);

    if (!LeftType || !RightType)
        return;

    if (LeftType->Kind != RightType->Kind)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Invalid Assignment",
            string(TypeToString(LeftType->Kind)) +
            " Expected, But Got: " +
            string(TypeToString(RightType->Kind)),
            "Assign A Compatible Value",
            Node->pos.line,
            Node->pos.collumn
        );

        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);

        return;
    }
}

// --- EXPRESSION --- //

// Visit General Expressions | Visita Expressões Gerais.
void SemanticAnalizer::VisitExpression(ExpressionNode* Node)
{
    if (!Node)
        return;

    switch (Node->Type)
    {
        case NodeType::LITERAL:
            VisitLiteral(static_cast<LiteralNode*>(Node));
            break;

        case NodeType::IDENTIFIER:
            VisitIdentifier(static_cast<IdentifierNode*>(Node));
            break;

        case NodeType::BINARY:
            VisitBinary(static_cast<BinaryNode*>(Node));
            break;

        case NodeType::ASSIGNMENT:
            VisitAssignment(static_cast<AssignmentNode*>(Node));
            break;
        
        default: {};
    }
}

// Visit Literal Nodes | Visita Nó de Valores Literais
void SemanticAnalizer::VisitLiteral(LiteralNode* Node)
{
    if (!Node)
        return;

    TypeInfo* Type = M_Memory->New<TypeInfo>();

    if (holds_alternative<i64>(Node->Value))
        Type->Kind = TypeKind::INT;

    else if (holds_alternative<float>(Node->Value))
        Type->Kind = TypeKind::FLOAT;

    else if (holds_alternative<str_view>(Node->Value))
        Type->Kind = TypeKind::STRING;

    else if (holds_alternative<bool>(Node->Value))
        Type->Kind = TypeKind::BOOL;

    else if (holds_alternative<NoneLitVal>(Node->Value))
        Type->Kind = TypeKind::NONE;

    else if (holds_alternative<NullLitVal>(Node->Value))
        Type->Kind = TypeKind::NULLVAL;

    SARes.ExpressionInfos[Node].Info = Type;
}

// Visit Binary Expressions | Visita Expressoes Binarias.
void SemanticAnalizer::VisitBinary(BinaryNode* Node)
{
    if (!Node)
        return;
}

// ======= ENTRY-POINT ======= //
// Entry-Point of SA Program | Ponto-de-Entrada do Programa de AS
SAResult SemanticAnalizer::InitSA(ParseResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: Semantic Analizing");

    // Create Data | Cria a Data
    M_Memory = &Memory;
    M_Data = &Data;

    SARes = SAResult{};
   
    // Create Global Scope | Cria o Escopo Global
    SAState.CurrScope = M_Memory->New<Scope>();
    SAState.CurrScope->Parent = nullptr;
    SAState.CurrScope->Kind = BodyTypes::PROGRAM; // ou o enum que você usar
    SAState.currScopeLvl = 0;

    // Visit AST | Visita a AST
    Visit(Res.AST);

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: Semantic Analizing. .. ...");
    return SARes;
}