#define MY_ADDR 0x28
#define BLINK_MS 200

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
#define I2C1_OAR1 (*(volatile unsigned int *)0x40005408)
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
    I2C1_OAR1 = (MY_ADDR << 1) | (1 << 14);
    I2C1_CR2 = 16;
    I2C1_CCR = 80;
    I2C1_TRISE = 17;
    I2C1_CR1 |= (1 << 10) | (1 << 0); /* ACK + enable */

    while (1)
    {
        while (!(I2C1_SR1 & (1 << 1)))
            ;                        /* ADDR: addressed by master */
        unsigned int sr2 = I2C1_SR2; /* clears ADDR, gives direction */

        if (!(sr2 & (1 << 2)))
        { /* TRA=0: master WRITING to me */
            while (!(I2C1_SR1 & (1 << 6)))
                ; /* RXNE */
            unsigned char cmd = (unsigned char)I2C1_DR;
            while (!(I2C1_SR1 & (1 << 4)))
                ; /* STOPF */
            (void)I2C1_SR1;
            I2C1_CR1 |= (1 << 0);

            if (cmd == 'S')
                blink(5); /* master pressed → I blink */
            else if (cmd == 'K')
                little_blink(); /* my press acked           */
        }
        else
        { /* TRA=1: master READING me */
            while (!(I2C1_SR1 & (1 << 7)))
                ;                             /* TXE */
            I2C1_DR = pressed() ? 'B' : 0x00; /* report my button */
            while (!(I2C1_SR1 & (1 << 10)))
                ; /* AF: master NACKed after 1 byte */
            I2C1_SR1 &= ~(1 << 10);
        }
    }
}