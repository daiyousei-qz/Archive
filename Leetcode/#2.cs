/**
 * Definition for singly-linked list.
 * public class ListNode {
 *     public int val;
 *     public ListNode next;
 *     public ListNode(int x) { val = x; }
 * }
 */
public class Solution {
    public ListNode AddTwoNumbers(ListNode l1, ListNode l2) {
        ListNode dummy = new ListNode(0);
        
        var node = dummy;
        int carry = 0;
        
        while(l1 != null || l2 != null) {
            int value = carry;
            
            if (l1 != null) {
                value += l1.val;
                l1 = l1.next;
            }
            if (l2 != null) {
                value += l2.val;
                l2 = l2.next;
            }
            
            carry = value / 10;
            
            node.next = new ListNode(value % 10);
            node = node.next;
        }
        
        if(carry != 0) node.next = new ListNode(carry);
        
        return dummy.next;
    }
}