
// memoization upon #121
// there is better state-machine solution that works for even at most k-transaction
public class Solution {
    int[] BuildLeftLookup(int[] prices)
    {
        int n = prices.Length;
        var lookup = new int[n];
        lookup[0] = 0;
        
        int minIndex = 0;
        int maxIndex = 0;
        
        for(int i = 1; i < n; ++i)
        {
            if(prices[i-1] < prices[minIndex])
            {
                minIndex = i-1;
                maxIndex = i;                
            }
            else if(prices[i] > prices[maxIndex])
            {
                maxIndex = i;                
            }

            lookup[i] = Math.Max(lookup[i-1], prices[maxIndex] - prices[minIndex]);
        }
        
        return lookup;
    }
    
    int[] BuildRightLookup(int[] prices)
    {
        int n = prices.Length;
        var lookup = new int[n];
        lookup[n-1] = 0;
        
        int minIndex = n-1;
        int maxIndex = n-1;
        
        for(int i = n-2; i >= 0; --i)
        {
            if(prices[i+1] > prices[maxIndex])
            {
                maxIndex = i+1;
                minIndex = i;
            }
            else if(prices[i] < prices[minIndex])
            {
                minIndex = i;                
            }

            lookup[i] = Math.Max(lookup[i+1], prices[maxIndex] - prices[minIndex]);
        }
        
        return lookup;
    }
    
    public int MaxProfit(int[] prices) 
    {
        if(prices.Length < 2)
            return 0;
        
        var leftLookup = BuildLeftLookup(prices);
        var rightLookup = BuildRightLookup(prices);
        
        int result = 0;
        for(int i = 1; i < prices.Length; ++i)
        {
            result = Math.Max(result, leftLookup[i] + rightLookup[i]);
        }
        
        return result;
    }
}