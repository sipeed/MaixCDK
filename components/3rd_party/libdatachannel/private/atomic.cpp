#include <mutex>

/* risv: undefined reference to `__atomic_compare_exchange_1'、`__atomic_fetch_add_2`... */
#if 1
#if defined(__riscv) || defined(__riscv__) || defined(__riscv32__) || defined(__riscv64__)
/* patch: jemalloc-risv */
static struct __ATOMIC final {
public:
    typedef std::mutex                              SynchronizedObject;
    typedef std::lock_guard<SynchronizedObject>     SynchronizedObjectScope;

public:
    SynchronizedObject                              Lock;
}                                                   __ATOMIC_;

// LLVM-CC:
// GUNL-CC:
// https://doc.dpdk.org/api-18.11/rte__atomic_8h_source.html
#ifdef __cplusplus
extern "C" {
#endif
    /*
     * __atomic_exchange_n(dst, val, __ATOMIC_SEQ_CST);
     * __atomic_exchange_4(dst, val, __ATOMIC_SEQ_CST);
     */
    __attribute__((visibility("default"))) unsigned char __atomic_exchange_1(volatile void* ptr, unsigned char value, int memorder) noexcept {
        (void)memorder;
        __ATOMIC::SynchronizedObjectScope scope(__ATOMIC_.Lock);
        unsigned char* dst = (unsigned char*)ptr;
        unsigned char old = *dst;
        *dst = value;
        return old;
    }

    /*
        volatile void *ptr: Pointer to the variable to be operated on.
        void *expected: Pointer to the value to be compared.
        void *desired: expected new value.
        bool weak: Indicates whether to use weak memory order (true indicates weak memory order, false indicates strong memory order).
        int success_memorder: Memory order upon success.
        int failure_memorder: Memory sequence of failure.
    */
    __attribute__((visibility("default"))) bool __atomic_compare_exchange_1(volatile void* ptr, void* expected, unsigned char desired, bool weak, int success_memorder, int failure_memorder) noexcept {
        __ATOMIC::SynchronizedObjectScope scope(__ATOMIC_.Lock);
        unsigned char* dst = (unsigned char*)ptr;
        unsigned char* exchange = (unsigned char*)expected;
        (void)weak;
        (void)success_memorder;
        (void)failure_memorder;
        unsigned char old = *dst;
        if (old != *exchange) {
            return false;
        }

        *dst = desired;
        *exchange = old;
        return true;
    }
#ifdef __cplusplus
}
#endif
#endif
#endif