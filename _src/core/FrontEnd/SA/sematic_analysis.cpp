

// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o Codigo em Busca de Erros Semanticos.
// Developed By: SpyK3(2026) | License: GitHub(MIT).

// INCLUDE HEADERS 'N DEPENDENCES
#include "semantic_analysis.hpp" // HEADER FILE | CABEÇALHO.

#include "../parser/parser.hpp"
#include "../parser/AST/AST.hpp"

#include "../lexer/lexer.hpp"
#include "../tokenizer/tokenizer.hpp"

#include "utils/aliases.hpp"
#include "tools/console.hpp"
#include "../../RunTimeData.hpp"
#include <algorithm>
#include <fstream>
#include <format>
#include <string>
#include <utility>

// ========= UTILS ========== //

// Semantic Analysis Utils | Utilidades do Analisador.
namespace SAUtils {

    // Find Scope Whit Type | Encontra O Escopo Com o Tupo.
    Scope* FindScopeType(BodyTypes Type, SAState& State)
    {
        Scope* S = State.CurrScope;
        while (S)
        {
            if (S->Type == Type)
                return S;
            else S = S->Parent;
        }
        return nullptr;
    }

    // In Object Scopes | Em Escopos de Objetos.
    Scope* InObjScope(SAState& State)
    {
        return 
            FindScopeType(BodyTypes::NAMESPACE, State)
            //or
            //FindScopeType(BodyTypes::, State)
        ;
    }

    // Create A New Symbol | Cria um Novo Simbolo.
    Symbol* CreateSymbol(const string& Name, ASTNode& Node, SAState& State, SAResult& Res, Arena& Memory)
    {
        Symbol* Sym = Memory.New<Symbol>();
        TypeInfo* TInfo = Memory.New<TypeInfo>();
        TypeInfo* InferType = Memory.New<TypeInfo>();

        TInfo->Father     = Sym;
        InferType->Father = Sym;

        Sym->Name = Name;
        Sym->DeclaredScope = State.CurrScope;
        Sym->TInfo = TInfo;
        Sym->InferType = InferType;
        Sym->Pos = Node.pos;

        Sym->TInfo->Kind = TypeKind::_NULL;
        Sym->TInfo->SubKind = SubTypeKind::NONE;

        Sym->InferType->Kind = TypeKind::_NULL;
        Sym->InferType->SubKind = SubTypeKind::NONE;

        if (State.CurrScope)
            State.CurrScope->Symbols[Sym->Name] = Sym;

        Res.SymbolTable[Sym->Name] = Sym;
        Res.Symbols[State.nextId]  = Sym;
        Node.SymbolId              = State.nextId;
        Sym->Id                    = Node.SymbolId;
        State.nextId++;

        return Sym;
    }

    // Return Kind Version of Literal | Retorna a Versão Kind do Literal.
    TypeInfo* GetKindOfLiteral(const LiteralTypes& Type)
    {
        TypeInfo* TInfo = new TypeInfo();

        switch (Type)
        {
            case LiteralTypes::INT:
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SubTypeKind::INT;
                break;

            case LiteralTypes::FLOAT:
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SubTypeKind::FLOAT;
                break;

            case LiteralTypes::STRING:
                TInfo->Kind = TypeKind::STRING;
                TInfo->SubKind = SubTypeKind::NONE;
                break;

            case LiteralTypes::BOOL:
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SubTypeKind::NONE;
                break;

            case LiteralTypes::NONE:
                TInfo->Kind = TypeKind::NONE;
                TInfo->SubKind = SubTypeKind::NONE;
                break;

            case LiteralTypes::_NULL:
                TInfo->Kind = TypeKind::_NULL;
                TInfo->SubKind = SubTypeKind::NONE;
                break;

            default:
                TInfo->Kind = TypeKind::MONO_STATE;
                TInfo->SubKind = SubTypeKind::NONE;
                break;
        }

        return TInfo;
    }

    // Return String Version of Kind | Retorna a Versão de String do kind.
    string GetStringOfKind(TypeKind K)
    {
        switch (K)
        {
            case TypeKind::UNK:             return "<UNK>";
            case TypeKind::MONO_STATE:      return "<MONO_STATE>";

            case TypeKind::NUMBER:          return "<NUMBER>";
            case TypeKind::STRING:          return "<STRING>";
            case TypeKind::BOOL:            return "<BOOL>";
            case TypeKind::ANY:             return "<ANY>";
            case TypeKind::_NULL:           return "<NULL>";
            case TypeKind::NONE:            return "<NONE>";

            case TypeKind::ARRAY:           return "<ARRAY>";
            case TypeKind::TABLE:           return "<TABLE>";

            case TypeKind::TABLE_INT:       return "<TABLE_INT>";
            case TypeKind::TABLE_FLOAT:     return "<TABLE_FLOAT>";
            case TypeKind::TABLE_STRING:    return "<TABLE_STRING>";
            case TypeKind::TABLE_BOOL:      return "<TABLE_BOOL>";
            case TypeKind::TABLE_ANY:       return "<TABLE_ANY>";
            case TypeKind::TABLE_NULL:      return "<TABLE_NULL>";
            case TypeKind::TABLE_NONE:      return "<TABLE_NONE>";

            case TypeKind::STRUCT:           return "<STRUCT>";
            case TypeKind::CLASS:            return "<CLASS>";

            case TypeKind::ITERATOR:         return "<ITERATOR>";

            case TypeKind::FN:               return "<FN>";

            default:                         return "<UNKNOWN>";
        }
    }

    // Return Name of IValues | Retorna o Nome dos I-Values.
    string GetIValueName(ExpressionNode* Node)
    {
        if (!Node)
            return "UNKNOW";

        switch (Node->Type)
        {
            case NodeType::IDENTIFIER:
            {
                IdentifierNode* Id = static_cast<IdentifierNode*>(Node);
                return Id->Name;
            }

            case NodeType::MEMBER_ACCESS:
            {
                MemberAccessNode* Ma = static_cast<MemberAccessNode*>(Node);

                string ObjectName = GetIValueName(Ma->Object);

                if (ObjectName == "UNKNOW")
                    return "UNKNOW";

                return ObjectName;
            }

            default:
                return "UNKNOW";
        }
    }

    // Return If Node is a IValue | Retorna Se o No e um IValue.
    bool IsIValue(ExpressionNode* Node)
    {
        if (!Node)
            return false;

        if (Node->Type == NodeType::IDENTIFIER)
            return true;
        else if (Node->Type == NodeType::MEMBER_ACCESS)
            return true;
        else if (Node->Type == NodeType::INDEX_ACCESS)
            return true;

        return false;
    }
}

// ========== CORE ========== //

// Return if L and R Is Same Type | Retoran se E e D Sao do Mesmo Tipo.
bool TypesEqual(TypeInfo& L, TypeInfo& R)
{
    if (L.Kind != R.Kind)
        return false;

    if (L.Kind == TypeKind::NUMBER)
        return L.SubKind == R.SubKind;

    return true;
}

// Return if L-KIND and R-KIND Is Same Type | Retoran se E-KIND e D-KIND Sao do Mesmo Tipo.
bool TypesKindEqual(TypeKind L, TypeKind R)
{ return L == R; }

