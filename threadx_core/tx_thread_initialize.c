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

#define TX_SOURCE_CODE




/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread.h"


/* Define the pointer that contains the system stack pointer.  This is
   utilized when control returns from a thread to the system to reset the
   current stack.  This is setup in the low-level initialization function. */

VOID *          _tx_thread_system_stack_ptr;


/* Define the current thread pointer.  This variable points to the currently
   executing thread.  If this variable is NULL, no thread is executing.  */

TX_THREAD *     _tx_thread_current_ptr;


/* Define the variable that holds the next thread to execute.  It is important
   to remember that this is not necessarily equal to the current thread 
   pointer.  */

TX_THREAD *     _tx_thread_execute_ptr;


/* Define the head pointer of the created thread list.  */

TX_THREAD *     _tx_thread_created_ptr;


/* Define the variable that holds the number of created threads. */

ULONG           _tx_thread_created_count;


/* Define the current state variable.  When this value is 0, a thread
   is executing or the system is idle.  Other values indicate that 
   interrupt or initialization processing is active.  This variable is
   initialized to TX_INITIALIZE_IN_PROGRESS to indicate initialization is
   active.  */

volatile ULONG  _tx_thread_system_state =  TX_INITIALIZE_IN_PROGRESS;


/* Define the 32-bit priority bit-maps. There is one priority bit map for each
   32 priority levels supported. If only 32 priorities are supported there is 
   only one bit map. Each bit within a priority bit map represents that one 
   or more threads at the associated thread priority are ready.  */

ULONG           _tx_thread_priority_maps[TX_MAX_PRIORITIES/32];


/* Define the priority map active bit map that specifies which of the previously 
   defined priority maps have something set. This is only necessary if more than 
   32 priorities are supported.  */




/* Define the 32-bit preempt priority bit maps.  There is one preempt bit map 
   for each 32 priority levels supported. If only 32 priorities are supported 
   there is only one bit map. Each set set bit corresponds to a preempted priority 
   level that had preemption-threshold active to protect against preemption of a 
   range of relatively higher priority threads.  */

ULONG           _tx_thread_preempted_maps[TX_MAX_PRIORITIES/32];


/* Define the preempt map active bit map that specifies which of the previously 
   defined preempt maps have something set. This is only necessary if more than 
   32 priorities are supported.  */


/* Define the variable that holds the highest priority group ready for 
   execution.  It is important to note that this is not necessarily the same
   as the priority of the thread pointed to by _tx_execute_thread.  */

UINT            _tx_thread_highest_priority;


/* Define the array of thread pointers.  Each entry represents the threads that
   are ready at that priority group.  For example, index 10 in this array
   represents the first thread ready at priority 10.  If this entry is NULL,
   no threads are ready at that priority.  */

TX_THREAD *     _tx_thread_priority_list[TX_MAX_PRIORITIES];


/* Define the global preempt disable variable.  If this is non-zero, preemption is
   disabled.  It is used internally by ThreadX to prevent preemption of a thread in 
   the middle of a service that is resuming or suspending another thread.  */

volatile UINT   _tx_thread_preempt_disable;


/* Define the global function pointer for mutex cleanup on thread completion or 
   termination. This pointer is setup during mutex initialization.  */

VOID            (*_tx_thread_mutex_release)(TX_THREAD *thread_ptr);


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

ULONG           _tx_build_options;





/* Define special string.  */



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_initialize                               PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the thread control component.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_initialize_high_level         High level initialization         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Yuxin Zhou               Modified comment(s), added    */
/*                                            Execution Profile support,  */
/*                                            resulting in version 6.1.7  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            stack check error handling, */
/*                                            resulting in version 6.1.9  */   
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_initialize(VOID)
{

    /* Note: the system stack pointer and the system state variables are 
       initialized by the low and high-level initialization functions,
       respectively.  */


    /* Set current thread pointer to NULL.  */
    TX_THREAD_SET_CURRENT(TX_NULL)

    /* Initialize the execute thread pointer to NULL.  */
    _tx_thread_execute_ptr =  TX_NULL;

    /* Initialize the priority information.  */
    TX_MEMSET(&_tx_thread_priority_maps[0], 0, (sizeof(_tx_thread_priority_maps)));

    TX_MEMSET(&_tx_thread_preempted_maps[0], 0, (sizeof(_tx_thread_preempted_maps)));

    /* Setup the highest priority variable to the max, indicating no thread is currently
       ready.  */
    _tx_thread_highest_priority =  ((UINT) TX_MAX_PRIORITIES);



    /* Initialize the array of priority head pointers.  */
    TX_MEMSET(&_tx_thread_priority_list[0], 0, (sizeof(_tx_thread_priority_list)));

    /* Initialize the head pointer of the created threads list and the
       number of threads created.  */
    _tx_thread_created_ptr =        TX_NULL;
    _tx_thread_created_count =      TX_EMPTY;

    /* Clear the global preempt disable variable.  */
    _tx_thread_preempt_disable =    ((UINT) 0);

    /* Initialize the thread mutex release function pointer.  */
    _tx_thread_mutex_release =      TX_NULL;



    /* Setup the build options flag. This is used to identify how the ThreadX library was constructed.  */
    _tx_build_options =  _tx_build_options 
                            | (((ULONG) (TX_MAX_PRIORITIES/32)) << 24) 
                            ;
}

