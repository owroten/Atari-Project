#include <msp430fr5994.h>
#include <stdint.h>

#include "GrLib/grlib/grlib.h"
#include "LcdDriver/lcd_driver.h"
#include "LcdDriver/middle_driver.h"

uint8_t Lcd_Orientation = LCD_ORIENTATION_UP;
uint16_t Lcd_ScreenWidth = LCD_HORIZONTAL_MAX;
uint16_t Lcd_ScreenHeight = LCD_VERTICAL_MAX;

static void Crystalfontz128x128_PixelDraw(void *pvDisplayData,
                                         int16_t lX,
                                         int16_t lY,
                                         uint16_t ulValue);
static void Crystalfontz128x128_PixelDrawMultiple(void *pvDisplayData,
                                                 int16_t lX,
                                                 int16_t lY,
                                                 int16_t lX0,
                                                 int16_t lCount,
                                                 int16_t lBPP,
                                                 const uint8_t *pucData,
                                                 const uint32_t *pucPalette);
static void Crystalfontz128x128_LineDrawH(void *pvDisplayData,
                                         int16_t lX1,
                                         int16_t lX2,
                                         int16_t lY,
                                         uint16_t ulValue);
static void Crystalfontz128x128_LineDrawV(void *pvDisplayData,
                                         int16_t lX,
                                         int16_t lY1,
                                         int16_t lY2,
                                         uint16_t ulValue);
static void Crystalfontz128x128_RectFill(void *pvDisplayData,
                                        const Graphics_Rectangle *pRect,
                                        uint16_t ulValue);
static uint32_t Crystalfontz128x128_ColorTranslate(void *pvDisplayData,
                                                  uint32_t ulValue);
static void Crystalfontz128x128_Flush(void *pvDisplayData);
static void Crystalfontz128x128_ClearScreen(void *pvDisplayData,
                                           uint16_t ulValue);

void Crystalfontz128x128_Init(void)
{
    int i;

    HAL_LCD_PortInit();
    HAL_LCD_SpiInit();

    /* The BoosterPack LCD reset is connected to the LaunchPad reset line.
     * Use the controller's software reset after startup as well.
     */
    __delay_cycles(400000);
    HAL_LCD_writeCommand(CM_SWRESET);
    __delay_cycles(1200000);

    HAL_LCD_writeCommand(CM_SLPOUT);
    __delay_cycles(960000);

    HAL_LCD_writeCommand(CM_GAMSET);
    HAL_LCD_writeData(0x04);

    HAL_LCD_writeCommand(CM_SETPWCTR);
    HAL_LCD_writeData(0x0A);
    HAL_LCD_writeData(0x14);

    HAL_LCD_writeCommand(CM_SETSTBA);
    HAL_LCD_writeData(0x0A);
    HAL_LCD_writeData(0x00);

    HAL_LCD_writeCommand(CM_COLMOD);
    HAL_LCD_writeData(0x05);

    HAL_LCD_writeCommand(CM_MADCTL);
    HAL_LCD_writeData(CM_MADCTL_MX | CM_MADCTL_MY | CM_MADCTL_BGR);

    HAL_LCD_writeCommand(CM_NORON);
    __delay_cycles(80000);

    Crystalfontz128x128_SetDrawFrame(0, 0, 127, 127);
    HAL_LCD_writeCommand(CM_RAMWR);
    for(i = 0; i < 16384; i++)
    {
        HAL_LCD_writeData(0x00);
        HAL_LCD_writeData(0x00);
    }

    HAL_LCD_writeCommand(CM_DISPON);
    __delay_cycles(80000);
}

void Crystalfontz128x128_SetDrawFrame(uint16_t x0,
                                     uint16_t y0,
                                     uint16_t x1,
                                     uint16_t y1)
{
    switch(Lcd_Orientation)
    {
        case LCD_ORIENTATION_UP:
            x0 += 2;
            y0 += 3;
            x1 += 2;
            y1 += 3;
            break;
        case LCD_ORIENTATION_LEFT:
            x0 += 3;
            y0 += 2;
            x1 += 3;
            y1 += 2;
            break;
        case LCD_ORIENTATION_DOWN:
            x0 += 2;
            y0 += 1;
            x1 += 2;
            y1 += 1;
            break;
        case LCD_ORIENTATION_RIGHT:
            x0 += 1;
            y0 += 2;
            x1 += 1;
            y1 += 2;
            break;
        default:
            break;
    }

    HAL_LCD_writeCommand(CM_CASET);
    HAL_LCD_writeData((uint8_t)(x0 >> 8));
    HAL_LCD_writeData((uint8_t)x0);
    HAL_LCD_writeData((uint8_t)(x1 >> 8));
    HAL_LCD_writeData((uint8_t)x1);

    HAL_LCD_writeCommand(CM_RASET);
    HAL_LCD_writeData((uint8_t)(y0 >> 8));
    HAL_LCD_writeData((uint8_t)y0);
    HAL_LCD_writeData((uint8_t)(y1 >> 8));
    HAL_LCD_writeData((uint8_t)y1);
}

