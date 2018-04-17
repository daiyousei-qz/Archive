public class Solution {
    public int NumDistinct(string s, string t) {
        if(s.Length < t.Length)
            return 0;
        
        int m = s.Length;
        int n = t.Length;
        var dp = new int[m, n];
        
        dp[0, 0] = s[0]==t[0] ? 1 : 0;
        for(int i = 1; i < m; ++i)
        {
            dp[i, 0] = dp[i-1, 0] + (s[i]==t[0] ? 1 : 0);
        }
        
        for(int j = 1; j < n; ++j)
        {
            for(int i = 0; i < m; ++i)
            {
                if(i < j)
                {
                    dp[i, j] = 0;
                }
                else
                {
                    dp[i, j] = dp[i-1, j];
                    
                    if(s[i] == t[j])
                    {
                        dp[i, j] += dp[i-1, j-1];
                    }
                }
            }
        }
        
        return dp[m-1, n-1];
    }
}