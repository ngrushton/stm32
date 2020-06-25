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

void SaveActiveAppAddress(uint64_t activeAppAddress) {

    volatile HAL_StatusTypeDef flashStatus = HAL_OK;

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = FLASH_BANK_2;
    EraseInitStruct.Page        = 254;
    EraseInitStruct.NbPages     = 1;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();
    uint32_t PageError;
    flashStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    HAL_UART_Transmit(&huart2, (uint8_t*)"App Address Erased\r\n", 20, 100);

    HAL_UART_Transmit(&huart2, (uint8_t*)"Flashing\r\n", 10, 100);

    // uint64_t flashData = (((uint64_t) 0x00000000) << 32) + ((uint64_t) activeAppAddress);

    if (flashStatus == HAL_OK) {
        flashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, 0x080FF000, activeAppAddress);
    }
    HAL_FLASH_Lock();

    // return flashStatus;
}

uint64_t ReadActiveAppAddress()
{
	volatile uint32_t read_data;
	uint32_t buffered[2];
	volatile uint32_t read_cnt = 0;
	int index = 0;
	do
	{
		read_data = *(uint32_t*)(0x080FF000 + read_cnt);

		if(read_data != 0xFFFFFFFF)
		{
			buffered[index] = read_data;
            
            char buffer[16];
            sprintf(buffer, "%ld\n", buffered[index]);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, 10, 100);

			read_cnt += 4;
		}
		index++;
	} while(read_data != 0xFFFFFFFF);

	uint64_t jammed = ((uint64_t)buffered[1]<<32) + (uint64_t)buffered[0];
	return jammed;
}