#include "flash_firmware.h"
#include "usbd_cdc_if.h"

HAL_StatusTypeDef Save_Rec_FW(char *data_to_flash, uint32_t len) {

    volatile HAL_StatusTypeDef flashStatus = HAL_OK;

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = FLASH_BANK_2;
    EraseInitStruct.Page        = 0;
    EraseInitStruct.NbPages     = 240;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    if (firstPackage) {
        uint32_t PageError;
        flashStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
        HAL_UART_Transmit(&huart2, (uint8_t*)"Erased\r\n", 8, 100);
        firstPackage = 0;
    }

    char chunk[8];
    char sendString[64];

    HAL_UART_Transmit(&huart2, (uint8_t*)"Flashing\r\n", 10, 100);

    if (flashStatus == HAL_OK) {
        for (int i = 0; i < (int)(len/8); i++) {
            // sprintf(sendString, "Add: %ld\r\n", flashAddress);
            HAL_UART_Transmit(&huart2, (uint8_t*)sendString, 15, 100);
            memcpy(chunk, data_to_flash+(i*8), 8);
            flashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flashAddress, *(uint64_t*)chunk);
            HAL_UART_Transmit(&huart2, (uint8_t*)chunk, 8, 100);
            HAL_UART_Transmit(&huart2, (uint8_t*)" ", 1, 100);
            if (flashStatus != HAL_OK) {
                break;
            }
            else if (flashStatus == HAL_OK) {
                flashAddress += 8;
            }
        }
    }
    HAL_FLASH_Lock();

    HAL_UART_Transmit(&huart2, (uint8_t*)"\n", 1, 100);

    return flashStatus;
}