// Get Expression Types | Pega o Tipo das Expressoes.
TypeInfo* GetExpressionType(ExpressionNode* Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (!Node)
    {
        TypeInfo* TInfo = Memory.New<TypeInfo>();
        TInfo->Kind = TypeKind::_NULL;
        TInfo->SubKind = SubTypeKind::NONE;
        return TInfo;
    }

    auto Cached = Res.ExpressionTypes.find(Node);
    if (Cached != Res.ExpressionTypes.end())
        return &Cached->second;

    TypeInfo* TInfo = Memory.New<TypeInfo>();
    TInfo->Kind = TypeKind::MONO_STATE;
    TInfo->SubKind = SubTypeKind::NONE;

    switch (Node->Type)
    {
        case NodeType::LITERAL:
        {
            LiteralNode& Lit = static_cast<LiteralNode&>(*Node);

            if (holds_alt<i64>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SubTypeKind::INT;
            }
            else if (holds_alt<float>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SubTypeKind::FLOAT;
            }
            else if (holds_alt<string>(Lit.Value))
            {
                TInfo->Kind = TypeKind::STRING;
            }
            else if (holds_alt_value<bool>(Lit.Value, true))
            {
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SubTypeKind::TRUE;
            }
            else if (holds_alt_value<bool>(Lit.Value, false))
            {
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SubTypeKind::FALSE;
            }
            else if (holds_alt<NoneLitVal>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NONE;
            }
            else if (holds_alt<NullLitVal>(Lit.Value))
            {
                TInfo->Kind = TypeKind::_NULL;
            }
            else if (holds_alt<nullptr_t>(Lit.Value))
            {
                TInfo->Kind = TypeKind::MONO_STATE;
            }

            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::ARRAY_VALUE:
        {
            TInfo->Kind = TypeKind::ARRAY;

            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::TABLE_VALUE:
        {
            TInfo->Kind = TypeKind::TABLE;

            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::MEMBER_ACCESS:
        {
            // Take Object | Pega O Objeto
            MemberAccessNode& Ma = static_cast<MemberAccessNode&>(*Node);
            TypeInfo* ObjInfo = GetExpressionType
            (Ma.Object, State, Res, Data, Memory);

            // MonoState or Unknow | Estado Desconhecido ou Não-Definido.
            if (
                ObjInfo->Kind == TypeKind::MONO_STATE ||
                ObjInfo->Kind == TypeKind::UNK
            )
            {
                TInfo->Kind = TypeKind::MONO_STATE;
            }
            else
            {
                TInfo->Kind = TypeKind::MONO_STATE;
            }

            // Take Object Data | Pega Os Dados Do Objeto.
            string ObjName = SAUtils::GetIValueName(Ma.Object);
            Symbol* ObjSym = State.CurrScope->FindSym(ObjName);

            // Undeclared Object | Objeto Não Declarado.
            if (!ObjSym)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic", 
                    "Trying to Get Symbol of Undeclared Object", 
                    "Object Has Not Been Declared", 
                    "Declare Object, or Ajust Name",
                    Node->pos.line, Node->pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                TInfo->Kind = TypeKind::UNK;
            }

            // NAMESPACES
            if (ObjSym->Type == SymbolTypes::NAMESPACE && ObjSym->LinkedScope)
            {
                // Take Member Name | Pega O Nome Do Membro.
                string M_Name;
                if (Ma.Member && Ma.Member->Type == NodeType::IDENTIFIER)
                    M_Name = static_cast<IdentifierNode*>(Ma.Member)->Name;
                else if (Ma.Member && Ma.Member->Type == NodeType::MEMBER_ACCESS)
                    return GetExpressionType(Ma.Member, State, Res, Data, Memory);
                else { // Invalid Acess | Acesso Invalido:

                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic", 
                        "Trying To Acess A Invalid Member", 
                        "Member Of Type: " +Ma.GetNodeType()+" Cannot Be Acessed",
                        "Acess A Valid Member",
                        Ma.pos.line, Ma.pos.collumn
                    );
                    if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                    TInfo->Kind = TypeKind::UNK;
                }

                // Take Symbol of Member | Pega O Simbolo Do Membro.
                Symbol* MemberSym = ObjSym->LinkedScope->FindSymLocal(M_Name);
                if (!MemberSym)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Trying To Acess A Undeclared Member ",
                        "Member '" +M_Name+ "' Not Found in Namespace '" + ObjName + "'",
                        "Check the Name or Declare-It",
                        Ma.pos.line, Ma.pos.collumn
                    );
                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);

                    TInfo->Kind = TypeKind::UNK;
                }
                else // Try Copy Sym | Tenta Copiar O Simbolo:
                {
                    *TInfo = *MemberSym->TInfo;
                    if (Ma.Member)
                        Ma.Member->SymbolId = MemberSym->Id;
                }
            } else // OTHERS:
                TInfo->Kind = TypeKind::MONO_STATE;

            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::INDEX_ACCESS:
        {
            IndexAccessNode& Index = static_cast<IndexAccessNode&>(*Node);

            TypeInfo* ObjInfo = GetExpressionType
            (Index.Object, State, Res, Data, Memory);

            TypeInfo* IndexInfo = GetExpressionType
            (Index.Index, State, Res, Data, Memory);

            if (ObjInfo->Kind == TypeKind::ARRAY)
            {
                TypeInfo ExpectedIndex;
                ExpectedIndex.Kind = TypeKind::NUMBER;
                ExpectedIndex.SubKind = SubTypeKind::INT;

                if (!TypesEqual(*IndexInfo, ExpectedIndex))
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Invalid Index value Type",
                        "Expected <INT> For <ARRAY>, But Got: "+SAUtils::GetStringOfKind(IndexInfo->Kind),
                        "Add a Valid Type Or Convert",
                        Index.Index->pos.line, Index.Index->pos.collumn
                    );

                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);

                    TInfo->Kind = TypeKind::UNK;
                    Res.ExpressionTypes[Node] = *TInfo;
                    return &Res.ExpressionTypes[Node];
                }

                TInfo->Kind = TypeKind::MONO_STATE;

                Res.ExpressionTypes[Node] = *TInfo;
                return &Res.ExpressionTypes[Node];
            }

            if (ObjInfo->Kind == TypeKind::TABLE)
            {
                TypeInfo StringIndex;
                StringIndex.Kind = TypeKind::STRING;
                StringIndex.SubKind = SubTypeKind::NONE;

                TypeInfo IntIndex;
                IntIndex.Kind = TypeKind::NUMBER;
                IntIndex.SubKind = SubTypeKind::INT;

                if (
                    !TypesEqual(*IndexInfo, StringIndex) &&
                    !TypesEqual(*IndexInfo, IntIndex)
                )
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Invalid Index value Type",
                        "Expected <STRING/INT> For <TABLE>, But Got: "+SAUtils::GetStringOfKind(IndexInfo->Kind),
                        "Add a Valid Type Or Convert",
                        Index.Index->pos.line, Index.Index->pos.collumn
                    );

                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);

                    TInfo->Kind = TypeKind::UNK;
                    Res.ExpressionTypes[Node] = *TInfo;
                    return &Res.ExpressionTypes[Node];
                }

                TInfo->Kind = TypeKind::MONO_STATE;

                Res.ExpressionTypes[Node] = *TInfo;
                return &Res.ExpressionTypes[Node];
            }

            if (ObjInfo->Kind == TypeKind::STRING)
            {
                TypeInfo ExpectedIndex;
                ExpectedIndex.Kind = TypeKind::NUMBER;
                ExpectedIndex.SubKind = SubTypeKind::INT;

                if (!TypesEqual(*IndexInfo, ExpectedIndex))
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Invalid Index value Type",
                        "Expected <INT> For <STRING>, But Got: "+SAUtils::GetStringOfKind(IndexInfo->Kind),
                        "Add a Valid Type Or Convert",
                        Index.Index->pos.line, Index.Index->pos.collumn
                    );

                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);

                    TInfo->Kind = TypeKind::UNK;
                    Res.ExpressionTypes[Node] = *TInfo;
                    return &Res.ExpressionTypes[Node];
                }

                TInfo->Kind = TypeKind::STRING;
                TInfo->SubKind = SubTypeKind::NONE;

                Res.ExpressionTypes[Node] = *TInfo;
                return &Res.ExpressionTypes[Node];
            }

            if (
                ObjInfo->Kind == TypeKind::MONO_STATE ||
                ObjInfo->Kind == TypeKind::UNK
            )
            {
                TInfo->Kind = TypeKind::MONO_STATE;
                Res.ExpressionTypes[Node] = *TInfo;
                return &Res.ExpressionTypes[Node];
            }

            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid Index Object",
                "Expected <ARRAY>/<TABLE>, But Got: "+SAUtils::GetStringOfKind(ObjInfo->Kind),
                "Use an <ARRAY> or <TABLE> to Access an Index",
                Index.Object->pos.line, Index.Object->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            TInfo->Kind = TypeKind::UNK;
            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::IDENTIFIER:
        {
            IdentifierNode& Id = static_cast<IdentifierNode&>(*Node);

            Symbol* Sym = State.CurrScope
                ? State.CurrScope->FindSym(Id.Name)
                : nullptr;

            if (!Sym)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Used a Undeclared <IDENTIFIER>",
                    "Ident: '"+Id.Name+"' Dont Exists",
                    "Declare Ident or Use a Valid Identifier",
                    Node->pos.line, Node->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                TInfo->Kind = TypeKind::UNK;
                Res.ExpressionTypes[Node] = *TInfo;
                return &Res.ExpressionTypes[Node];
            }

            Id.SymbolId = Sym->Id;

            if (!Sym->inited)
            {
                TInfo->Kind = TypeKind::_NULL;
                Res.ExpressionTypes[Node] = *TInfo;
                return &Res.ExpressionTypes[Node];
            }
            
            Res.ExpressionTypes[Node] = *Sym->TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::BINARY:
        {
            BinaryNode& Binary = static_cast<BinaryNode&>(*Node);

            TypeInfo* LInfo = GetExpressionType
            (Binary.L, State, Res, Data, Memory);

            TypeInfo* RInfo = GetExpressionType
            (Binary.R, State, Res, Data, Memory);

            TypeKind LKind = LInfo->Kind;
            TypeKind RKind = RInfo->Kind;

            auto IsComparable = [](TypeKind Kind) -> bool
            {
                return Kind != TypeKind::UNK &&
                    Kind != TypeKind::MONO_STATE &&
                    Kind != TypeKind::NONE &&
                    Kind != TypeKind::FN;
            };

            auto InvalidOperation = [&]()
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Invalid Operation",
                    "Trying to Make A Expr Whit Invalid Operands: " +
                    SAUtils::GetStringOfKind(LKind) +
                    " And: " +
                    SAUtils::GetStringOfKind(RKind),
                    "Add a Valid Type or Convert",
                    Node->pos.line, Node->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);
            };

            auto SetNumberResult = [&]()
            {
                TInfo->Kind = TypeKind::NUMBER;

                if (
                    LInfo->SubKind == SubTypeKind::FLOAT ||
                    RInfo->SubKind == SubTypeKind::FLOAT
                )
                {
                    TInfo->SubKind = SubTypeKind::FLOAT;
                }
                else
                {
                    TInfo->SubKind = SubTypeKind::INT;
                }
            };

            switch (Binary.Op)
            {
                // ARITHMETIC | ARITMETICOS.
                case Operator::ADD:
                {
                    if (LKind == TypeKind::MONO_STATE || RKind == TypeKind::MONO_STATE)
                    {
                        TInfo->Kind = TypeKind::MONO_STATE;
                    }
                    else if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
                    {
                        SetNumberResult();
                    }
                    else if (LKind == TypeKind::STRING && RKind == TypeKind::STRING)
                    {
                        TInfo->Kind = TypeKind::STRING;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::SUB:
                case Operator::MUL:
                case Operator::DIV:
                case Operator::MOD:
                case Operator::POWER:
                {
                    if (LKind == TypeKind::MONO_STATE || RKind == TypeKind::MONO_STATE)
                    {
                        TInfo->Kind = TypeKind::MONO_STATE;
                    }
                    else if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
                    {
                        SetNumberResult();
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                // COMPARISON | COMPARAÇOES.
                case Operator::EQUAL:
                case Operator::NOT_EQUAL:
                {
                    if (
                        LKind == TypeKind::MONO_STATE ||
                        RKind == TypeKind::MONO_STATE
                    )
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else if (IsComparable(LKind) && IsComparable(RKind))
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::LESS:
                case Operator::GREATER:
                case Operator::LESS_EQUAL:
                case Operator::GREATER_EQUAL:
                {
                    if (
                        LKind == TypeKind::MONO_STATE ||
                        RKind == TypeKind::MONO_STATE
                    )
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                // LOGICAL | LOGICOS.
                case Operator::AND:
                case Operator::OR:
                {
                    if (
                        LKind == TypeKind::MONO_STATE ||
                        RKind == TypeKind::MONO_STATE
                    )
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else if (LKind == TypeKind::BOOL && RKind == TypeKind::BOOL)
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::NOT:
                {
                    if (LKind == TypeKind::MONO_STATE)
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else if (LKind == TypeKind::BOOL)
                    {
                        TInfo->Kind = TypeKind::BOOL;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                // ASSIGN | ASSIGNAÇÃO.
                case Operator::ASSIGN:
                {
                    if (
                        LKind == TypeKind::MONO_STATE ||
                        RKind == TypeKind::MONO_STATE
                    )
                    {
                        TInfo->Kind = LKind;
                        TInfo->SubKind = LInfo->SubKind;
                    }
                    else if (TypesEqual(*LInfo, *RInfo))
                    {
                        TInfo->Kind = LInfo->Kind;
                        TInfo->SubKind = LInfo->SubKind;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::ADD_ASSIGN:
                {
                    if (
                        LKind == TypeKind::MONO_STATE ||
                        RKind == TypeKind::MONO_STATE
                    )
                    {
                        TInfo->Kind = LKind;
                        TInfo->SubKind = LInfo->SubKind;
                    }
                    else if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
                    {
                        SetNumberResult();
                    }
                    else if (LKind == TypeKind::STRING && RKind == TypeKind::STRING)
                    {
                        TInfo->Kind = TypeKind::STRING;
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::SUB_ASSIGN:
                case Operator::MUL_ASSIGN:
                case Operator::DIV_ASSIGN:
                case Operator::MOD_ASSIGN:
                case Operator::POWER_ASSIGN:
                {
                    if (
                        LKind == TypeKind::MONO_STATE ||
                        RKind == TypeKind::MONO_STATE
                    )
                    {
                        TInfo->Kind = LKind;
                        TInfo->SubKind = LInfo->SubKind;
                    }
                    else if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
                    {
                        SetNumberResult();
                    }
                    else
                    {
                        InvalidOperation();
                        TInfo->Kind = TypeKind::UNK;
                    }

                    break;
                }

                case Operator::NONE:
                {
                    TInfo->Kind = TypeKind::MONO_STATE;
                    break;
                }
            }

            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::UNARY:
        {
            UnaryNode& Un = static_cast<UnaryNode&>(*Node);

            TypeInfo* OperandInfo = GetExpressionType
            (Un.Operand, State, Res, Data, Memory);

            switch (Un.Operator)
            {
                case Operator::NOT:
                {
                    if (
                        OperandInfo->Kind != TypeKind::BOOL &&
                        OperandInfo->Kind != TypeKind::MONO_STATE
                    )
                    {
                        OrbitLog::SyntaxLog::SyntaxError(
                            "Semantic",
                            "Invalid Unary Operation",
                            "Expected <BOOL> For <NOT>, But Got: "
                            +SAUtils::GetStringOfKind(OperandInfo->Kind),
                            "Add a Valid Type or Convert",
                            Node->pos.line, Node->pos.collumn
                        );

                        if (!Data.flags.debugMode)
                            OrbitLog::SyntaxLog::ThrowLog(Data);

                        TInfo->Kind = TypeKind::UNK;
                        break;
                    }

                    TInfo->Kind = TypeKind::BOOL;
                    TInfo->SubKind = SubTypeKind::NONE;
                    break;
                }

                default:
                {
                    TInfo->Kind = TypeKind::UNK;
                    break;
                }
            }

            Res.ExpressionTypes[Node] = *TInfo;
            return &Res.ExpressionTypes[Node];
        }

        case NodeType::RANGE:
        {
            TInfo->Kind = TypeKind::ITERATOR;
            break;
        }

        case NodeType::FN_CALL:
        {
            TInfo->Kind = TypeKind::MONO_STATE;
            break;
        }

        case NodeType::ASSIGNMENT:
        {
            TInfo->Kind = TypeKind::MONO_STATE;
            break;
        }

        default:
        {
            TInfo->Kind = TypeKind::UNK;
            break;
        }
    }

    Res.ExpressionTypes[Node] = *TInfo;
    return &Res.ExpressionTypes[Node];
}

// ===== PROGRAM ===== //

// Entry In A New Scope | Entra em um novo Escopo.
Scope* EntryScope(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Mem)
{
    Scope* S = Mem.New<Scope>();

    S->Parent = State.CurrScope;
    S->Type = Node.Type;
    S->Owner = &Node;

    if (State.CurrScope)
        State.CurrScope->Next = S;

    State.CurrScope = S;
    State.ScopeStack.push_back(S);

    return S;
}

// Leave The Current Scope | Sai do Escopo Atual.
Scope* LeaveScope(SAState& State, SAResult& Res, RunTimeData& Data)
{
    if (!State.CurrScope)
        return nullptr;

    if (State.CurrScope->Type == BodyTypes::PROGRAM)
    {
        OrbitLog::Warn(
            "semantic_analisys.cpp",
            "Trying to Close the GlobalScope In(line/index): ~/~"
        );
        return State.CurrScope;
    }

    if (State.ScopeStack.empty())
        return State.CurrScope;

    State.ScopeStack.pop_back();

    if (State.ScopeStack.empty())
    {
        State.CurrScope = nullptr;
        return nullptr;
    }

    State.CurrScope = State.ScopeStack.back();

    return State.CurrScope;
}

// LookUp A General Node | Olha Um Node Geral.
void SemanticAnalizer::LookUpNode(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner, ParseResult* PRes)
{
    if (Data.flags.generateLog)
        State.NodesChecked.push_back({++State.logInd, &Node});

    switch (Node.Type)
    {

        // PROGRAM
        case NodeType::PROGRAM:
            LookUpProgram
            (static_cast<ProgramNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::BODY:
            LookUpBody
            (static_cast<BodyNode&>(Node), State, Res, Data, Memory);
            break;

        // SPECIALS
        case NodeType::LIBRARY:
            LookUpLibraryDef
            (static_cast<LibraryNode&>(Node), State, *PRes, Res, Data, Memory);
            break;
        case NodeType::IMPORT:
            LookUpImport
            (static_cast<ImportNode&>(Node), State, *PRes, Res, Data, Memory);
            break;

        // CONTROLS
        case NodeType::WHILE:
            LookUpWhile
            (static_cast<WhileNode&>(Node), State, Res, Data, Memory, Owner);
            break;
        case NodeType::FOR:
            LookUpFor
            (static_cast<ForNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::IF_CONTROL:
            LookUpIf
            (static_cast<IfNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::ECHO:
            LookUpEcho
            (static_cast<EchoNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        // DECLARATIONS
        case NodeType::VAR_DECL:
            LookUpVarDecl
            (static_cast<VarDeclNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::FN_DECL:
            LookUpFunction
            (static_cast<FnDecl&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::NAMESPACE_DECL:
            LookUpNameSpace
            (static_cast<NameSpaceDecl&>(Node), State, Res, Data, Memory, Owner);
            break;        

        // EXPRESSIONS
        case NodeType::LITERAL:
            LookUpLiteral
            (static_cast<LiteralNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::IDENTIFIER:
            LookUpIdentifier
            (static_cast<IdentifierNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::UNARY:
            LookUpUnary
            (static_cast<UnaryNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::BINARY:
            LookUpBinary
            (static_cast<BinaryNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::ASSIGNMENT:
            LookUpAssignment
            (static_cast<AssignmentNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::MEMBER_ACCESS:
            LookUpMemberAccess
            (static_cast<MemberAccessNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::INDEX_ACCESS:
            LookUpIndexAccess
            (static_cast<IndexAccessNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::RANGE:
            LookUpRange
            (static_cast<RangeNode&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::FN_CALL:
            LookUpFunctionCall
            (static_cast<FunctionCall&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::ARRAY_VALUE:
            LookUpArray
            (static_cast<ArrayValue&>(Node), State, Res, Data, Memory, Owner);
            break;

        case NodeType::TABLE_VALUE:
            LookUpTable
            (static_cast<TableValue&>(Node), State, Res, Data, Memory, Owner);
            break;

        default: {};
    }
}

// LookUp a ProgramNode | Olha um ProgramNode.
void SemanticAnalizer::LookUpProgram(ProgramNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    Scope* GlobalScope = Memory.New<Scope>();

    GlobalScope->Owner = nullptr;
    GlobalScope->Type = BodyTypes::PROGRAM;

    State.ScopeStack.push_back(GlobalScope);
    State.CurrScope = GlobalScope;

    Res.GlobalScope = GlobalScope;

    for (ASTNode* N : Node.Node->Data)
    {
        if (N)
            LookUpNode(*N, State, Res, Data, Memory);
    }
}

// LookUp BodyNode | Olha um BodyBode.
void SemanticAnalizer::LookUpBody(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Scope* S)
{
    if (!S)
        EntryScope(Node, State, Res, Data, Memory);

    for (ASTNode* N : Node.Data)
    {
        if (N)
            LookUpNode(*N, State, Res, Data, Memory);
    }

    LeaveScope(State, Res, Data);
}

// ===== SPECIALS ===== //

// LookUp Library Definitions Nodes | Olha Nós de Definição da Biblioteca.
void SemanticAnalizer::LookUpLibraryDef(LibraryNode& Node, SAState& State, ParseResult& Res, SAResult& SARes, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    if (State.Flags.libraryDefined)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "<LIBRARY>'s Already Defined", 
            "<LIBRARY> Statement Is Defined BEFORE These Statement", 
            "Remove Statement",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;        
    }
    if (State.Flags.importsDefined or State.Flags.methodDefined)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "<LIBRARY>'s Already Defined After <IMPORT> Statement", 
            "<LIBRARY> Statement CAN ONLY Stay BEFORE <IMPORT> Statement", 
            "Move <LIBRARY> Definition To A Valid Place",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
    Symbol* Sym = 
            SAUtils::CreateSymbol(Node.Name, Node, State, SARes, Memory);
    Sym->Type   = 
            SymbolTypes::LIBRARIE;
    State.Flags.libraryDefined 
                = true;
}

// LookUp Importation Nodes | Olha Um Nó de Importações.
void SemanticAnalizer::LookUpImport(ImportNode& Node, SAState& State, ParseResult& Res, SAResult& SARes, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error Prev | Prevenção de Erros.
    if (State.CurrScope->Type != BodyTypes::PROGRAM)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic", 
            "Import Statement WithOut <PROGRAM> Node", 
            "Imports ONLY  Can Say In <GLOBAL-SCOPE>",
            "Move <IMPORT> Statement To A Valid Place",
            Node.pos.line, Node.Path->pos.collumn 
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    // Take Data | Pega os Dados.
    bool finded=false;
    string Name;
    if (Node.Base->Type == NodeType::IDENTIFIER)
    {
        Name = static_cast<IdentifierNode*>(Node.Base)->Name;
    } else 
        Name = static_cast<IdentifierNode*>(static_cast<MemberAccessNode*>(Node.Base)->Object)->Name;

    // Already Import Warns | Avisos de 'Ja Importado'.
    for (ImportNode* N : Res.ImportRefs)
    {
        if (N == &Node)
        {
            if (finded)
            {
                OrbitLog::SyntaxLog::SyntaxWarn(
                    "Semantic", 
                    "Lib Already Imported", 
                    "Lib: '"+Name+"' Already Have A Valid Reference In Import Data",
                    "Remove Import Statement", 
                    Node.pos.line, Node.pos.collumn
                );
            } else finded = !finded;
        }
    }

    // Try Find Library | Tenta Encontrar A biblioteca.
    OrbitLibrary* FoundLib = nullptr;
    for (OrbitLibrary* Lib : Data.Librarys)
    {
        if (Lib->Name == Name)
        {
            FoundLib = Lib;
            break;
        }
    }

    if (FoundLib)
    {
        // Dependence Detector | Detector de Dependencias
        if (
            std::find
            (
                Data.ImportStack.begin(), 
                Data.ImportStack.end(),
                Name
            ) != Data.ImportStack.end()
        )
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic", 
                "Import Dependence Detected", 
                "Import: "+Name+" Has Been Imported",
                "Remove Import Or change The Project Architeture",
                Node.pos.line, Node.pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return;
        }

        // Set Data | Define os Dados.
        Data.ImportStack.push_back(Name);
        vec<OrbitLibrary*> Libs = std::move(Data.Librarys);
        Data.Librarys.clear();

        // Instantiate Steps | Instancia os Passos.
        Lexer            L;
        Tokenizer        T;
        Parser           P;
        SemanticAnalizer SA;

        // Open File | Abre o Arquivo.
        fstream file(FoundLib->MainFile);
        if (!file.is_open())
        {
            OrbitLog::Error("semantic_analisys.hpp", "Cannot Open Library, Returning", false, 1);
            Data.Librarys = std::move(Libs);
            Data.ImportStack.pop_back();
            
            return;
        }

        // Parse Lib | Parseia a Biblioteca.
        LexResult   LR = L.InitL(file, Data, Memory);
        LR = T.InitT(LR, Data, Memory);
        ParseResult PR = P.InitP(LR, Data, Memory);
        SA.InitSA(PR, Data, Memory);

        // Set Data | Define os Dados.
        Res.Contexts.push_back(std::move(PR));
        Data.Librarys = std::move(Libs);
        Data.ImportStack.pop_back();
    }

    State.Flags.importsDefined=true;
}

// ===== CONTROLS ===== //

// LookUp For Nodes | Olha um ForNode.
void SemanticAnalizer::LookUpFor(ForNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Init | Inicio
    LookUpRange(*Node.End, State, Res, Data, Memory);
    
    Scope* S = EntryScope(*Node.Body, State, Res, Data, Memory);
    Symbol* Sym = SAUtils::CreateSymbol
        (Node.Identifier->Name, Node, State, Res, Memory);

    // Set Symbol Def Config | Define a Configuração Padrao dos Simbolos.    
    Sym->Type               = SymbolTypes::PARAM;
    Sym->TInfo->Kind        = TypeKind::NUMBER;
    Sym->TInfo->SubKind     = SubTypeKind::INT;
    Sym->InferType->Kind    = TypeKind::NUMBER;
    Sym->InferType->SubKind = SubTypeKind::INT;
    Sym->Mut                = MutableTypes::MUT;
    Sym->inited             = true;
    Node.Identifier->SymbolId = Sym->Id;

    // LookUp The 'For' Body | Olha o Corpo do 'For'.
    LookUpBody(*Node.Body, State, Res, Data, Memory, S);
}

// LookUp While Nodes | Olha um WhileNode.
void SemanticAnalizer::LookUpWhile(WhileNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error & Warn Prev | Prevenção de Erros de Avisos.
    TypeInfo* TInfo = 
        GetExpressionType(Node.Cond, State, Res, Data, Memory);
    if (TInfo->Kind != TypeKind::BOOL)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <BOOLEAN> Condition",
            "While Needs a <BOOL> Condition to Check",
            "Add a Valid Type or Convert",
            Node.Cond->pos.line,
            Node.Cond->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    } else if (TInfo->SubKind ==  SubTypeKind::TRUE) {

        OrbitLog::SyntaxLog::SyntaxWarn(
            "Semantic",
            "Condition Maybe Always is <TRUE>",
            "Semantic Analizer Check <TRUE> In All Ways, Skiping...",
            "use 'do' Modifier Instead", Node.Cond->pos.line, Node.Cond->pos.collumn
        );
    }

    LookUpBody(*Node.Body, State, Res, Data, Memory);
}

// LookUp Else Nodes | Olha um ElseNode.
void SemanticAnalizer::LookUpElse(ElseNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{ LookUpBody(*Node.Body, State, Res, Data, Memory); }

// LookUp Elif Nodes | Olha um ElifNode.
void SemanticAnalizer::LookUpElif(ElifNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error & Warn Prev | Prevenção de Erros de Avisos.
    TypeInfo* TInfo = 
        GetExpressionType(Node.Cond, State, Res, Data, Memory);
    if (TInfo->Kind != TypeKind::BOOL)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <BOOLEAN> Condition",
            "While Needs a <BOOL> Condition to Check",
            "Add a Valid Type or Convert",
            Node.Cond->pos.line,
            Node.Cond->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    } else if (TInfo->SubKind ==  SubTypeKind::TRUE)
        OrbitLog::SyntaxLog::SyntaxWarn(
            "Semantic",
            "Condition Maybe Always is <TRUE>",
            "Semantic Analizer Check <TRUE> In All Ways, Skiping...",
            "~", Node.Cond->pos.line, Node.Cond->pos.collumn
        );

    LookUpBody(*Node.Body, State, Res, Data, Memory);
}

// LookUp If Nodes | Olha um IfNode.
void SemanticAnalizer::LookUpIf(IfNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error & Warn Prev | Prevenção de Erros de Avisos.
    TypeInfo* TInfo = 
        GetExpressionType(Node.Cond, State, Res, Data, Memory);
    if (TInfo->Kind != TypeKind::BOOL)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <BOOLEAN> Condition",
            "While Needs a <BOOL> Condition to Check",
            "Add a Valid Type or Convert",
            Node.Cond->pos.line,
            Node.Cond->pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    } else if (TInfo->SubKind ==  SubTypeKind::TRUE)
    {
        Node.alwaysTrue=true;
        OrbitLog::SyntaxLog::SyntaxWarn(
            "Semantic",
            "Condition Maybe Always is <TRUE>",
            "Semantic Analizer Check <TRUE> In All Ways, Skiping...",
            "~", Node.Cond->pos.line, Node.Cond->pos.collumn
        );
    }

    LookUpBody(*Node.IfBody, State, Res, Data, Memory);
    for (ElifNode* Elif : Node.ElifBodyStack)
        LookUpElif(*Elif, State, Res, Data, Memory);
    if (Node.ElseBody)
        LookUpElse(*Node.ElseBody, State, Res, Data, Memory);
}

// LookUp Return Nodes | Olha um Return Node.
void SemanticAnalizer::LookUpReturn(ReturnNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    LookUpNode(*Node.Value, State, Res, Data, Memory);
    if (Node.isIf)
        if (GetExpressionType(Node.Value, State, Res, Data, Memory)->Kind != TypeKind::BOOL)
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Expected <BOOLEAN> Condition",
                "ReturnIfs Needs a <BOOL> Condition to Check",
                "Add a Valid Type or Convert",
                Node.Value->pos.line,
                Node.Value->pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return;
        }
}

// LookUp Echo Nodes | Olha Nós de Echo.
void SemanticAnalizer::LookUpEcho(EchoNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{ LookUpNode(*Node.Value, State, Res, Data, Memory); }

// ===== DECLARATIONS ===== //

// LookUp Var Decl Node | Olha um VarDeclNode.
void SemanticAnalizer::LookUpVarDecl(VarDeclNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Shadowing Error Prev | Prevenção de Erros de Sombreamento.
    if (!State.CurrScope)
        return;

    if (State.CurrScope->FindSymLocal(Node.Name))
    {
        try {

            int i = Node.Name.back() - '0';
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                Node.Name+" Already Exists",
                "Shadowing Is ONLY Allowed In Diff Scopes",
                "Change Name, Ex: "+Node.Name+std::to_string(++i),
                Node.pos.line, Node.pos.collumn
            );
        } catch (...) {
            OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            Node.Name+" Already Exists",
            "Shadowing Is ONLY Allowed In Diff Scopes",
            "Change Name, Ex: "+Node.Name+"2",
            Node.pos.line, Node.pos.collumn
            );
        }
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    // ERROR PREV | PREVENÇÃO DE ERROS.
    if (Node.Val)
        LookUpNode(*Node.Val, State, Res, Data, Memory);

    TypeInfo* ValTInfo = Node.Val
        ? GetExpressionType(Node.Val, State, Res, Data, Memory)
        : nullptr;

    {
        auto err = [&]()
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                SAUtils::GetStringOfKind(SAUtils::GetKindOfLiteral(Node.InferType)->Kind)+" Expected, But Got: "
                +SAUtils::GetStringOfKind(ValTInfo ? ValTInfo->Kind : TypeKind::UNK),
                "Have Diff BeetWeen <INFER_TYPE> And <RECIVED_TYPE>",
                "Add a Valid Type or Convert",
                Node.pos.line, Node.pos.collumn
            );

            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        };

        if (Node.InferType != LiteralTypes::MONO_STATE && ValTInfo)
        {
            TypeInfo* ExpectedTInfo = SAUtils::GetKindOfLiteral(Node.InferType);

            if (
                Node.InferType >= LiteralTypes::ARRAY_INT &&
                Node.InferType <= LiteralTypes::ARRAY_NULL
            )
            {
                if (ValTInfo->Kind != TypeKind::ARRAY)
                {
                    err();
                }
                else if (Node.Val->Type == NodeType::ARRAY_VALUE)
                {
                    ArrayValue& Array = static_cast<ArrayValue&>(*Node.Val);

                    TypeInfo ExpectedElementType;
                    ExpectedElementType.Kind = TypeKind::UNK;
                    ExpectedElementType.SubKind = SubTypeKind::NONE;

                    switch (Node.InferType)
                    {
                        case LiteralTypes::ARRAY_INT:
                        {
                            ExpectedElementType.Kind = TypeKind::NUMBER;
                            ExpectedElementType.SubKind = SubTypeKind::INT;
                            break;
                        }

                        case LiteralTypes::ARRAY_FLOAT:
                        {
                            ExpectedElementType.Kind = TypeKind::NUMBER;
                            ExpectedElementType.SubKind = SubTypeKind::FLOAT;
                            break;
                        }

                        case LiteralTypes::ARRAY_STRING:
                        {
                            ExpectedElementType.Kind = TypeKind::STRING;
                            break;
                        }

                        case LiteralTypes::ARRAY_BOOL:
                        {
                            ExpectedElementType.Kind = TypeKind::BOOL;
                            break;
                        }

                        case LiteralTypes::ARRAY_NONE:
                        {
                            ExpectedElementType.Kind = TypeKind::NONE;
                            break;
                        }

                        case LiteralTypes::ARRAY_NULL:
                        {
                            ExpectedElementType.Kind = TypeKind::_NULL;
                            break;
                        }

                        default:
                            break;
                    }

                    for (ExpressionNode* Arg : Array.Args)
                    {
                        if (!Arg)
                            continue;

                        TypeInfo* ArgTInfo = GetExpressionType
                        (Arg, State, Res, Data, Memory);

                        if (ArgTInfo->Kind == TypeKind::MONO_STATE)
                            continue;

                        if (!TypesEqual(*ArgTInfo, ExpectedElementType))
                        {
                            err();
                            break;
                        }
                    }
                }
            }
            else if (
                Node.InferType >= LiteralTypes::TABLE_INT &&
                Node.InferType <= LiteralTypes::TABLE_NULL
            )
            {
                if (ValTInfo->Kind != TypeKind::TABLE)
                {
                    err();
                }
                else if (Node.Val->Type == NodeType::TABLE_VALUE)
                {
                    TableValue& Table = static_cast<TableValue&>(*Node.Val);

                    TypeInfo ExpectedValueType;
                    ExpectedValueType.Kind = TypeKind::UNK;
                    ExpectedValueType.SubKind = SubTypeKind::NONE;

                    switch (Node.InferType)
                    {
                        case LiteralTypes::TABLE_INT:
                        {
                            ExpectedValueType.Kind = TypeKind::NUMBER;
                            ExpectedValueType.SubKind = SubTypeKind::INT;
                            break;
                        }

                        case LiteralTypes::TABLE_FLOAT:
                        {
                            ExpectedValueType.Kind = TypeKind::NUMBER;
                            ExpectedValueType.SubKind = SubTypeKind::FLOAT;
                            break;
                        }

                        case LiteralTypes::TABLE_STRING:
                        {
                            ExpectedValueType.Kind = TypeKind::STRING;
                            break;
                        }

                        case LiteralTypes::TABLE_BOOL:
                        {
                            ExpectedValueType.Kind = TypeKind::BOOL;
                            break;
                        }

                        case LiteralTypes::TABLE_NONE:
                        {
                            ExpectedValueType.Kind = TypeKind::NONE;
                            break;
                        }

                        case LiteralTypes::TABLE_NULL:
                        {
                            ExpectedValueType.Kind = TypeKind::_NULL;
                            break;
                        }

                        default:
                            break;
                    }

                    for (ArrayEntry& Entry : Table.Args)
                    {
                        if (!Entry.Value)
                            continue;

                        TypeInfo* ValueTInfo = GetExpressionType
                        (Entry.Value, State, Res, Data, Memory);

                        if (ValueTInfo->Kind == TypeKind::MONO_STATE)
                            continue;

                        if (!TypesEqual(*ValueTInfo, ExpectedValueType))
                        {
                            err();
                            break;
                        }
                    }
                }
            }
            else if (ValTInfo)
            {
                if (
                    ValTInfo->Kind != TypeKind::MONO_STATE &&
                    !TypesEqual(*ValTInfo, *ExpectedTInfo)
                )
                {
                    err();
                }
            }
        }
    }

    // Symbol | Simbolo.
    Symbol* Sym = SAUtils::CreateSymbol(Node.Name, Node, State, Res, Memory);

    Sym->Mut = Node.MutType;
    Sym->Type = SymbolTypes::VAR;

    TypeInfo* InferType = SAUtils::GetKindOfLiteral(Node.InferType);

    // Take Kind
    Sym->InferType->Kind = InferType->Kind;
    Sym->InferType->SubKind = InferType->SubKind;
    if (SAUtils::InObjScope(State))
        Sym->LinkedScope = SAUtils::InObjScope(State);

    // Type Prev | Prevenção de Tipos.
    if (!Node.Val)
    {
        Sym->TInfo->Kind = TypeKind::_NULL;
        Sym->TInfo->SubKind = SubTypeKind::NONE;
        Sym->inited = false;

        // Se veio como NONE (default do parser) ou MONO_STATE, força NULL
        if (Node.InferType == LiteralTypes::NONE || Node.InferType == LiteralTypes::MONO_STATE)
        {
            Sym->InferType->Kind = TypeKind::_NULL;
            Sym->InferType->SubKind = SubTypeKind::NONE;
        }
    }
    else if (Node.InferType == LiteralTypes::MONO_STATE)
    {
        Sym->TInfo->Kind = ValTInfo->Kind;
        Sym->TInfo->SubKind = ValTInfo->SubKind;
        Sym->inited = true;
    }
    else
    {
        Sym->TInfo->Kind = Sym->InferType->Kind;
        Sym->TInfo->SubKind = Sym->InferType->SubKind;
        Sym->inited = true;
    }
}

// LookUp Fn Decl Node | Olha um FunctionDeclNode.
void SemanticAnalizer::LookUpFunction(FnDecl& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    if (!State.CurrScope)
        return;

    if (State.CurrScope->FindSymbol(Node.Name, SymbolTypes::FN))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "<FN> Name Already Exists",
            "Fn: "+Node.Name+" Has Been Declared",
            "Add a Diff Name",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    Symbol* FnSym = SAUtils::CreateSymbol(Node.Name, Node, State, Res, Memory);

    FnSym->Type = SymbolTypes::FN;
    FnSym->TInfo->Kind = TypeKind::FN;
    FnSym->TInfo->SubKind = SubTypeKind::NONE;
    FnSym->InferType->Kind = TypeKind::FN;
    FnSym->InferType->SubKind = SubTypeKind::NONE;
    FnSym->inited = true;
    if (SAUtils::InObjScope(State))
        FnSym->LinkedScope = SAUtils::InObjScope(State);

    Scope* PreviousScope = State.CurrScope;
    BodyNode* Body = Node.Body;

    if (!Body)
        return;

    Scope* S = EntryScope(*Body, State, Res, Data, Memory);

    for (ExpressionNode* Parm : Node.Params)
    {
        if (!Parm)
            continue;

        if (Parm->Type == NodeType::IDENTIFIER)
        {
            IdentifierNode& Id = static_cast<IdentifierNode&>(*Parm);
            if (State.CurrScope->FindSymLocal(Id.Name))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Parameter Already Exists",
                    "Param: '"+Id.Name+"' Has Been Declared",
                    "Add a Diff Name",
                    Parm->pos.line, Parm->pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                continue;
            }

            Symbol* ParamSym = SAUtils::CreateSymbol
            (Id.Name, *Parm, State, Res, Memory);

            ParamSym->Type               = SymbolTypes::PARAM;
            ParamSym->Mut                = MutableTypes::MUT;
            ParamSym->TInfo->Kind        = TypeKind::MONO_STATE;
            ParamSym->TInfo->SubKind     = SubTypeKind::NONE;
            ParamSym->InferType->Kind    = TypeKind::MONO_STATE;
            ParamSym->InferType->SubKind = SubTypeKind::NONE;
            ParamSym->inited             = true;
            Id.SymbolId                  = ParamSym->Id;
        }
        else
        {
            LookUpNode(*Parm, State, Res, Data, Memory);
        }
    }

    LookUpBody(*Body, State, Res, Data, Memory, S);
    State.CurrScope = PreviousScope;
}

// LookUp NameSpace Decl Node | Olha um NameSpaceDeclNode.
void SemanticAnalizer::LookUpNameSpace(NameSpaceDecl& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Check Redeclaration | Verifica Redeclaração.
    Symbol* Existing = State.CurrScope->FindSymLocal(Node.Name);
    if (Existing)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Namespace Redeclaration",
            "Namespace '" + Node.Name + "' Already Declared in This Scope",
            "Use a Different Name or Nest It",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    // Create Symbol | Cria o Simbolo.
    Symbol* NsSym = SAUtils::CreateSymbol(Node.Name, Node, State, Res, Memory);
    NsSym->Type               = SymbolTypes::NAMESPACE;
    NsSym->TInfo->Kind        = TypeKind::NAMESPACE;
    NsSym->TInfo->SubKind     = SubTypeKind::NONE;
    NsSym->InferType->Kind    = TypeKind::NAMESPACE;
    NsSym->InferType->SubKind = SubTypeKind::NONE;
    NsSym->inited             = true;
    // Create Scope for Namespace | Cria Escopo do Namespace.
    Scope* NsScope     = Memory.New<Scope>();
    NsScope->Parent    = State.CurrScope;
    NsScope->Owner     = &Node;
    NsScope->Type      = BodyTypes::NAMESPACE;

    // Enter Scope | Entra no Escopo.
    Scope* PreviousScope = State.CurrScope;
    State.ScopeStack.push_back(NsScope);
    State.CurrScope = NsScope;
    NsSym->LinkedScope = SAUtils::InObjScope(State);
    
    // Analyze Body | Analisa o Corpo.
    if (Node.Body)
    {
        LookUpBody(*Node.Body, State, Res, Data, Memory, NsScope);
        // LookUpBody já fez LeaveScope. Só garantimos o CurrScope (mesmo padrão do LookUpFunction).
        State.CurrScope = PreviousScope;
    }
    else
    {
        // Exit Scope | Sai do Escopo. (só quando não tem body)
        State.CurrScope = PreviousScope;
        State.ScopeStack.pop_back();
    }
}

// ===== EXPRESSIONS ===== //

// LookUp LiteralNode | Olha um LiteralNode.
void SemanticAnalizer::LookUpLiteral(LiteralNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    GetExpressionType(&Node, State, Res, Data, Memory);
}

// LookUp IdentifierNode | Olha um IdentifierNode.
void SemanticAnalizer::LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error Prev | Prevenção de Erros.
    bool found = false;
    if (Owner && Owner->LinkedScope)
        found = Owner->LinkedScope->FindSymLocal(Node.Name) != nullptr;
    else if (State.CurrScope)
        found = State.CurrScope->FindSym(Node.Name) != nullptr;
    if (!found)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Used a Undeclared <IDENTIFIER>",
            "Ident: '"+Node.Name+"' Dont Exists",
            "Declare Ident or Use a Valid Identifier",
            Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    // Data
    Symbol* Sym = State.CurrScope->FindSym(Node.Name);
    if (!Sym and Owner)
        Sym = Owner->LinkedScope->FindSym(Node.Name);

    if (Sym)
    {
        Node.SymbolId = Sym->Id;
        Sym->read_count++;
    }
}

// LookUp Binary Node | Olha um BinaryNode.
void SemanticAnalizer::LookUpBinary(BinaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    LookUpNode(*Node.L, State, Res, Data, Memory);
    LookUpNode(*Node.R, State, Res, Data, Memory);

    TypeInfo* LInfo = GetExpressionType(Node.L, State, Res, Data, Memory);
    TypeInfo* RInfo = GetExpressionType(Node.R, State, Res, Data, Memory);

    TypeKind LKind = LInfo->Kind;
    TypeKind RKind = RInfo->Kind;

    auto IsComparable = [](TypeKind Kind) -> bool
    {
        return Kind != TypeKind::UNK &&
            Kind != TypeKind::MONO_STATE &&
            Kind != TypeKind::NONE &&
            Kind != TypeKind::FN;
    };

    auto InvalidOperation = [&]()
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Invalid Operation",
            "Trying to Make A Expr Whit Invalid Operands: " +
            SAUtils::GetStringOfKind(LKind) +
            " And: " +
            SAUtils::GetStringOfKind(RKind),
            "Add a Valid Type or Convert",
            Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
    };

    switch (Node.Op)
    {
        // ARITHMETIC | ARITMETICOS.
        case Operator::ADD:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE
            ) {}
            else if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) {}
            else if (LKind == TypeKind::STRING && RKind == TypeKind::STRING) {}
            else if (LKind == TypeKind::NUMBER && RKind == TypeKind::STRING) {}
            else if (LKind == TypeKind::STRING && RKind == TypeKind::NUMBER) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        case Operator::SUB:
        case Operator::MUL:
        case Operator::DIV:
        case Operator::MOD:
        case Operator::POWER:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        // COMPARISON | COMPARAÇOES.
        case Operator::EQUAL:
        case Operator::NOT_EQUAL:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                (IsComparable(LKind) && IsComparable(RKind))
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        case Operator::LESS:
        case Operator::GREATER:
        case Operator::LESS_EQUAL:
        case Operator::GREATER_EQUAL:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        // LOGICAL | LOGICOS.
        case Operator::AND:
        case Operator::OR:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                (LKind == TypeKind::BOOL && RKind == TypeKind::BOOL)
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        case Operator::NOT:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                LKind == TypeKind::BOOL
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        // ASSIGN | ASSIGNAÇÃO.
        case Operator::ASSIGN:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                TypesEqual(*LInfo, *RInfo)
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        case Operator::ADD_ASSIGN:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) ||
                (LKind == TypeKind::STRING && RKind == TypeKind::STRING)
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        case Operator::SUB_ASSIGN:
        case Operator::MUL_ASSIGN:
        case Operator::DIV_ASSIGN:
        case Operator::MOD_ASSIGN:
        case Operator::POWER_ASSIGN:
        {
            if (
                LKind == TypeKind::MONO_STATE ||
                RKind == TypeKind::MONO_STATE ||
                (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
            ) {}
            else
            {
                InvalidOperation();
                return;
            }

            break;
        }

        case Operator::NONE:
        {
            break;
        }
    }
}

// LookUp Unary Nodes | Olha um UnaryNode.
void SemanticAnalizer::LookUpUnary(UnaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    if (Node.Operand)
        LookUpNode(*Node.Operand, State, Res, Data, Memory);

    switch (Node.Operator) {

        case Operator::NOT:
        {
            TypeKind Kind = GetExpressionType
            (Node.Operand, State, Res, Data, Memory)->Kind;

            if (
                Kind != TypeKind::BOOL &&
                Kind != TypeKind::MONO_STATE
            )
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Invalid <UNAARY>",
                    "<NOT> Operator ONLY Can Be In <BOOL> Operations",
                    "Add a Valid Type or Convert",
                    Node.pos.line, Node.pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                return;
            }

            return;
        }

        default:
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid <UNARY>",
                "Operator DONT Can be a Unary Operator",
                "~", Node.pos.line, Node.pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return;
    }
}

// LookUp Assign Node | Olha um Assign Node.
void SemanticAnalizer::LookUpAssignment(AssignmentNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error Prev | Prevenção de Erros.
    if (!SAUtils::IsIValue(Node.Left))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            " Expected",
            "Trying To Assign A Non  Value",
            "~", Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return;
    }

    // Data
    LookUpNode(*Node.Left, State, Res, Data, Memory);
    if (Node.Left->Type == NodeType::INDEX_ACCESS)
    {
        IndexAccessNode& Index = static_cast<IndexAccessNode&>(*Node.Left);

        LookUpNode(*Node.Right, State, Res, Data, Memory);

        TypeInfo* ObjInfo = GetExpressionType
        (Index.Object, State, Res, Data, Memory);

        TypeInfo* RightInfo = GetExpressionType
        (Node.Right, State, Res, Data, Memory);

        if (
            ObjInfo->Kind != TypeKind::ARRAY &&
            ObjInfo->Kind != TypeKind::TABLE &&
            ObjInfo->Kind != TypeKind::MONO_STATE
        )
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid Index Assignment Object",
                "Expected <ARRAY>/<TABLE>, But Got: "+SAUtils::GetStringOfKind(ObjInfo->Kind),
                "Use an <ARRAY> or <TABLE> to Assign an Index",
                Index.Object->pos.line, Index.Object->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return;
        }

        if (ObjInfo->Kind == TypeKind::ARRAY)
        {
            TypeInfo ExpectedIndex;
            ExpectedIndex.Kind = TypeKind::NUMBER;
            ExpectedIndex.SubKind = SubTypeKind::INT;

            TypeInfo* IndexInfo = GetExpressionType
            (Index.Index, State, Res, Data, Memory);

            if (!TypesEqual(*IndexInfo, ExpectedIndex))
                return;
        }

        if (ObjInfo->Kind == TypeKind::TABLE)
        {
            TypeInfo StringIndex;
            StringIndex.Kind = TypeKind::STRING;
            StringIndex.SubKind = SubTypeKind::NONE;

            TypeInfo IntIndex;
            IntIndex.Kind = TypeKind::NUMBER;
            IntIndex.SubKind = SubTypeKind::INT;

            TypeInfo* IndexInfo = GetExpressionType
            (Index.Index, State, Res, Data, Memory);

            if (
                !TypesEqual(*IndexInfo, StringIndex) &&
                !TypesEqual(*IndexInfo, IntIndex)
            )
                return;
        }

        (void)RightInfo;
        return;
    }

    string N = SAUtils::GetIValueName(Node.Left);

    if (N == "UNKNOW")
        return;

    Symbol* Sym = State.CurrScope
        ? State.CurrScope->FindSym(N)
        : nullptr;

    if (!Sym)
        return;
    else if (Node.Left) Node.Left->SymbolId = Sym->Id;
    if (Sym->Mut == MutableTypes::CONST)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Trying to Assign a <CONSTANT> Value",
            N+" Is Declared as Constant in(line/index): "+std::to_string(Sym->Pos.line)+"/"+std::to_string(Sym->Pos.collumn),
            "~", Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return;
    }

    LookUpNode(*Node.Right, State, Res, Data, Memory);

    TypeInfo* RightInfo = GetExpressionType
    (Node.Right, State, Res, Data, Memory);

    if (
        Sym->InferType &&
        Sym->InferType->Kind != TypeKind::MONO_STATE &&
        RightInfo->Kind != TypeKind::MONO_STATE
    )
    {
        if (!TypesEqual(*RightInfo, *Sym->InferType))
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                SAUtils::GetStringOfKind(Sym->InferType->Kind)+" Expected",
                "Have A Difference BeetWeen Type and Expected",
                "Add a Valid Type or Convert",
                Node.pos.line, Node.pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return;
        }
    }

    if (
        Sym->InferType &&
        Sym->InferType->Kind == TypeKind::MONO_STATE &&
        RightInfo->Kind != TypeKind::MONO_STATE
    )
    {
        Sym->TInfo->Kind = RightInfo->Kind;
        Sym->TInfo->SubKind = RightInfo->SubKind;
    }

    Sym->inited = true;
    Sym->write_count++;
}

// LookUp Member Acess Node | Olha um Member Acess Node.
void SemanticAnalizer::LookUpMemberAccess(MemberAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error Prevention | Prevenção De Erros.
    if (!Node.Object)
        return;

    LookUpNode(*Node.Object, State, Res, Data, Memory, Owner);
    Symbol* Sym = nullptr;

    // ERR PREV | PREVENÇÃO DE ERROS:
    if (Node.Object->SymbolId != 0)
    {
        auto It = Res.Symbols.find(Node.Object->SymbolId);
        if (It != Res.Symbols.end())
            Sym = It->second;
    }

    if (!Sym)
    {
        string N = SAUtils::GetIValueName(Node.Object);

        if (N == "UNKNOW")
            return;
        if (Owner && Owner->LinkedScope)
            Sym = Owner->LinkedScope->FindSymLocal(N);
        else if (State.CurrScope)
            Sym = State.CurrScope->FindSym(N);
    }

    if (!Sym)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Parsing", 
            "Trying to Acess a Undeclared Object", 
            "<OBJECT> Cannot Be Acessed, Because Dont Exists",
            "Declare <OBJECT>",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    // Set Data
    Sym->read_count++;
    Node.Object->SymbolId = Sym->Id;

    // Main Switch | Switch Principal.
    switch (Sym->Type) 
    {
    
        case SymbolTypes::NAMESPACE:
        {
            if (Node.Member)
                LookUpNode(*Node.Member, State, Res, Data, Memory, Sym);

            if (Node.Member && Node.Member->SymbolId != 0)
                Node.SymbolId = Node.Member->SymbolId;

            break;
        }
        case SymbolTypes::STRUCT:
        {
            if (Node.Member)
                LookUpNode(*Node.Member, State, Res, Data, Memory, Sym);

            if (Node.Member && Node.Member->SymbolId != 0)
                Node.SymbolId = Node.Member->SymbolId;

            break;
        }
        case SymbolTypes::CLASS:
        {
            if (Node.Member)
                LookUpNode(*Node.Member, State, Res, Data, Memory, Sym);

            if (Node.Member && Node.Member->SymbolId != 0)
                Node.SymbolId = Node.Member->SymbolId;

            break;
        }
        default:
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic", 
                "Trying to Acess A Non-Object", 
                "<OBJECT>: "+Node.Object->GetNodeType()+" Cannot Be Acessed, Because  Dont Have Members",
                "Use A Valid <OBJECT>",
                Node.pos.line, Node.pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
            return;
        }
    }
}

// LookUp IndexAcessNode | Olha um IndexAcessNode.
void SemanticAnalizer::LookUpIndexAccess(IndexAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    // Error Prev | Prevenção de Erros.
    if (!Node.Object || !Node.Index)
        return;

    LookUpNode(*Node.Object, State, Res, Data, Memory);
    LookUpNode(*Node.Index, State, Res, Data, Memory);

    TypeInfo* ObjInfo = GetExpressionType(Node.Object, State, Res, Data, Memory);
    TypeInfo* IndexInfo = GetExpressionType(Node.Index, State, Res, Data, Memory);

    if (
        ObjInfo->Kind == TypeKind::MONO_STATE ||
        IndexInfo->Kind == TypeKind::MONO_STATE
    )
        return;

    if (ObjInfo->Kind == TypeKind::ARRAY)
    {
        TypeInfo ExpectedIndex;
        ExpectedIndex.Kind = TypeKind::NUMBER;
        ExpectedIndex.SubKind = SubTypeKind::INT;

        if (!TypesEqual(*IndexInfo, ExpectedIndex))
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid Index value Type",
                "Expected <INT> For <ARRAY>, But Got: "+SAUtils::GetStringOfKind(IndexInfo->Kind),
                "Add a Valid Type Or Convert",
                Node.Index->pos.line, Node.Index->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return;
        }
    }
    else if (ObjInfo->Kind == TypeKind::TABLE)
    {
        TypeInfo StringIndex;
        StringIndex.Kind = TypeKind::STRING;
        StringIndex.SubKind = SubTypeKind::NONE;

        TypeInfo IntIndex;
        IntIndex.Kind = TypeKind::NUMBER;
        IntIndex.SubKind = SubTypeKind::INT;

        if (
            !TypesEqual(*IndexInfo, StringIndex) &&
            !TypesEqual(*IndexInfo, IntIndex)
        )
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid Index value Type",
                "Expected <STRING/INT> For <TABLE>, But Got: "+SAUtils::GetStringOfKind(IndexInfo->Kind),
                "Add a Valid Type Or Convert",
                Node.Index->pos.line, Node.Index->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            return;
        }
    }
    else if (ObjInfo->Kind == TypeKind::STRING)
    {
        TypeInfo ExpectedIndex;
        ExpectedIndex.Kind = TypeKind::NUMBER;
        ExpectedIndex.SubKind = SubTypeKind::INT;

        if (!TypesEqual(*IndexInfo, ExpectedIndex))
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid Index value Type",
                "Expected <INT> For <STRING>, But Got: "+SAUtils::GetStringOfKind(IndexInfo->Kind),
                "Add a Valid Type Or Convert",
                Node.Index->pos.line, Node.Index->pos.collumn
            );
            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
            return;
        }
    }
    else
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Invalid Index Object",
            "Expected <ARRAY>/<TABLE>, But Got: "+SAUtils::GetStringOfKind(ObjInfo->Kind),
            "Use an <ARRAY> or <TABLE> to Access an Index",
            Node.Object->pos.line, Node.Object->pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return;
    }
}

// LookUp Range Nodes | Olha os RangeNodes.
void SemanticAnalizer::LookUpRange(RangeNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    TypeInfo* BeginInfo = GetExpressionType
    (Node.Begin, State, Res, Data, Memory);

    TypeInfo* EndInfo = GetExpressionType
    (Node.End, State, Res, Data, Memory);

    TypeInfo ExpectedInt;
    ExpectedInt.Kind = TypeKind::NUMBER;
    ExpectedInt.SubKind = SubTypeKind::INT;

    if (
        BeginInfo->Kind != TypeKind::MONO_STATE &&
        !TypesEqual(*BeginInfo, ExpectedInt)
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <INT> To Make a <BEGIN> Iterator",
            "<BEGIN> Needs a <INT> START  to Iterate",
            "Add a Valid Type or Convert",
            Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return;
    }

    if (
        EndInfo->Kind != TypeKind::MONO_STATE &&
        !TypesEqual(*EndInfo, ExpectedInt)
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <INT> To Make a <END> Iterator",
            "<END> Needs a <INT> FINAL to Iterate",
            "Add a Valid Type or Convert",
            Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return;
    }
}

// LookUp Fn Call Node | Olha um FnCallNode.
void SemanticAnalizer::LookUpFunctionCall(FunctionCall& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    LookUpNode(*Node.Callee, State, Res, Data, Memory);

    for (ExpressionNode* Arg : Node.Args)
    {
        if (Arg)
            LookUpNode(*Arg, State, Res, Data, Memory);
    }
}

// LookUp Array Values | Olha um ArrayValue.
void SemanticAnalizer::LookUpArray(ArrayValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    for (ExpressionNode* Val : Node.Args)
    {
        if (Val)
            LookUpNode(*Val, State, Res, Data, Memory);
    }
}

// LookUp Table Values | Olha um TableValue.
void SemanticAnalizer::LookUpTable(TableValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory, Symbol* Owner)
{
    for (ArrayEntry& E : Node.Args)
    {
        // Error Prev | Prevenção de Erros:
        if (!E.Key || !E.Value)
            continue;

        LookUpNode(*E.Key, State, Res, Data, Memory);
        LookUpNode(*E.Value, State, Res, Data, Memory);

        TypeInfo* KeyInfo = GetExpressionType
        (E.Key, State, Res, Data, Memory);

        TypeInfo StringKey;
        StringKey.Kind = TypeKind::STRING;
        StringKey.SubKind = SubTypeKind::NONE;

        TypeInfo IntKey;
        IntKey.Kind = TypeKind::NUMBER;
        IntKey.SubKind = SubTypeKind::INT;

        if (
            KeyInfo->Kind != TypeKind::MONO_STATE &&
            !TypesEqual(*KeyInfo, StringKey) &&
            !TypesEqual(*KeyInfo, IntKey)
        )
        {
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Expected <STRING/INT>, But Got: "+SAUtils::GetStringOfKind(KeyInfo->Kind),
                "<TABLE>s Need a <STRING/INT> To Refer a Value",
                "Add a Valid Type or Convert",
                E.Key->pos.line, E.Key->pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);
        }
    }
}

// ========== ENTRY-POINT ========== //

// Generate Log of SA Program | Gera o Log da Analise Semantica
void GenerateSALog(SAState& State, SAResult& Res, RunTimeData& Data)
{
    fstream file(Data.LogDir, std::ios::out | std::ios::app);
    string t1 = "\n\n// ========== SEMANTIC ANALYSIS ========= //\n\n";
    file << t1;

    int Spaces = 0;

    for (pair<int, ASTNode*>& N : State.NodesChecked)
        if (N.second && N.second->GetNodeType().size() > Spaces)
            Spaces = N.second->GetNodeType().size();

    for (pair<int, ASTNode*>& N : State.NodesChecked)
    {
        if (!N.second)
            continue;

        string text = "Node["+std::to_string(N.first)+"]: "+N.second->GetNodeType();

        for (int i = N.second->GetNodeType().size(); i < Spaces; i++)
            text += " ";

        text += "  | Pos(line, index): "+std::to_string(N.second->pos.line)+":"+std::to_string(N.second->pos.collumn);
        file << text << "\n";
    }

    file << "\n\nSYMBOLS: \n\n";

    int i=0;

    if (Res.Symbols.size() > 0)
        for (auto& [id, Sym] : Res.Symbols)
        {
            string text = "SYM["+std::to_string(i)+"]: \n";
            
            text += "NAME: "+string(Sym->Name);
            text += "\n\tID: "+std::to_string(id);
            text += "\n\tKIND: ";
            text += "\n\t\t|  Kind: "+std::to_string(static_cast<int>(Sym->TInfo->Kind));
            text += "\n\t\t|_ Kind: "+std::to_string(static_cast<int>(Sym->TInfo->SubKind));
            text += "\tSCOPE: " + std::format("{:p}", static_cast<void*>(Sym->DeclaredScope));
            text += "\n\tDATA: \n\t\t|  RA: "+std::to_string(Sym->read_count);
            text += "\n\t\t|  WA: "+std::to_string(Sym->write_count);
            text += "\n\t\t|_ INIT: "+std::to_string(Sym->inited)+"\n\n";
            
            file << text;
            i++;
        }
    else
        file << "~ NONE ~";

    string t2 = "\n// ========== ENDOF: 'SEMANTIC ANALYSIS' ========= //\n";
    file << t2;
    file.close();
}

// Entry-Point of SA Program | Ponto-de-Entrada do programa do SA.
SAResult SemanticAnalizer::InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory)
{
    // Debug:
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: SemanticAnalysis");

    // Data | Dados
    SAResult Res;
    SAState State;

    // Dont Have AST to Check | Sem AST Para Checar.
    if (!PRes.AST)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Invalid AST",
            "Semantic Analysis received a NULL AST",
            "Fix the Parser before running Semantic Analysis",
            0, 0
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return Res;
    }

    // LookUp | Olha
    LookUpNode(*PRes.AST, State, Res, Data, Memory);

    // Symbol Final Tratament | Tratamento Final dos Simbolos.
    for (auto& [id, Sym] : Res.Symbols)
    {
        if (Sym->write_count == 0 and Sym->read_count == 0)
        {
            OrbitLog::SyntaxLog::SyntaxWarn(
                "Semantic", 
                "Unused Symbol", 
                "Symbol: "+Sym->Name+" Dont Has Been Used", 
                "Remove Unused  Symbol",
                Sym->Pos.line, Sym->Pos.collumn
            );
        } else if (Sym->write_count == 0)
        {
            OrbitLog::SyntaxLog::SyntaxWarn(
                "Semantic",
                "Non-Const Symbol Has Not ReWrited", 
                "Symbol: "+Sym->Name+" Dont Has Been ReWrited, But Not Has Been Declared As <CONST>", 
                "Add 'const' Flag",
                Sym->Pos.line, Sym->Pos.collumn
            );            
        } else if (!Sym->inited) 
        {
            OrbitLog::SyntaxLog::SyntaxWarn(
                "Semantic",
                "Non-Const Symbol Never Be Initialized", 
                "Symbol: "+Sym->Name+" Dont Has Been Initialized", 
                "Remove 'Symbol'",
                Sym->Pos.line, Sym->Pos.collumn
            );             
        }
    }

    // Flags | Marcações:
    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: SemanticAnalysis. .. ...");
    if (Data.flags.generateLog)
        GenerateSALog(State, Res, Data);

    return Res;
}

// EOF