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
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_byte_allocate                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates bytes from the specified memory byte        */
/*    pool.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to pool control block     */
/*    memory_ptr                        Pointer to place allocated bytes  */
/*                                        pointer                         */
/*    memory_size                       Number of bytes to allocate       */
/*    wait_option                       Suspension option                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_suspend         Suspend thread service            */
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */
/*    _tx_byte_pool_search              Search byte pool for memory       */
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
UINT  _tx_byte_allocate(TX_BYTE_POOL *pool_ptr, VOID **memory_ptr, ULONG memory_size,  ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

UINT                        status;
TX_THREAD                   *thread_ptr;
UCHAR                       *work_ptr;
UINT                        suspended_count;
TX_THREAD                   *next_thread;
TX_THREAD                   *previous_thread;
UINT                        finished;


    /* Round the memory size up to the next size that is evenly divisible by
       an ALIGN_TYPE (this is typically a 32-bit ULONG).  This guarantees proper alignment.  */
    memory_size = (((memory_size + (sizeof(ALIGN_TYPE)))-((ALIGN_TYPE) 1))/(sizeof(ALIGN_TYPE))) * (sizeof(ALIGN_TYPE));

    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)




    /* Set the search finished flag to false.  */
    finished =  TX_FALSE;

    /* Loop to handle cases where the owner of the pool changed.  */
    do
    {

        /* Indicate that this thread is the current owner.  */
        pool_ptr -> tx_byte_pool_owner =  thread_ptr;

        /* Restore interrupts.  */
        TX_RESTORE

        /* At this point, the executing thread owns the pool and can perform a search
           for free memory.  */
        work_ptr =  _tx_byte_pool_search(pool_ptr, memory_size);

        /* Optional processing extension.  */
        TX_BYTE_ALLOCATE_EXTENSION

        /* Lockout interrupts.  */
        TX_DISABLE

        /* Determine if we are finished.  */
        if (work_ptr != TX_NULL)
        {

            /* Yes, we have found a block the search is finished.  */
            finished =  TX_TRUE;
        }
        else
        {

            /* No block was found, does this thread still own the pool?  */
            if (pool_ptr -> tx_byte_pool_owner == thread_ptr)
            {

                /* Yes, then we have looked through the entire pool and haven't found the memory.  */
                finished =  TX_TRUE;
            }
        }

    } while (finished == TX_FALSE);

    /* Copy the pointer into the return destination.  */
    *memory_ptr =  (VOID *) work_ptr;

    /* Determine if memory was found.  */
    if (work_ptr != TX_NULL)
    {



        /* Restore interrupts.  */
        TX_RESTORE

        /* Set the status to success.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* No memory of sufficient size was found...  */

        /* Determine if the request specifies suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Determine if the preempt disable flag is non-zero.  */
            if (_tx_thread_preempt_disable != ((UINT) 0))
            {

                /* Suspension is not allowed if the preempt disable flag is non-zero at this point - return error completion.  */
                status =  TX_NO_MEMORY;

                /* Restore interrupts.  */
                TX_RESTORE
            }
            else
            {

                /* Prepare for suspension of this thread.  */


                /* Setup cleanup routine pointer.  */
                thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_byte_pool_cleanup);

                /* Setup cleanup information, i.e. this pool control
                   block.  */
                thread_ptr -> tx_thread_suspend_control_block =  (VOID *) pool_ptr;

                /* Save the return memory pointer address as well.  */
                thread_ptr -> tx_thread_additional_suspend_info =  (VOID *) memory_ptr;

                /* Save the byte size requested.  */
                thread_ptr -> tx_thread_suspend_info =  memory_size;


                /* Increment the suspension sequence number, which is used to identify
                   this suspension event.  */
                thread_ptr -> tx_thread_suspension_sequence++;

                /* Pickup the number of suspended threads.  */
                suspended_count =  pool_ptr -> tx_byte_pool_suspended_count;

                /* Increment the suspension count.  */
                (pool_ptr -> tx_byte_pool_suspended_count)++;

                /* Setup suspension list.  */
                if (suspended_count == TX_NO_SUSPENSIONS)
                {

                    /* No other threads are suspended.  Setup the head pointer and
                       just setup this threads pointers to itself.  */
                    pool_ptr -> tx_byte_pool_suspension_list =      thread_ptr;
                    thread_ptr -> tx_thread_suspended_next =        thread_ptr;
                    thread_ptr -> tx_thread_suspended_previous =    thread_ptr;
                }
                else
                {

                    /* This list is not NULL, add current thread to the end. */
                    next_thread =                                   pool_ptr -> tx_byte_pool_suspension_list;
                    thread_ptr -> tx_thread_suspended_next =        next_thread;
                    previous_thread =                               next_thread -> tx_thread_suspended_previous;
                    thread_ptr -> tx_thread_suspended_previous =    previous_thread;
                    previous_thread -> tx_thread_suspended_next =   thread_ptr;
                    next_thread -> tx_thread_suspended_previous =   thread_ptr;
                }

                /* Set the state to suspended.  */
                thread_ptr -> tx_thread_state =       TX_BYTE_MEMORY;


                /* Set the suspending flag.  */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Setup the timeout period.  */
                thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

                /* Temporarily disable preemption.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);



                /* Return the completion status.  */
                status =  thread_ptr -> tx_thread_suspend_status;
            }
        }
        else
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Immediate return, return error completion.  */
            status =  TX_NO_MEMORY;
        }
    }

    /* Return completion status.  */
    return(status);
}

