public class Solution {
    public int MaxProfit(int[] prices) {
        int result = 0;
        int minIndex = 0;
        int maxIndex = 0;
        
        for(int i = 1; i < prices.Length; ++i)
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

            result = Math.Max(result, prices[maxIndex] - prices[minIndex]);
        }
        
        return result;
    }
}