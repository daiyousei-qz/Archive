public class Solution
{
    public int LengthOfLongestSubstring(string s)
    {
        var lookup = new Dictionary<char, int>();
        int result = 0;
        int offset = 0;
        for(int i = 0; i < s.Length; ++i)
        {
            var ch = s[i];

            if(lookup.ContainsKey(ch))
            {
                if(lookup[ch] >= offset)
                {
                    result = Math.Max(result, i - offset);
                    offset = lookup[ch] + 1;
                }

                lookup[ch] = i;
            }
            else
            {
                lookup.Add(ch, i);
            }
        }

        result = Math.Max(result, s.Length - offset);
        return result;
    }
}