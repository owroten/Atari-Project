#include <msp430fr5994.h>
#include <stdint.h>
#include "LcdDriver/middle_driver.h"

/* BOOSTXL-EDUMKII on MSP-EXP430FR5994:
 * J1.7  LCD CLK  -> P5.2 / UCB1CLK
 * J2.15 LCD MOSI -> P5.0 / UCB1SIMO
 * J2.13 LCD CS   -> P8.3
 * J4.31 LCD RS   -> P4.1
 * J4.17 LCD RST  -> LaunchPad RST line
 * J4.39 LCD BL   -> P3.6
 */

#define LCD_CS BIT3
#define LCD_DC BIT1

void HAL_LCD_PortInit(void)
{
    /* UCB1 peripheral function on P5.2 and P5.0. */
    P5SEL1 &= ~(BIT2 | BIT0);
    P5SEL0 |=  (BIT2 | BIT0);

    P8DIR |= LCD_CS;
    P8OUT |= LCD_CS;

    P4DIR |= LCD_DC;
    P4OUT |= LCD_DC;
}

void HAL_LCD_SpiInit(void)
{
    UCB1CTLW0 = UCSWRST;
    UCB1CTLW0 |= UCCKPH | UCMSB | UCMST | UCSYNC | UCSSEL_2;
    UCB1CTLW0 &= ~UCCKPL;

    /* 8 MHz SMCLK / 2 = 4 MHz SPI. Conservative and reliable for the LCD. */
    UCB1BRW = 2;

    UCB1CTLW0 &= ~UCSWRST;
    P8OUT &= ~LCD_CS;
    P4OUT |= LCD_DC;
}

void HAL_LCD_writeCommand(uint8_t command)
{
    while(UCB1STATW & UCBUSY)
    {
    }

    P4OUT &= ~LCD_DC;

    while((UCB1IFG & UCTXIFG) == 0)
    {
    }
    UCB1TXBUF = command;

    while(UCB1STATW & UCBUSY)
    {
    }

    P4OUT |= LCD_DC;
}

void HAL_LCD_writeData(uint8_t data)
{
    while((UCB1IFG & UCTXIFG) == 0)
    {
    }
    UCB1TXBUF = data;
}
