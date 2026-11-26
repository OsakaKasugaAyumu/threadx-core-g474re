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


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_event_flags.h                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX event flags management component,     */
/*    including all data types and external references.  It is assumed    */
/*    that tx_api.h and tx_port.h have already been included.             */
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

#ifndef TX_EVENT_FLAGS_H
#define TX_EVENT_FLAGS_H


/* Define event flags control specific data definitions.  */

#define TX_EVENT_FLAGS_ID                       ((ULONG) 0x4456444E)
#define TX_EVENT_FLAGS_AND_MASK                 ((UINT) 0x2)
#define TX_EVENT_FLAGS_CLEAR_MASK               ((UINT) 0x1)


/* Determine if in-line component initialization is supported by the
   caller.  */

/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_event_flags_initialize(VOID);


/* Define internal event flags management function prototypes.  */

VOID        _tx_event_flags_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Event flags management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#define EVENT_FLAGS_DECLARE extern


/* Define the head pointer of the created event flags list.  */

EVENT_FLAGS_DECLARE  TX_EVENT_FLAGS_GROUP * _tx_event_flags_created_ptr;


/* Define the variable that holds the number of created event flag groups. */

EVENT_FLAGS_DECLARE  ULONG                  _tx_event_flags_created_count;



/* Define default post event flag group delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#define TX_EVENT_FLAGS_GROUP_DELETE_PORT_COMPLETION(g)


#endif

