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
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_thread_performance_system_info_get              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves thread system performance information.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    resumptions                       Destination for total number of   */
/*                                        thread resumptions              */
/*    suspensions                       Destination for total number of   */
/*                                        thread suspensions              */
/*    solicited_preemptions             Destination for total number of   */
/*                                        thread preemption from thread   */
/*                                        API calls                       */
/*    interrupt_preemptions             Destination for total number of   */
/*                                        thread preemptions as a result  */
/*                                        of threads made ready inside of */
/*                                        Interrupt Service Routines      */
/*    priority_inversions               Destination for total number of   */
/*                                        priority inversions             */
/*    time_slices                       Destination for total number of   */
/*                                        time-slices                     */
/*    relinquishes                      Destination for total number of   */
/*                                        relinquishes                    */
/*    timeouts                          Destination for total number of   */
/*                                        timeouts                        */
/*    wait_aborts                       Destination for total number of   */
/*                                        wait aborts                     */
/*    non_idle_returns                  Destination for total number of   */
/*                                        times threads return when       */
/*                                        another thread is ready         */
/*    idle_returns                      Destination for total number of   */
/*                                        times threads return when no    */
/*                                        other thread is ready           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _tx_thread_performance_system_info_get(ULONG *resumptions, ULONG *suspensions,
                ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions,
                ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts,
                ULONG *non_idle_returns, ULONG *idle_returns)
{


UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (resumptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (solicited_preemptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (interrupt_preemptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (priority_inversions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (time_slices != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (relinquishes != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (wait_aborts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (non_idle_returns != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (idle_returns != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
}

