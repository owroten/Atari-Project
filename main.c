#include <msp430fr5994.h>
#include <stdint.h>

#include "GrLib/grlib/grlib.h"
#include "LcdDriver/lcd_driver.h"
#include "graphics.h"
#include "levels.h"
#include "math_utils.h"

#define MAX_BLOCKS       30
#define PLAYFIELD_BOTTOM 117

#define GAME_READY       0
#define GAME_PLAYING     1
#define GAME_WIN         2
#define GAME_OVER        3

#define BOOSTER_BUTTON   BIT2       /* J4.32 -> P4.2 */
#define BOOSTER2_BUTTON   BIT3       /* J4.33 -> P4.3 */
#define JOYSTICK_X_PIN   BIT2       /* J1.2  -> P1.2/A2 */

#define JOY_CENTER 1892
#define JOY_DEADZONE 200
#define PADDLE_SPEED 3

static Graphics_Context g_context;
static Graphics_Rectangle g_blocks[MAX_BLOCKS];
static const Level *g_level;

static volatile unsigned char g_tick = 0;

static unsigned char g_state = GAME_READY;
static unsigned char g_levelNumber = 0;
static unsigned char g_numBlocks = 0;
static unsigned char g_activeBlocks = 0;
static unsigned char g_lives = 3;
static unsigned int g_score = 0;

static unsigned int g_paddleX = 54;
static int g_ballX = 64;
static int g_ballY = 104;
static int g_ballVX = 1;
static int g_ballVY = -2;
static unsigned char g_lastButton = 0;
static int speedDivider = 1;
static int button2IsPressed = 0;

static void Clock_Init(void);
static void Button_Init(void);
static void ADC_Init(void);
static unsigned int ADC_ReadJoystickX(void);
static void Timer_Init(void);
static unsigned char Button_IsPressed(void);

static void Game_LoadLevel(unsigned char levelNumber);
static void Game_ResetBall(void);
static void Game_Update(void);
static void Game_UpdatePaddle(void);
static void Game_UpdateBall(void);
static void Game_LoseLife(void);
static void Game_StartNewGame(void);
static void Game_AdvanceLevel(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    Clock_Init();
    Button_Init();
    ADC_Init();

    /* Enable configured GPIO on FRAM MSP430 devices. */
    PM5CTL0 &= ~LOCKLPM5;

    Crystalfontz128x128_Init();
    Initialize_Graphics(&g_context);

    Game_StartNewGame();
    Timer_Init();

    __enable_interrupt();

    while(1)
    {
        __bis_SR_register(LPM0_bits | GIE);

        if(g_tick)
        {
            g_tick = 0;
            Game_Update();
        }
    }
}

static void Clock_Init(void)
{
    /* DCO = 8 MHz, MCLK = SMCLK = DCO. */
    CSCTL0_H = CSKEY_H;
    CSCTL1 = DCOFSEL_6;
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
    CSCTL0_H = 0;
}

static void Button_Init(void)
{
    /* BOOSTXL-EDUMKII S2: J4.32 -> P4.2, active low. */
    P4DIR &= ~BOOSTER_BUTTON;
    P4REN |= BOOSTER_BUTTON;
    P4OUT |= BOOSTER_BUTTON;
    P4DIR &= ~BOOSTER2_BUTTON;
    P4REN |= BOOSTER2_BUTTON;
    P4OUT |= BOOSTER2_BUTTON;

}

static void ADC_Init(void)
{
    /* BOOSTXL-EDUMKII joystick X: J1.2 -> P1.2/A2. */
    P1SEL0 |= JOYSTICK_X_PIN;
    P1SEL1 |= JOYSTICK_X_PIN;

    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12CTL2 = ADC12RES_2;
    ADC12MCTL0 = ADC12INCH_2;
    ADC12CTL0 |= ADC12ENC;
}

static unsigned int ADC_ReadJoystickX(void)
{
    ADC12CTL0 |= ADC12SC;
    while(ADC12CTL1 & ADC12BUSY)
    {
    }
    return ADC12MEM0;
}

static void Timer_Init(void)
{
    /* SMCLK = 8 MHz, /8 = 1 MHz. 33333 counts ~= 30 Hz. */
    TA0CCR0 = 33333/speedDivider - 1;
    TA0CCTL0 = CCIE;
    TA0CTL = TASSEL_2 | ID_3 | MC_1 | TACLR;
}