void Crystalfontz128x128_SetOrientation(uint8_t orientation)
{
    Lcd_Orientation = orientation;
    HAL_LCD_writeCommand(CM_MADCTL);

    switch(orientation)
    {
        case LCD_ORIENTATION_UP:
            HAL_LCD_writeData(CM_MADCTL_MX | CM_MADCTL_MY | CM_MADCTL_BGR);
            Lcd_ScreenWidth = 128;
            Lcd_ScreenHeight = 128;
            break;
        case LCD_ORIENTATION_LEFT:
            HAL_LCD_writeData(CM_MADCTL_MY | CM_MADCTL_MV | CM_MADCTL_BGR);
            Lcd_ScreenWidth = 128;
            Lcd_ScreenHeight = 128;
            break;
        case LCD_ORIENTATION_DOWN:
            HAL_LCD_writeData(CM_MADCTL_BGR);
            Lcd_ScreenWidth = 128;
            Lcd_ScreenHeight = 128;
            break;
        case LCD_ORIENTATION_RIGHT:
            HAL_LCD_writeData(CM_MADCTL_MX | CM_MADCTL_MV | CM_MADCTL_BGR);
            Lcd_ScreenWidth = 128;
            Lcd_ScreenHeight = 128;
            break;
        default:
            break;
    }
}

static void Crystalfontz128x128_PixelDraw(void *pvDisplayData,
                                         int16_t lX,
                                         int16_t lY,
                                         uint16_t ulValue)
{
    (void)pvDisplayData;
    Crystalfontz128x128_SetDrawFrame((uint16_t)lX, (uint16_t)lY,
                                    (uint16_t)lX, (uint16_t)lY);
    HAL_LCD_writeCommand(CM_RAMWR);
    HAL_LCD_writeData((uint8_t)(ulValue >> 8));
    HAL_LCD_writeData((uint8_t)ulValue);
}

static void Crystalfontz128x128_PixelDrawMultiple(void *pvDisplayData,
                                                 int16_t lX,
                                                 int16_t lY,
                                                 int16_t lX0,
                                                 int16_t lCount,
                                                 int16_t lBPP,
                                                 const uint8_t *pucData,
                                                 const uint32_t *pucPalette)
{
    uint16_t data;
    uint16_t color;

    (void)pvDisplayData;

    if(lCount <= 0)
    {
        return;
    }

    Crystalfontz128x128_SetDrawFrame((uint16_t)lX,
                                    (uint16_t)lY,
                                    (uint16_t)(lX + lCount - 1),
                                    (uint16_t)lY);
    HAL_LCD_writeCommand(CM_RAMWR);

    if(lBPP == 1)
    {
        while(lCount > 0)
        {
            data = *pucData++;
            while(lX0 < 8 && lCount > 0)
            {
                color = (uint16_t)pucPalette[(data >> (7 - lX0)) & 1];
                HAL_LCD_writeData((uint8_t)(color >> 8));
                HAL_LCD_writeData((uint8_t)color);
                lX0++;
                lCount--;
            }
            lX0 = 0;
        }
    }
    else if(lBPP == 4)
    {
        while(lCount > 0)
        {
            if((lX0 & 1) == 0)
            {
                data = (uint16_t)((*pucData >> 4) & 0x0F);
                color = *((const uint16_t *)(pucPalette + data));
                HAL_LCD_writeData((uint8_t)(color >> 8));
                HAL_LCD_writeData((uint8_t)color);
                lCount--;
                lX0++;
            }
            else
            {
                data = (uint16_t)(*pucData & 0x0F);
                color = *((const uint16_t *)(pucPalette + data));
                HAL_LCD_writeData((uint8_t)(color >> 8));
                HAL_LCD_writeData((uint8_t)color);
                pucData++;
                lCount--;
                lX0 = 0;
            }
        }
    }
    else if(lBPP == 8)
    {
        while(lCount > 0)
        {
            data = *pucData++;
            color = *((const uint16_t *)(pucPalette + data));
            HAL_LCD_writeData((uint8_t)(color >> 8));
            HAL_LCD_writeData((uint8_t)color);
            lCount--;
        }
    }
    else if(lBPP == 16)
    {
        while(lCount > 0)
        {
            color = *((const uint16_t *)pucData);
            pucData += 2;
            HAL_LCD_writeData((uint8_t)(color >> 8));
            HAL_LCD_writeData((uint8_t)color);
            lCount--;
        }
    }
}

