
// ============= SEMANTIC ANALIZER =========== //
// Analyzes the Code for Semantic Errors | Analiza o Codigo em Busca de Erros Semanticos.
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
#include <cstddef>
#include <cwchar>
#include <string>

// ========= UTILS ========== //

// Semantic Analysis Utils | Utilidades do Analisador.
namespace SAUtils {

    // Create A New Symbol | Cria um Novo Simbolo.
    Symbol* CreateSymbol(const string& Name, ASTNode& Node, SAState& State, SAResult& Res, Arena& Memory)
    {
        Symbol* Sym = Memory.New<Symbol>();
        TypeInfo* TInfo = Memory.New<TypeInfo>();

        Sym->Name = Name;
        Sym->DeclaredScope = State.CurrScope;
        Sym->TInfo = TInfo;
        Sym->Pos = Node.pos;

        return Sym;
    }

    // Return String Version of Kind.
    string GetStringOfKind(TypeKind K)
    {
        switch (K)
        {
            case TypeKind::UNK:			return "<UNK>";
            case TypeKind::MONO_STATE:	return "<MONO_STATE>";

            case TypeKind::NUMBER:		return "<NUMBER>";
            case TypeKind::STRING:		return "<STRING>";
            case TypeKind::BOOL:		return "<BOOL>";
            case TypeKind::ANY:			return "<ANY>";
            case TypeKind::_NULL:		return "<NULL>";
            case TypeKind::NONE:		return "<NONE>";

            case TypeKind::ARRAY:		return "<ARRAY>";
            case TypeKind::TABLE:		return "<TABLE>";

            case TypeKind::TABLE_INT:	return "<TABLE_INT>";
            case TypeKind::TABLE_FLOAT:	return "<TABLE_FLOAT>";
            case TypeKind::TABLE_STRING:return "<TABLE_STRING>";
            case TypeKind::TABLE_BOOL:	return "<TABLE_BOOL>";
            case TypeKind::TABLE_ANY:	return "<TABLE_ANY>";
            case TypeKind::TABLE_NULL:	return "<TABLE_NULL>";
            case TypeKind::TABLE_NONE:	return "<TABLE_NONE>";

            case TypeKind::STRUCT:		return "<STRUCT>";
            case TypeKind::CLASS:		return "<CLASS>";

            case TypeKind::FN:			return "<FN>";

            default:					return "<UNKNOWN>";
        }
    }