static unsigned char Button_IsPressed(void)
{
    if((P4IN & BOOSTER_BUTTON) == 0)
    {
        return 1;
    }
    return 0;
}
static unsigned char Button2_IsPressed(void)
{
    if(((P4IN & BOOSTER2_BUTTON) == 0) && (button2IsPressed == 0))
    {
        speedDivider = speedDivider * 2;
        if(speedDivider == 8){
            speedDivider = 1;
        }
        button2IsPressed = 1;
        Timer_Init();
        return 1;
    }
    if((P4IN & BOOSTER2_BUTTON) != 0){
        button2IsPressed = 0;
    }
    return 0;
}

static void Game_StartNewGame(void)
{
    g_score = 0;
    g_lives = 3;
    g_levelNumber = 0;
    Game_LoadLevel(g_levelNumber);
}

static void Game_LoadLevel(unsigned char levelNumber)
{
    unsigned char i;

    g_level = &LEVELS[levelNumber % NUM_LEVELS];
    g_numBlocks = g_level->numBlocks;
    g_activeBlocks = g_numBlocks;

    for(i = 0; i < MAX_BLOCKS; i++)
    {
        if(i < g_numBlocks)
        {
            g_blocks[i] = g_level->blocks[i];
        }
        else
        {
            SetNullBlock(&g_blocks[i]);
        }
    }

    g_paddleX = (127 - PADDLE_WIDTH) / 2;
    g_state = GAME_READY;
    Game_ResetBall();

    Draw_Playspace(&g_context,
                   g_blocks,
                   g_level->colors,
                   g_level->bindings,
                   g_numBlocks,
                   g_lives,
                   levelNumber);

    Draw_Paddle(&g_context, g_paddleX, GRAPHICS_COLOR_RED);
    Draw_Ball(&g_context, g_ballX, g_ballY, GRAPHICS_COLOR_WHITE);
}

static void Game_ResetBall(void)
{
    g_ballX = (int)g_paddleX + (PADDLE_WIDTH / 2);
    g_ballY = PADDLE_Y - BALL_RADIUS - 1;
    g_ballVX = 1;
    g_ballVY = -2;
}

static void Game_Update(void)
{
    unsigned char button;
    unsigned char button2;
    unsigned char newPress;

    button = Button_IsPressed();
    button2 = Button2_IsPressed();
    newPress = (unsigned char)(button && !g_lastButton);
    g_lastButton = button;

    if(g_state == GAME_WIN)
    {
        if(newPress)
        {
            Game_AdvanceLevel();
        }
        return;
    }

    if(g_state == GAME_OVER)
    {
        if(newPress)
        {
            Game_StartNewGame();
        }
        return;
    }

    Game_UpdatePaddle();

    if(g_state == GAME_READY)
    {
        Draw_Ball(&g_context, g_ballX, g_ballY, GRAPHICS_COLOR_BLACK);
        g_ballX = (int)g_paddleX + (PADDLE_WIDTH / 2);
        g_ballY = PADDLE_Y - BALL_RADIUS - 1;
        Draw_Ball(&g_context, g_ballX, g_ballY, GRAPHICS_COLOR_WHITE);

        if(newPress)
        {
            g_state = GAME_PLAYING;
        }
        return;
    }

    Game_UpdateBall();
}

