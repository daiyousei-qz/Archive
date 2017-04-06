module MyParser.Parser

open MyParser.Ast
open FParsec

// Helper Parser
//
let pstr_ws str = pstring str .>> spaces
let pAnyStr_ws strs = strs |> List.map pstr_ws |> choice

// a wraper for result, to simplify Reply<'TResult>
type Result<'a> =
    | Val of 'a
    | Err of string

// assume associative
let pPrefixExpr opParser termParser fn =
    let rec exprProc (opList, oprand) = 
        match oprand, List.rev opList with
        | term, [] -> preturn term
        | term, op::rest ->
            match fn op term with
            | Val r -> exprProc (rest, r)
            | Err e -> fail e
    
    (many opParser) .>>. termParser >>= exprProc

// assume left-associative
let pInfixExpr2 opParser lhsParser rhsParser fn =
    let rec exprProc = function
        | term, [] -> preturn term
        | lhs, (op, rhs)::rest -> 
            match fn op lhs rhs with
            | Val r -> exprProc (r, rest)
            | Err e -> fail e

    lhsParser .>>. (many (opParser .>>. rhsParser)) >>= exprProc


let pInfixExpr opParser termParser fn =
    pInfixExpr2 opParser termParser termParser fn

// Primary Parsers
//

let keywordParser =
    let keywords = ["let"; "var"; "if";"then";"else"; "while";"do"; 
                    "true"; "false"; "bool"; "int"; "float"; ]

    keywords |> pAnyStr_ws |>> Keyword

let idParser = 
    let isAsciiIdStart ch =
        isAsciiLetter ch || ch = '_'
    let isAsciiIdContinue c =
        isAsciiLetter c || isDigit c || c = '_' || c = '\''
    
    let options = IdentifierOptions(isAsciiIdStart = isAsciiIdStart,
                                    isAsciiIdContinue = isAsciiIdContinue)

    notFollowedBy keywordParser >>. identifier options .>> spaces

let boolConstParser =
        (pstr_ws "true" >>% BoolConst true)
    <|> (pstr_ws "false" >>% BoolConst false)

let numberConstParser = 
    let numberFormat =     NumberLiteralOptions.AllowMinusSign
                       ||| NumberLiteralOptions.AllowFraction
                       ||| NumberLiteralOptions.AllowExponent

    numberLiteral numberFormat "number" .>> spaces |>> 
        fun result -> if result.IsInteger 
                      then IntConst (int32 result.String)
                      else FloatConst (float result.String)

// TODO: exclude keywords

let varParser = 
    let findVar (id, ctx) = 
        match ctx |> Context.LookupVar (VarId id) with
        | Some var -> preturn var
        | None -> pzero
    
    tuple2 idParser getUserState >>= findVar <?> "variable"

let typeParser = 
    let findType (id, ctx) =
        match ctx |> Context.LookupType (TypeId id) with
        | Some type' -> preturn type'
        | None -> fail "Unexptected failure"

    ["bool"; "int"; "float"; ]
        |> pAnyStr_ws .>>. getUserState >>= findType

// Expression Parser
//

