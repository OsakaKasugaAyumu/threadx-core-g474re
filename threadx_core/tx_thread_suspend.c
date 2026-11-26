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
#include "tx_trace.h"
#include "tx_thread.h"
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_suspend                                  PORTABLE C      */
/*                                                           6.1.1        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles application suspend requests.  If the suspend */
/*    requires actual processing, this function calls the actual suspend  */
/*    thread routine.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Return completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend         Actual thread suspension          */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-16-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added type cast to address  */
/*                                            a MISRA compliance issue,   */
/*                                            resulting in version 6.1.1  */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_suspend(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD  *current_thread;
UINT        status;



    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_SUSPEND_INSERT

    /* Check the specified thread's current status.  */
    if (thread_ptr -> tx_thread_state == TX_READY)
    {

        /* Initialize status to success.  */
        status =  TX_SUCCESS;

        /* Determine if we are in a thread context.  */
        if (TX_THREAD_GET_SYSTEM_STATE() == ((ULONG) 0))
        {

            /* Yes, we are in a thread context.  */

            /* Determine if the current thread is also the suspending thread.  */
            if (current_thread == thread_ptr)
            {

                /* Now determine if the preempt disable flag is non-zero.  */
                if (_tx_thread_preempt_disable != ((UINT) 0))
                {

                    /* Current thread cannot suspend when the preempt disable flag is non-zero,
                       return an error.  */
                    status =  TX_SUSPEND_ERROR;
                }
            }
        }

        /* Determine if the status is still successful.  */
        if (status == TX_SUCCESS)
        {

            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;


            /* Set the suspending flag. */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Setup for no timeout period.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Call actual thread suspension routine.  */
            _tx_thread_system_suspend(thread_ptr);


            /* Disable interrupts.  */
            TX_DISABLE

            /* Return success.  */
            status =  TX_SUCCESS;
        }
    }
    else if (thread_ptr -> tx_thread_state == TX_TERMINATED)
    {

        /* Thread is terminated.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_COMPLETED)
    {

        /* Thread is completed.  */
        status =  TX_SUSPEND_ERROR;
    }
    else if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

        /* Already suspended, just set status to success.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* Just set the delayed suspension flag.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_TRUE;

        /* Set status to success.  */
        status =  TX_SUCCESS;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Always return success, since this function does not perform error
       checking.  */
    return(status);

}

