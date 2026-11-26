/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/
#include "autoconf.h"

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** ThreadX Component                                                     */
/**                                                                       */
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_thread.h                                         PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX thread control component, including   */
/*    data types and external references.  It is assumed that tx_api.h    */
/*    and tx_port.h have already been included.                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            moved TX_THREAD_GET_SYSTEM_ */
/*                                            STATE to tx_api.h,          */
/*                                            resulting in version 6.1.2  */
/*  10-15-2021     Scott Larson             Modified comment(s), improved */
/*                                            stack check error handling, */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef TX_THREAD_H
#define TX_THREAD_H


/* Define thread control specific data definitions.  */

#define TX_THREAD_ID                            ((ULONG) 0x54485244)
#define TX_THREAD_PRIORITY_GROUP_MASK           ((ULONG) 0xFF)
#define TX_THREAD_EXECUTE_LOG_SIZE              ((UINT) 8)


/* Define the MOD32 bit set macro that is used to set/clear a priority bit within a specific
   priority group.  */

#define MAP_INDEX                               (0)
#define TX_MOD32_BIT_SET(a,b)                   (b) = (((ULONG) 1) << ((a)));


/* Define the DIV32 bit set macro that is used to set/clear a priority group bit and is
   only necessary when using priorities greater than 32.  */



/* Define state change macro that can be used by run-mode debug agents to keep track of thread
   state changes. By default, it is mapped to white space.  */

#define TX_THREAD_STATE_CHANGE(a, b)


/* Define the macro to get the current thread pointer. This is particularly useful in SMP
   versions of ThreadX to add additional processing.  The default implementation is to simply
   access the global current thread pointer directly.  */

#define TX_THREAD_GET_CURRENT(a)            (a) =  _tx_thread_current_ptr;


/* Define the macro to set the current thread pointer. This is particularly useful in SMP
   versions of ThreadX to add additional processing.  The default implementation is to simply
   access the global current thread pointer directly.  */

#define TX_THREAD_SET_CURRENT(a)            _tx_thread_current_ptr =  (a);



/* Define the get system state macro. By default, it simply maps to the variable _tx_thread_system_state.  */
/* This symbol is moved to tx_api.h. Therefore removed from this file.
#ifndef TX_THREAD_GET_SYSTEM_STATE
#define TX_THREAD_GET_SYSTEM_STATE()        _tx_thread_system_state
#endif
*/

/* Define the check for whether or not to call the _tx_thread_system_return function.  A non-zero value
   indicates that _tx_thread_system_return should not be called.  */



/* Define the timeout setup macro used in _tx_thread_create.  */

#define TX_THREAD_CREATE_TIMEOUT_SETUP(t)    (t) -> tx_thread_timer.tx_timer_internal_timeout_function =  &(_tx_thread_timeout);                \
                                             (t) -> tx_thread_timer.tx_timer_internal_timeout_param =     TX_POINTER_TO_ULONG_CONVERT((t));


/* Define the thread timeout pointer setup macro used in _tx_thread_timeout.  */

#define TX_THREAD_TIMEOUT_POINTER_SETUP(t)   (t) =  TX_ULONG_TO_THREAD_POINTER_CONVERT(timeout_input);


/* Define the lowest bit set macro. Note, that this may be overridden
   by a port specific definition if there is supporting assembly language
   instructions in the architecture.  */

