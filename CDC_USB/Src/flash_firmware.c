#include "flash_firmware.h"

void save_rec_fw(char *data_to_flash, uint32_t len, uint32_t address, uint8_t first_package) {

    volatile HAL_StatusTypeDef flash_status;

    if (first_package) {
        FLASH_EraseInitTypeDef EraseInitStruct;
        EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Banks       = FLASH_BANK_1;
        EraseInitStruct.Page        = 16;
        EraseInitStruct.NbPages     = 240;

        /* Unlock the Flash to enable the flash control register access *************/
        HAL_FLASH_Unlock();

        uint32_t PageError;
        flash_status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
        HAL_UART_Transmit(&huart2, (uint8_t*)"Erased\r\n", 8, 100);
    }

    const char **fragments;
    fragments = malloc(sizeof(*fragments) * 4);

    if (flash_status == HAL_OK) {
        for (int i = 0; i < 8; i++) {
            fragments[i] = strndup(data_to_flash + 8 * i, 8);
            flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, *(uint64_t*)fragments[i]);
            HAL_UART_Transmit(&huart2, (uint8_t*)"Progged\r\n", 9, 100);
            if (flash_status != HAL_OK) {
                break;
            }
            else if (flash_status == HAL_OK) {
                address += 8;
            }
        }
    }
    HAL_FLASH_Lock();
}
