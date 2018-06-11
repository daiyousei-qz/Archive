open System.IO

let calcOffset =
    Seq.map (function | '[' -> 1 | ']' -> -1 | _ -> 0)
    >> Seq.scan (+) 0
    >> Seq.skip 1
    >> Seq.findIndex ((=) 0)

let rec interpret program (data: int[]) dataPtr instPtr =
    let mutable cell = &data.[dataPtr]

    match program |> Array.tryItem instPtr with
    | Some '>' ->                                 1, 1
    | Some '<' ->                                -1, 1
    | Some '+' -> cell <- cell+1;                 0, 1
    | Some '-' -> cell <- cell-1;                 0, 1
    | Some '.' -> stdout.Write(cell |> char);     0, 1
    | Some ',' -> cell <- stdin.Read();           0, 1
    | Some '[' -> 0, if cell=0  then program |> Seq.skip instPtr                |> calcOffset         else 1
    | Some ']' -> 0, if cell<>0 then program |> Seq.take (instPtr+1) |> Seq.rev |> calcOffset |> (~-) else 1
    | Some _   -> 0, 1
    | None     -> exit 0
    |> fun (x, y) -> interpret program data (dataPtr+x) (instPtr+y)

[<EntryPoint>]
let main args =
    let program = File.ReadAllText(args.[0]).ToCharArray()
    let data = Array.init 30000 (fun _ -> 0)
    interpret program data 0 0
    0