let expressionParser = 
    let termParser, termParserRef = createParserForwardedToRef()

    let ensureBinaryArgs title blacklist lhs rhs success failure =
        match [lhs; rhs] |> List.map Expr.Type with
        | [x; y] when x = y ->
            if blacklist |> List.contains x 
            then failure (sprintf "Operands must be %s" title)
            else success ()
        | _ -> 
            failure "Two oprands must share a common type"

    let ensureSameArgs =
        ensureBinaryArgs "" []

    let ensureArithmeticArgs = 
        ensureBinaryArgs "arithmetic" [Bool]
    
    let ensureIntegralArgs =
        ensureBinaryArgs "integral" [Bool; Float]

    let ensureBooleanArgs =
        ensureBinaryArgs "boolean" [Int; Float]

    let infixExprGen op lhs rhs () = Val (InfixExpr (op, lhs, rhs))
    let errorGen msg = Err msg
        
    let infixExprSyntaxChecker syntaxChecker op lhs rhs =
        syntaxChecker lhs rhs
            (infixExprGen op lhs rhs)
            errorGen

    let logicalOrExpr = 
        pInfixExpr (pstr_ws "||") termParser
            (infixExprSyntaxChecker ensureBooleanArgs)

    let logicalAndExpr = 
        pInfixExpr (pstr_ws "&&") logicalOrExpr
            (infixExprSyntaxChecker ensureBooleanArgs)

    let logicalNotExpr = 
        pPrefixExpr (pstr_ws "!") logicalAndExpr
            (fun op oprand ->
                match Expr.Type oprand with
                | Bool -> Val (PrefixExpr (op, oprand))
                | _ -> Err "Expected oprand of type bool"
            )

    let equalityExpr = 
        pInfixExpr (pAnyStr_ws ["=="; "!="]) logicalNotExpr
            (infixExprSyntaxChecker ensureSameArgs)

    let comparisonExpr =
        pInfixExpr (pAnyStr_ws [">"; "<"; ">="; "<="]) equalityExpr
            (infixExprSyntaxChecker ensureArithmeticArgs)

    let testExpr =
        pInfixExpr2 (pstr_ws "is") comparisonExpr typeParser
            (fun op oprand type' -> 
                Val (TypeExpr (op, oprand, type'))
            )

    let additiveExpr =
        pInfixExpr (pAnyStr_ws ["+"; "-"]) testExpr
            (infixExprSyntaxChecker ensureArithmeticArgs)

    let multiplicativeExpr =
        pInfixExpr (pAnyStr_ws ["*"; "/"; "%"]) additiveExpr
            (fun op lhs rhs ->
                let syntaxChecker = 
                    match op with
                    | "*" | "/" ->
                        ensureArithmeticArgs
                    | "%" ->
                        ensureIntegralArgs
                    | _ ->
                        failwith "assertion failure"

                syntaxChecker lhs rhs
                    (infixExprGen op lhs rhs)
                    errorGen
            )

    let castExpr =
        pInfixExpr2 (pstr_ws "as") multiplicativeExpr typeParser
            (fun op oprand type' -> 
                Val (TypeExpr (op, oprand, type'))
            )

    (*
    let unaryExpr =
        pPrefixExpr (pAnyStr_ws ["+"; "-"]) castExpr
            (fun _ operand ->
                // ?
                Err ""
            )
    *)

    do termParserRef := choice[
            boolConstParser;
            numberConstParser;
            varParser |>> NamedExpr;

            between (pstr_ws "(") (pstr_ws ")") logicalOrExpr;
        ]

    // unaryExpr
    castExpr

// Statement Parser
//
let typeAnnotParser =
    pstr_ws ":" >>. typeParser

let statementParser =
    let stmtParser, stmtParserRef = createParserForwardedToRef()

    let declStmtParser =
        tuple4 (pstr_ws "var" >>. idParser)
               (opt typeAnnotParser)
               (pstr_ws "=" >>. expressionParser)
               (getUserState)
               >>= (fun (id, typeAnnot, init, ctx) ->
                        match typeAnnot, Expr.Type init with
                        | (Some x, y) when x<>y ->
                            fail "init expression has different type with annotation"
                        | (_ , y) ->
                            let {CurrentScope=scope} = ctx
                            let varId = VarId id

                            match scope |> ScopeDef.LookupVar varId with
                            | None ->
                                updateUserState (Context.DefineVar y varId)
                                    >>. preturn (DeclStmt (varId, y, init))
                            | Some _ ->
                                fail (sprintf "duplicate definition of %s" id)
                   )

    let letStmtParser =
        tuple2 (pstr_ws "let" >>. varParser)
               (pstr_ws "=" >>. expressionParser)
               >>= (fun ({Type=type'} as var, expr) ->
                        match Expr.Type expr with
                        | t when t=type' ->
                            preturn (LetStmt (var, expr))
                        | _ ->
                            fail "expression has different type with variable definition"
                   )

    let ifStmtParser =
        tuple3 (pstr_ws "if" >>. expressionParser)
               (pstr_ws "then" >>. stmtParser)
               (opt (pstr_ws "else" >>. stmtParser))
               >>= function
                   | (cond, yes, no) when Expr.Type cond = Bool
                        -> preturn (IfStmt (cond, yes, no))
                   | _ 
                        -> fail "type of a if-then condition must be boolean"

    let whileStmtParser =
        tuple2 (pstr_ws "while" >>. expressionParser)
               (pstr_ws "do" >>. stmtParser)
               >>= function 
                   | (pred, body) when Expr.Type pred = Bool ->
                        preturn (WhileStmt (pred, body))
                   | _ -> 
                        fail "type of a while predicate must be boolean"

    let compoundStmtParser =
        let beginScopeParser = 
            pstr_ws "{" >>. updateUserState Context.OpenScope
                
        let endScopeParser =
            pstr_ws "}" >>. updateUserState Context.CloseScope

        let getScopeParser =
            getUserState |>> Context.GetCurrentScope

        between beginScopeParser endScopeParser ((many stmtParser) .>>. getScopeParser)
            |>> (fun (body, scope) -> CompoundStmt (scope, body))

    do stmtParserRef := choice [
        declStmtParser;
        letStmtParser;
        ifStmtParser;
        whileStmtParser;
        compoundStmtParser;
    ]

    stmtParser

let scriptParser = many statementParser