    // Return Name of IValues | Retorna o Nome dos I-Values.
    string GetIValueName(ExpressionNode* Node)
    {
        if (!Node)
            return "UNKNOW";

        switch (Node->Type) {
        
            case NodeType::IDENTIFIER:
            {
                IdentifierNode* Id = static_cast<IdentifierNode*>(Node);
                return Id->Name;
            }

            case NodeType::MEMBER_ACCESS:
            {
                MemberAccessNode* Ma = static_cast<MemberAccessNode*>(Node);
                return GetIValueName(Ma->Object);
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

        return false;
    }

}

// ========== CORE ========== //

// ===== PROGRAM ===== //

// Get Expression Types | Pega o Tipo das Expressoes.
TypeInfo* GetExpressionType(ExpressionNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    TypeInfo* TInfo = Memory.New<TypeInfo>();

    switch (Node.Type) 
    {
    
        case NodeType::LITERAL:
        {
            LiteralNode& Lit = static_cast<LiteralNode&>(Node);

            if (holds_alt<i64>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SybTypeKind::INT;
            }
            else if (holds_alt<float>(Lit.Value))
            {
                TInfo->Kind = TypeKind::NUMBER;
                TInfo->SubKind = SybTypeKind::FLOAT;
            }
            else if (holds_alt<str_view>(Lit.Value))
            {
                TInfo->Kind = TypeKind::STRING;
            }
            else if (holds_alt_value<bool>(Lit.Value, true))
            {
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SybTypeKind::TRUE;
            }
            else if (holds_alt_value<bool>(Lit.Value, false))
            {
                TInfo->Kind = TypeKind::BOOL;
                TInfo->SubKind = SybTypeKind::FALSE;
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

            Res.ExpressionTypes[&Node] = *TInfo;
            return TInfo;
        }

        case NodeType::ARRAY_VALUE:
        {
            TInfo->Kind = TypeKind::ARRAY;

            Res.ExpressionTypes[&Node] = *TInfo;
            return TInfo;
        }

        case NodeType::TABLE_VALUE:
        {
            TInfo->Kind = TypeKind::TABLE;

            Res.ExpressionTypes[&Node] = *TInfo;
            return TInfo;
        }

        case NodeType::MEMBER_ACCESS:
            TInfo->Kind = TypeKind::MONO_STATE;
            break;

        case NodeType::INDEX_ACCESS:
        {
            IndexAccessNode& Index = static_cast<IndexAccessNode&>(Node);

            TypeInfo* ObjInfo = GetExpressionType
            (*Index.Object, State, Res, Data, Memory);

            TypeInfo* IndexInfo = GetExpressionType
            (*Index.Index, State, Res, Data, Memory);

            if (ObjInfo->Kind == TypeKind::ARRAY)
            {
                if (
                    IndexInfo->Kind != TypeKind::NUMBER ||
                    IndexInfo->SubKind != SybTypeKind::INT
                )
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
                    Res.ExpressionTypes[&Node] = *TInfo;
                    return TInfo;
                }

                TInfo->Kind = TypeKind::MONO_STATE;

                Res.ExpressionTypes[&Node] = *TInfo;
                return TInfo;
            }

            if (ObjInfo->Kind == TypeKind::TABLE)
            {
                if (
                    IndexInfo->Kind != TypeKind::STRING &&
                    (
                        IndexInfo->Kind != TypeKind::NUMBER ||
                        IndexInfo->SubKind != SybTypeKind::INT
                    )
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
                    Res.ExpressionTypes[&Node] = *TInfo;
                    return TInfo;
                }

                TInfo->Kind = TypeKind::MONO_STATE;

                Res.ExpressionTypes[&Node] = *TInfo;
                return TInfo;
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
            Res.ExpressionTypes[&Node] = *TInfo;
            return TInfo;
        }
        
        case NodeType::IDENTIFIER:
        {
            IdentifierNode& Id = static_cast<IdentifierNode&>(Node);

            Symbol* Sym = State.CurrScope
                ? State.CurrScope->FindSym(Id.Name)
                : nullptr;

            if (!Sym)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Used a Undeclared <IDENTIFIER>",
                    "Ident: "+Id.Name+" Dont Exists",
                    "Declare Ident or Use a Valid Identifier",
                    Node.pos.line, Node.pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                TInfo->Kind = TypeKind::UNK;
                Res.ExpressionTypes[&Node] = *TInfo;
                return TInfo;
            }

            if (!Sym->inited)
            {
                TInfo->Kind = TypeKind::NONE;
                Res.ExpressionTypes[&Node] = *TInfo;
                return TInfo;
            }
            
            Sym->read_count++;

            Res.ExpressionTypes[&Node] = *Sym->TInfo;
            return Sym->TInfo;
        }

        case NodeType::BINARY:
		{
			BinaryNode& Binary = static_cast<BinaryNode&>(Node);

			TypeInfo* LInfo = GetExpressionType
			(*Binary.L, State, Res, Data, Memory);

			TypeInfo* RInfo = GetExpressionType
			(*Binary.R, State, Res, Data, Memory);

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

				if (!Data.flags.debugMode)
					OrbitLog::SyntaxLog::ThrowLog(Data);
			};

			auto SetNumberResult = [&]()
			{
				TInfo->Kind = TypeKind::NUMBER;

				if (
					LInfo->SubKind == SybTypeKind::FLOAT ||
					RInfo->SubKind == SybTypeKind::FLOAT
				)
				{
					TInfo->SubKind = SybTypeKind::FLOAT;
				}
				else
				{
					TInfo->SubKind = SybTypeKind::INT;
				}
			};

			switch (Binary.Op)
			{
				// ARITHMETIC | ARITMETICOS.
				case Operator::ADD:
				{
					if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
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
					if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
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
					if (IsComparable(LKind) && IsComparable(RKind))
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
					if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
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
					if (LKind == TypeKind::BOOL && RKind == TypeKind::BOOL)
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
					if (LKind == TypeKind::BOOL)
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
					if (IsComparable(LKind) && IsComparable(RKind))
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
					if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
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
					if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER)
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

			Res.ExpressionTypes[&Node] = *TInfo;
			return TInfo;
		}

        case NodeType::UNARY:
        {
            UnaryNode& Un = static_cast<UnaryNode&>(Node);
            switch (Un.Operator) {
            
                case Operator::NOT:
                    TInfo = GetExpressionType(*Un.Operand, State, Res, Data, Memory);
                    if (TInfo->SubKind == SybTypeKind::TRUE)
                        TInfo->SubKind = SybTypeKind::FALSE;
                    else TInfo->SubKind = SybTypeKind::TRUE;
                default: TInfo->Kind = TypeKind::UNK;
            }
            break;
        }

        case NodeType::RANGE:
            TInfo->Kind = TypeKind::ITERATOR; break;

        case NodeType::FN_CALL:
            TInfo->Kind = TypeKind::MONO_STATE; break;

        case NodeType::MEMBER_ACCESS:
        {
            MemberAccessNode& Ma = static_cast<MemberAccessNode&>(Node);

            if (!SAUtils::IsIValue(Ma.Object))
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "<I-VALUE> Expected, But Got: "+Ma.GetNodeType(),
                    "<MEMBER_CESS> Need a VALID Thing to Acess",
                    "~", Ma.pos.line, Ma.pos.collumn
                );

                if (!Data.flags.debugMode)
                    OrbitLog::SyntaxLog::ThrowLog(Data);

                TInfo->Kind = TypeKind::UNK;
                Res.ExpressionTypes[&Node] = *TInfo;
                return TInfo;
            }

            TypeInfo* ObjInfo = GetExpressionType
            (*Ma.Object, State, Res, Data, Memory);

            if (ObjInfo->Kind == TypeKind::UNK)
            {
                TInfo->Kind = TypeKind::UNK;
                Res.ExpressionTypes[&Node] = *TInfo;
                return TInfo;
            }

            string N = SAUtils::GetIValueName(Ma.Object);

            Symbol* Sym = nullptr;

            if (Ma.Object->Type == NodeType::IDENTIFIER)
            {
                Sym = State.CurrScope
                    ? State.CurrScope->FindSym(N)
                    : nullptr;

                if (!Sym)
                {
                    OrbitLog::SyntaxLog::SyntaxError(
                        "Semantic",
                        "Used of Undeclared Identifier: "+N,
                        "Member Acess Need a <I-VALUE> To Acess",
                        "~", Ma.Object->pos.line, Ma.Object->pos.collumn
                    );

                    if (!Data.flags.debugMode)
                        OrbitLog::SyntaxLog::ThrowLog(Data);

                    TInfo->Kind = TypeKind::UNK;
                    Res.ExpressionTypes[&Node] = *TInfo;
                    return TInfo;
                }
            }

            if (Ma.Object->Type == NodeType::MEMBER_ACCESS)
            {
                for (pair<string, TypeInfo*> P : Sym ? Sym->Objs : vec<pair<string, TypeInfo*>>{})
                {
                    string MemberName = SAUtils::GetIValueName(Ma.Member);
                    if (P.first == MemberName)
                    {
                        TInfo->Kind = P.second->Kind;
                        TInfo->SubKind = P.second->SubKind;

                        Res.ExpressionTypes[&Node] = *TInfo;
                        return TInfo;
                    }
                }
            }
            else if (Sym)
            {
                for (pair<string, TypeInfo*> P : Sym->Objs)
                {
                    string MemberName = SAUtils::GetIValueName(Ma.Member);
                    if (P.first == MemberName)
                    {
                        TInfo->Kind = P.second->Kind;
                        TInfo->SubKind = P.second->SubKind;

                        Res.ExpressionTypes[&Node] = *TInfo;
                        return TInfo;
                    }
                }
            }

            string MemberName = SAUtils::GetIValueName(Ma.Member);
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Cannot Find Member: "+MemberName,
                "Member does not Exist in: "+N,
                "~", Ma.pos.line, Ma.pos.collumn
            );

            if (!Data.flags.debugMode)
                OrbitLog::SyntaxLog::ThrowLog(Data);

            TInfo->Kind = TypeKind::UNK;
            Res.ExpressionTypes[&Node] = *TInfo;
            return TInfo;
        }

