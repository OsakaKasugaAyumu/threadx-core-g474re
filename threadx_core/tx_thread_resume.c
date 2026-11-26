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
#include "tx_initialize.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_resume                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes application resume thread services. Actual  */
/*    thread resumption is performed in the core service.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Service return status         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_resume          Resume thread                     */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _tx_thread_resume(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT        status;
TX_THREAD   *saved_thread_ptr;
UINT        saved_threshold =  ((UINT) 0);


    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESUME_API, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_RESUME_INSERT

    /* Determine if the thread is suspended or in the process of suspending.
       If so, call the thread resume processing.  */
    if (thread_ptr -> tx_thread_state == TX_SUSPENDED)
    {

        /* Determine if the create call is being called from initialization.  */
        if (TX_THREAD_GET_SYSTEM_STATE() >= TX_INITIALIZE_IN_PROGRESS)
        {

            /* Yes, this resume call was made from initialization.  */

            /* Pickup the current thread execute pointer, which corresponds to the
               highest priority thread ready to execute.  Interrupt lockout is
               not required, since interrupts are assumed to be disabled during
               initialization.  */
            saved_thread_ptr =  _tx_thread_execute_ptr;

            /* Determine if there is thread ready for execution.  */
            if (saved_thread_ptr != TX_NULL)
            {

                /* Yes, a thread is ready for execution when initialization completes.  */

                /* Save the current preemption-threshold.  */
                saved_threshold =  saved_thread_ptr -> tx_thread_preempt_threshold;

                /* For initialization, temporarily set the preemption-threshold to the
                   priority level to make sure the highest-priority thread runs once
                   initialization is complete.  */
                saved_thread_ptr -> tx_thread_preempt_threshold =  saved_thread_ptr -> tx_thread_priority;
            }
        }
        else
        {

            /* Simply set the saved thread pointer to NULL.  */
            saved_thread_ptr =  TX_NULL;
        }



        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Call the actual resume service to resume the thread.  */
        _tx_thread_system_resume(thread_ptr);

        /* Determine if the thread's preemption-threshold needs to be restored.  */
        if (saved_thread_ptr != TX_NULL)
        {

            /* Yes, restore the previous highest-priority thread's preemption-threshold. This
               can only happen if this routine is called from initialization.  */
            saved_thread_ptr -> tx_thread_preempt_threshold =  saved_threshold;
        }


        /* Disable interrupts.  */
        TX_DISABLE

        /* Setup successful return status.  */
        status =  TX_SUCCESS;


    }
    else if (thread_ptr -> tx_thread_delayed_suspend == TX_TRUE)
    {

        /* Clear the delayed suspension.  */
        thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;

        /* Setup delayed suspend lifted return status.  */
        status =  TX_SUSPEND_LIFTED;
    }
    else
    {

        /* Setup invalid resume return status.  */
        status =  TX_RESUME_ERROR;
    }

    /* Restore interrupts.  */
    TX_RESTORE


    /* Return completion status. */
    return(status);
}

