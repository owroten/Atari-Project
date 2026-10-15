#ifndef MATH_UTILS_H_
#define MATH_UTILS_H_

#include <msp430fr5994.h>
#include "GrLib/grlib/grlib.h"

#define COLLIDING_NORTH BIT0
#define COLLIDING_EAST  BIT1
#define COLLIDING_SOUTH BIT2
#define COLLIDING_WEST  BIT3

unsigned char IsOverlapping(const Graphics_Rectangle *r1,
                            const Graphics_Rectangle *r2);
unsigned char IsCollidingAABB(const Graphics_Rectangle *moving,
                              const Graphics_Rectangle *target);
unsigned char IsCollidingWalls(const Graphics_Rectangle *ball,
                               int bottomY);
unsigned char IsNullBlock(const Graphics_Rectangle *block);
void SetNullBlock(Graphics_Rectangle *block);
void MakeBallRect(Graphics_Rectangle *rect, int x, int y, int radius);

#endif
