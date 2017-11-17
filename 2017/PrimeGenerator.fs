// functional style
let primesUpTo n = 
   let rec sieve l  = 
      match l with 
      | [] -> []
      | p::xs -> 
            p::sieve [for x in xs do if (x % p) > 0 then yield x]
   [2..n] |> sieve

// imperative style
let primesUpTo' n = 
    let mutable sieves = Array.create (n+1) true

    sieves.[0] <- false
    sieves.[1] <- false

    for i in 2..n do
        if sieves.[i] then 
            for j in [i*2 .. i .. n] do
                sieves.[j] <- false
            
    [2..n] |> List.filter (fun x -> sieves.[x])