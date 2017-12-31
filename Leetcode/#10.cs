class Transition
{
    public char Value = ' ';
    public bool Closure = false;
}

class Solution
{
    public bool IsMatch(string s, string p)
    {
        // parse transitions
        var edges = new List<Transition>();
        for(int i = 0; i<p.Length; ++i)
        {
            char value = p[i];
            bool closure = i < p.Length - 1 && p[i+1] == '*';

            edges.Add(new Transition{ Value = value, Closure = closure });

            if (closure)
                i += 1;
        }

        // simulate nfa
        var src = new SortedSet<int> { };
        var dest = new SortedSet<int> { 0 };
        for(int i = 0; i<s.Length; ++i)
        {
            // load solid source states
            src.Clear();
            foreach (var state in dest)
            {
                if (state >= edges.Count)
                    continue;

                for(int j = state; j < edges.Count; ++j)
                {
                    src.Add(j);

                    if (!edges[j].Closure)
                        break;
                }
            }

            // feed character
            dest.Clear();
            foreach(var state in src)
            {
                var transition = edges[state];
                if (s[i] == transition.Value || transition.Value == '.')
                {
                    if(transition.Closure)
                        dest.Add(state);

                    dest.Add(state + 1);
                }
            }

            // break if no transition is possible
            if (dest.Count == 0)
                return false;
        }

        // finalize
        for (int i = dest.Max; i < edges.Count; ++i)
        {
            if (!edges[i].Closure)
                return false;
        }

        return true;
    }
}