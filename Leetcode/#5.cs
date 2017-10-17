public class Solution
{
    int SearchBound(string s, int begin, int end)
    {
        int max_bound = Math.Min(begin, s.Length - end - 1);
        for (int i = 0; i < max_bound; ++i)
        {
            if (s[begin - i - 1] != s[end + i + 1])
            {
                return i;
            }
        }

        return max_bound;
    }

    public string LongestPalindrome(string s)
    {
        if (s.Length == 0) return "";

        int begin = 0, end = 0;
        int bound = 0;

        for (int i = 0; i < s.Length; ++i)
        {
            if (i + 1 + bound > s.Length) break;

            for (int j = 1; j < 3 && i + j < s.Length; ++j)
            {
                if (s[i] == s[i + j])
                {
                    int b = SearchBound(s, i, i + j);
                    int old_len = (end - begin + 1) + 2 * bound;
                    int new_len = 1 + j + 2 * b;

                    if (new_len > old_len)
                    {
                        begin = i;
                        end = i + j;
                        bound = b;
                    }
                }
            }
        }

        return s.Substring(begin - bound, (end - begin + 1) + 2 * bound);
    }
}