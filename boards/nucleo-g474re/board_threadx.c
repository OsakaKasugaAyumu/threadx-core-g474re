/* Project-owned board glue. The application is still main/demo_threadx.c.
 * Use ST's G474RE reset handler, SystemInit and flash layout, not generic crt0.
 */
#include "autoconf.h"
#include "tx_api.h"
#include "tx_thread.h"
#include "tx_initialize.h"
#include "stm32g474xx.h"

#define BOARD_HCLK_HZ 16000000UL  /* HSI16 after reset; no PLL, HCLK divider = 1 */
#define BOARD_TICK_CYCLES (BOARD_HCLK_HZ / TX_TIMER_TICKS_PER_SECOND)
_Static_assert((BOARD_HCLK_HZ % TX_TIMER_TICKS_PER_SECOND) == 0, "non-integral tick");
_Static_assert((BOARD_TICK_CYCLES >= 1) && (BOARD_TICK_CYCLES <= SysTick_LOAD_RELOAD_Msk + 1UL), "SysTick reload overflow");
_Static_assert(__NVIC_PRIO_BITS == 4U, "priorities below are for the STM32G474");

extern unsigned char _estack;    /* Linker symbol; use its address, never dereference it. */
extern void _tx_timer_interrupt(void); /* GNU Cortex-M4 port in tx_timer_interrupt.S */

void _tx_initialize_low_level(void)
{
    __disable_irq();    /* The port's scheduler will unmask after initialization. */
    __DSB(); __ISB();   /* SystemInit has enabled the FPU and set VTOR to flash. */
    if ((RCC->CFGR & (RCC_CFGR_SWS_Msk | RCC_CFGR_HPRE_Msk)) != RCC_CFGR_SWS_HSI) {
        for (;;) { __NOP(); }  /* Unexpected clock tree: do not run with an incorrect tick. */
    }
    _tx_thread_system_stack_ptr = (VOID *)&_estack;
    _tx_initialize_unused_memory = TX_NULL; /* Application owns memory_area; no implicit heap. */

    SysTick->CTRL = 0U;
    NVIC_SetPriority(SVCall_IRQn, 15U);
    NVIC_SetPriority(PendSV_IRQn, 15U);    /* Context switches must have lowest priority. */
    NVIC_SetPriority(SysTick_IRQn, 4U);    /* Same ordering as the upstream port example. */
    SysTick->LOAD = BOARD_TICK_CYCLES - 1UL;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

/* Strong override of the vector table's weak default. PendSV_Handler itself
 * comes straight from tx_thread_schedule.S, not from a C interrupt wrapper. */
void SysTick_Handler(void)
{
    _tx_timer_interrupt();
}
