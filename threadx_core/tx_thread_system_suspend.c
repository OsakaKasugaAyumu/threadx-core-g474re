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
/*    _tx_thread_system_suspend                           PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function suspends the specified thread and changes the thread  */
/*    state to the value specified.  Note: delayed suspension processing  */
/*    is handled outside of this routine.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    thread_ptr                            Pointer to thread to suspend  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_return              Return to system              */
/*    _tx_thread_system_preempt_check       System preemption check       */
/*    _tx_timer_system_activate             Activate timer for timeout    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_thread_priority_change            Thread priority change        */
/*    _tx_thread_shell_entry                Thread shell function         */
/*    _tx_thread_sleep                      Thread sleep                  */
/*    _tx_thread_suspend                    Application thread suspend    */
/*    _tx_thread_terminate                  Thread terminate              */
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
VOID  _tx_thread_system_suspend(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT            priority;
UINT            base_priority;
ULONG           priority_map;
ULONG           priority_bit;
ULONG           combined_flags;
TX_THREAD       *ready_next;
TX_THREAD       *ready_previous;
TX_THREAD       *current_thread;


ULONG           timeout;


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)


    /* Lockout interrupts while the thread is being suspended.  */
    TX_DISABLE


    /* Is the current thread suspending?  */
    if (thread_ptr == current_thread)
    {

        /* Pickup the wait option.  */
        timeout =  thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks;

        /* Determine if an activation is needed.  */
        if (timeout != TX_NO_WAIT)
        {

            /* Make sure the suspension is not a wait-forever.  */
            if (timeout != TX_WAIT_FOREVER)
            {

                /* Activate the thread timer with the timeout value setup in the caller.  */
                _tx_timer_system_activate(&(thread_ptr -> tx_thread_timer));
            }
        }

        /* Yes, reset time slice for current thread.  */
        _tx_timer_time_slice =  thread_ptr -> tx_thread_new_time_slice;
    }

    /* Decrease the preempt disabled count.  */
    _tx_thread_preempt_disable--;


    /* Check to make sure the thread suspending flag is still set.  If not, it
       has already been resumed.  */
    if (thread_ptr -> tx_thread_suspending == TX_TRUE)
    {

        /* Thread state change.  */
        TX_THREAD_STATE_CHANGE(thread_ptr, thread_ptr -> tx_thread_state)

        /* Log the thread status change.  */
        TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, thread_ptr -> tx_thread_state)


        /* Log the thread status change.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SUSPEND, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&priority), TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr), TX_TRACE_INTERNAL_EVENTS)


        /* Actually suspend this thread.  But first, clear the suspending flag.  */
        thread_ptr -> tx_thread_suspending =  TX_FALSE;

        /* Pickup priority of thread.  */
        priority =  thread_ptr -> tx_thread_priority;

        /* Pickup the next ready thread pointer.  */
        ready_next =      thread_ptr -> tx_thread_ready_next;

        /* Determine if there are other threads at this priority that are
           ready.  */
        if (ready_next != thread_ptr)
        {

            /* Yes, there are other threads at this priority ready.  */

            /* Pickup the previous ready thread pointer.  */
            ready_previous =  thread_ptr -> tx_thread_ready_previous;

            /* Just remove this thread from the priority list.  */
            ready_next -> tx_thread_ready_previous =    ready_previous;
            ready_previous -> tx_thread_ready_next =    ready_next;

            /* Determine if this is the head of the priority list.  */
            if (_tx_thread_priority_list[priority] == thread_ptr)
            {

                /* Update the head pointer of this priority list.  */
                _tx_thread_priority_list[priority] =  ready_next;



                /* Check for a thread preempted that had preemption threshold set.  */
                if (_tx_thread_preempted_maps[MAP_INDEX] != ((ULONG) 0))
                {

                    /* Ensure that this thread's priority is clear in the preempt map.  */
                    TX_MOD32_BIT_SET(priority, priority_bit)
                    _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

                }
            }
        }
        else
        {

            /* This is the only thread at this priority ready to run.  Set the head
               pointer to NULL.  */
            _tx_thread_priority_list[priority] =    TX_NULL;


            /* Clear this priority bit in the ready priority bit map.  */
            TX_MOD32_BIT_SET(priority, priority_bit)
            _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] & (~(priority_bit));



            /* Check for a thread preempted that had preemption-threshold set.  */
            if (_tx_thread_preempted_maps[MAP_INDEX] != ((ULONG) 0))
            {

                /* Ensure that this thread's priority is clear in the preempt map.  */
                TX_MOD32_BIT_SET(priority, priority_bit)
                _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

            }


            /* Setup the base priority to zero.  */
            base_priority =   ((UINT) 0);

            /* Setup working variable for the priority map.  */
            priority_map =    _tx_thread_priority_maps[MAP_INDEX];

            /* Make a quick check for no other threads ready for execution.  */
            if (priority_map == ((ULONG) 0))
            {

                /* Nothing else is ready.  Set highest priority and execute thread
                   accordingly.  */
                _tx_thread_highest_priority =  ((UINT) TX_MAX_PRIORITIES);
                _tx_thread_execute_ptr =       TX_NULL;

            }
            else
            {

                /* Other threads at different priority levels are ready to run.  */

                /* Calculate the lowest bit set in the priority map. */
                TX_LOWEST_SET_BIT_CALCULATE(priority_map, priority_bit)

                /* Setup the next highest priority variable.  */
                _tx_thread_highest_priority =  base_priority + ((UINT) priority_bit);
            }
        }

        /* Determine if the suspending thread is the thread designated to execute.  */
        if (thread_ptr == _tx_thread_execute_ptr)
        {

            /* Pickup the highest priority thread to execute.  */
            _tx_thread_execute_ptr =  _tx_thread_priority_list[_tx_thread_highest_priority];


            /* Determine if a previous thread with preemption-threshold was preempted.  */
            if (_tx_thread_preempted_maps[MAP_INDEX] != ((ULONG) 0))
            {

                /* Yes, there was a thread preempted when it was using preemption-threshold.  */

                /* Disable preemption.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Interrupts are enabled briefly here to keep the interrupt
                   lockout time deterministic.  */

                /* Disable interrupts again.  */
                TX_DISABLE

                /* Decrement the preemption disable variable.  */
                _tx_thread_preempt_disable--;

                /* Calculate the thread with preemption threshold set that
                   was interrupted by a thread above the preemption level.  */


                /* Setup the base priority to zero.  */
                base_priority =   ((UINT) 0);

                /* Setup temporary preempted map.  */
                priority_map =  _tx_thread_preempted_maps[MAP_INDEX];

                /* Calculate the lowest bit set in the priority map. */
                TX_LOWEST_SET_BIT_CALCULATE(priority_map, priority_bit)

                /* Setup the highest priority preempted thread.  */
                priority =  base_priority + ((UINT) priority_bit);

                /* Determine if the next highest priority thread is above the highest priority threshold value.  */
                if (_tx_thread_highest_priority >= (_tx_thread_priority_list[priority] -> tx_thread_preempt_threshold))
                {

                    /* Thread not allowed to execute until earlier preempted thread finishes or lowers its
                       preemption-threshold.  */
                    _tx_thread_execute_ptr =  _tx_thread_priority_list[priority];

                    /* Clear the corresponding bit in the preempted map, since the preemption has been restored.  */
                    TX_MOD32_BIT_SET(priority, priority_bit)
                    _tx_thread_preempted_maps[MAP_INDEX] =  _tx_thread_preempted_maps[MAP_INDEX] & (~(priority_bit));

                }
            }

        }


    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Determine if a preemption condition is present.  */
    if (current_thread != _tx_thread_execute_ptr)
    {


        /* Determine if preemption should take place. This is only possible if the current thread pointer is
           not the same as the execute thread pointer AND the system state and preempt disable flags are clear.  */
        TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)
        if (combined_flags == ((ULONG) 0))
        {


            /* Preemption is needed - return to the system!  */
            _tx_thread_system_return();
        }
    }

    /* Return to caller.  */
    return;
}

