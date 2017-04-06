module MyParser.Ast

// Ident
//

type VarId = VarId of string
// type FuncId = FuncId of string
type TypeId = TypeId of string
type Keyword = Keyword of string

type Ident =
    | IdentVar of VarId
    // | IdentFunc of FuncId
    | IdentType of TypeId
    | IdentKeyword of Keyword
    | Unknown of string
    with
    static member Value = function
        | IdentVar (VarId id) -> id
        // | IdentFunc (FuncId id) -> id
        | IdentType (TypeId id) -> id
        | IdentKeyword (Keyword id) -> id
        | Unknown id -> id

// Metadata
//
type Type =
    | Bool
    | Int
    | Float
        
type Context =
    { CurrentScope: ScopeDef; ScopeStack: ScopeDef list } with

    static member GetCurrentScope {CurrentScope=result} = Scope result
        
    static member OpenScope ctx =
        let currentScope = ctx |> Context.GetCurrentScope
        let emptyScopeDef = {Parent=currentScope; Vars=Map.empty}

        { 
            CurrentScope=emptyScopeDef; 
            ScopeStack=ctx.CurrentScope::ctx.ScopeStack;
        }

    static member CloseScope ctx =
        match ctx with
        | {ScopeStack=fst::rest} ->
            {
                CurrentScope = fst;
                ScopeStack = rest;
            }
        | _ ->
            failwith "assertion failure"

    static member DefineVar type' id ctx =
        let {CurrentScope=scope} = ctx

        let var = {Type=type'; Name=id}
        let newScope = {scope with Vars=Map.add id var scope.Vars}
        let newContext = {ctx with CurrentScope=newScope}

        newContext

    static member LookupVar id ctx =
        let rec findVar scopes id =
            match scopes with
            | [] -> None
            | fst::rest ->
                match fst |> ScopeDef.LookupVar id with
                | Some _ as result -> result
                | None -> findVar rest id

        let {CurrentScope=x; ScopeStack=xs} = ctx
        findVar (x::xs) id
    
    static member LookupType (TypeId id) (ctx:Context) =
        match id with
            | "bool" -> Some Bool
            | "int" -> Some Int
            | "float" -> Some Float
            | _ -> None

and ScopeDef =
    { Parent: Scope; Vars: Map<VarId, Variable> } with
    
    static member LookupVar id scope =
        let {Vars=vars} = scope in vars |> Map.tryFind id

and Scope =
    | RootScope
    | Scope of ScopeDef

and Variable = {Type: Type; Name: VarId}

// Ast
//

type Expr =
    | BoolConst of bool
    | IntConst of int
    | FloatConst of float
    // | CharConst of char
    // | StringConst of string
    | NamedExpr of Variable
    | PrefixExpr of op:string * operand:Expr
    | InfixExpr of op:string * lhs:Expr * rhs:Expr
    | TypeExpr of op:string * operand:Expr * type':Type
    with
    static member Type = function
        | BoolConst _ -> Bool
        | IntConst _ -> Int
        | FloatConst _ -> Float
        | NamedExpr {Type=type'} -> type'
        | PrefixExpr (op, oprand) ->
            match op with
            | "+" | "-" | "!" -> Expr.Type oprand
            | _ -> failwith "assertion failure"
        | InfixExpr (op, _, rhs) ->
            match op with
            | "||" | "&&" | "==" | "!=" 
            | "<" | ">" | ">=" | "<=" 
                -> Bool
            | "+" | "-" | "*" | "/" 
                -> Expr.Type rhs
            | "%" 
                -> Int
            | _ -> failwith "assertion failure"
        | TypeExpr (op, _, type') ->
            match op with
            | "is" -> Bool
            | "as" -> type'
            | _ -> failwith "assertion failure"



type Stmt =
    | DeclStmt of name:VarId * typeAnnot:Type * init:Expr
    | LetStmt of name:Variable * expr:Expr
    | IfStmt of choice:Expr * yes:Stmt * no:Stmt option
    | WhileStmt of pred:Expr * body:Stmt
    | CompoundStmt of scope: Scope * body:Stmt list

