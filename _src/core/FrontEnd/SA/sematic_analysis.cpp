
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

// Create a New TInfo | Cria um Novo TInfo
TypeInfo* SemanticAnalizer::CreateTInfo(TypeKind K, ASTNode* Node)
{
    TypeInfo* TInfo = M_Memory->New<TypeInfo>();

    TInfo->Kind = K;
    TInfo->ValueType   = nullptr;

    return TInfo;
}

// Get Type of Expressions | Pega o tipos de Expressoes.
TypeInfo* SemanticAnalizer::GetExpressionType(ExpressionNode* Node)
{
    // ERROR PREV | PREVENÇÃO DE ERROS
    if (!Node)
        return nullptr;

    // ALREADY COMPUTED | JA CALCULADO
    auto It = SARes.ExpressionInfos.find(Node);
    if (It != SARes.ExpressionInfos.end())
        return It->second.Info;

    // COMPUTE NOW | CALCULA AGORA
    TypeInfo* Type = nullptr;

    switch (Node->Type)
    {
        case NodeType::LITERAL:
        {
            LiteralNode* Lit = static_cast<LiteralNode*>(Node);

            if (holds_alternative<i64>(Lit->Value))
                Type = CreateTInfo(TypeKind::INT, Node);
            else if (holds_alternative<float>(Lit->Value))
                Type = CreateTInfo(TypeKind::FLOAT, Node);
            else if (holds_alternative<str_view>(Lit->Value))
                Type = CreateTInfo(TypeKind::STRING, Node);
            else if (holds_alternative<bool>(Lit->Value))
                Type = CreateTInfo(TypeKind::BOOL, Node);
            else if (holds_alternative<NoneLitVal>(Lit->Value))
                Type = CreateTInfo(TypeKind::NONE, Node);
            else if (holds_alternative<NullLitVal>(Lit->Value))
                Type = CreateTInfo(TypeKind::NULLVAL, Node);

            break;
        }

        case NodeType::IDENTIFIER:
        {
            IdentifierNode* Id = static_cast<IdentifierNode*>(Node);
            Symbol* Sym = SAState.CurrScope->LookUp(Id->Name);
            if (Sym)
                Type = Sym->Type;
            break;
        }

        case NodeType::ARRAY_VALUE:
        {
            Type = CreateTInfo(TypeKind::ARRAY, Node);
            break;
        }

        case NodeType::TABLE_VALUE:
        {
            Type = CreateTInfo(TypeKind::TABLE, Node);
            break;
        }

        case NodeType::MEMBER_ACCESS:
        {
            MemberAccessNode* Access = static_cast<MemberAccessNode*>(Node);
            TypeInfo* Object = GetExpressionType(Access->Object);

            if (Object &&
                (Object->Kind == TypeKind::STRUCT || Object->Kind == TypeKind::CLASS))
            {
                IdentifierNode* Member = static_cast<IdentifierNode*>(Access->Member);
                auto ItM = Object->Members.find(Member->Name);
                if (ItM != Object->Members.end())
                    Type = ItM->second;
            }
            break;
        }

        case NodeType::INDEX_ACCESS:
        {
            IndexAccessNode* Access = static_cast<IndexAccessNode*>(Node);
            TypeInfo* ObjType = GetExpressionType(Access->Object);

            if (ObjType)
            {
                if (ObjType->Kind == TypeKind::ARRAY)
                    Type = ObjType->ElementType;
                else if (ObjType->Kind == TypeKind::TABLE)
                    Type = ObjType->ValueType;
            }
            break;
        }

        case NodeType::ASSIGNMENT:
        case NodeType::BINARY:
        case NodeType::UNARY:
        {
            // Esses precisam do Visit* para calcular corretamente
            // (o Visit já preenche o cache)
            break;
        }

        default:
            break;
    }

    // CACHE RESULT | GUARDA O RESULTADO
    if (Type)
        SARes.ExpressionInfos[Node].Info = Type;

    return Type;
}