        default: {};
    }

    Res.ExpressionTypes[&Node] = *TInfo;
    return TInfo;
}

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
void SemanticAnalizer::LookUpNode(ASTNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.generateLog)
        State.NodesChecked.push_back({++State.logInd, &Node});

    switch (Node.Type) {
        
        // PROGRAM
        case NodeType::PROGRAM:  
            LookUpProgram
            (static_cast<ProgramNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::BODY:
            LookUpBody
            (static_cast<BodyNode&>(Node), State, Res, Data, Memory);
            break;

        // EXPRESSIONS
        case NodeType::LITERAL:
            LookUpLiteral
            (static_cast<LiteralNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::IDENTIFIER:
            LookUpIdentifier
            (static_cast<IdentifierNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::MEMBER_ACCESS:
        {
            MemberAccessNode& Ma = static_cast<MemberAccessNode&>(Node);

            GetExpressionType
            (Ma, State, Res, Data, Memory);

            break;
        }

        case NodeType::INDEX_ACCESS:
            LookUpIndexAccess
            (static_cast<IndexAccessNode&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::ARRAY_VALUE:
            LookUpArray
            (static_cast<ArrayValue&>(Node), State, Res, Data, Memory);
            break;

        case NodeType::TABLE_VALUE:
            LookUpTable
            (static_cast<TableValue&>(Node), State, Res, Data, Memory);
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
void SemanticAnalizer::LookUpBody(BodyNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    EntryScope(Node, State, Res, Data, Memory);

    for (ASTNode* N : Node.Data)
    {
        if (N)
            LookUpNode(*N, State, Res, Data, Memory);
    }

    LeaveScope(State, Res, Data);
}

// ===== EXPRESSIONS ===== //

// LookUp LiteralNode | Olha um LiteralNode.
void SemanticAnalizer::LookUpLiteral(LiteralNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    GetExpressionType(Node, State, Res, Data, Memory);
}

// LookUp IdentifierNode | Olha um IdentifierNode.
void SemanticAnalizer::LookUpIdentifier(IdentifierNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Error Prev | Prevenção de Erros.
    if (!State.CurrScope || !State.CurrScope->FindSym(Node.Name))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Used a Undeclared <IDENTIFIER>",
            "Ident: "+Node.Name+" Dont Exists",
            "Declare Ident or Use a Valid Identifier",
            Node.pos.line, Node.pos.collumn
        );

        if (!Data.flags.debugMode)
            OrbitLog::SyntaxLog::ThrowLog(Data);

        return;
    }
}

// LookUp Binary Node | Olha um BinaryNode.
void SemanticAnalizer::LookUpBinary(BinaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
	LookUpNode(*Node.L, State, Res, Data, Memory);
	LookUpNode(*Node.R, State, Res, Data, Memory);

	TypeKind LKind = GetExpressionType(*Node.L, State, Res, Data, Memory)->Kind;
	TypeKind RKind = GetExpressionType(*Node.R, State, Res, Data, Memory)->Kind;

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
			if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) {}
			else if (LKind == TypeKind::STRING && RKind == TypeKind::STRING) {}
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
			if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) {}
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
			if (IsComparable(LKind) && IsComparable(RKind)) {}
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
			if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) {}
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
			if (LKind == TypeKind::BOOL && RKind == TypeKind::BOOL) {}
			else
			{
				InvalidOperation();
				return;
			}

			break;
		}

		case Operator::NOT:
		{
			if (LKind == TypeKind::BOOL) {}
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
			if (IsComparable(LKind) && IsComparable(RKind)) {}
			else
			{
				InvalidOperation();
				return;
			}

			break;
		}

		case Operator::ADD_ASSIGN:
		{
			if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) {}
			else if (LKind == TypeKind::STRING && RKind == TypeKind::STRING) {}
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
			if (LKind == TypeKind::NUMBER && RKind == TypeKind::NUMBER) {}
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
void SemanticAnalizer::LookUpUnary(UnaryNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    switch (Node.Operator) {
    
        case Operator::NOT:

            if (GetExpressionType(*Node.Operand, State, Res, Data, Memory)->Kind != TypeKind::BOOL)
            {
                OrbitLog::SyntaxLog::SyntaxError(
                    "Semantic",
                    "Invalid <UNAARY>",
                    "<NOT> Operator ONLY Can Be In <BOOL> Operations",
                    "Add a Valid Type or Convert",
                    Node.pos.line, Node.pos.collumn
                );
                if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
                return;
            }
        default:
            OrbitLog::SyntaxLog::SyntaxError(
                "Semantic",
                "Invalid <UNARY>",
                "Operator DONT Can be a Unary Operator",
                "~", Node.pos.line, Node.pos.collumn
            );
            if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
    }
}

