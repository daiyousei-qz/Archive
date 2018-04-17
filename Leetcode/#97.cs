public class Solution {
    public bool IsInterleave(string s1, string s2, string s3) {
        int m = s1.Length;
        int n = s2.Length;
        
        if (m+n != s3.Length)
            return false;
        
        var history = new bool[m+1, n+1];
        history[0, 0] = true;
        
        for (int i = 1; i <= m; ++i)
            history[i, 0] = history[i-1, 0] && s1[i-1]==s3[i-1];
        
        for (int j = 1; j <= n; ++j)
            history[0, j] = history[0, j-1] && s2[j-1]==s3[j-1];
        
        for (int b = 2; b <= m+n; ++b)
        {
            for (int i = 1; i < b; ++i)
            {
                int j = b-i;
                
                if(i > m || j > n)
                    continue;
                
                bool p1 = history[i-1, j] && s3[b-1] == s1[i-1];
                bool p2 = history[i, j-1] && s3[b-1] == s2[j-1];

                history[i, j] = p1 || p2;
            }
        }
        
        return history[m, n];
    }
}