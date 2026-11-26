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
/**   Port Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    tx_port.h                                         Cortex-M4/GNU     */
/*                                                           6.1.12       */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains data type definitions that make the ThreadX      */
/*    real-time kernel function identically on a variety of different     */
/*    processor architectures.  For example, the size or number of bits   */
/*    in an "int" data type vary between microprocessor architectures and */
/*    even C compilers for the same microprocessor.  ThreadX does not     */
/*    directly use native C data types.  Instead, ThreadX creates its     */
/*    own special types that can be mapped to actual data types by this   */
/*    file to guarantee consistency in the interface and functionality.   */
/*                                                                        */
/*    This file replaces the previous Cortex-M3/M4/M7 files. It unifies   */
/*    the ARMv7-M architecture and compilers into one common file.        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021      Scott Larson            Initial Version 6.1.7         */
/*  01-31-2022      Scott Larson            Modified comments, updated    */
/*                                            typedef to fix misra        */
/*                                            violation,                  */
/*                                            fixed predefined macro,     */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022      Scott Larson            Modified comments and added   */
/*                                            volatile to registers,      */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022      Scott Larson            Modified comments and         */
/*                                            described BASEPRI usage,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#ifndef TX_PORT_H
#define TX_PORT_H


/* Determine if the optional ThreadX user define file should be used.  */


/* Yes, include the user defines in tx_user.h. The defines in this file may
   alternately be defined on the command line.  */

#include "tx_user.h"


/* Define compiler library include files.  */

#include <stdlib.h>
#include <string.h>






/* Define ThreadX basic types for this port.  */

#define VOID                                    void
typedef char                                    CHAR;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef unsigned long long                      ULONG64;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;
#define ULONG64_DEFINED

/* Define the priority levels for ThreadX.  Legal values range
   from 32 to 1024 and MUST be evenly divisible by 32.  */



/* Define the minimum stack for a ThreadX thread on this processor. If the size supplied during
   thread creation is less than this value, the thread create call will return an error.  */



/* Define the system timer thread's default stack size and priority.  These are only applicable
   if TX_TIMER_PROCESS_IN_ISR is not defined.  */



/* By default, ThreadX for Cortex-M uses the PRIMASK register to enable/disable interrupts.
If using BASEPRI is desired, define the following two symbols for both c and assembly files:
TX_PORT_USE_BASEPRI - This tells ThreadX to use BASEPRI instead of PRIMASK.
TX_PORT_BASEPRI = (priority_mask << (8 - number_priority_bits)) - this defines the maximum priority level to mask.
Any interrupt with a higher priority than priority_mask will not be masked, thus the interrupt will run.
*/

/* Define various constants for the ThreadX Cortex-M port.  */

#define TX_INT_DISABLE                          1           /* Disable interrupts               */
#define TX_INT_ENABLE                           0           /* Enable interrupts                */


/* Define the clock source for trace event entry time stamp. The following two item are port specific.
   For example, if the time source is at the address 0x0a800024 and is 16-bits in size, the clock
   source constants would be:

#define TX_TRACE_TIME_SOURCE                    *((volatile ULONG *) 0x0a800024)
#define TX_TRACE_TIME_MASK                      0x0000FFFFUL

*/

ULONG   _tx_misra_time_stamp_get(VOID);
#define TX_TRACE_TIME_SOURCE                    _tx_misra_time_stamp_get()

#define TX_TRACE_TIME_MASK                      0xFFFFFFFFUL


/* Define the port specific options for the _tx_build_options variable. This variable indicates
   how the ThreadX library was built.  */

#define TX_PORT_SPECIFIC_BUILD_OPTIONS          (0)


/* Define the in-line initialization constant so that modules with in-line
   initialization capabilities can prevent their initialization from being
   a function call.  */

#define TX_DISABLE_INLINE


/* Determine whether or not stack checking is enabled. By default, ThreadX stack checking is
   disabled. When the following is defined, ThreadX thread stack checking is enabled.  If stack
   checking is enabled (TX_ENABLE_STACK_CHECKING is defined), the TX_DISABLE_STACK_FILLING
   define is negated, thereby forcing the stack fill which is necessary for the stack checking
   logic.  */



/* Define the TX_THREAD control block extensions for this port. The main reason
   for the multiple macros is so that backward compatibility can be maintained with
   existing ThreadX kernel awareness modules.  */

#define TX_THREAD_EXTENSION_0
#define TX_THREAD_EXTENSION_1
#define TX_THREAD_EXTENSION_2


#define TX_THREAD_EXTENSION_3



/* Define the port extensions of the remaining ThreadX objects.  */

#define TX_BLOCK_POOL_EXTENSION
#define TX_BYTE_POOL_EXTENSION
#define TX_EVENT_FLAGS_GROUP_EXTENSION
#define TX_MUTEX_EXTENSION
#define TX_QUEUE_EXTENSION
#define TX_SEMAPHORE_EXTENSION
#define TX_TIMER_EXTENSION


/* Define the user extension field of the thread control block.  Nothing
   additional is needed for this port so it is defined as white space.  */

#define TX_THREAD_USER_EXTENSION


/* Define the macros for processing extensions in tx_thread_create, tx_thread_delete,
   tx_thread_shell_entry, and tx_thread_terminate.  */


#define TX_THREAD_CREATE_EXTENSION(thread_ptr)
#define TX_THREAD_DELETE_EXTENSION(thread_ptr)



ULONG  _tx_misra_control_get(void);
void   _tx_misra_control_set(ULONG value);
ULONG  _tx_misra_fpccr_get(void);
void   _tx_misra_vfp_touch(void);