static void Crystalfontz128x128_LineDrawH(void *pvDisplayData,
                                         int16_t lX1,
                                         int16_t lX2,
                                         int16_t lY,
                                         uint16_t ulValue)
{
    int16_t i;
    (void)pvDisplayData;

    Crystalfontz128x128_SetDrawFrame((uint16_t)lX1, (uint16_t)lY,
                                    (uint16_t)lX2, (uint16_t)lY);
    HAL_LCD_writeCommand(CM_RAMWR);

    for(i = lX1; i <= lX2; i++)
    {
        HAL_LCD_writeData((uint8_t)(ulValue >> 8));
        HAL_LCD_writeData((uint8_t)ulValue);
    }
}

static void Crystalfontz128x128_LineDrawV(void *pvDisplayData,
                                         int16_t lX,
                                         int16_t lY1,
                                         int16_t lY2,
                                         uint16_t ulValue)
{
    int16_t i;
    (void)pvDisplayData;

    Crystalfontz128x128_SetDrawFrame((uint16_t)lX, (uint16_t)lY1,
                                    (uint16_t)lX, (uint16_t)lY2);
    HAL_LCD_writeCommand(CM_RAMWR);

    for(i = lY1; i <= lY2; i++)
    {
        HAL_LCD_writeData((uint8_t)(ulValue >> 8));
        HAL_LCD_writeData((uint8_t)ulValue);
    }
}

static void Crystalfontz128x128_RectFill(void *pvDisplayData,
                                        const Graphics_Rectangle *pRect,
                                        uint16_t ulValue)
{
    int16_t x0;
    int16_t x1;
    int16_t y0;
    int16_t y1;
    long pixels;
    long i;

    (void)pvDisplayData;

    x0 = pRect->xMin;
    x1 = pRect->xMax;
    y0 = pRect->yMin;
    y1 = pRect->yMax;

    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 > 127) x1 = 127;
    if(y1 > 127) y1 = 127;

    if(x1 < x0 || y1 < y0)
    {
        return;
    }

    pixels = (long)(x1 - x0 + 1) * (long)(y1 - y0 + 1);

    Crystalfontz128x128_SetDrawFrame((uint16_t)x0, (uint16_t)y0,
                                    (uint16_t)x1, (uint16_t)y1);
    HAL_LCD_writeCommand(CM_RAMWR);

    for(i = 0; i < pixels; i++)
    {
        HAL_LCD_writeData((uint8_t)(ulValue >> 8));
        HAL_LCD_writeData((uint8_t)ulValue);
    }
}

static uint32_t Crystalfontz128x128_ColorTranslate(void *pvDisplayData,
                                                  uint32_t ulValue)
{
    (void)pvDisplayData;
    return ((((ulValue & 0x00F80000UL) >> 8) |
             ((ulValue & 0x0000FC00UL) >> 5) |
             ((ulValue & 0x000000F8UL) >> 3)));
}

static void Crystalfontz128x128_Flush(void *pvDisplayData)
{
    (void)pvDisplayData;
}

static void Crystalfontz128x128_ClearScreen(void *pvDisplayData,
                                           uint16_t ulValue)
{
    Graphics_Rectangle rect;

    rect.xMin = 0;
    rect.yMin = 0;
    rect.xMax = 127;
    rect.yMax = 127;

    Crystalfontz128x128_RectFill(pvDisplayData, &rect, ulValue);
}

const Graphics_Display g_sCrystalfontz128x128 =
{
    sizeof(tDisplay),
    0,
    LCD_VERTICAL_MAX,
    LCD_HORIZONTAL_MAX,
    Crystalfontz128x128_PixelDraw,
    Crystalfontz128x128_PixelDrawMultiple,
    Crystalfontz128x128_LineDrawH,
    Crystalfontz128x128_LineDrawV,
    Crystalfontz128x128_RectFill,
    Crystalfontz128x128_ColorTranslate,
    Crystalfontz128x128_Flush,
    Crystalfontz128x128_ClearScreen
};
