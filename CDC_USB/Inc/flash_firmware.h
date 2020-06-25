#include <stdio.h>
#include "main.h"
#include <stdlib.h>
#include <string.h>

HAL_StatusTypeDef Save_Rec_FW(char *data_to_flash, uint32_t len);
void SaveActiveAppAddress(uint64_t activeAppAddress);
uint64_t ReadActiveAppAddress();