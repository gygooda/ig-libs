#ifndef IG_LIBS_SYS_UTILS_H
#define IG_LIBS_SYS_UTILS_H

namespace LibSys
{

#define barrier() __asm__ __volatile__("": : :"memory")

#ifdef CONFIG_SMP
/* 
 * memory barrier
 * only support x86_64 cpu 
 */
#define mb()    asm volatile("mfence":::"memory")
#define rmb()   asm volatile("lfence":::"memory")
#define wmb()   asm volatile("sfence" ::: "memory")
#else
#define mb() barrier()
#define rmb() barrier()
#define wmb() barrier()
#endif

/*           
 *  Determine whether some value is a power of two, where zero is
 * *not* considered a power of two.
 */
inline bool is_power_of_2(unsigned long n)
{   
    return (n != 0 && ((n & (n - 1)) == 0));
}       

/**
 * roundup_pow_of_two - round the given value up to nearest power of two
 * @n - parameter
 *
 * round the given value up to the nearest power of two
 * - the result is undefined when n == 0
 * - this can be used to initialise global variables from constant data
 */
inline unsigned long roundup_pow_of_two(unsigned long n)
{
    if(n == 0)
        return 0;

    int left_zero_count = 0;
    if(sizeof(n) == 4) // 32bit
        left_zero_count = __builtin_clz(n);
    else // 64bit
        left_zero_count = __builtin_clzl(n);

    return 1UL << (sizeof(n) * 8UL - left_zero_count - 1);
}


} // namespace LibSys

#endif

