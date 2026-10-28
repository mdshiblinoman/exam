/**
 * UART TWO-WAY SWITCH — Pure Bare Metal — NUCLEO-F446RE  (SLAVE board)
 * The SAME code runs on both boards (UART_Switch_Master == UART_Switch_Slave).
 * Each board has the USER button (PC13) and the LED (PA5).
 *
 * Press the button on EITHER board:
 *   → it sends 'S' to the other board
 *   → the OTHER board's LED blinks 5×, then it replies 'D' (ack)
 *   → the board you pressed gives a little 2-blink: "ack received".
 * Idle (no press): both boards dark.
 *
 * WIRING (2 wires + GND): PA9(TX) → other PA10(RX), PA10(RX) ← other PA9(TX), GND↔GND
 * LED: PA5 (LD2)   BUTTON: PC13 (USER, active-low)   LINK: USART1 @ 9600
 */

#define BLINK_MS 200     /* 5-blink speed */
#define ACK_TIMEOUT 6000 /* ms to wait for 'D' (> other board's 5-blink time) */

#define RCC_AHB1ENR (*(volatile unsigned int *)0x40023830)
#define RCC_APB2ENR (*(volatile unsigned int *)0x40023844)
#define GPIOA_MODER (*(volatile unsigned int *)0x40020000)
#define GPIOA_PUPDR (*(volatile unsigned int *)0x4002000C)
#define GPIOA_ODR (*(volatile unsigned int *)0x40020014)
#define GPIOA_AFRH (*(volatile unsigned int *)0x40020024)
#define GPIOC_PUPDR (*(volatile unsigned int *)0x4002080C)
#define GPIOC_IDR (*(volatile unsigned int *)0x40020810)
#define USART1_SR (*(volatile unsigned int *)0x40011000)
#define USART1_DR (*(volatile unsigned int *)0x40011004)
#define USART1_BRR (*(volatile unsigned int *)0x40011008)
#define USART1_CR1 (*(volatile unsigned int *)0x4001100C)

void SystemInit(void) __attribute__((weak));
void SystemInit(void) {}
void __libc_init_array(void) __attribute__((weak));
void __libc_init_array(void) {}

static void delay_ms(unsigned int ms)
{
    for (unsigned int m = 0; m < ms; m++)
        for (volatile unsigned int i = 0; i < 1300; i++)
            ;
}
static int pressed(void) { return (GPIOC_IDR & (1 << 13)) == 0; }

/* The OTHER board pressed its button → my LED blinks 5× */
static void blink(int times)
{
    for (int i = 0; i < times; i++)
    {
        GPIOA_ODR |= (1 << 5);
        delay_ms(BLINK_MS);
        GPIOA_ODR &= ~(1 << 5);
        delay_ms(BLINK_MS);
    }
}
/* My press was acked → little 2-blink */
static void little_blink(void)
{
    for (int i = 0; i < 2; i++)
    {
        GPIOA_ODR |= (1 << 5);
        delay_ms(120);
        GPIOA_ODR &= ~(1 << 5);
        delay_ms(120);
    }
}
static void send(char c)
{
    while (!(USART1_SR & (1 << 7)))
        ;
    USART1_DR = c;
}

int main(void)
{
    RCC_AHB1ENR |= (1 << 0) | (1 << 2); /* GPIOA + GPIOC */
    RCC_APB2ENR |= (1 << 4);            /* USART1 */

    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |= (1 << 10);                /* PA5 LED */
    GPIOA_MODER &= ~((3 << 18) | (3 << 20)); /* PA9/PA10 AF7 */
    GPIOA_MODER |= ((2 << 18) | (2 << 20));
    GPIOA_AFRH &= ~((0xF << 4) | (0xF << 8));
    GPIOA_AFRH |= ((7 << 4) | (7 << 8));
    GPIOA_PUPDR &= ~(3 << 20);
    GPIOA_PUPDR |= (1 << 20); /* RX pull-up */
    GPIOC_PUPDR &= ~(3 << 26);
    GPIOC_PUPDR |= (1 << 26); /* PC13 pull-up */

    USART1_BRR = 0x0683; /* 9600 @ 16MHz */
    USART1_CR1 = (1 << 13) | (1 << 3) | (1 << 2);

    int prev = 0, awaiting = 0;
    unsigned int timer = 0;

    while (1)
    {
        /* ---- MY button → ask the other board to blink ---- */
        int now = pressed();
        if (now && !prev)
        {
            delay_ms(20); /* debounce */
            if (pressed())
            {
                send('S');
                awaiting = 1;
                timer = ACK_TIMEOUT;
            }
        }
        prev = now;

        /* ---- anything from the OTHER board? ---- */
        if (USART1_SR & (1 << 5))
        {
            char c = (char)USART1_DR;
            if (c == 'S')
            {              /* other pressed */
                blink(5);  /* my LED blinks   */
                send('D'); /* ack it          */
            }
            else if (c == 'D' && awaiting)
            { /* my press acked  */
                little_blink();
                awaiting = 0;
            }
        }
        if (awaiting && --timer == 0)
            awaiting = 0; /* no reply: give up quietly */

        delay_ms(1);
    }
}