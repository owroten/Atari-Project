#ifndef LEVELS_H_
#define LEVELS_H_

#include <stdint.h>
#include "GrLib/grlib/grlib.h"

#define NUM_LEVELS 5

typedef struct
{
    const Graphics_Rectangle *blocks;
    const int32_t *colors;
    const unsigned char *bindings;
    unsigned char numBlocks;
} Level;

extern const Level LEVELS[NUM_LEVELS];

#endif
