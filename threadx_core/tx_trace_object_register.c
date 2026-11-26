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
/*    _tx_trace_object_register                           PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers a ThreadX system object in the trace        */
/*    registry area. This provides a mapping between the object pointers  */
/*    stored in each trace event to the actual ThreadX objects.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_type                           Type of system object         */
/*    object_ptr                            Address of system object      */
/*    object_name                           Name of system object         */
/*    parameter_1                           Supplemental parameter 1      */
/*    parameter_2                           Supplemental parameter 2      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
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
/*  05-19-2020      William E. Lamie        Initial Version 6.0           */
/*  09-30-2020      Yuxin Zhou              Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022      Scott Larson            Modified comment(s),          */
/*                                            check for null name,        */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_trace_object_register(UCHAR object_type, VOID *object_ptr, CHAR *object_name, ULONG parameter_1, ULONG parameter_2)
{


TX_INTERRUPT_SAVE_AREA


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (object_type != ((UCHAR) 0))
    {

        if (object_ptr != TX_NULL)
        {

            if (object_name != TX_NULL)
            {

                if (parameter_1 != ((ULONG) 0))
                {

                    if (parameter_2 != ((ULONG) 0))
                    {

                        /* NOP code.  */
                        TX_DISABLE
                        TX_RESTORE
                    }
                }
            }
        }
    }
}

