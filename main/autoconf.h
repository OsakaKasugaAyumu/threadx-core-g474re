/* ThreadX Learn #215. Project-owned learning profile, not a safety certificate.
 * Each reading file explicitly includes this before its first configuration test. Directives only:
 * do not include tx_api.h, tx_port.h, or any C declarations here.
 * Unselected presence switches must stay UNDEFINED, not defined as zero.
 */
#ifndef THREADX_LEARN_AUTOCONF_H
#define THREADX_LEARN_AUTOCONF_H 1

/* Validate the compiler and ABI; do not manufacture compiler predefined macros. */
#if !defined(__GNUC__) || defined(__clang__) || defined(__ICCARM__) || defined(__ghs__) || defined(__CC_ARM) || defined(__cplusplus)
#error "ThreadX Learn requires GNU Arm GCC, C or assembler-with-cpp"
#endif
#if !defined(__ARM_ARCH_7EM__) || !defined(__thumb2__) || !defined(__ARM_ARCH_PROFILE) || (__ARM_ARCH_PROFILE != 'M')
#error "ThreadX Learn requires Thumb-2 ARMv7E-M (-mcpu=cortex-m4 -mthumb)"
#endif
#if !defined(__ARM_FP) || (__ARM_FP != 4) || !defined(__ARM_PCS_VFP) || (__SIZEOF_POINTER__ != 4) || (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#error "ThreadX Learn requires 32-bit little-endian FPv4-SP hard-float ABI"
#endif

/* This file owns these eight choices. Refuse even -DTX_MISRA_ENABLE=0:
 * upstream #ifdef treats zero as ON. Never accept per-file overrides. */
#if defined(TX_MISRA_ENABLE) || defined(TX_SAFETY_CRITICAL) || defined(TX_INCLUDE_USER_DEFINE_FILE) || \
    defined(TX_MAX_PRIORITIES) || defined(TX_MINIMUM_STACK) || defined(TX_TIMER_THREAD_PRIORITY) || \
    defined(TX_TIMER_THREAD_STACK_SIZE) || defined(TX_TIMER_TICKS_PER_SECOND)
#error "ThreadX Learn: set project choices here, not on a per-file command line"
#endif
#define TX_MISRA_ENABLE             1       /* Named helpers; use the M4 assembly provider. */
#define TX_SAFETY_CRITICAL          1       /* Upstream restrictions + example exception path, NOT a safe state. */
#define TX_INCLUDE_USER_DEFINE_FILE 1       /* The locked tx_user.h has no active project choices. */
#define TX_MAX_PRIORITIES           32      /* A single ready bitmap. */
#define TX_MINIMUM_STACK            200     /* Bytes: creation threshold, not a worst-case bound. */
#define TX_TIMER_THREAD_PRIORITY    0       /* Use the timer thread, not callbacks in SysTick. */
#define TX_TIMER_THREAD_STACK_SIZE  1024    /* Bytes: example value, not ECU sizing evidence. */
#define TX_TIMER_TICKS_PER_SECOND   100UL   /* Matches the example assembly's literal divisor. */

/* Keep API checking, timers, interruptible kernel paths, stack filling and clearing. */
#if defined(TX_DISABLE_ERROR_CHECKING) || defined(TX_ENABLE_MULTI_ERROR_CHECKING) || defined(TX_NO_TIMER) || \
    defined(TX_TIMER_PROCESS_IN_ISR) || defined(TX_DISABLE_STACK_FILLING) || defined(TX_ENABLE_STACK_CHECKING) || \
    defined(TX_DISABLE_NOTIFY_CALLBACKS) || defined(TX_DISABLE_REDUNDANT_CLEARING) || \
    defined(TX_DISABLE_PREEMPTION_THRESHOLD) || defined(TX_INLINE_THREAD_RESUME_SUSPEND) || \
    defined(TX_NOT_INTERRUPTABLE) || defined(TX_REACTIVATE_INLINE) || defined(TX_NO_FILEX_POINTER)
#error "ThreadX Learn: incompatible kernel policy (leave unselected switches undefined)"
#endif
/* No instrumentation, low power, BASEPRI or alternative ABI/runtime in this profile. */
#if defined(TX_ENABLE_EVENT_TRACE) || defined(TX_ENABLE_EVENT_LOGGING) || defined(TX_EXECUTION_PROFILE_ENABLE) || \
    defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_ENABLE_WFI) || defined(TX_LOW_POWER) || \
    defined(TX_PORT_USE_BASEPRI) || defined(TX_PORT_BASEPRI) || defined(TX_ENABLE_IAR_LIBRARY_SUPPORT) || defined(TX_64_BIT)
#error "ThreadX Learn: PRIMASK profile without optional instrumentation or foreign runtime"
#endif
#if defined(TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO) || defined(TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO) || \
    defined(TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO) || defined(TX_MUTEX_ENABLE_PERFORMANCE_INFO) || \
    defined(TX_QUEUE_ENABLE_PERFORMANCE_INFO) || defined(TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO) || \
    defined(TX_THREAD_ENABLE_PERFORMANCE_INFO) || defined(TX_TIMER_ENABLE_PERFORMANCE_INFO)
#error "ThreadX Learn: performance counters are outside this profile"
#endif
/* Leave upstream constants and hooks to their owners, including the example's fault loop. */
#if defined(TX_SAFETY_CRITICAL_EXCEPTION) || defined(TX_SAFETY_CRITICAL_EXCEPTION_HANDLER) || \
    defined(TX_BYTE_BLOCK_FREE) || defined(TX_BYTE_BLOCK_MIN) || defined(TX_BYTE_POOL_MIN) || \
    defined(TX_BYTE_POOL_MULTIPLE_BLOCK_SEARCH) || defined(TX_BYTE_POOL_DELAY_VALUE) || \
    defined(TX_TRACE_OBJECT_REGISTRY_NAME) || defined(TX_TRACE_TIME_MASK) || defined(TX_TRACE_TIME_SOURCE)
#error "ThreadX Learn: do not override upstream hooks or internal constants"
#endif
#if defined(TX_DISABLE_INLINE) || defined(TX_INLINE_INITIALIZATION) || defined(TX_INVOKE_INLINE_INITIALIZATION) || \
    defined(TX_SOURCE_CODE) || defined(TX_THREAD_INIT) || defined(TX_API_H) || defined(TX_PORT_H) || defined(TX_USER_H)
#error "ThreadX Learn: include this header before ThreadX choices; do not inject internal symbols or guards"
#endif
#endif /* THREADX_LEARN_AUTOCONF_H */
