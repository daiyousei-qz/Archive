public class Solution {
    int Min3(int x, int y, int z)
    {
        return Math.Min(x, Math.Min(y, z));
    }
    
    public int MinDistance(string word1, string word2) {
        var m = word1.Length;
        var n = word2.Length;     
        
        if(m == 0 || n == 0)
            return Math.Max(m, n);
        
        var history = new int[m+1, n+1];
        
        for (int i = 0; i <= m; ++i)
            history[i, 0] = i;
        
        for (int j = 0; j <= n; ++j)
            history[0, j] = j;
            
        
        for (int b = 2; b <= m+n; ++b)
        {
            for (int i = 1; i<b; ++i)
            {
                int j = b-i;
                
                if (i > m || j > n)
                    continue;
                
                int fromLeft = history[i-1, j] + 1;
                int fromTop =  history[i, j-1] + 1;
                int fromTopLeft = history[i-1, j-1] + (word1[i-1]!=word2[j-1] ? 1 : 0);
                
                history[i, j] = Min3(fromLeft, fromTop, fromTopLeft);
            }
        }
        
        return history[m, n];
    }
}