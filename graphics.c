#include "graphics.h"
#include "math_utils.h"

static void UIntToText(unsigned int value, char *buffer);
static void MakeLevelText(unsigned char levelNumber, char *buffer);
static void MakePointsText(unsigned int points, char *buffer);

void Initialize_Graphics(Graphics_Context *context)
{
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    Graphics_initContext(context, &g_sCrystalfontz128x128);

    Graphics_setBackgroundColor(context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(context, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(context, &g_sFontFixed6x8);
    Graphics_clearDisplay(context);

    /* BOOSTXL-EDUMKII LCD backlight: J4.39 -> P3.6.
     * J5 on the BoosterPack must be in the LCD BACKLT position.
     */
    P3DIR |= BIT6;
    P3OUT |= BIT6;
}

void Draw_Playspace(Graphics_Context *context,
                    Graphics_Rectangle *blocks,
                    const int32_t *colors,
                    const unsigned char *bindings,
                    unsigned char numBlocks,
                    unsigned char lives,
                    unsigned char levelNumber)
{
    unsigned char i;
    char levelText[12];

    Graphics_setBackgroundColor(context, GRAPHICS_COLOR_BLACK);
    Graphics_setForegroundColor(context, GRAPHICS_COLOR_WHITE);
    Graphics_clearDisplay(context);

    Graphics_drawLineH(context, 0, 127, 118);

    MakeLevelText(levelNumber, levelText);
    Graphics_drawStringCentered(context,
                                (int8_t *)levelText,
                                AUTO_STRING_LENGTH,
                                64,
                                123,
                                OPAQUE_TEXT);

    for(i = 0; i < lives; i++)
    {
        Graphics_fillCircle(context, (i * 6) + 4, 123, BALL_RADIUS);
    }

    for(i = 0; i < numBlocks; i++)
    {
        if(!IsNullBlock(&blocks[i]))
        {
            Graphics_setForegroundColor(context, colors[bindings[i]]);
            Graphics_fillRectangle(context, &blocks[i]);
        }
    }
}

void Draw_Paddle(Graphics_Context *context, unsigned int paddleX, int32_t color)
{
    Graphics_Rectangle paddle;

    paddle.xMin = (int16_t)paddleX;
    paddle.yMin = PADDLE_Y;
    paddle.xMax = (int16_t)(paddleX + PADDLE_WIDTH);
    paddle.yMax = PADDLE_Y + PADDLE_HEIGHT;

    Graphics_setForegroundColor(context, color);
    Graphics_fillRectangle(context, &paddle);
}

void Draw_Ball(Graphics_Context *context, int ballX, int ballY, int32_t color)
{
    Graphics_setForegroundColor(context, color);
    Graphics_fillCircle(context, ballX, ballY, BALL_RADIUS);
}

void Draw_ModalBox(Graphics_Context *context,
                   const char *line1,
                   const char *line2,
                   unsigned char showPoints,
                   unsigned int points)
{
    Graphics_Rectangle outer;
    Graphics_Rectangle inner;
    char pointText[14];

    outer.xMin = 10;
    outer.yMin = 20;
    outer.xMax = 117;
    outer.yMax = 92;

    inner.xMin = 12;
    inner.yMin = 22;
    inner.xMax = 115;
    inner.yMax = 90;

    Graphics_setForegroundColor(context, GRAPHICS_COLOR_BLUE);
    Graphics_fillRectangle(context, &outer);
    Graphics_setForegroundColor(context, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(context, &inner);

    Graphics_setForegroundColor(context, GRAPHICS_COLOR_WHITE);
    Graphics_drawStringCentered(context,
                                (int8_t *)line1,
                                AUTO_STRING_LENGTH,
                                64,
                                38,
                                OPAQUE_TEXT);
    Graphics_drawStringCentered(context,
                                (int8_t *)line2,
                                AUTO_STRING_LENGTH,
                                64,
                                56,
                                OPAQUE_TEXT);

    if(showPoints)
    {
        MakePointsText(points, pointText);
        Graphics_drawStringCentered(context,
                                    (int8_t *)pointText,
                                    AUTO_STRING_LENGTH,
                                    64,
                                    76,
                                    OPAQUE_TEXT);
    }
}

static void UIntToText(unsigned int value, char *buffer)
{
    char reverse[6];
    unsigned char count;
    unsigned char i;

    count = 0;

    if(value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while(value > 0 && count < 5)
    {
        reverse[count] = (char)('0' + (value % 10));
        value = value / 10;
        count++;
    }

    for(i = 0; i < count; i++)
    {
        buffer[i] = reverse[count - i - 1];
    }
    buffer[count] = '\0';
}

static void MakeLevelText(unsigned char levelNumber, char *buffer)
{
    char number[6];
    unsigned char i;
    unsigned char j;

    buffer[0] = 'L';
    buffer[1] = 'e';
    buffer[2] = 'v';
    buffer[3] = 'e';
    buffer[4] = 'l';
    buffer[5] = ' ';

    UIntToText((unsigned int)levelNumber + 1U, number);

    i = 6;
    j = 0;
    while(number[j] != '\0')
    {
        buffer[i++] = number[j++];
    }
    buffer[i] = '\0';
}

static void MakePointsText(unsigned int points, char *buffer)
{
    char number[6];
    unsigned char i;
    unsigned char j;

    UIntToText(points, number);

    i = 0;
    j = 0;
    while(number[j] != '\0')
    {
        buffer[i++] = number[j++];
    }

    buffer[i++] = ' ';
    buffer[i++] = 'p';
    buffer[i++] = 't';
    buffer[i++] = 's';
    buffer[i] = '\0';
}
