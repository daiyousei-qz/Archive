/**
 * Definition for a binary tree node.
 * public class TreeNode {
 *     public int val;
 *     public TreeNode left;
 *     public TreeNode right;
 *     public TreeNode(int x) { val = x; }
 * }
 */
public class Solution
{
    public IList<int> PostorderTraversal(TreeNode root)
    {
        if(root == null)
            return new List<int>();
        
        var inBuffer = new List<(bool, TreeNode)> { (false, root) };
        var outBuffer = new List<(bool, TreeNode)>();

        while(true)
        {
            bool exit = true;
            foreach (var (expanded, node) in inBuffer)
            {
                exit &= expanded;
                outBuffer.Add((true, node));

                if(!expanded)
                {
                    if (node.right != null)
                        outBuffer.Add((false, node.right));

                    if (node.left != null)
                        outBuffer.Add((false, node.left));
                }
            }

            if(exit)
            {
                return outBuffer.Select(t => t.Item2.val).Reverse().ToList();
            }
            else
            {
                var tmp = inBuffer;
                inBuffer = outBuffer;
                outBuffer = tmp;

                outBuffer.Clear();
            }
        }
    }
}