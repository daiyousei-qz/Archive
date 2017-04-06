open System
open MyParser.Ast
open MyParser.Parser
open FParsec

[<EntryPoint>]
let main argv = 
    let scope = {Parent=RootScope; Vars=Map.empty}
    let ctx = {CurrentScope=scope; ScopeStack=[]}

    """
    var x = true is int
    if x then {
        var y = 1
        while y<10 do {
            let y = y+1
        }
    }
    else {
        let x = false
    }
    """.Trim()
    |> runParserOnString scriptParser ctx "script"
    |> printfn "%A"

    Console.ReadKey() |> ignore
    0 // return an integer exit code
