/**
 * Definition for singly-linked list.
 * public class ListNode {
 *     public int val;
 *     public ListNode next;
 *     public ListNode(int x) {
 *         val = x;
 *         next = null;
 *     }
 * }
 */
public class Solution
{
    public ListNode FindArbitraryCycleMember(ListNode head)
    {
        var slow = head;
        var fast = head;
        while(fast != null && fast.next != null)
        {
            slow = slow.next;
            fast = fast.next.next;

            if (slow == fast)
                return slow;
        }

        return null;
    }

    public ListNode DetectCycle(ListNode head)
    {
        var spy = FindArbitraryCycleMember(head);

        if (spy == null)
            return null;

        for(var node = head; node != null; node = node.next)
        {
            if (node == spy)
                return spy;

            for(var p = spy.next; p != spy; p = p.next)
            {
                if (node == p)
                    return p;
            }
        }

        // impossible path
        return null;
    }
}