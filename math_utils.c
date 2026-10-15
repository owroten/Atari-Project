#include "math_utils.h"

unsigned char IsOverlapping(const Graphics_Rectangle *r1,
                            const Graphics_Rectangle *r2)
{
    if(r1->xMax < r2->xMin) return 0;
    if(r1->xMin > r2->xMax) return 0;
    if(r1->yMax < r2->yMin) return 0;
    if(r1->yMin > r2->yMax) return 0;
    return 1;
}

unsigned char IsCollidingAABB(const Graphics_Rectangle *moving,
                              const Graphics_Rectangle *target)
{
    int overlapLeft;
    int overlapRight;
    int overlapTop;
    int overlapBottom;
    int minX;
    int minY;
    int movingCenterX;
    int movingCenterY;
    int targetCenterX;
    int targetCenterY;

    if(!IsOverlapping(moving, target))
    {
        return 0;
    }

    overlapLeft = moving->xMax - target->xMin;
    overlapRight = target->xMax - moving->xMin;
    overlapTop = moving->yMax - target->yMin;
    overlapBottom = target->yMax - moving->yMin;

    minX = overlapLeft < overlapRight ? overlapLeft : overlapRight;
    minY = overlapTop < overlapBottom ? overlapTop : overlapBottom;

    movingCenterX = (moving->xMin + moving->xMax) / 2;
    movingCenterY = (moving->yMin + moving->yMax) / 2;
    targetCenterX = (target->xMin + target->xMax) / 2;
    targetCenterY = (target->yMin + target->yMax) / 2;

    if(minX < minY)
    {
        if(movingCenterX < targetCenterX)
        {
            return COLLIDING_EAST;
        }
        return COLLIDING_WEST;
    }

    if(movingCenterY < targetCenterY)
    {
        return COLLIDING_SOUTH;
    }
    return COLLIDING_NORTH;
}

unsigned char IsCollidingWalls(const Graphics_Rectangle *ball,
                               int bottomY)
{
    unsigned char result;

    result = 0;

    if(ball->yMin < 0) result |= COLLIDING_NORTH;
    if(ball->xMax > 127) result |= COLLIDING_EAST;
    if(ball->yMax > bottomY) result |= COLLIDING_SOUTH;
    if(ball->xMin < 0) result |= COLLIDING_WEST;

    return result;
}

unsigned char IsNullBlock(const Graphics_Rectangle *block)
{
    if(block->xMin == 128 && block->yMin == 128)
    {
        return 1;
    }
    return 0;
}

void SetNullBlock(Graphics_Rectangle *block)
{
    block->xMin = 128;
    block->yMin = 128;
    block->xMax = 128;
    block->yMax = 128;
}

void MakeBallRect(Graphics_Rectangle *rect, int x, int y, int radius)
{
    rect->xMin = (int16_t)(x - radius);
    rect->yMin = (int16_t)(y - radius);
    rect->xMax = (int16_t)(x + radius);
    rect->yMax = (int16_t)(y + radius);
}
