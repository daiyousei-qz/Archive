open System
open XPlot.Plotly

// mu: death rate
// mu': birth rate
// beta: contact rate
// gamma: recover rate
// alpha: immunity loss rate
// epsilon: incubation rate
type ModelParameters = 
    | ModelParameters of mu:float * mu':float * beta:float * gamma:float * alpha:float * epsilon:float

type ModelCategory =
    | SI    of beta:float
    | SIS   of beta:float*gamma:float
    | SIR   of beta:float*gamma:float
    | SIRS  of beta:float*gamma:float*alpha:float
    | SEIRS of beta:float*gamma:float*alpha:float*epsilon:float

module Model =
    let createDynamic mu mu' category =
        let beta, gamma, alpha, epsilon =
            match category with
            | SI(b)             -> b, 0., 1., 1.
            | SIS(b, g)         -> b, g , 1., 1.
            | SIR(b, g)         -> b, g , 0., 1.
            | SIRS(b, g, a)     -> b, g , a , 1.
            | SEIRS(b, g, a, e) -> b, g , a , e
    
        ModelParameters(mu, mu', beta, gamma, alpha, epsilon)

    let createStatic =
        createDynamic 0. 0.

module Simulation =
    type EntityState =
        | Susceptible
        | Exposed
        | Infected
        | Recovered

    let reproduce infectShare population n model (rand: Random) =
        let mu, mu', beta, gamma, alpha, epsilon =
            match model with | ModelParameters(m, m', b, g, a, e) -> m, m', b, g, a, e

        // sample and test a probability
        let test prob =
            rand.NextDouble() < prob
        
        // iterate on a sample
        let transformSample (s, e, i, r) state =
            // total number of population
            let n = float (s+e+i+r)
            // next state of the sample
            match state with
            | Susceptible ->
                if test <| (beta * float i) / n
                then (if epsilon<1. then Exposed else Infected)
                else Susceptible
            | Exposed ->
                if test <| epsilon
                then Infected
                else Exposed
            | Infected ->
                if test <| gamma
                then (if alpha<1. then Recovered else Susceptible)
                else Infected
            | Recovered -> 
                if test <| alpha
                then Susceptible
                else Recovered

        // returns (s, e, i, r)
        let computePopulation samples =
            samples 
            |> List.countBy id
            |> List.map (fun (state, cnt) -> 
                            match state with
                            | Susceptible -> (cnt, 0, 0, 0)
                            | Exposed     -> (0, cnt, 0, 0)
                            | Infected    -> (0, 0, cnt, 0)
                            | Recovered   -> (0, 0, 0, cnt)
                        )
            |> List.reduce (fun (s, e, i, r) (s', e', i', r') -> (s+s', e+e', i+i', r+r'))
    
        // init model with initial probability of being infected
        let initModel prob n =
            List.init n (fun _ -> if test prob then Infected else Susceptible)

        // iterate model via Monte Carlo simulation
        let iterateModel population samples = 
            let n = let (s, e, i, r) = population in float (s+e+i+r)

            samples 
            // deal with disease transmission
            |> Seq.map (transformSample population)
            // deal with death
            |> Seq.filter (fun _ -> test mu |> not)
            // deal with birth
            |> Seq.append (initModel 0. (mu' * n |> int))
            // realize
            |> Seq.toList

        let result =
            initModel infectShare population
            |> Seq.unfold (fun samples -> 
                                let population = computePopulation samples
                                Some (population, iterateModel population samples)
                          )
            |> Seq.take n
    
        let s = result |> Seq.map (fun (x, _, _, _) -> x)
        let e = result |> Seq.map (fun (_, x, _, _) -> x)
        let i = result |> Seq.map (fun (_, _, x, _) -> x)
        let r = result |> Seq.map (fun (_, _, _, x) -> x)

        s, e, i, r

    let sample infectShare population n model =
        reproduce infectShare population n model <| Random()


// Example Parameters: [mu=0.001; mu'=0.005; beta=0.6; gamma=0.1; alpha=0.01]
[<EntryPoint>]
let main argv =
    // hyper-parameters:
    let infectShare = 0.01
    let population = 1000
    let iteration = 100

    // parameters:
    // respectively, transmission rate, recover rate, immunity loss rate, incubation rate
    let beta, gamma, alpha, epsilon = 0.6, 0.1, 0.05, 0.5

    let simulateModel (title, category) =
         // simulate
         let s, e, i, r =
             Model.createStatic category
             |> Simulation.sample infectShare population iteration

         // display
         [s; e; i; r]
         |> List.map (Seq.zip (Seq.initInfinite id))
         |> Chart.Line 
         |> Chart.WithTitle title
         |> Chart.WithLabels ["Susceptible"; "Exposed"; "Infected"; "Recovered"]
         |> Chart.WithXTitle "Iteration"
         |> Chart.WithYTitle "Population"
         |> Chart.Show

    [ "SI",    SI(beta); 
      "SIS",   SIS(beta, gamma); 
      "SIR",   SIR(beta, gamma); 
      "SIRS",  SIRS(beta, gamma, alpha);
      "SEIRS", SEIRS(beta, gamma, alpha, epsilon)]
    |> List.iter simulateModel

    0

    (*
    let template = 
        """{ name:"{name}", xaxis:"{xaxis}", yaxis:"{yaxis}", x:[{x}], y:[{y}], type:"scatter", line:{ color:"{color}" }, legendgroup:"{name}", showlegend:{showlegend}, visible:{visible} }"""

    let colors = [| "blue"; "yellow"; "orange"; "red"; |]
    let sx = String.Join(",", (Seq.init iteration id))

    [ "SI",    SI(beta); 
      "SIS",   SIS(beta, gamma); 
      "SIR",   SIR(beta, gamma); 
      "SIRS",  SIRS(beta, gamma, alpha);
      "SEIRS", SEIRS(beta, gamma, alpha, epsilon)]
    |> Seq.mapi (fun index (title, category) ->
                     let s, e, i, r =
                         Model.createDynamic 0.001 0.002 category
                         |> Simulation.sample infectShare population iteration

                     
                     let xaxis = index+1 |> sprintf "x%d"
                     let yaxis = index+1 |> sprintf "y%d"
                     let group = [ s, "Susceptible", "rgb(31, 119, 180)"; 
                                   e, "Exposed", "rgb(255, 127, 14)"; 
                                   i, "Infected", "rgb(44, 160, 44)"; 
                                   r, "Recovered", "rgb(214, 39, 40)"; ]

                     [ for y, name, color in group ->
                           let showlegend = if index=4 then "true" else "false"
                           let visible = if y |> Seq.forall (fun v -> v=0) then "false" else "true"
                           
                           template.Replace("{x}", sx)
                                   .Replace("{y}", String.Join(",", y))
                                   .Replace("{name}", name)
                                   .Replace("{showlegend}", showlegend)
                                   .Replace("{visible}", visible)
                                   .Replace("{color}", color)
                                   .Replace("{xaxis}", xaxis)
                                   .Replace("{yaxis}", yaxis) ]
                )
    |> Seq.collect id
    |> Seq.iter (printfn "%s,")

    Console.ReadKey() |> ignore
    *)
