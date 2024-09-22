#include <stdio.h>
#include "bit_ops.h"

// Return the nth bit of x.
// Assume 0 <= n <= 31
unsigned get_bit(unsigned x,
                 unsigned n) {
    // YOUR CODE HERE
    // Returning -1 is a placeholder (it makes
    // no sense, because get_bit only returns 
    // 0 or 1)
    return (x >> n) & 1;
    return -1;
}
// Set the nth bit of the value of x to v.
// Assume 0 <= n <= 31, and v is 0 or 1
void set_bit(unsigned * x,
             unsigned n,
             unsigned v) {
    // YOUR CODE HERE
    // 我的做法: 分别抽出n上边的位, n下边的位. 但是用了加减法
    // unsigned x_copy = *x;
    // unsigned upper = x_copy >> (n + 1) << (n + 1);
    // unsigned lower = x_copy & ((1 << n) - 1);
    // v = v << n;
    // *x = upper | lower | v;

    // 参考解法: https://github.com/PKUFlyingPig/CS61C-labs/blob/master/lab02_AdvancedC/bit_ops.c
    unsigned mask = ~(1 << n);
    *x = (*x & mask) | (v << n);
}
// Flip the nth bit of the value of x.
// Assume 0 <= n <= 31
void flip_bit(unsigned * x,
              unsigned n) {
    // YOUR CODE HERE
    unsigned v = get_bit(*x, n);
    set_bit(x, n, !v);
    // (*x)^=(1<<n);
}

