// A simple regex matcher that simulate NFA in F#

open System
open System.Collections.Generic

type CharRange = 
    { MinValue: char; MaxValue: char } with

    member self.Contains ch = 
        ch >= self.MinValue && ch <= self.MaxValue

type RegexExpr =
    | EntityExpr of CharRange
    | SequenceExpr of RegexExpr*RegexExpr list
    | ChoiceExpr of RegexExpr*RegexExpr list
    | StarExpr of RegexExpr

type Transition =
    | EpsilonTransition
    | AcceptTransition
    | RangeTransition of CharRange

type State() = class end

type NfaEdge = { SourceState: State; DestState: State; Transition: Transition }
type NfaStateMachine = { InitialState: State; AccState: State; EdgeLookup: IDictionary<State, NfaEdge list> }

// active pattern recognizer for models
//
let (|NfaEdge|) edge =
    edge.SourceState, edge.DestState, edge.Transition

let (|NfaStateMachine|) nfa =
    nfa.InitialState, nfa.AccState, nfa.EdgeLookup
    
// factories
//
let createNfaEdge trans (src, dest) =
    { SourceState=src; DestState=dest; Transition=trans }

let createEpsilonEdge = createNfaEdge <| EpsilonTransition
let createAcceptEdge = createNfaEdge <| AcceptTransition
let createRangeEdge range = createNfaEdge <| RangeTransition(range)

// NFA builder functions
//
let rec connectNfaBranch regexNode (src, dest) accEdges =
    match regexNode with
    //       rg
    // src ------> dest
    | EntityExpr(range) ->
        (createRangeEdge range (src, dest)) :: accEdges

    //     n1       n2           nn       e
    // src --> mid1 --> mid2 ... --> midn --> dest
    | SequenceExpr(first, rest) ->
        let accEdges2, lastState =
            List.fold (fun (acc, lastState) node -> 
                           let midBranch = lastState, State()
                           (connectNfaBranch node midBranch acc), snd midBranch)
                      (accEdges, src) (first::rest)

        (createEpsilonEdge (lastState, dest)) :: accEdges2

    //     e            ni            e
    // src --> subSrc_i --> subDest_i --> dest
    | ChoiceExpr(first, rest) ->
        let accEdges2, subBranches =
            List.fold (fun (acc, accBranches) node ->
                           let newBranch = State(), State()
                           (connectNfaBranch node newBranch acc), newBranch::accBranches)
                      (accEdges, []) (first::rest)

        let epsilons =
            subBranches
            |> List.collect (fun (subSrc, subDest) -> [ src, subSrc; subDest, dest; ])
            |> List.map createEpsilonEdge

        List.append epsilons accEdges2

    //     e          n           e
    // src --> subSrc --> subDest --> dest
    //  \         ^____e____/          ^
    //   \_____________e______________/
    | StarExpr(child) ->
        let subSrc, subDest = State(), State()
        let epsilons = [ src, subSrc; 
                         subDest, dest;
                         src, dest;
                         subDest, subSrc; ] |> List.map createEpsilonEdge

        connectNfaBranch child (subSrc, subDest) accEdges
        |> List.append epsilons
     
let createEpsilonNfa regexNode =
    let src, dest = State(), State()
    let edgeLookup = 
        connectNfaBranch regexNode (src, dest) []   
        |> List.groupBy (fun x -> x.SourceState)
        |> dict

    { InitialState=src; AccState=dest; EdgeLookup=edgeLookup }

let compactEpsilonNfa nfa =
    let eliminateEpsilonEdges =
        let expandEpsilonOnce =
            List.collect (fun (NfaEdge(src, dest, trans) as edge) ->
                              match trans with
                              // expand epsilon transitions
                              | EpsilonTransition ->
                                  if dest = nfa.AccState
                                  then [createAcceptEdge (src, dest)]
                                  else nfa.EdgeLookup.Item dest 
                                       |> List.map (fun x -> { x with SourceState=src})
                          
                              // duplicate other transitions
                              | _ -> [edge]
                         )

        let isEpsilonEdge = 
            function
            | NfaEdge(_, _, EpsilonTransition) -> true
            | _ -> false

        let rec expansionSeries edges = 
            seq {
                let result = expandEpsilonOnce edges

                yield result
                yield! expansionSeries result
            }

        expansionSeries
        >> Seq.skipWhile (List.exists isEpsilonEdge)
        >> Seq.head

    let newEdgeLookup =
        nfa.EdgeLookup
        |> Seq.map (fun (KeyValue(srcState, edges)) -> 
                        srcState, eliminateEpsilonEdges edges)
        |> dict

    { nfa with EdgeLookup=newEdgeLookup }

//
//
let matchRegex nfa text =
    let feed ch =
        List.collect (function
                      | NfaEdge(_, dest, RangeTransition(rg)) 
                        when rg.Contains ch ->
                            nfa.EdgeLookup.Item dest
                      | _ -> [])

    text
    |> Seq.fold (fun edges ch -> feed ch edges) (nfa.EdgeLookup.Item nfa.InitialState)
    |> List.exists (function | NfaEdge(_, _, AcceptTransition) -> true | _ -> false)


// regex factory
//
let createEntityExpr (min, max) = EntityExpr <| { MinValue=min; MaxValue=max; }

let createSequenceExpr = function
    | x::xs -> SequenceExpr(x, xs)
    | [] -> failwith "empty sequence is not allowed"

let createChoiceExpr = function
    | x::xs -> ChoiceExpr(x, xs)
    | [] -> failwith "empty choice is not allowed"

let createStarExpr = StarExpr

[<EntryPoint>]
let main argv =
    let regex = 
        [ createEntityExpr ('a', 'f');
          createEntityExpr ('0', '9'); ]
        |> createSequenceExpr
        |> createStarExpr

    let nfa =
        regex
        |> createEpsilonNfa
        |> compactEpsilonNfa

    [ "a7";
      "b8c7";
      "cc"; ]
    |> List.map (matchRegex nfa)
    |> printfn "%A"

    Console.ReadKey() |> ignore
    0