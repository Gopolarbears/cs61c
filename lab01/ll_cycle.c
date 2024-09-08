#include <stddef.h>
#include "ll_cycle.h"

int ll_has_cycle(node *head) {
    /* your code here */
    if (head == NULL || head->next == NULL || head->next->next == NULL) {
        return 0;
    }
    node* hare = head->next->next;
    node* tortoise = head;
    while (hare != NULL && hare->next != NULL) {
        if (hare == tortoise) {
            return 1;
        }
        hare = hare->next->next;
        tortoise = tortoise->next;
    }
    return 0;
}