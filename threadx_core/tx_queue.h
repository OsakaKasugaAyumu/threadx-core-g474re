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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_queue.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX queue management component,           */
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

#ifndef TX_QUEUE_H
#define TX_QUEUE_H


/* Define queue control specific data definitions.  */

#define TX_QUEUE_ID                             ((ULONG) 0x51554555)


/* Determine if in-line component initialization is supported by the
   caller.  */

/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_queue_initialize(VOID);


/* Define the message copy macro. Note that the source and destination
   pointers must be modified since they are used subsequently.  */



/* Define internal queue management function prototypes.  */

VOID        _tx_queue_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Queue management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#define QUEUE_DECLARE extern


/* Define the head pointer of the created queue list.  */

QUEUE_DECLARE  TX_QUEUE *   _tx_queue_created_ptr;


/* Define the variable that holds the number of created queues. */

QUEUE_DECLARE  ULONG        _tx_queue_created_count;




/* Define default post queue delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#define TX_QUEUE_DELETE_PORT_COMPLETION(q)


#endif

