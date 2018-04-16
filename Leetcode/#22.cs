public class Solution {
    public IList<string> GenerateParenthesis(int n)
    {
        var table = new List<string[]> { new string[0], new[] { "()" } };

        for(int i = 2; i <= n; ++i)
        {
            var buffer = table[i - 1].Select(s => "(" + s + ")").ToHashSet();

            for(int j = 1; j < i; ++j)
            {
                foreach(var x in table[j].SelectMany(s1 => table[i - j].Select(s2 => s1 + s2)))
                {
                    buffer.Add(x);
                }
            }

            table.Add(buffer.ToArray());
        }

        return table[n];
    }
}