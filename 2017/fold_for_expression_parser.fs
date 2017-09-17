open FParsec

// Version 1
let pInfixExprHelper1 opParser lhsParser rhsParser fn =
    let rec exprProc = function
        | term, [] -> preturn term
        | lhs, (op, rhs)::rest -> 
            match fn op lhs rhs with
            | Val r -> exprProc (r, rest)
            | Err e -> fail e

    lhsParser .>>. (many (opParser .>>. rhsParser)) >>= exprProc

// Version 2
let pInfixExprHelper2 opParser lhsParser rhsParser fn =
    let rec procExpr = function
        | term, [] -> 
            term
        | lhs, (op, rhs)::rest ->
            let r = fn op lhs rhs
            procExpr (r, rest)

    lhsParser .>>. (many (opParser .>>. rhsParser)) |>> procExpr

// Version 3
let pInfixExprHelper3 opParser lhsParser rhsParser fn =
    let fnProxy lhs (op, rhs) = fn op lhs rhs

    pipe2 lhsParser (many (opParser .>>. rhsParser))
          (fun term group -> group |> List.fold fnProxy term)