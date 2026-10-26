#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <msp430fr5994.h>
#include <stdint.h>
#include "GrLib/grlib/grlib.h"
#include "LcdDriver/lcd_driver.h"

#define PADDLE_WIDTH   20
#define PADDLE_HEIGHT   4
#define PADDLE_Y      110
#define BALL_RADIUS     2

void Initialize_Graphics(Graphics_Context *context);
void Draw_Playspace(Graphics_Context *context,
                    Graphics_Rectangle *blocks,
                    const int32_t *colors,
                    const unsigned char *bindings,
                    unsigned char numBlocks,
                    unsigned char lives,
                    unsigned char levelNumber);
void Draw_Paddle(Graphics_Context *context, unsigned int paddleX, int32_t color);
void Draw_Ball(Graphics_Context *context, int ballX, int ballY, int32_t color);
void Draw_ModalBox(Graphics_Context *context,
                   const char *line1,
                   const char *line2,
                   unsigned char showPoints,
                   unsigned int points);

#endif