/* A completed thread falls into _thread_shell_entry and we can simply deactivate the FPU via CONTROL.FPCA
   in order to ensure no lazy stacking will occur. */


#define TX_THREAD_COMPLETED_EXTENSION(thread_ptr)   {                                                       \
                                                    ULONG  _tx_vfp_state;                                   \
                                                        _tx_vfp_state =  _tx_misra_control_get();           \
                                                        _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);    \
                                                        _tx_misra_control_set(_tx_vfp_state);               \
                                                    }


/* A thread can be terminated by another thread, so we first check if it's self-terminating and not in an ISR.
   If so, deactivate the FPU via CONTROL.FPCA. Otherwise we are in an interrupt or another thread is terminating
   this one, so if the FPCCR.LSPACT bit is set, we need to save the CONTROL.FPCA state, touch the FPU to flush 
   the lazy FPU save, then restore the CONTROL.FPCA state. */


#define TX_THREAD_TERMINATED_EXTENSION(thread_ptr)  {                                                                                       \
                                                    ULONG  _tx_system_state;                                                                \
                                                        _tx_system_state =  TX_THREAD_GET_SYSTEM_STATE();                                   \
                                                        if ((_tx_system_state == ((ULONG) 0)) && ((thread_ptr) == _tx_thread_current_ptr))  \
                                                        {                                                                                   \
                                                        ULONG  _tx_vfp_state;                                                               \
                                                            _tx_vfp_state =  _tx_misra_control_get();                                       \
                                                            _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                                \
                                                            _tx_misra_control_set(_tx_vfp_state);                                           \
                                                        }                                                                                   \
                                                        else                                                                                \
                                                        {                                                                                   \
                                                        ULONG  _tx_fpccr;                                                                   \
                                                            _tx_fpccr =  _tx_misra_fpccr_get();                                             \
                                                            _tx_fpccr =  _tx_fpccr & ((ULONG) 0x01);                                        \
                                                            if (_tx_fpccr == ((ULONG) 0x01))                                                \
                                                            {                                                                               \
                                                            ULONG _tx_vfp_state;                                                            \
                                                                _tx_vfp_state = _tx_misra_control_get();                                    \
                                                                _tx_vfp_state =  _tx_vfp_state & ((ULONG) 0x4);                             \
                                                                _tx_misra_vfp_touch();                                                      \
                                                                if (_tx_vfp_state == ((ULONG) 0))                                           \
                                                                {                                                                           \
                                                                    _tx_vfp_state =  _tx_misra_control_get();                               \
                                                                    _tx_vfp_state =  _tx_vfp_state & ~((ULONG) 0x4);                        \
                                                                    _tx_misra_control_set(_tx_vfp_state);                                   \
                                                                }                                                                           \
                                                            }                                                                               \
                                                        }                                                                                   \
                                                    }



/* Define the ThreadX object creation extensions for the remaining objects.  */

#define TX_BLOCK_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_CREATE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_CREATE_EXTENSION(group_ptr)
#define TX_MUTEX_CREATE_EXTENSION(mutex_ptr)
#define TX_QUEUE_CREATE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)
#define TX_TIMER_CREATE_EXTENSION(timer_ptr)


/* Define the ThreadX object deletion extensions for the remaining objects.  */

#define TX_BLOCK_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)
#define TX_EVENT_FLAGS_GROUP_DELETE_EXTENSION(group_ptr)
#define TX_MUTEX_DELETE_EXTENSION(mutex_ptr)
#define TX_QUEUE_DELETE_EXTENSION(queue_ptr)
#define TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)
#define TX_TIMER_DELETE_EXTENSION(timer_ptr)


/* Define the get system state macro.  */

ULONG   _tx_misra_ipsr_get(VOID);
#define TX_THREAD_GET_SYSTEM_STATE()        (_tx_thread_system_state | _tx_misra_ipsr_get())


/* Define the check for whether or not to call the _tx_thread_system_return function.  A non-zero value
   indicates that _tx_thread_system_return should not be called. This overrides the definition in tx_thread.h
   for Cortex-M since so we don't waste time checking the _tx_thread_system_state variable that is always
   zero after initialization for Cortex-M ports. */

#define TX_THREAD_SYSTEM_RETURN_CHECK(c)    (c) = ((ULONG) _tx_thread_preempt_disable);

/* Define the macro to ensure _tx_thread_preempt_disable is set early in initialization in order to
   prevent early scheduling on Cortex-M parts.  */

#define TX_PORT_SPECIFIC_POST_INITIALIZATION    _tx_thread_preempt_disable++;





UINT                                            _tx_thread_interrupt_disable(VOID);
VOID                                            _tx_thread_interrupt_restore(UINT previous_posture);

#define TX_INTERRUPT_SAVE_AREA                  register UINT interrupt_save;

#define TX_DISABLE                              interrupt_save = _tx_thread_interrupt_disable();
#define TX_RESTORE                              _tx_thread_interrupt_restore(interrupt_save);


/* Define FPU extension for the Cortex-M. Each is assumed to be called in the context of the executing
   thread. These are no longer needed, but are preserved for backward compatibility only.  */

void    tx_thread_fpu_enable(void);
void    tx_thread_fpu_disable(void);


/* Define the version ID of ThreadX.  This may be utilized by the application.  */

#ifdef TX_THREAD_INIT
CHAR                            _tx_version_id[] =
                                    "Copyright (c) Microsoft Corporation. All rights reserved.  *  ThreadX Cortex-M4/GNU Version 6.1.12 *";
#else
extern  CHAR                    _tx_version_id[100];
#endif


#endif
