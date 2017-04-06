// requires FParsec
open System
open FParsec

type Expr =
    | BinaryExpr of string * Expr * Expr
    | Number of float

let rec eval = function
    | Number(x) -> x
    | BinaryExpr(op, lhs, rhs) -> 
        let op2func = function
            | "+" -> (+)
            | "-" -> (-)
            | "*" -> (*)
            | "/" -> (/)
            | _ -> failwith "not an operator"
        (op2func op) (eval lhs) (eval rhs)

let ws = spaces
let str_ws s = pstring s .>> ws

let opp = new OperatorPrecedenceParser<Expr, unit, unit>()
opp.TermParser <- (pfloat .>> ws |>> Number)
                  <|> between (str_ws "(") (str_ws ")") opp.ExpressionParser

let addOp prefix prec assoc =
    let op = InfixOperator(prefix, ws, prec, assoc,
                           (fun lhs rhs -> BinaryExpr(prefix, lhs, rhs)))
    opp.AddOperator(op)

addOp "+" 1 Associativity.Left
addOp "-" 1 Associativity.Left
addOp "*" 2 Associativity.Left
addOp "/" 2 Associativity.Left

[<EntryPoint>]
let main argv = 
    match run opp.ExpressionParser "1 + 2*(3 + 4)" with
        | Success(result, _, _) -> printfn "%f" (eval result)
        | Failure(errorMsg, _, _) -> printfn "%s" errorMsg
    
    Console.ReadKey() |> ignore
    0 // return an integer exit code
