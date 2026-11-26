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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_thread_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function manages thread and application timer expirations.     */
/*    Actually, from this thread's point of view, there is no difference. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_thread_input                Used just for verification        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    Timer Expiration Function                                           */
/*    _tx_thread_system_suspend         Thread suspension                 */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
/*    _tx_timer_system_activate         Timer reactivate processing       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX Scheduler                                                   */
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
VOID  _tx_timer_thread_entry(ULONG timer_thread_input)
{

TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL           *expired_timers;
TX_TIMER_INTERNAL           *reactivate_timer;
TX_TIMER_INTERNAL           *next_timer;
TX_TIMER_INTERNAL           *previous_timer;
TX_TIMER_INTERNAL           *current_timer;
VOID                        (*timeout_function)(ULONG id);
ULONG                       timeout_param =  ((ULONG) 0);
TX_THREAD                   *thread_ptr;


    /* Make sure the timer input is correct.  This also gets rid of the
       silly compiler warnings.  */
    if (timer_thread_input == TX_TIMER_ID)
    {

        /* Yes, valid thread entry, proceed...  */

        /* Now go into an infinite loop to process timer expirations.  */
        while (TX_LOOP_FOREVER)
        {

            /* First, move the current list pointer and clear the timer
               expired value.  This allows the interrupt handling portion
               to continue looking for timer expirations.  */
            TX_DISABLE

            /* Save the current timer expiration list pointer.  */
            expired_timers =  *_tx_timer_current_ptr;

            /* Modify the head pointer in the first timer in the list, if there
               is one!  */
            if (expired_timers != TX_NULL)
            {

                expired_timers -> tx_timer_internal_list_head =  &expired_timers;
            }

            /* Set the current list pointer to NULL.  */
            *_tx_timer_current_ptr =  TX_NULL;

            /* Move the current pointer up one timer entry wrap if we get to
               the end of the list.  */
            _tx_timer_current_ptr =  TX_TIMER_POINTER_ADD(_tx_timer_current_ptr, 1);
            if (_tx_timer_current_ptr == _tx_timer_list_end)
            {

                _tx_timer_current_ptr =  _tx_timer_list_start;
            }

            /* Clear the expired flag.  */
            _tx_timer_expired =  TX_FALSE;

            /* Restore interrupts temporarily.  */
            TX_RESTORE

            /* Disable interrupts again.  */
            TX_DISABLE

            /* Next, process the expiration of the associated timers at this
               time slot.  */
            while (expired_timers != TX_NULL)
            {

                /* Something is on the list.  Remove it and process the expiration.  */
                current_timer =  expired_timers;

                /* Pickup the next timer.  */
                next_timer =  expired_timers -> tx_timer_internal_active_next;

                /* Set the reactivate_timer to NULL.  */
                reactivate_timer =  TX_NULL;

                /* Determine if this is the only timer.  */
                if (current_timer == next_timer)
                {

                    /* Yes, this is the only timer in the list.  */

                    /* Set the head pointer to NULL.  */
                    expired_timers =  TX_NULL;
                }
                else
                {

                    /* No, not the only expired timer.  */

                    /* Remove this timer from the expired list.  */
                    previous_timer =                                   current_timer -> tx_timer_internal_active_previous;
                    next_timer -> tx_timer_internal_active_previous =  previous_timer;
                    previous_timer -> tx_timer_internal_active_next =  next_timer;

                    /* Modify the next timer's list head to point at the current list head.  */
                    next_timer -> tx_timer_internal_list_head =  &expired_timers;

                    /* Set the list head pointer.  */
                    expired_timers =  next_timer;
                }

                /* In any case, the timer is now off of the expired list.  */

                /* Determine if the timer has expired or if it is just a really
                   big timer that needs to be placed in the list again.  */
                if (current_timer -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
                {

                    /* Timer is bigger than the timer entries and must be
                       rescheduled.  */


                    /* Decrement the remaining ticks of the timer.  */
                    current_timer -> tx_timer_internal_remaining_ticks =
                            current_timer -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES;

                    /* Set the timeout function to NULL in order to bypass the
                       expiration.  */
                    timeout_function =  TX_NULL;

                    /* Make the timer appear that it is still active while interrupts
                       are enabled.  This will permit proper processing of a timer
                       deactivate from an ISR.  */
                    current_timer -> tx_timer_internal_list_head =    &reactivate_timer;
                    current_timer -> tx_timer_internal_active_next =  current_timer;

                    /* Setup the temporary timer list head pointer.  */
                    reactivate_timer =  current_timer;
                }
                else
                {

                    /* Timer did expire.  */


                    /* Copy the calling function and ID into local variables before interrupts
                       are re-enabled.  */
                    timeout_function =  current_timer -> tx_timer_internal_timeout_function;
                    timeout_param =     current_timer -> tx_timer_internal_timeout_param;

                    /* Copy the reinitialize ticks into the remaining ticks.  */
                    current_timer -> tx_timer_internal_remaining_ticks =  current_timer -> tx_timer_internal_re_initialize_ticks;

                    /* Determine if the timer should be reactivated.  */
                    if (current_timer -> tx_timer_internal_remaining_ticks != ((ULONG) 0))
                    {

                        /* Make the timer appear that it is still active while processing
                           the expiration routine and with interrupts enabled.  This will
                           permit proper processing of a timer deactivate from both the
                           expiration routine and an ISR.  */
                        current_timer -> tx_timer_internal_list_head =    &reactivate_timer;
                        current_timer -> tx_timer_internal_active_next =  current_timer;

                        /* Setup the temporary timer list head pointer.  */
                        reactivate_timer =  current_timer;
                    }
                    else
                    {

                        /* Set the list pointer of this timer to NULL.  This is used to indicate
                           the timer is no longer active.  */
                        current_timer -> tx_timer_internal_list_head =  TX_NULL;
                    }
                }

                /* Set pointer to indicate the expired timer that is currently being processed.  */
                _tx_timer_expired_timer_ptr =  current_timer;

                /* Restore interrupts for timer expiration call.  */
                TX_RESTORE

                /* Call the timer-expiration function, if non-NULL.  */
                if (timeout_function != TX_NULL)
                {

                    (timeout_function) (timeout_param);
                }

                /* Lockout interrupts again.  */
                TX_DISABLE

                /* Clear expired timer pointer.  */
                _tx_timer_expired_timer_ptr =  TX_NULL;

                /* Determine if the timer needs to be reactivated.  */
                if (reactivate_timer == current_timer)
                {

                    /* Reactivate the timer.  */



                    /* Reactivate through the timer activate function.  */

                    /* Clear the list head for the timer activate call.  */
                    current_timer -> tx_timer_internal_list_head = TX_NULL;

                    /* Activate the current timer.  */
                    _tx_timer_system_activate(current_timer);
                }

                /* Restore interrupts.  */
                TX_RESTORE

                /* Lockout interrupts again.  */
                TX_DISABLE
            }

            /* Finally, suspend this thread and wait for the next expiration.  */

            /* Determine if another expiration took place while we were in this
               thread.  If so, process another expiration.  */
            if (_tx_timer_expired == TX_FALSE)
            {

                /* Otherwise, no timer expiration, so suspend the thread.  */

                /* Build pointer to the timer thread.  */
                thread_ptr =  &_tx_timer_thread;

                /* Set the status to suspending, in order to indicate the
                   suspension is in progress.  */
                thread_ptr -> tx_thread_state =  TX_SUSPENDED;


                /* Set the suspending flag. */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Increment the preempt disable count prior to suspending.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);
            }
            else
            {

                /* Restore interrupts.  */
                TX_RESTORE
            }
        }
    }


    /* If we ever get here, raise safety critical exception.  */
#line 477
    TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);

}