// LookUp Assign Node | Olha um Assign Node.
void SemanticAnalizer::LookUpAssignment(AssignmentNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    // Error Prev | Prevenção de Erros.
    if (!SAUtils::IsIValue(Node.Left))
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "<I-VALUE> Expected",
            "Trying To Assign A Non <I-VALUE> Value",
            "~", Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
    // Data
    LookUpNode(*Node.Left, State, Res, Data, Memory);
    string N = SAUtils::GetIValueName(Node.Left);
    Symbol* Sym = State.CurrScope->FindSym(N);
    if (!Sym) // <- Error Prevention | Prevenção de Erros.
        return;
    if (Sym->Mut == MutableTypes::CONST)
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Trying to Assign a <CONSTANT> Value",
            N+" Is Declared as Constant in(line/index): "+std::to_string(Sym->Pos.line)+"/"+std::to_string(Sym->Pos.collumn),
            "~", Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
    if (
        Sym->InferType->Kind 
        != 
        TypeKind::MONO_STATE 
        and
        ( 
            GetExpressionType(*Node.Right, State, Res, Data, Memory)->Kind
            !=
            Sym->InferType->Kind 
            or
            Sym->InferType->SubKind != GetExpressionType(*Node.Right, State, Res, Data, Memory)->SubKind
        )
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            SAUtils::GetStringOfKind(GetExpressionType(*Node.Right, State, Res, Data, Memory)->Kind)+" Expected",
            "Have A Difference BeetWeen Type and Expected",
            "Add a Valid Type or Convert",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }

    Sym->inited=true;
    Sym->write_count++;
}

