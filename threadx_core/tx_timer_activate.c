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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_timer_activate                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function activates the specified application timer.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    timer_ptr                         Pointer to timer control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_SUCCESS                        Always returns success            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_timer_system_activate         Actual timer activation function  */
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
UINT  _tx_timer_activate(TX_TIMER *timer_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT        status;


    /* Disable interrupts to put the timer on the created list.  */
    TX_DISABLE



    /* Check for an already active timer.  */
    if (timer_ptr -> tx_timer_internal.tx_timer_internal_list_head != TX_NULL)
    {

        /* Timer is already active, return an error.  */
        status =  TX_ACTIVATE_ERROR;
    }

    /* Check for a timer with a zero expiration.  */
    else if (timer_ptr -> tx_timer_internal.tx_timer_internal_remaining_ticks == ((ULONG) 0))
    {

        /* Timer is being activated with a zero expiration.  */
        status =  TX_ACTIVATE_ERROR;
    }
    else
    {


        /* Call actual activation function.  */
        _tx_timer_system_activate(&(timer_ptr -> tx_timer_internal));

        /* Return a successful status.  */
        status =  TX_SUCCESS;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(status);
}