// Convert Literal to a TypeInfo | Converte um Literal para TypeInfo
TypeInfo* SemanticAnalizer::ConvertLiteralTypeInfo(LiteralTypes Type)
{
    TypeInfo* Info = nullptr;

    switch (Type)
    {
        case LiteralTypes::INT:
            Info = CreateTInfo(TypeKind::INT, nullptr);
            break;

        case LiteralTypes::FLOAT:
            Info = CreateTInfo(TypeKind::FLOAT, nullptr);
            break;

        case LiteralTypes::STRING:
            Info = CreateTInfo(TypeKind::STRING, nullptr);
            break;

        case LiteralTypes::BOOL:
            Info = CreateTInfo(TypeKind::BOOL, nullptr);
            break;

        case LiteralTypes::NONE:
            Info = CreateTInfo(TypeKind::NONE, nullptr);
            break;

        case LiteralTypes::_NULL:
            Info = CreateTInfo(TypeKind::NULLVAL, nullptr);
            break;

        case LiteralTypes::ARRAY_INT:
            Info = CreateTInfo(TypeKind::ARRAY, nullptr);
            Info->ElementType = CreateTInfo(TypeKind::INT, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::ARRAY_FLOAT:
            Info = CreateTInfo(TypeKind::ARRAY, nullptr);
            Info->ElementType = CreateTInfo(TypeKind::FLOAT, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::ARRAY_STRING:
            Info = CreateTInfo(TypeKind::ARRAY, nullptr);
            Info->ElementType = CreateTInfo(TypeKind::STRING, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::ARRAY_BOOL:
            Info = CreateTInfo(TypeKind::ARRAY, nullptr);
            Info->ElementType = CreateTInfo(TypeKind::BOOL, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::ARRAY_NONE:
            Info = CreateTInfo(TypeKind::ARRAY, nullptr);
            Info->ElementType = CreateTInfo(TypeKind::NONE, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::ARRAY_NULL:
            Info = CreateTInfo(TypeKind::ARRAY, nullptr);
            Info->ElementType = CreateTInfo(TypeKind::NULLVAL, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::TABLE_INT:
            Info = CreateTInfo(TypeKind::TABLE, nullptr);
            Info->ValueType = CreateTInfo(TypeKind::INT, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::TABLE_FLOAT:
            Info = CreateTInfo(TypeKind::TABLE, nullptr);
            Info->ValueType = CreateTInfo(TypeKind::FLOAT, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::TABLE_STRING:
            Info = CreateTInfo(TypeKind::TABLE, nullptr);
            Info->ValueType = CreateTInfo(TypeKind::STRING, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::TABLE_BOOL:
            Info = CreateTInfo(TypeKind::TABLE, nullptr);
            Info->ValueType = CreateTInfo(TypeKind::BOOL, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::TABLE_NONE:
            Info = CreateTInfo(TypeKind::TABLE, nullptr);
            Info->ValueType = CreateTInfo(TypeKind::NONE, nullptr);
            Info->ExplElemType = true;
            break;

        case LiteralTypes::TABLE_NULL:
            Info = CreateTInfo(TypeKind::TABLE, nullptr);
            Info->ValueType = CreateTInfo(TypeKind::NULLVAL, nullptr);
            Info->ExplElemType = true;
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

// Convert TypeKind to a String | Converte TypeKind Para uma String.
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
    for (ASTNode* Child : Node->Data)
    {
        Visit(Child);
    }
}

// --- DECLARATION --- //

// Visit Var Declarations | Visita Criações de Variaveis.
void SemanticAnalizer::VisitVarDecl(VarDeclNode* Node)
{
    // Error Prev | Prevenção de Erros
    if (SAState.CurrScope->LookUp(Node->Name))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "<IDENTIFIER> Already Exists",
            "Ident: " + Node->Name + " Already Exists, First Declareated Here: "
            + std::to_string(SAState.CurrScope->LookUp(Node->Name)->Pos.line)
            + ":" + std::to_string(SAState.CurrScope->LookUp(Node->Name)->Pos.collumn),
            "Add a Diferent Name, Ex: " + Node->Name + "2",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }

    // Validate Expression and Create Symbol | Valida Expressoes e Cria o Simbolo.
    bool inited = false;
    if (Node->Val)
    {
        VisitExpression(Node->Val);
        inited = true;
    }

    Symbol* S = M_Memory->New<Symbol>();
    S->initialized = inited;

    // Set Symbol | Define o Simbolo.
    S->Mut  = Node->MutType;
    S->Pos  = Node->pos;
    S->name = Node->Name;

    // TYPE CHECKING | CHECK DE TIPOS.
    TypeInfo* Expected = ConvertLiteralTypeInfo(Node->InferType);
    TypeInfo* Actual   = Node->Val ? GetExpressionType(Node->Val) : nullptr;

    // ERROR PREV | PREVENÇÃO DE ERRO.
    if (Node->InferType != LiteralTypes::MONO_STATE)
    {
        if (!Actual)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Variable with explicit type requires initializer",
                "Type was declared but no value given",
                "Add a value or remove the type annotation",
                S->Pos.line, S->Pos.collumn
            );

            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);

            return;
        }

        if (!TypesEqual(Expected, Actual))
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

        // ARRAY ELEMENT CHECK | CHECAGEM DE ELEMENTOS DE ARRAY.
        if (Expected &&
            Actual &&
            Expected->Kind == TypeKind::ARRAY &&
            Expected->ExplElemType)
        {
            for (TypeInfo* ElemType : Actual->Elements)
            {
                if (!TypesEqual(ElemType, Expected->ElementType))
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Invalid Array Element",
                        "Expected " +
                        TypeToString(Expected->ElementType->Kind) +
                        ", But Got: " +
                        TypeToString(ElemType->Kind),
                        "Use a compatible value",
                        S->Pos.line,
                        S->Pos.collumn
                    );

                    if (!M_Data->flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(*M_Data);

                    return;
                }
            }
        }
    }

    if (Expected)
        S->Type = Expected;
    else
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
            "Ident: " + Node->Name + " Non Exists",
            "Create or Change the Symbol",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }

    SARes.ExpressionInfos[Node].Info = Sym->Type;
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
            "Expected <IVALUE> But Got: " + Node->Member->GetNodeType(),
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
            "Expected <IVALUE> But Got: " + Node->Member->GetNodeType(),
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
    if (!Node) return;
    if (
        !IsIValue(Node->Object) &&
        Node->Object->Type != NodeType::ARRAY_VALUE &&
        Node->Object->Type != NodeType::TABLE_VALUE
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing",
            "Trying to Assign A Non <I_VALUE>",
            "Expected <IVALUE> But Got: " + Node->Object->GetNodeType(),
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
    if (ObjType->Kind == TypeKind::ARRAY) // ARRAY | LISTAS. 
    {
        if (!IdxType || IdxType->Kind != TypeKind::INT)
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
        IdentifierNode* Id = static_cast<IdentifierNode*>(Node->Left);
        Symbol* Sym = SAState.CurrScope->LookUp(Id->Name);

        if (!Sym)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Use a Undefined <IDENTIFIER>",
                "Ident: " + Id->Name + " Non Exists",
                "Create or Change the Symbol",
                Node->pos.line,
                Node->pos.collumn
            );
            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);
            return;
        }

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

    TypeInfo* LeftType  = GetExpressionType(Node->Left);
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

    SARes.ExpressionInfos[Node].Info = RightType;
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

        case NodeType::UNARY:
            VisitUnary(static_cast<UnaryNode*>(Node));
            break;

        case NodeType::MEMBER_ACCESS:
            VisitMemberAccess(static_cast<MemberAccessNode*>(Node));
            break;

        case NodeType::INDEX_ACCESS:
            VisitIndexAccess(static_cast<IndexAccessNode*>(Node), false);
            break;

        case NodeType::ARRAY_VALUE:
            VisitArray(static_cast<ArrayValue*>(Node));
            break;

        // case NodeType::TABLE_VALUE:
        //     VisitTable(static_cast<TableValue*>(Node));
        //     break;

        default: {};
    }
}

// Visit Literal Nodes | Visita Nó de Valores Literais
void SemanticAnalizer::VisitLiteral(LiteralNode* Node)
{
    if (!Node)
        return;

    TypeInfo* Type = nullptr;

    if (holds_alternative<i64>(Node->Value))
        Type = CreateTInfo(TypeKind::INT, Node);

    else if (holds_alternative<float>(Node->Value))
        Type = CreateTInfo(TypeKind::FLOAT, Node);

    else if (holds_alternative<str_view>(Node->Value))
        Type = CreateTInfo(TypeKind::STRING, Node);

    else if (holds_alternative<bool>(Node->Value))
        Type = CreateTInfo(TypeKind::BOOL, Node);

    else if (holds_alternative<NoneLitVal>(Node->Value))
        Type = CreateTInfo(TypeKind::NONE, Node);

    else if (holds_alternative<NullLitVal>(Node->Value))
        Type = CreateTInfo(TypeKind::NULLVAL, Node);

    SARes.ExpressionInfos[Node].Info = Type;
}

// Visit Array Values | Visita Valores de Arrays.
void SemanticAnalizer::VisitArray(ArrayValue* Node)
{
    if (!Node)
        return;
    TypeInfo* ArrayType = CreateTInfo(TypeKind::ARRAY, Node);
    for (ExpressionNode* C : Node->Args)
    {
        VisitExpression(C);
        TypeInfo* ElemType = GetExpressionType(C);
        if (!ElemType)
            continue;
        ArrayType->Elements.push_back(ElemType);
    }

    SARes.ExpressionInfos[Node].Info = ArrayType;
}

// Visit Binary Expressions | Visita Expressoes Binarias.
void SemanticAnalizer::VisitBinary(BinaryNode* Node)
{
    // Error Prev | Prevenção de Erros.
    if (!Node)
    {
        SARes.ExpressionInfos[Node].Info = nullptr;
        return;
    }

    // Valid Expression of L and R | Valida as Expressoes De Ambos os Lados.
    VisitExpression(Node->L);
    VisitExpression(Node->R);

    // Take Types of Sides | Pega o Tipo dos 2 Lados.
    TypeInfo* L = GetExpressionType(Node->L);
    TypeInfo* R = GetExpressionType(Node->R);

    if (!L || !R)
    {
        SARes.ExpressionInfos[Node].Info = nullptr;
        return;
    }

    TypeInfo* Result = CreateTInfo(TypeKind::UNKNOWN, Node);

    // NONE / NULL | Apenas Comparação.
    if (L->Kind == TypeKind::NONE ||
        R->Kind == TypeKind::NONE ||
        L->Kind == TypeKind::NULLVAL ||
        R->Kind == TypeKind::NULLVAL)
    {
        switch (Node->Op)
        {
            case Operator::EQUAL:
            case Operator::NOT_EQUAL:
                Result->Kind = TypeKind::BOOL;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            default:
                break;
        }
    }

    // BOOL | Operações Booleanas.
    if (L->Kind == TypeKind::BOOL || R->Kind == TypeKind::BOOL)
    {
        switch (Node->Op)
        {
            case Operator::AND:
            case Operator::OR:
                if (L->Kind == TypeKind::BOOL && R->Kind == TypeKind::BOOL)
                {
                    Result->Kind = TypeKind::BOOL;
                    SARes.ExpressionInfos[Node].Info = Result;
                }
                return;

            case Operator::EQUAL:
            case Operator::NOT_EQUAL:
                if (L->Kind == TypeKind::BOOL && R->Kind == TypeKind::BOOL)
                {
                    Result->Kind = TypeKind::BOOL;
                    SARes.ExpressionInfos[Node].Info = Result;
                }
                return;

            default:
                break;
        }
    }

    // STRING CONCAT | Concatenação de Strings.
    if (Node->Op == Operator::ADD)
    {
        if (L->Kind == TypeKind::STRING &&
            (R->Kind == TypeKind::STRING ||
             R->Kind == TypeKind::INT ||
             R->Kind == TypeKind::FLOAT))
        {
            Result->Kind = TypeKind::STRING;
            SARes.ExpressionInfos[Node].Info = Result;
            return;
        }

        if (R->Kind == TypeKind::STRING &&
            (L->Kind == TypeKind::INT ||
             L->Kind == TypeKind::FLOAT))
        {
            Result->Kind = TypeKind::STRING;
            SARes.ExpressionInfos[Node].Info = Result;
            return;
        }
    }

    // FLOAT | Qualquer operação envolvendo FLOAT retorna FLOAT.
    if (L->Kind == TypeKind::FLOAT || R->Kind == TypeKind::FLOAT)
    {
        switch (Node->Op)
        {
            case Operator::ADD:
            case Operator::SUB:
            case Operator::MUL:
            case Operator::DIV:
            case Operator::MOD:
            case Operator::POWER:
                Result->Kind = TypeKind::FLOAT;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            case Operator::EQUAL:
            case Operator::NOT_EQUAL:
            case Operator::LESS:
            case Operator::GREATER:
            case Operator::LESS_EQUAL:
            case Operator::GREATER_EQUAL:
                Result->Kind = TypeKind::BOOL;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            default:
                break;
        }
    }

    // INT | Operações entre inteiros.
    if (L->Kind == TypeKind::INT && R->Kind == TypeKind::INT)
    {
        switch (Node->Op)
        {
            case Operator::ADD:
            case Operator::SUB:
            case Operator::MUL:
            case Operator::POWER:
                Result->Kind = TypeKind::INT;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            case Operator::DIV:
            case Operator::MOD:
                Result->Kind = TypeKind::FLOAT;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            case Operator::EQUAL:
            case Operator::NOT_EQUAL:
            case Operator::LESS:
            case Operator::GREATER:
            case Operator::LESS_EQUAL:
            case Operator::GREATER_EQUAL:
                Result->Kind = TypeKind::BOOL;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            default:
                break;
        }
    }

    // STRING COMPARISON | Comparação de Strings.
    if (L->Kind == TypeKind::STRING && R->Kind == TypeKind::STRING)
    {
        switch (Node->Op)
        {
            case Operator::EQUAL:
            case Operator::NOT_EQUAL:
                Result->Kind = TypeKind::BOOL;
                SARes.ExpressionInfos[Node].Info = Result;
                return;

            default:
                break;
        }
    }

    // Invalid Operation | Operação Inválida.
    OrbitLog::SyntaxLog::SyntaxError(
        "Semantic",
        "Invalid Binary Operation",
        string(TypeToString(L->Kind)) +
        " Operator " +
        string(TypeToString(R->Kind)),
        "Use Compatible Types",
        Node->pos.line,
        Node->pos.collumn
    );

    if (!M_Data->flags.debugMode)
        OrbitLog::SyntaxLog::ThrowLog(*M_Data);
}

// Visit Unary Expressions | Visita Expressoes Unarias.
void SemanticAnalizer::VisitUnary(UnaryNode* Node)
{
    // Error Prev | Prevenção de Erros.
    if (!Node)
    {
        SARes.ExpressionInfos[Node].Info = nullptr;
        return;
    }

    // Valid Operand | Valida Operando.
    VisitExpression(Node->Operand);

    if (Node->Operator == Operator::NOT)
    {
        TypeInfo* OpType = GetExpressionType(Node->Operand);
        if (!OpType || OpType->Kind != TypeKind::BOOL)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid Operands for <UNARY> Expression",
                "Expected <BOOL> or <IDENTIFIER>... But Got: " +
                TypeToString(OpType ? OpType->Kind : TypeKind::UNKNOWN),
                "~",
                Node->Operand->pos.line,
                Node->Operand->pos.collumn
            );
            if (!M_Data->flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(*M_Data);
            return;
        }
    }
    else
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Invalid Operator for <UNARY> Value",
            "~",
            "~",
            Node->pos.line,
            Node->pos.collumn
        );
        if (!M_Data->flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(*M_Data);
        return;
    }

    TypeInfo* Result = CreateTInfo(TypeKind::BOOL, Node);
    SARes.ExpressionInfos[Node].Info = Result;
}

// ======= ENTRY-POINT ======= //
// Entry-Point of SA Program | Ponto-de-Entrada do Programa de AS
SAResult SemanticAnalizer::InitSA(ParseResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: Semantic Analizing");

    // Create Data | Cria a Data
    M_Memory = &Memory;
    M_Data   = &Data;

    SARes = SAResult{};

    // Create Global Scope | Cria o Escopo Global
    SAState.CurrScope = M_Memory->New<Scope>();
    SAState.CurrScope->Parent = nullptr;
    SAState.CurrScope->Kind = BodyTypes::PROGRAM;
    SAState.currScopeLvl = 0;

    // Visit AST | Visita a AST
    Visit(Res.AST);

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: Semantic Analizing. .. ...");

    return SARes;
}
