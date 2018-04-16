/**
 * Definition for singly-linked list.
 * public class ListNode {
 *     public int val;
 *     public ListNode next;
 *     public ListNode(int x) { val = x; }
 * }
 */
public class Solution {
        public ListNode MergeKLists(ListNode[] lists)
        {
            var dict = new Dictionary<int, int>();
            foreach(var n in lists)
            {
                for(var x = n; x!=null; x= x.next)
                {
                    if(dict.ContainsKey(x.val))
                    {
                        dict[x.val] += 1;
                    }
                    else
                    {
                        dict.Add(x.val, 1);
                    }
                }
            }

            var dummy = new ListNode(0);
            var node = dummy;
            foreach(var pair in dict.OrderBy(kv => kv.Key))
            {
                for(int i=0;i<pair.Value; ++i)
                {
                    node.next = new ListNode(pair.Key);
                    node = node.next;
                }
            }

            return dummy.next;
        }
}