#define BLINK_MS 200
#define SLAVE_BLINK_WAIT 2500 /* > slave's 5-blink time; don't poll while it blinks */

#define RCC_AHB1ENR (*(volatile unsigned int *)0x40023830)
#define RCC_APB1ENR (*(volatile unsigned int *)0x40023840)
#define GPIOA_MODER (*(volatile unsigned int *)0x40020000)
#define GPIOA_ODR (*(volatile unsigned int *)0x40020014)
#define GPIOB_MODER (*(volatile unsigned int *)0x40020400)
#define GPIOB_PUPDR (*(volatile unsigned int *)0x4002040C)
#define GPIOB_AFRH (*(volatile unsigned int *)0x40020424)
#define GPIOC_PUPDR (*(volatile unsigned int *)0x4002080C)
#define GPIOC_IDR (*(volatile unsigned int *)0x40020810)
#define SPI2_CR1 (*(volatile unsigned int *)0x40003800)
#define SPI2_SR (*(volatile unsigned int *)0x40003808)
#define SPI2_DR (*(volatile unsigned int *)0x4000380C)

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
static unsigned char spi_xfer(unsigned char b)
{
    while (!(SPI2_SR & (1 << 1)))
        ;
    SPI2_DR = b;
    while (!(SPI2_SR & (1 << 0)))
        ;
    return (unsigned char)SPI2_DR;
}

int main(void)
{
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2); /* GPIOA + GPIOB + GPIOC */
    RCC_APB1ENR |= (1 << 14);                      /* SPI2 */

    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |= (1 << 10);                             /* PA5 LED */
    GPIOB_MODER &= ~((3 << 26) | (3 << 28) | (3U << 30)); /* PB13/14/15 AF5 */
    GPIOB_MODER |= ((2 << 26) | (2 << 28) | (2U << 30));
    GPIOB_AFRH &= ~((0xF << 20) | (0xF << 24) | (0xFU << 28));
    GPIOB_AFRH |= ((5 << 20) | (5 << 24) | (5U << 28));
    GPIOB_PUPDR &= ~(3 << 28);
    GPIOB_PUPDR |= (2 << 28); /* MISO pull-down */
    GPIOC_PUPDR &= ~(3 << 26);
    GPIOC_PUPDR |= (1 << 26); /* PC13 pull-up */

    SPI2_CR1 = (1 << 2) | (5 << 3) | (1 << 9) | (1 << 8) | (1 << 6); /* master */

    int prev = 0, prevB = 0;
    while (1)
    {
        /* ---- MY button → slave blinks, then I little-blink ONLY if it answered ---- */
        int now = pressed();
        if (now && !prev)
        {
            delay_ms(20);
            if (pressed())
            {
                spi_xfer('S');              /* tell slave: blink             */
                delay_ms(SLAVE_BLINK_WAIT); /* let it finish                 */
                if (spi_xfer(0x00) == 'D')  /* 'D' comes only if it really blinked */
                    little_blink();         /* my ack. No 'D' → stay dark    */
            }
        }
        prev = now;

        /* ---- poll the SLAVE's button ---- */
        unsigned char r = spi_xfer(0x00);
        int nowB = (r == 'B');
        if (nowB && !prevB)
        {                  /* slave just pressed */
            blink(5);      /* my LED blinks      */
            spi_xfer('K'); /* ack → slave little-blinks */
        }
        prevB = nowB;

        delay_ms(10);
    }
}