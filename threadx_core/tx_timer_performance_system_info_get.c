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
/*    _tx_timer_performance_system_info_get               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves timer performance information.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    activates                         Destination for total number of   */
/*                                        activations                     */
/*    reactivates                       Destination for total number of   */
/*                                        reactivations                   */
/*    deactivates                       Destination for total number of   */
/*                                        deactivations                   */
/*    expirations                       Destination for total number of   */
/*                                        expirations                     */
/*    expiration_adjusts                Destination for total number of   */
/*                                        expiration adjustments          */
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
UINT  _tx_timer_performance_system_info_get(ULONG *activates, ULONG *reactivates,
                    ULONG *deactivates, ULONG *expirations, ULONG *expiration_adjusts)
{


UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (activates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (reactivates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (deactivates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (expirations != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (expiration_adjusts != TX_NULL)
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

