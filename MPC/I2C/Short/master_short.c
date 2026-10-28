#define SLAVE_ADDR 0x28
#define BLINK_MS 200
#define SLAVE_BLINK_WAIT 2500
#define GUARD 200000

#define RCC_AHB1ENR (*(volatile unsigned int *)0x40023830)
#define RCC_APB1ENR (*(volatile unsigned int *)0x40023840)
#define GPIOA_MODER (*(volatile unsigned int *)0x40020000)
#define GPIOA_ODR (*(volatile unsigned int *)0x40020014)
#define GPIOB_MODER (*(volatile unsigned int *)0x40020400)
#define GPIOB_OTYPER (*(volatile unsigned int *)0x40020404)
#define GPIOB_PUPDR (*(volatile unsigned int *)0x4002040C)
#define GPIOB_AFRH (*(volatile unsigned int *)0x40020424)
#define GPIOC_PUPDR (*(volatile unsigned int *)0x4002080C)
#define GPIOC_IDR (*(volatile unsigned int *)0x40020810)
#define I2C1_CR1 (*(volatile unsigned int *)0x40005400)
#define I2C1_CR2 (*(volatile unsigned int *)0x40005404)
#define I2C1_DR (*(volatile unsigned int *)0x40005410)
#define I2C1_SR1 (*(volatile unsigned int *)0x40005414)
#define I2C1_SR2 (*(volatile unsigned int *)0x40005418)
#define I2C1_CCR (*(volatile unsigned int *)0x4000541C)
#define I2C1_TRISE (*(volatile unsigned int *)0x40005420)

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

/* Write one byte to the slave. 1 = ACKed, 0 = no slave / error. */
static int i2c_write(unsigned char addr, unsigned char data)
{
    unsigned int g;
    g = GUARD;
    while ((I2C1_SR2 & (1 << 1)) && --g)
        ;
    if (!g)
        return 0;
    I2C1_CR1 |= (1 << 8);
    g = GUARD;
    while (!(I2C1_SR1 & (1 << 0)) && --g)
        ;
    if (!g)
    {
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    I2C1_DR = (addr << 1) | 0;
    g = GUARD;
    while (!(I2C1_SR1 & ((1 << 1) | (1 << 10))) && --g)
        ;
    if (I2C1_SR1 & (1 << 10))
    {
        I2C1_SR1 &= ~(1 << 10);
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    if (!(I2C1_SR1 & (1 << 1)))
    {
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    (void)I2C1_SR1;
    (void)I2C1_SR2;
    g = GUARD;
    while (!(I2C1_SR1 & (1 << 7)) && --g)
        ;
    I2C1_DR = data;
    g = GUARD;
    while (!(I2C1_SR1 & (1 << 2)) && --g)
        ;
    I2C1_CR1 |= (1 << 9);
    return 1;
}

/* Read one byte from the slave into *out. 1 = ok, 0 = no slave / error. */
static int i2c_read(unsigned char addr, unsigned char *out)
{
    unsigned int g;
    I2C1_CR1 |= (1 << 10);
    g = GUARD;
    while ((I2C1_SR2 & (1 << 1)) && --g)
        ;
    if (!g)
        return 0;
    I2C1_CR1 |= (1 << 8);
    g = GUARD;
    while (!(I2C1_SR1 & (1 << 0)) && --g)
        ;
    if (!g)
    {
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    I2C1_DR = (addr << 1) | 1;
    g = GUARD;
    while (!(I2C1_SR1 & ((1 << 1) | (1 << 10))) && --g)
        ;
    if (I2C1_SR1 & (1 << 10))
    {
        I2C1_SR1 &= ~(1 << 10);
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    if (!(I2C1_SR1 & (1 << 1)))
    {
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    I2C1_CR1 &= ~(1 << 10); /* NACK the single byte */
    (void)I2C1_SR1;
    (void)I2C1_SR2;
    I2C1_CR1 |= (1 << 9);
    g = GUARD;
    while (!(I2C1_SR1 & (1 << 6)) && --g)
        ;
    if (!g)
        return 0;
    *out = (unsigned char)I2C1_DR;
    return 1;
}

int main(void)
{
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2); /* GPIOA + GPIOB + GPIOC */
    RCC_APB1ENR |= (1 << 21);                      /* I2C1 */

    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |= (1 << 10);                /* PA5 LED */
    GPIOB_MODER &= ~((3 << 16) | (3 << 18)); /* PB8/9 AF4 OD PU */
    GPIOB_MODER |= ((2 << 16) | (2 << 18));
    GPIOB_OTYPER |= (1 << 8) | (1 << 9);
    GPIOB_PUPDR &= ~((3 << 16) | (3 << 18));
    GPIOB_PUPDR |= ((1 << 16) | (1 << 18));
    GPIOB_AFRH &= ~((0xF << 0) | (0xF << 4));
    GPIOB_AFRH |= ((4 << 0) | (4 << 4));
    GPIOC_PUPDR &= ~(3 << 26);
    GPIOC_PUPDR |= (1 << 26); /* PC13 pull-up */

    I2C1_CR1 |= (1 << 15);
    I2C1_CR1 &= ~(1 << 15);
    I2C1_CR2 = 16;
    I2C1_CCR = 80;
    I2C1_TRISE = 17;
    I2C1_CR1 |= (1 << 0);

    int prev = 0, prevB = 0;
    while (1)
    {
        /* ---- MY button → slave blinks, then I little-blink ---- */
        int now = pressed();
        if (now && !prev)
        {
            delay_ms(20);
            if (pressed() && i2c_write(SLAVE_ADDR, 'S'))
            {
                delay_ms(SLAVE_BLINK_WAIT); /* let the slave finish */
                little_blink();             /* my ack */
            }
        }
        prev = now;

        /* ---- poll the SLAVE's button ---- */
        unsigned char r = 0;
        if (i2c_read(SLAVE_ADDR, &r))
        {
            int nowB = (r == 'B');
            if (nowB && !prevB)
            {                               /* slave just pressed */
                blink(5);                   /* my LED blinks      */
                i2c_write(SLAVE_ADDR, 'K'); /* ack → slave little-blinks */
            }
            prevB = nowB;
        }

        delay_ms(10);
    }
}