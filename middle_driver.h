#ifndef MIDDLE_DRIVER_H_
#define MIDDLE_DRIVER_H_

#include <stdint.h>

void HAL_LCD_PortInit(void);
void HAL_LCD_SpiInit(void);
void HAL_LCD_writeCommand(uint8_t command);
void HAL_LCD_writeData(uint8_t data);

#endif