// LookUp Member Acess Node | Olha um Member Acess Node.
void SemanticAnalizer::LookUpMemberAccess(MemberAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    LookUpNode(*Node.Object, State, Res, Data, Memory);
}

// LookUp IndexAcessNode | Olha um IndexAcessNode.
void SemanticAnalizer::LookUpIndexAccess(IndexAccessNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
	// Error Prev | Prevenção de Erros.
	if (!Node.Object || !Node.Index)
        return;

    LookUpNode(*Node.Object, State, Res, Data, Memory);
    LookUpNode(*Node.Index, State, Res, Data, Memory);

	TypeInfo* ObjInfo = GetExpressionType(*Node.Object, State, Res, Data, Memory);
	TypeInfo* IndexInfo = GetExpressionType(*Node.Index, State, Res, Data, Memory);

	if (ObjInfo->Kind == TypeKind::ARRAY)
	{
		if (
			IndexInfo->Kind != TypeKind::NUMBER ||
			IndexInfo->SubKind != SybTypeKind::INT
		)
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
		if (
			IndexInfo->Kind != TypeKind::STRING &&
			(
				IndexInfo->Kind != TypeKind::NUMBER ||
				IndexInfo->SubKind != SybTypeKind::INT
			)
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
void SemanticAnalizer::LookUpRange(RangeNode& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    if (
        GetExpressionType(*Node.Begin, State, Res, Data, Memory)->Kind
        != TypeKind::MONO_STATE
        or
        GetExpressionType(*Node.Begin, State, Res, Data, Memory)->SubKind 
        != SybTypeKind::
        INT
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <INT> To Make a <BEGIN> Iterator",
            "<BEGIN> Needs a <INT> START  to Iterate",
            "Add a Valid Type or Convert",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
    if (
        GetExpressionType(*Node.End, State, Res, Data, Memory)->Kind
        != TypeKind::MONO_STATE
        or
        GetExpressionType(*Node.End, State, Res, Data, Memory)->SubKind 
        != SybTypeKind::
        INT
    )
    {
        OrbitLog::SyntaxLog::SyntaxError(
            "Semantic",
            "Expected <INT> To Make a <END> Iterator",
            "<END> Needs a <INT> FINAL to Iterate",
            "Add a Valid Type or Convert",
            Node.pos.line, Node.pos.collumn
        );
        if (!Data.flags.debugMode) OrbitLog::SyntaxLog::ThrowLog(Data);
        return;
    }
}

// LookUp Fn Call Node | Olha um FnCallNode.
void SemanticAnalizer::LookUpFunctionCall(FunctionCall& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    LookUpNode(*Node.Callee, State, Res, Data, Memory);
    for (ExpressionNode* Arg : Node.Args)
        LookUpNode(*Arg, State, Res, Data, Memory);
}

// LookUp Array Values | Olha um ArrayValue.
void SemanticAnalizer::LookUpArray(ArrayValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    for (ExpressionNode* Val : Node.Args)
    {
        if (Val)
            LookUpNode(*Val, State, Res, Data, Memory);
    }
}

// LookUp Table Values | Olha um TableValue.
void SemanticAnalizer::LookUpTable(TableValue& Node, SAState& State, SAResult& Res, RunTimeData& Data, Arena& Memory)
{
    for (ArrayEntry E : Node.Args)
    {
        // Error Prev | Prevenção de Erros:
        if (!E.Key || !E.Value)
            continue;

        TypeInfo* KeyInfo = GetExpressionType
        (*E.Key, State, Res, Data, Memory);

        if
        (
            KeyInfo->Kind != TypeKind::STRING &&
            (
                KeyInfo->Kind != TypeKind::NUMBER ||
                KeyInfo->SubKind != SybTypeKind::INT
            )
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

        LookUpNode(*E.Key, State, Res, Data, Memory);
        LookUpNode(*E.Value, State, Res, Data, Memory);
    }
}

// ========== ENTRY-POINT ========== //

// Generate Log of SA Program | Gera o Log da Analise Semantica
void GenerateSALog(SAState& State, SAResult& Res, RunTimeData& Data)
{
	fstream file(Data.LogDir, std::ios::out | std::ios::app);
    string t1 = "\n\n// ========== SEMANTIC ANALYSIS ========= //\n";
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

    string t2 = "\n\n// ========== ENDOF: 'SEMANTIC ANALYSIS' ========= //\n";
    file << t2;
    file.close();
}

// Entry-Point of SA Program | Ponto-de-Entrada do programa do SA.
SAResult SemanticAnalizer::InitSA(ParseResult& PRes, RunTimeData& Data, Arena& Memory)
{
    if (Data.flags.debugMode)
        PrintIn("STARTING TASK: SemanticAnalysis");

    SAResult Res;
    SAState State;

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

    LookUpNode(*PRes.AST, State, Res, Data, Memory);

    if (Data.flags.debugMode)
        PrintIn("ENDOF TASK: SemanticAnalysis. .. ...");

    if (Data.flags.generateLog)
        GenerateSALog(State, Res, Data);

    return Res;
}
