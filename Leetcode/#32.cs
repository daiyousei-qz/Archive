public class Solution {
    public int LongestValidParentheses(string s) {
        var maxHistory = new int[s.Length + 2];
        maxHistory[0] = 0;
        maxHistory[1] = s.StartsWith("()") ? 2 : 0;
        
        for (int i = 2; i < s.Length; ++i)
        {
            maxHistory[i] = 0;
            
            if (s[i] == ')')
            {
                var mirrorIndex = i-maxHistory[i-1]-1;
                if (s[i-1] == '(')
                {
                    maxHistory[i] = maxHistory[i-2] + 2;
                }
                else if (mirrorIndex >= 0 && s[mirrorIndex] == '(')
                {
                    var preceedingLength = mirrorIndex > 0 ? maxHistory[mirrorIndex-1] : 0;
                    maxHistory[i] = preceedingLength + maxHistory[i-1] + 2;
                }
            }
        }
        
        return maxHistory.Max();
    }
}