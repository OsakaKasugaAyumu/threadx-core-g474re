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
/*    _tx_timer_expiration_process                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes thread and application timer expirations.   */
/*    It is called from the _tx_timer_interrupt handler and either        */
/*    processes the timer expiration in the ISR or defers to the system   */
/*    timer thread. The actual processing is determined during            */
/*    compilation.                                                        */
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
/*    _tx_thread_system_resume          Thread resume processing          */
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */
/*    _tx_timer_system_activate         Timer reactivate processing       */
/*    Timer Expiration Function                                           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _tx_timer_interrupt               Timer interrupt handler           */
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
VOID  _tx_timer_expiration_process(VOID)
{

TX_INTERRUPT_SAVE_AREA



    /* Don't process in the ISR, wakeup the system timer thread to process the
       timer expiration.  */

    /* Disable interrupts.  */
    TX_DISABLE


    /* Increment the preempt disable flag.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Call the system resume function to activate the timer thread.  */
    _tx_thread_system_resume(&_tx_timer_thread);

}

