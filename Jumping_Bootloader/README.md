# Simple, jumping bootloader: Things to remember

### Set FLASH space in LinkerScript

Make sure the FLASH start and size is correct for the firmware in the LinkerScript.

16K for bootloader at start address `0x08000000`:

```
/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 96K
RAM2 (xrw)      : ORIGIN = 0x10000000, LENGTH = 32K
FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 16K
}
```

Application/firmware (1024K - 16K = 1008K) at address `0x08004000`:

```
/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 96K
RAM2 (xrw)      : ORIGIN = 0x10000000, LENGTH = 32K
FLASH (rx)      : ORIGIN = 0x8004000, LENGTH = 1008K
}
```

### Set Vector Table Offset Register (VTOR) on startup of bootloader and firmware

This is vital - it will not work without it. Relocate the VTOR to the start address of each bootloader and firmware.

For bootloader:

```
int main(void)
{
    HAL_Init();

    SCB->VTOR = 0x08000000;
    __enable_irq();

    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    while (1) {
    }
}
```

For application/firmware at address `0x08004000`:

```
int main(void)
{
    HAL_Init();

    SCB->VTOR = 0x08004000;
    __enable_irq();

    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    while (1) {
    }
}
```