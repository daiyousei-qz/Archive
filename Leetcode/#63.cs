public class Solution
{
    public int UniquePathsWithObstacles(int[,] obstacleGrid)
    {
        var m = obstacleGrid.GetUpperBound(0) + 1;
        var n = obstacleGrid.GetUpperBound(1) + 1;
        var resultGrid = new int[m, n];
        
        resultGrid[0, 0] = obstacleGrid[0, 0] == 1 ? 0 : 1;
        for (int b = 1; b < m + n - 1; ++b)
        {
            for (int i = 0; i <= b; ++i)
            {
                int j = b - i;

                if (i >= m || j >= n)
                {
                    continue;
                }
                else if (obstacleGrid[i, j] == 1)
                {
                    resultGrid[i, j] = 0;
                }
                else
                {
                    var fromLeft = i > 0 ? resultGrid[i - 1, j] : 0;
                    var fromTop = j > 0 ? resultGrid[i, j - 1] : 0;

                    resultGrid[i, j] = fromLeft + fromTop;
                }
            }
        }

        return resultGrid[m - 1, n - 1];
    }
}