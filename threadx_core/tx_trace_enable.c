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
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_trace_enable                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the ThreadX trace buffer and the          */
/*    associated control variables, enabling it for operation.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    trace_buffer_start                    Start of trace buffer         */
/*    trace_buffer_size                     Size (bytes) of trace buffer  */
/*    registry_entries                      Number of object registry     */
/*                                            entries.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    _tx_trace_object_register             Register existing objects     */
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
UINT  _tx_trace_enable(VOID *trace_buffer_start, ULONG trace_buffer_size, ULONG registry_entries)
{


UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (trace_buffer_start != TX_NULL)
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (trace_buffer_size == ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (registry_entries == ((ULONG) 0))
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
}



