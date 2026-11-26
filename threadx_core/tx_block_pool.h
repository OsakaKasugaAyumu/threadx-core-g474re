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
/**   Block Memory                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_block_pool.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX block memory management component,    */
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

#ifndef TX_BLOCK_POOL_H
#define TX_BLOCK_POOL_H


/* Define block memory control specific data definitions.  */

#define TX_BLOCK_POOL_ID                        ((ULONG) 0x424C4F43)


/* Determine if in-line component initialization is supported by the
   caller.  */


/* No in-line initialization is supported, use standard function call.  */
VOID        _tx_block_pool_initialize(VOID);


/* Define internal block memory pool management function prototypes.  */

VOID        _tx_block_pool_cleanup(TX_THREAD *thread_ptr, ULONG suspension_sequence);


/* Block pool management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#define BLOCK_POOL_DECLARE extern


/* Define the head pointer of the created block pool list.  */

BLOCK_POOL_DECLARE  TX_BLOCK_POOL *         _tx_block_pool_created_ptr;


/* Define the variable that holds the number of created block pools. */

BLOCK_POOL_DECLARE  ULONG                   _tx_block_pool_created_count;




/* Define default post block pool delete macro to whitespace, if it hasn't been defined previously (typically in tx_port.h).  */

#define TX_BLOCK_POOL_DELETE_PORT_COMPLETION(p)


#endif