#define TX_LOWEST_SET_BIT_CALCULATE(m, b)       \
    (b) =  ((ULONG) 0);                         \
    (m) =  (m) & ((~(m)) + ((ULONG) 1));        \
    if ((m) < ((ULONG) 0x10))                   \
    {                                           \
        if ((m) >= ((ULONG) 4))                 \
        {                                       \
            (m) = (m) >> ((ULONG) 2);           \
            (b) = (b) + ((ULONG) 2);            \
        }                                       \
        (b) = (b) + ((m) >> ((ULONG) 1));       \
    }                                           \
    else if ((m) < ((ULONG) 0x100))             \
    {                                           \
        (m) = (m) >> ((ULONG) 4);               \
        (b) = (b) + ((ULONG) 4);                \
        if ((m) >= ((ULONG) 4))                 \
        {                                       \
            (m) = (m) >> ((ULONG) 2);           \
            (b) = (b) + ((ULONG) 2);            \
        }                                       \
        (b) = (b) + ((m) >> ((ULONG) 1));       \
    }                                           \
    else if ((m) < ((ULONG) 0x10000))           \
    {                                           \
        (m) = (m) >> ((ULONG) 8);               \
        (b) = (b) + ((ULONG) 8);                \
        if ((m) >= ((ULONG) 0x10))              \
        {                                       \
            (m) = (m) >> ((ULONG) 4);           \
            (b) = (b) + ((ULONG) 4);            \
        }                                       \
        if ((m) >= ((ULONG) 4))                 \
        {                                       \
            (m) = (m) >> ((ULONG) 2);           \
            (b) = (b) + ((ULONG) 2);            \
        }                                       \
        (b) = (b) + ((m) >> ((ULONG) 1));       \
    }                                           \
    else                                        \
    {                                           \
        (m) = (m) >> ((ULONG) 16);              \
        (b) = (b) + ((ULONG) 16);               \
        if ((m) >= ((ULONG) 0x100))             \
        {                                       \
            (m) = (m) >> ((ULONG) 8);           \
            (b) = (b) + ((ULONG) 8);            \
        }                                       \
        if ((m) >= ((ULONG) 16))                \
        {                                       \
            (m) = (m) >> ((ULONG) 4);           \
            (b) = (b) + ((ULONG) 4);            \
        }                                       \
        if ((m) >= ((ULONG) 4))                 \
        {                                       \
            (m) = (m) >> ((ULONG) 2);           \
            (b) = (b) + ((ULONG) 2);            \
        }                                       \
        (b) = (b) + ((m) >> ((ULONG) 1));       \
    }


/* Define the default thread stack checking. This can be overridden by
   a particular port, which is necessary if the stack growth is from
   low address to high address (the default logic is for stacks that
   grow from high address to low address.  */



/* Define default post thread delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#define TX_THREAD_DELETE_PORT_COMPLETION(t)


/* Define default post thread reset macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#define TX_THREAD_RESET_PORT_COMPLETION(t)


/* Define the thread create internal extension macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#define TX_THREAD_CREATE_INTERNAL_EXTENSION(t)


/* Define internal thread control function prototypes.  */

