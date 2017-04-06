open System
open FParsec

let pstr_ws str = pstring str .>> spaces
let pAnyStr_ws strs = strs |> List.map pstr_ws |> choic

type Result<'a> =
    | Val of 'a
    | Err of string

let pPrefixExpr opParser termParser fn =
    let rec exprProc (opList, oprand) = 
        match oprand, List.rev opList with
        | term, [] -> preturn term
        | term, op::rest ->
            match fn op term with
            | Val r -> exprProc (rest, r)
            | Err e -> fail e
    
    (many opParser) .>>. termParser >>= exprProc

let pInfixExpr opParser termParser fn =
    let rec exprProc = function
        | term, [] -> preturn term
        | lhs, (op, rhs)::rest -> 
            match fn op lhs rhs with
            | Val r -> exprProc (r, rest)
            | Err e -> fail e

    termParser .>>. (many (opParser .>>. termParser)) >>= exprProc

let exprParser = 
    let p0, p0_ref = createParserForwardedToRef()
    
    let p1 = pPrefixExpr (pAnyStr_ws ["+"; "-"]) p0
                (fun op oprand -> 
                    match op with
                    | "+" -> Val oprand
                    | "-" -> Val (-oprand)
                    | _ -> Err ""
                )

    let p2 = pInfixExpr (pAnyStr_ws ["*"; "/"]) p1
                (fun op lhs rhs ->
                    match op with
                    | "*" -> Val (lhs * rhs)
                    | "/" -> Val (lhs / rhs)
                    | _ -> Err ""
                )

    let p3 = pInfixExpr (pAnyStr_ws ["+"; "-"]) p2
                (fun op lhs rhs ->
                    match op with
                    | "+" -> Val (lhs + rhs)
                    | "-" -> Val (lhs - rhs)
                    | _ -> Err ""
                )
    
    do p0_ref := (pfloat .>> spaces) <|> (between (pstr_ws "(") (pstr_ws ")") p3)
    
    p3 .>> eof

[<EntryPoint>]
let main argv = 
    run exprParser "(-(1))+2*--3" |> printfn "%A"

    Console.ReadKey() |> ignore
    0 // return an integer exit code
