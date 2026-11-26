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
#include "tx_timer.h"
#include "tx_thread.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_system_resume                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function places the specified thread on the list of ready      */
/*    threads at the thread's specific priority.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to resume   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_return              Return to the system          */
/*    _tx_thread_system_ni_resume           Noninterruptable thread resume*/
/*    _tx_timer_system_deactivate           Timer deactivate              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_create                     Thread create function        */
/*    _tx_thread_priority_change            Thread priority change        */
/*    _tx_thread_resume                     Application resume service    */
/*    _tx_thread_timeout                    Thread timeout                */
/*    _tx_thread_wait_abort                 Thread wait abort             */
/*    Other ThreadX Components                                            */
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
VOID  _tx_thread_system_resume(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT            priority;
ULONG           priority_bit;
TX_THREAD       *head_ptr;
TX_THREAD       *tail_ptr;
TX_THREAD       *execute_ptr;
TX_THREAD       *current_thread;
ULONG           combined_flags;





    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE


    /* Deactivate the timeout timer if necessary.  */
    if (thread_ptr -> tx_thread_timer.tx_timer_internal_list_head != TX_NULL)
    {

        /* Deactivate the thread's timeout timer.  */
        _tx_timer_system_deactivate(&(thread_ptr -> tx_thread_timer));
    }
    else
    {

        /* Clear the remaining time to ensure timer doesn't get activated.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);
    }


    /* Log the thread status change.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESUME, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&execute_ptr), TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr), TX_TRACE_INTERNAL_EVENTS)


    /* Decrease the preempt disabled count.  */
    _tx_thread_preempt_disable--;

    /* Determine if the thread is in the process of suspending.  If so, the thread
       control block is already on the linked list so nothing needs to be done.  */
    if (thread_ptr -> tx_thread_suspending == TX_FALSE)
    {

        /* Thread is not in the process of suspending. Now check to make sure the thread
           has not already been resumed.  */
        if (thread_ptr -> tx_thread_state != TX_READY)
        {

            /* No, now check to see if the delayed suspension flag is set.  */
            if (thread_ptr -> tx_thread_delayed_suspend == TX_FALSE)
            {

                /* Resume the thread!  */

                /* Make this thread ready.  */

                /* Change the state to ready.  */
                thread_ptr -> tx_thread_state =  TX_READY;

                /* Pickup priority of thread.  */
                priority =  thread_ptr -> tx_thread_priority;

                /* Thread state change.  */
                TX_THREAD_STATE_CHANGE(thread_ptr, TX_READY)

                /* Log the thread status change.  */
                TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_READY)


                /* Determine if there are other threads at this priority that are
                   ready.  */
                head_ptr =  _tx_thread_priority_list[priority];
                if (head_ptr == TX_NULL)
                {

                    /* First thread at this priority ready.  Add to the front of the list.  */
                    _tx_thread_priority_list[priority] =       thread_ptr;
                    thread_ptr -> tx_thread_ready_next =       thread_ptr;
                    thread_ptr -> tx_thread_ready_previous =   thread_ptr;


                    /* Or in the thread's priority bit.  */
                    TX_MOD32_BIT_SET(priority, priority_bit)
                    _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] | priority_bit;

                    /* Determine if this newly ready thread is the highest priority.  */
                    if (priority < _tx_thread_highest_priority)
                    {

                        /* A new highest priority thread is present. */

                        /* Update the highest priority variable.  */
                        _tx_thread_highest_priority =  priority;

                        /* Pickup the execute pointer. Since it is going to be referenced multiple
                           times, it is placed in a local variable.  */
                        execute_ptr =  _tx_thread_execute_ptr;

                        /* Determine if no thread is currently executing.  */
                        if (execute_ptr == TX_NULL)
                        {

                            /* Simply setup the execute pointer.  */
                            _tx_thread_execute_ptr =  thread_ptr;
                        }
                        else
                        {

                            /* Another thread has been scheduled for execution.  */

                            /* Check to see if this is a higher priority thread and determine if preemption is allowed.  */
                            if (priority < execute_ptr -> tx_thread_preempt_threshold)
                            {


                                /* Determine if the preempted thread had preemption-threshold set.  */
                                if (execute_ptr -> tx_thread_preempt_threshold != execute_ptr -> tx_thread_priority)
                                {


                                    /* Remember that this thread was preempted by a thread above the thread's threshold.  */
                                    TX_MOD32_BIT_SET(execute_ptr -> tx_thread_priority, priority_bit)
                                    _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] | priority_bit;
                                }


                                /* Yes, modify the execute thread pointer.  */
                                _tx_thread_execute_ptr =  thread_ptr;

                            }
                        }
                    }
                }
                else
                {

                    /* No, there are other threads at this priority already ready.  */

                    /* Just add this thread to the priority list.  */
                    tail_ptr =                                 head_ptr -> tx_thread_ready_previous;
                    tail_ptr -> tx_thread_ready_next =         thread_ptr;
                    head_ptr -> tx_thread_ready_previous =     thread_ptr;
                    thread_ptr -> tx_thread_ready_previous =   tail_ptr;
                    thread_ptr -> tx_thread_ready_next =       head_ptr;
                }
            }

            /* Else, delayed suspend flag was set.  */
            else
            {

                /* Clear the delayed suspend flag and change the state.  */
                thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;
                thread_ptr -> tx_thread_state =            TX_SUSPENDED;
            }
        }
    }
    else
    {

        /* A resumption occurred in the middle of a previous thread suspension.  */

        /* Make sure the type of suspension under way is not a terminate or
           thread completion.  In either of these cases, do not void the
           interrupted suspension processing.  */
        if (thread_ptr -> tx_thread_state != TX_COMPLETED)
        {

            /* Make sure the thread isn't terminated.  */
            if (thread_ptr -> tx_thread_state != TX_TERMINATED)
            {

                /* No, now check to see if the delayed suspension flag is set.  */
                if (thread_ptr -> tx_thread_delayed_suspend == TX_FALSE)
                {

                    /* Clear the suspending flag.  */
                    thread_ptr -> tx_thread_suspending =   TX_FALSE;

                    /* Restore the state to ready.  */
                    thread_ptr -> tx_thread_state =        TX_READY;

                    /* Thread state change.  */
                    TX_THREAD_STATE_CHANGE(thread_ptr, TX_READY)

                    /* Log the thread status change.  */
                    TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_READY)
                }
                else
                {

                    /* Clear the delayed suspend flag and change the state.  */
                    thread_ptr -> tx_thread_delayed_suspend =  TX_FALSE;
                    thread_ptr -> tx_thread_state =            TX_SUSPENDED;
                }

            }
        }
    }



    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* Restore interrupts.  */
    TX_RESTORE

    /* Determine if a preemption condition is present.  */
    if (current_thread != _tx_thread_execute_ptr)
    {


        /* Now determine if preemption should take place. This is only possible if the current thread pointer is
           not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
        TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
        if (combined_flags == ((ULONG) 0))
        {


            /* Preemption is needed - return to the system!  */
            _tx_thread_system_return();
        }
    }
}
