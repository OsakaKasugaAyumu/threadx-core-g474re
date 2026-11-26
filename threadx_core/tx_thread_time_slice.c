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
#include "tx_timer.h"
#include "tx_thread.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_time_slice                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves the currently executing thread to the end of    */
/*    the threads ready at the same priority level as a result of a       */
/*    time-slice interrupt.  If no other thread of the same priority is   */
/*    ready, this function simply returns.                                */
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
/*    _tx_timer_interrupt                   Timer interrupt handling      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Scott Larson             Modified comment(s), and      */
/*                                            opt out of function when    */
/*                                            TX_NO_TIMER is defined,     */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_thread_time_slice(VOID)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)


    /* Lockout interrupts while the time-slice is evaluated.  */
    TX_DISABLE

    /* Clear the expired time-slice flag.  */
    _tx_timer_expired_time_slice =  TX_FALSE;

    /* Make sure the thread pointer is valid.  */
    if (thread_ptr != TX_NULL)
    {

        /* Make sure the thread is still active, i.e. not suspended.  */
        if (thread_ptr -> tx_thread_state == TX_READY)
        {

            /* Setup a fresh time-slice for the thread.  */
            thread_ptr -> tx_thread_time_slice =  thread_ptr -> tx_thread_new_time_slice;

            /* Reset the actual time-slice variable.  */
            _tx_timer_time_slice =  thread_ptr -> tx_thread_time_slice;

            /* Determine if there is another thread at the same priority and preemption-threshold
               is not set.  Preemption-threshold overrides time-slicing.  */
            if (thread_ptr -> tx_thread_ready_next != thread_ptr)
            {

                /* Check to see if preemption-threshold is not being used.  */
                if (thread_ptr -> tx_thread_priority == thread_ptr -> tx_thread_preempt_threshold)
                {

                    /* Preemption-threshold is not being used by this thread.  */

                    /* There is another thread at this priority, make it the highest at
                       this priority level.  */
                    _tx_thread_priority_list[thread_ptr -> tx_thread_priority] =  thread_ptr -> tx_thread_ready_next;

                    /* Designate the highest priority thread as the one to execute.  Don't use this
                       thread's priority as an index just in case a higher priority thread is now
                       ready!  */
                    _tx_thread_execute_ptr =  _tx_thread_priority_list[_tx_thread_highest_priority];



                }
            }
        }
    }


    /* Restore previous interrupt posture.  */
    TX_RESTORE

}

