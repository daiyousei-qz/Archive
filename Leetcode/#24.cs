/**
 * Definition for singly-linked list.
 * public class ListNode {
 *     public int val;
 *     public ListNode next;
 *     public ListNode(int x) { val = x; }
 * }
 */
public class Solution
{
    public void SwapPairsAux(ListNode pred, ListNode head)
    {
        if (head == null)
        {
            pred.next = null;
            return;
        }
        if (head.next == null)
        {
            pred.next = head;
            return;
        }

        var n1 = head;
        var n2 = head.next;
        var n3 = head.next.next;

        pred.next = n2;
        n2.next = n1;

        SwapPairsAux(n1, n3);
    }

    public ListNode SwapPairs(ListNode head)
    {
        var dummy = new ListNode(0);
        SwapPairsAux(dummy, head);

        return dummy.next;
    }
}