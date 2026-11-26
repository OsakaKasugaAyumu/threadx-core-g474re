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

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    tx_trace.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the ThreadX trace component, including constants  */
/*    and structure definitions as well as external references.  It is    */
/*    assumed that tx_api.h and tx_port.h have already been included.     */
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


/* Include necessary system files.  */

#ifndef TX_TRACE_H
#define TX_TRACE_H


/* Determine if tracing is enabled. If not, simply define the in-line trace
   macros to whitespace.  */

#define TX_TRACE_INITIALIZE
#define TX_TRACE_OBJECT_REGISTER(t,p,n,a,b)
#define TX_TRACE_OBJECT_UNREGISTER(o)
#define TX_TRACE_IN_LINE_INSERT(i,a,b,c,d,f)


#ifdef TX_SOURCE_CODE

/* Define internal function prototypes of the trace component, only if compiling ThreadX source code.  */

VOID    _tx_trace_initialize(VOID);
VOID    _tx_trace_object_register(UCHAR object_type, VOID *object_ptr, CHAR *object_name, ULONG parameter_1, ULONG parameter_2);
VOID    _tx_trace_object_unregister(VOID *object_ptr);


#endif
#endif