VOID        _tx_thread_initialize(VOID);
VOID        _tx_thread_schedule(VOID);
VOID        _tx_thread_shell_entry(VOID);
VOID        _tx_thread_stack_analyze(TX_THREAD *thread_ptr);
VOID        _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID));
VOID        _tx_thread_stack_error(TX_THREAD *thread_ptr);
VOID        _tx_thread_stack_error_handler(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_preempt_check(VOID);
VOID        _tx_thread_system_resume(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_ni_resume(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_return(VOID);
VOID        _tx_thread_system_suspend(TX_THREAD *thread_ptr);
VOID        _tx_thread_system_ni_suspend(TX_THREAD *thread_ptr, ULONG wait_option);
VOID        _tx_thread_time_slice(VOID);
VOID        _tx_thread_timeout(ULONG timeout_input);


/* Thread control component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#define THREAD_DECLARE extern


/* Define the pointer that contains the system stack pointer.  This is
   utilized when control returns from a thread to the system to reset the
   current stack.  This is setup in the low-level initialization function. */

THREAD_DECLARE  VOID *          _tx_thread_system_stack_ptr;


/* Define the current thread pointer.  This variable points to the currently
   executing thread.  If this variable is NULL, no thread is executing.  */

THREAD_DECLARE  TX_THREAD *     _tx_thread_current_ptr;


/* Define the variable that holds the next thread to execute.  It is important
   to remember that this is not necessarily equal to the current thread
   pointer.  */

THREAD_DECLARE  TX_THREAD *     _tx_thread_execute_ptr;


/* Define the head pointer of the created thread list.  */

THREAD_DECLARE  TX_THREAD *     _tx_thread_created_ptr;


/* Define the variable that holds the number of created threads. */

THREAD_DECLARE  ULONG           _tx_thread_created_count;


/* Define the current state variable.  When this value is 0, a thread
   is executing or the system is idle.  Other values indicate that
   interrupt or initialization processing is active.  This variable is
   initialized to TX_INITIALIZE_IN_PROGRESS to indicate initialization is
   active.  */

THREAD_DECLARE  volatile ULONG  _tx_thread_system_state;


/* Define the 32-bit priority bit-maps. There is one priority bit map for each
   32 priority levels supported. If only 32 priorities are supported there is
   only one bit map. Each bit within a priority bit map represents that one
   or more threads at the associated thread priority are ready.  */

THREAD_DECLARE  ULONG           _tx_thread_priority_maps[TX_MAX_PRIORITIES/32];


/* Define the priority map active bit map that specifies which of the previously
   defined priority maps have something set. This is only necessary if more than
   32 priorities are supported.  */




/* Define the 32-bit preempt priority bit maps.  There is one preempt bit map
   for each 32 priority levels supported. If only 32 priorities are supported
   there is only one bit map. Each set set bit corresponds to a preempted priority
   level that had preemption-threshold active to protect against preemption of a
   range of relatively higher priority threads.  */

THREAD_DECLARE  ULONG           _tx_thread_preempted_maps[TX_MAX_PRIORITIES/32];


/* Define the preempt map active bit map that specifies which of the previously
   defined preempt maps have something set. This is only necessary if more than
   32 priorities are supported.  */


/* Define the variable that holds the highest priority group ready for
   execution.  It is important to note that this is not necessarily the same
   as the priority of the thread pointed to by _tx_execute_thread.  */

THREAD_DECLARE  UINT            _tx_thread_highest_priority;


/* Define the array of thread pointers.  Each entry represents the threads that
   are ready at that priority group.  For example, index 10 in this array
   represents the first thread ready at priority 10.  If this entry is NULL,
   no threads are ready at that priority.  */

THREAD_DECLARE  TX_THREAD *     _tx_thread_priority_list[TX_MAX_PRIORITIES];


/* Define the global preempt disable variable.  If this is non-zero, preemption is
   disabled.  It is used internally by ThreadX to prevent preemption of a thread in
   the middle of a service that is resuming or suspending another thread.  */

THREAD_DECLARE  volatile UINT   _tx_thread_preempt_disable;


/* Define the global function pointer for mutex cleanup on thread completion or
   termination. This pointer is setup during mutex initialization.  */

THREAD_DECLARE  VOID            (*_tx_thread_mutex_release)(TX_THREAD *thread_ptr);


/* Define the global build options variable.  This contains a bit map representing
   how the ThreadX library was built. The following are the bit field definitions:

                    Bit(s)                   Meaning

                    31                  TX_NOT_INTERRUPTABLE defined
                    30                  TX_INLINE_THREAD_RESUME_SUSPEND define
                    29-24               Priority groups 1  -> 32 priorities
                                                        2  -> 64 priorities
                                                        3  -> 96 priorities

                                                        ...

                                                        32 -> 1024 priorities
                    23                  TX_TIMER_PROCESS_IN_ISR defined
                    22                  TX_REACTIVATE_INLINE defined
                    21                  TX_DISABLE_STACK_FILLING defined
                    20                  TX_ENABLE_STACK_CHECKING defined
                    19                  TX_DISABLE_PREEMPTION_THRESHOLD defined
                    18                  TX_DISABLE_REDUNDANT_CLEARING defined
                    17                  TX_DISABLE_NOTIFY_CALLBACKS defined
                    16                  TX_BLOCK_POOL_ENABLE_PERFORMANCE_INFO defined
                    15                  TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO defined
                    14                  TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO defined
                    13                  TX_MUTEX_ENABLE_PERFORMANCE_INFO defined
                    12                  TX_QUEUE_ENABLE_PERFORMANCE_INFO defined
                    11                  TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO defined
                    10                  TX_THREAD_ENABLE_PERFORMANCE_INFO defined
                    9                   TX_TIMER_ENABLE_PERFORMANCE_INFO defined
                    8                   TX_ENABLE_EVENT_TRACE defined
                    7                   TX_ENABLE_EXECUTION_CHANGE_NOTIFY defined
                    6-0                 Port Specific   */

THREAD_DECLARE  ULONG           _tx_build_options;




#endif

