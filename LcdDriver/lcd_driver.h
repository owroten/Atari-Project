#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_

#include <stdint.h>
#include "GrLib/grlib/grlib.h"

#define LCD_VERTICAL_MAX       128
#define LCD_HORIZONTAL_MAX     128

#define LCD_ORIENTATION_UP      0
#define LCD_ORIENTATION_LEFT    1
#define LCD_ORIENTATION_DOWN    2
#define LCD_ORIENTATION_RIGHT   3

#define CM_NOP             0x00
#define CM_SWRESET         0x01
#define CM_RDDID           0x04
#define CM_RDDST           0x09
#define CM_SLPIN           0x10
#define CM_SLPOUT          0x11
#define CM_PTLON           0x12
#define CM_NORON           0x13
#define CM_INVOFF          0x20
#define CM_INVON           0x21
#define CM_GAMSET          0x26
#define CM_DISPOFF         0x28
#define CM_DISPON          0x29
#define CM_CASET           0x2A
#define CM_RASET           0x2B
#define CM_RAMWR           0x2C
#define CM_RGBSET          0x2D
#define CM_RAMRD           0x2E
#define CM_PTLAR           0x30
#define CM_MADCTL          0x36
#define CM_COLMOD          0x3A
#define CM_SETPWCTR        0xB1
#define CM_SETDISPL        0xB2
#define CM_FRMCTR3         0xB3
#define CM_SETCYC          0xB4
#define CM_SETBGP          0xB5
#define CM_SETVCOM         0xB6
#define CM_SETSTBA         0xC0
#define CM_SETID           0xC3
#define CM_GETHID          0xD0
#define CM_SETGAMMA        0xE0

#define CM_MADCTL_MY       0x80
#define CM_MADCTL_MX       0x40
#define CM_MADCTL_MV       0x20
#define CM_MADCTL_ML       0x10
#define CM_MADCTL_BGR      0x08
#define CM_MADCTL_MH       0x04

extern uint8_t Lcd_Orientation;
extern uint16_t Lcd_ScreenWidth;
extern uint16_t Lcd_ScreenHeight;
extern const Graphics_Display g_sCrystalfontz128x128;

void Crystalfontz128x128_Init(void);
void Crystalfontz128x128_SetDrawFrame(uint16_t x0,
                                     uint16_t y0,
                                     uint16_t x1,
                                     uint16_t y1);
void Crystalfontz128x128_SetOrientation(uint8_t orientation);

#endif
