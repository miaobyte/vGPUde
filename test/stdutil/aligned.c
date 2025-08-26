#include <stdio.h>
#include <assert.h>
#include "stdutil/aligned_size.h"

void test_align_to() {
    // 测试 16 对齐
    AlignedSize result = align_to(123, 16);
    assert(result.aligned_value == 128); // 123 向上取整到 16 的倍数是 128
    assert(result.power == 1);           // 16^2 = 256
    assert(result.multiple == 8);        // 128 / 16 = 8

    // 测试 8 对齐
    result = align_to(65, 8);
    assert(result.aligned_value == 128);  // 65 向上取整到 8 的倍数是128
    assert(result.power == 2);           // 8^2 = 64
    assert(result.multiple ==2);        // 128 / 64 = 2

    // 测试 2 对齐
    result = align_to(5, 2);
    assert(result.aligned_value == 8);   // 5 向上取整到 2 的倍数是 8
    assert(result.power == 3);           // 2^2 = 4
    assert(result.multiple == 1);        // 8 /4 =2

    // 测试边界值
    result = align_to(16, 16);
    assert(result.aligned_value == 16);  // 已经是 16 的倍数
    assert(result.power == 1);           // 16^1 = 16
    assert(result.multiple == 1);        // 16 / 16 = 1

    // 测试小于 base 的值
    result = align_to(3, 4);
    assert(result.aligned_value == 4);   // 3 向上取整到 4 的倍数是 4
    assert(result.power == 1);           // 4^1 = 4
    assert(result.multiple ==1);        // 4 / 4 = 1

    // 测试 n = base - 1
    result = align_to(15, 16);
    assert(result.aligned_value == 16);  // 15 向上取整到 16 的倍数是 16
    assert(result.power ==1);           // 16^1 = 16
    assert(result.multiple == 1);        // 16 / 16 = 1

    // 测试 n = base
    result = align_to(16, 16);
    assert(result.aligned_value == 16);  // 已经是 16 的倍数
    assert(result.power == 1);           // 16^1 = 16
    assert(result.multiple == 1);        // 16 / 16 = 1

    // 测试大数值
    result = align_to(1000000, 1024);
    assert(result.aligned_value == 1000448); // 1000000 向上取整到 1024 的倍数是 1048576
    assert(result.power == 1);              // 1024^1 = 1024
    assert(result.multiple == 977);            //

    // 测试 base = 2
    result = align_to(7, 2);
    assert(result.aligned_value == 8);   // 7 向上取整到 2 的倍数是 8
    assert(result.power == 3);           // 2^3 = 8
    assert(result.multiple == 1);        // 8 / 8 = 1

    printf("All tests passed!\n");
}

#include <stdio.h>
#include <stdlib.h>
void test_input() {
    int input;
    printf("Enter an integer value: ");
    if (scanf("%d", &input) != 1) {
        printf("Invalid input. Please enter an integer.\n");
        return;
    }

    // 调用 align_to 函数
    AlignedSize result = align_to(input/8, 16);

    // 打印结果
    printf("Input: %d\n", input);
    printf("Base: %d\n", result.base);
    printf("Aligned Value: %llu\n", result.aligned_value*8);
    printf("Power: %d\n", result.power);
    printf("Multiple: %d\n", result.multiple);
}
int main() {
    test_align_to();

    test_input();
    return 0;
}