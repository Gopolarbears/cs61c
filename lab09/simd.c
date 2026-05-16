#include <time.h>
#include <stdio.h>
#include <x86intrin.h>
#include "simd.h"

long long int sum(int vals[NUM_ELEMS]) {
	clock_t start = clock();

	long long int sum = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS; i++) {
			if(vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	long long int sum = 0;

	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			if(vals[i] >= 128) sum += vals[i];
			if(vals[i + 1] >= 128) sum += vals[i + 1];
			if(vals[i + 2] >= 128) sum += vals[i + 2];
			if(vals[i + 3] >= 128) sum += vals[i + 3];
		}

		//This is what we call the TAIL CASE
		//For when NUM_ELEMS isn't a multiple of 4
		//NONTRIVIAL FACT: NUM_ELEMS / 4 * 4 is the largest multiple of 4 less than NUM_ELEMS
		for(unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_simd(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);		// This is a vector with 127s in it... Why might you need this?
	long long int result = 0;				   // This is where you should put your final result!
	/* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */

	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		/* YOUR CODE GOES HERE */
		__m128i sum = _mm_setzero_si128();
		unsigned int i = 0;
		for (; i + 4 <= NUM_ELEMS; i += 4) {
			__m128i temp = _mm_loadu_si128(vals + i);
			__m128i mask = _mm_cmpgt_epi32(temp, _127);
			temp = _mm_and_si128(mask, temp);
			sum = _mm_add_epi32(sum, temp);
		}

		int temp_arr[4];
        _mm_storeu_si128((__m128i*)temp_arr, sum);
        result += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3];

		/* You'll need a tail case. */
		for (; i < NUM_ELEMS; i++) {
			if (vals[i] > 127) {
                result += vals[i];
            }
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}

long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
    clock_t start = clock();
    __m128i _127 = _mm_set1_epi32(127);
    long long int result = 0;
    
    // 【优化 3】提前算出安全边界，循环内不再做额外的加法判断
    unsigned int limit = NUM_ELEMS / 16; 
    
    for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
        __m128i sum1 = _mm_setzero_si128();
        __m128i sum2 = _mm_setzero_si128();
        __m128i sum3 = _mm_setzero_si128();
        __m128i sum4 = _mm_setzero_si128();
        
        // 【优化 2】使用 128 位胖指针，消灭普通的数组地址加法偏移
        __m128i* ptr = (__m128i*)vals;
        
        for (unsigned int j = 0; j < limit; j++) {
            // 【优化 1】纵向排布：减轻寄存器压力，同时可以把 and 和 add 连写
            // ------ 第 1 组 ------
            __m128i t1 = _mm_loadu_si128(ptr);
            __m128i m1 = _mm_cmpgt_epi32(t1, _127);
            sum1 = _mm_add_epi32(sum1, _mm_and_si128(m1, t1));
            
            // ------ 第 2 组 ------
            // ptr + 1 在指针运算里，会自动跨越 sizeof(__m128i) 也就是 16 个字节！
            __m128i t2 = _mm_loadu_si128(ptr + 1);
            __m128i m2 = _mm_cmpgt_epi32(t2, _127);
            sum2 = _mm_add_epi32(sum2, _mm_and_si128(m2, t2));
            
            // ------ 第 3 组 ------
            __m128i t3 = _mm_loadu_si128(ptr + 2);
            __m128i m3 = _mm_cmpgt_epi32(t3, _127);
            sum3 = _mm_add_epi32(sum3, _mm_and_si128(m3, t3));
            
            // ------ 第 4 组 ------
            __m128i t4 = _mm_loadu_si128(ptr + 3);
            __m128i m4 = _mm_cmpgt_epi32(t4, _127);
            sum4 = _mm_add_epi32(sum4, _mm_and_si128(m4, t4));
            
            // 指针前进 4 个 128 位（即 64 字节，刚好是一个经典的 Cache Line 大小！）
            ptr += 4;
        }

        // 两两合并，形成规约树
        __m128i final_sum1 = _mm_add_epi32(sum1, sum2);
        __m128i final_sum2 = _mm_add_epi32(sum3, sum4);
        __m128i final_sum = _mm_add_epi32(final_sum1, final_sum2);

        int temp_arr[4];
        _mm_storeu_si128((__m128i*)temp_arr, final_sum);
        result += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3];

        /* Tail case */
        // 恢复之前大巴车拉走的总人数
        unsigned int i = limit * 16; 
        for (; i < NUM_ELEMS; i++) {
            if (vals[i] > 127) {
                result += vals[i];
            }
        }
    }
    
    clock_t end = clock();
    printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return result;
}

// long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
// 	clock_t start = clock();
// 	__m128i _127 = _mm_set1_epi32(127);
// 	long long int result = 0;
// 	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
// 		/* YOUR CODE GOES HERE */
// 		__m128i sum1 = _mm_setzero_si128();
// 		__m128i sum2 = _mm_setzero_si128();
// 		__m128i sum3 = _mm_setzero_si128();
// 		__m128i sum4 = _mm_setzero_si128();
// 		unsigned int i = 0;
// 		for (; i + 16 <= NUM_ELEMS; i += 16) {
// 			__m128i temp = _mm_loadu_si128(vals + i);
// 			__m128i temp2 = _mm_loadu_si128(vals + i + 4);
// 			__m128i temp3 = _mm_loadu_si128(vals + i + 8);
// 			__m128i temp4 = _mm_loadu_si128(vals + i + 12);
// 			__m128i mask = _mm_cmpgt_epi32(temp, _127);
// 			__m128i mask2 = _mm_cmpgt_epi32(temp2, _127);
// 			__m128i mask3= _mm_cmpgt_epi32(temp3, _127);
// 			__m128i mask4 = _mm_cmpgt_epi32(temp4, _127);
// 			temp = _mm_and_si128(mask, temp);
// 			temp2 = _mm_and_si128(mask2, temp2);
// 			temp3 = _mm_and_si128(mask3, temp3);
// 			temp4 = _mm_and_si128(mask4, temp4);
// 			sum1 = _mm_add_epi32(sum1, temp);
// 			sum2 = _mm_add_epi32(sum2, temp2);
// 			sum3 = _mm_add_epi32(sum3, temp3);
// 			sum4 = _mm_add_epi32(sum4, temp4);
// 		}
// 		__m128i final_sum1 = _mm_add_epi32(sum1, sum2);
// 		__m128i final_sum2 = _mm_add_epi32(sum3, sum4);
// 		__m128i final_sum = _mm_add_epi32(final_sum1, final_sum2);

// 		int temp_arr[4];
//         _mm_storeu_si128((__m128i*)temp_arr, final_sum);
//         result += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3];

// 		/* You'll need a tail case. */
// 		for (; i < NUM_ELEMS; i++) {
// 			if (vals[i] > 127) {
//                 result += vals[i];
//             }
// 		}
// 	}
// 	clock_t end = clock();
// 	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
// 	return result;
// }