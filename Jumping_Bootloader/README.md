# Bootloader

## Important points

### Set FLASH space in LinkerScript

Make sure the FLASH start and size is correct for the firmware in the LinkerScript.

### Set Vector Table Offset Register (VTOR) on startup of bootloader and firmware

This is vital - it will not work without it. Relocate the VTOR to the start address of each bootloader and firmware.

For bootloader:

```
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    SCB->VTOR = 0x08000000;
    __enable_irq();

    while (1) {
    }
}
```

For application/firmware at address `0x08004000`:

```
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    SCB->VTOR = 0x08004000;
    __enable_irq();

    while (1) {
    }
}
```