static void Game_UpdatePaddle(void)
{
    unsigned int adc;
    unsigned int newX;

    adc = ADC_ReadJoystickX();
    newX = g_paddleX;  // Keep the current position by default

    if (adc < JOY_CENTER - JOY_DEADZONE) //Check if the joystick is to the left
    {
        // Joystick is left
        if (g_paddleX > PADDLE_SPEED)
            newX = g_paddleX - PADDLE_SPEED; //Move the paddle to the left
        else
            newX = 0;
    }
    else if (adc > JOY_CENTER + JOY_DEADZONE) //Check if the joystick is to the right
    {
        // Joystick is right
        if (g_paddleX < (128 - PADDLE_WIDTH) - PADDLE_SPEED) //Move the paddle to the left
            newX = g_paddleX + PADDLE_SPEED;
        else
            newX = 128 - PADDLE_WIDTH;
    }

    // Redraw only if the paddle actually moved
    if (newX != g_paddleX)
    {
        Draw_Paddle(&g_context, g_paddleX, GRAPHICS_COLOR_BLACK);
        g_paddleX = newX;
        Draw_Paddle(&g_context, g_paddleX, GRAPHICS_COLOR_RED);
    }
}
static void Game_UpdateBall(void)
{
    int nextX;
    int nextY;
    unsigned char wallCollision;
    unsigned char blockCollision;
    unsigned char i;
    Graphics_Rectangle ballRect;
    Graphics_Rectangle paddleRect;

    nextX = g_ballX + g_ballVX;
    nextY = g_ballY + g_ballVY;

    MakeBallRect(&ballRect, nextX, nextY, BALL_RADIUS);
    wallCollision = IsCollidingWalls(&ballRect, PLAYFIELD_BOTTOM);

    if(wallCollision & COLLIDING_SOUTH)
    {
        Game_LoseLife();
        return;
    }

    if(wallCollision & (COLLIDING_EAST | COLLIDING_WEST))
    {
        g_ballVX = -g_ballVX;
        nextX = g_ballX + g_ballVX;
    }

    if(wallCollision & COLLIDING_NORTH)
    {
        g_ballVY = -g_ballVY;
        nextY = g_ballY + g_ballVY;
    }

    MakeBallRect(&ballRect, nextX, nextY, BALL_RADIUS);

    /* Paddle collision only matters while the ball is moving downward. */
    if(g_ballVY > 0)
    {
        paddleRect.xMin = (int16_t)g_paddleX;
        paddleRect.yMin = PADDLE_Y;
        paddleRect.xMax = (int16_t)(g_paddleX + PADDLE_WIDTH);
        paddleRect.yMax = PADDLE_Y + PADDLE_HEIGHT;

        if(IsOverlapping(&ballRect, &paddleRect))
        {
            g_ballVY = -g_ballVY;
            nextY = PADDLE_Y - BALL_RADIUS - 1;

            /* Give the player some directional control based on hit location. */
            if(g_ballX < (int)g_paddleX + (PADDLE_WIDTH / 3))
            {
                g_ballVX = -1;
            }
            else if(g_ballX > (int)g_paddleX + ((PADDLE_WIDTH * 2) / 3))
            {
                g_ballVX = 1;
            }
        }
    }

    MakeBallRect(&ballRect, nextX, nextY, BALL_RADIUS);

    for(i = 0; i < g_numBlocks; i++)
    {
        if(IsNullBlock(&g_blocks[i]))
        {
            continue;
        }

        blockCollision = IsCollidingAABB(&ballRect, &g_blocks[i]);
        if(blockCollision != 0)
        {
            Graphics_setForegroundColor(&g_context, GRAPHICS_COLOR_BLACK);
            Graphics_fillRectangle(&g_context, &g_blocks[i]);
            SetNullBlock(&g_blocks[i]);

            if(g_activeBlocks > 0)
            {
                g_activeBlocks--;
            }
            g_score++;

            if(blockCollision & (COLLIDING_EAST | COLLIDING_WEST))
            {
                g_ballVX = -g_ballVX;
            }
            else
            {
                g_ballVY = -g_ballVY;
            }

            nextX = g_ballX + g_ballVX;
            nextY = g_ballY + g_ballVY;
            break;
        }
    }

    Draw_Ball(&g_context, g_ballX, g_ballY, GRAPHICS_COLOR_BLACK);
    g_ballX = nextX;
    g_ballY = nextY;
    Draw_Ball(&g_context, g_ballX, g_ballY, GRAPHICS_COLOR_WHITE);

    if(g_activeBlocks == 0)
    {
        g_state = GAME_WIN;
        Draw_ModalBox(&g_context, "You Win!", "Press S2", 1, g_score);
    }
}

static void Game_LoseLife(void)
{
    if(g_lives > 0)
    {
        g_lives--;
    }

    if(g_lives == 0)
    {
        g_state = GAME_OVER;
        Draw_ModalBox(&g_context, "Game Over!", "Press S2", 1, g_score);
        return;
    }

    g_state = GAME_READY;
    Game_ResetBall();

    Draw_Playspace(&g_context,
                   g_blocks,
                   g_level->colors,
                   g_level->bindings,
                   g_numBlocks,
                   g_lives,
                   g_levelNumber);
    Draw_Paddle(&g_context, g_paddleX, GRAPHICS_COLOR_RED);
    Draw_Ball(&g_context, g_ballX, g_ballY, GRAPHICS_COLOR_WHITE);
}

static void Game_AdvanceLevel(void)
{
    g_levelNumber++;
    if(g_levelNumber >= NUM_LEVELS)
    {
        g_levelNumber = 0;
    }

    /* Give a fresh set of lives for each new level. */
    g_lives = 3;
    Game_LoadLevel(g_levelNumber);
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    g_tick = 1;
    __bic_SR_register_on_exit(LPM0_bits);
}
