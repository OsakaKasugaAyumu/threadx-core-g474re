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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_event_flags.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_event_flags_performance_system_info_get         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves system event flag performance information.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    sets                              Destination for total number of   */
/*                                        event flag sets                 */
/*    gets                              Destination for total number of   */
/*                                        event flag gets                 */
/*    suspensions                       Destination for total number of   */
/*                                        event flag suspensions          */
/*    timeouts                          Destination for total number of   */
/*                                        timeouts                        */
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
UINT  _tx_event_flags_performance_system_info_get(ULONG *sets, ULONG *gets, ULONG *suspensions, ULONG *timeouts)
{


UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (sets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (gets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
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

