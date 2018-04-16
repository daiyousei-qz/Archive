/**
 * Definition for a binary tree node.
 * public class TreeNode {
 *     public int val;
 *     public TreeNode left;
 *     public TreeNode right;
 *     public TreeNode(int x) { val = x; }
 * }
 */
public class ExtraNodeInfo {
    public int sumSoFar;
    public TreeNode parent;
}

public class Solution {
    public void Traverse(Dictionary<TreeNode, ExtraNodeInfo> output, TreeNode parent, TreeNode node) {
        if(node == null)
            return;
        
        var info = new ExtraNodeInfo {
            sumSoFar = output[parent].sumSoFar + node.val,
            parent = parent
        };
        output.Add(node, info);
        
        Traverse(output, node, node.left);
        Traverse(output, node, node.right);
    }
    
    public Dictionary<TreeNode, ExtraNodeInfo> ConstructLookup(TreeNode root) {
        if(root == null) {
            return new Dictionary<TreeNode, ExtraNodeInfo>();
        }
        
        var infoLookup = new Dictionary<TreeNode, ExtraNodeInfo> {
            { root, new ExtraNodeInfo { sumSoFar = root.val, parent = null } }
        };
        
        Traverse(infoLookup, root, root.left);
        Traverse(infoLookup, root, root.right);   
        
        return infoLookup;
    }
    
    public bool IsLeaf(TreeNode node) {
        return node != null && node.left == null && node.right == null;
    }
    
    public IList<IList<int>> PathSum(TreeNode root, int sum) {
        var infoLookup = ConstructLookup(root);
        var result = new List<IList<int>>();
        foreach (var kv in infoLookup) {
            if(IsLeaf(kv.Key) && kv.Value.sumSoFar == sum) {
                var item = new List<int>();
                for (var node = kv.Key; node != null; node = infoLookup[node].parent) {
                    item.Add(node.val);
                }
                
                item.Reverse();
                result.Add(item);
            }
        }
        
        return result;
    }
}