/* STM32F446RE I2C1 master: PB8/PB9, LED PA5, button PC13 */

typedef struct
{
    volatile unsigned int MODER, OTYPER, OSPEEDR, PUPDR;
    volatile unsigned int IDR, ODR, BSRR, LCKR, AFR[2];
} GPIO;
typedef struct
{
    volatile unsigned int CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR;
    volatile unsigned int AHB3RSTR;
    unsigned int r0;
    volatile unsigned int APB1RSTR, APB2RSTR;
    unsigned int r1[2];
    volatile unsigned int AHB1ENR, AHB2ENR, AHB3ENR;
    unsigned int r2;
    volatile unsigned int APB1ENR, APB2ENR;
} RCC_Type;
typedef struct
{
    volatile unsigned int CR1, CR2, OAR1, OAR2, DR;
    volatile unsigned int SR1, SR2, CCR, TRISE, FLTR;
} I2C;
typedef struct
{
    volatile unsigned int CTRL, LOAD, VAL, CALIB;
} SYSTICK_Type;

#define RCC ((RCC_Type *)0x40023800U)
#define PA ((GPIO *)0x40020000U)
#define PB ((GPIO *)0x40020400U)
#define PC ((GPIO *)0x40020800U)
#define I2C1 ((I2C *)0x40005400U)
#define ST ((SYSTICK_Type *)0xE000E010U)
#define B(n) (1U << (n))

#define PE B(0)
#define START B(8)
#define STOP B(9)
#define ACK B(10)
#define RESET B(15)
#define SB B(0)
#define ADDR B(1)
#define BTF B(2)
#define RXNE B(6)
#define TXE B(7)
#define AF B(10)

#define SLAVE 0x30U
#define TIMEOUT 100000U
#define SYSCLK 16000000U
#define PRESSED 1U

volatile unsigned char i2c_last_error;

static void GPIO_Init(void);
static void I2C_Init(void);
static void Recover(void);
static unsigned char Wait(unsigned int flag, unsigned char error);
static unsigned char Address(unsigned char read);
static unsigned char Write(unsigned char data);
static unsigned char Read(unsigned char *data);
static void LED(unsigned char on);
static void Blink(unsigned char count);
static void Delay(unsigned int ms);

int main(void)
{
    unsigned char own = 0, remote = 0, button, data;

    GPIO_Init();
    I2C_Init();
    LED(1);

    while (1)
    {
        button = (PC->IDR & B(13)) == 0;
        if (button != own)
        {
            LED(!button);
            if (!Write(button) && i2c_last_error == 1)
                Recover();
            own = button;
        }

        if (Read(&data))
        {
            if (data == PRESSED && !remote)
            {
                Blink(5);
                LED(0);
                remote = 1;
            }
            else if (data == 0 && remote)
            {
                LED(1);
                remote = 0;
            }
        }
        else if (i2c_last_error == 1)
        {
            Recover();
        }
        Delay(20);
    }
}

static void GPIO_Init(void)
{
    RCC->AHB1ENR |= B(0) | B(1) | B(2);

    PA->MODER = (PA->MODER & ~(3U << 10)) | B(10);
    PA->OTYPER &= ~B(5);
    PA->OSPEEDR |= 3U << 10;

    PC->MODER &= ~(3U << 26);
    PC->PUPDR &= ~(3U << 26);

    PB->MODER = (PB->MODER & ~(0xFU << 16)) | (0xAU << 16);
    PB->OTYPER |= B(8) | B(9);
    PB->OSPEEDR |= 0xFU << 16;
    PB->PUPDR = (PB->PUPDR & ~(0xFU << 16)) | (5U << 16);
    PB->AFR[1] = (PB->AFR[1] & ~0xFFU) | 0x44U;
}

static void I2C_Init(void)
{
    RCC->APB1ENR |= B(21);
    I2C1->CR1 |= RESET;
    I2C1->CR1 &= ~RESET;
    I2C1->CR2 = 16;
    I2C1->CCR = 80;
    I2C1->TRISE = 17;
    I2C1->CR1 = PE | ACK;
}

static void Recover(void)
{
    I2C1->CR1 |= STOP;
    Delay(2);
    I2C_Init();
}

static unsigned char Wait(unsigned int flag, unsigned char error)
{
    unsigned int n = TIMEOUT;
    while (!(I2C1->SR1 & flag))
        if (--n == 0)
        {
            i2c_last_error = error;
            return 0;
        }
    return 1;
}

static unsigned char Address(unsigned char read)
{
    I2C1->CR1 |= START;
    if (!Wait(SB, 1))
        return 0;

    I2C1->DR = (SLAVE << 1) | read;
    if (!Wait(ADDR | AF, 3))
        return 0;
    if (I2C1->SR1 & AF)
    {
        I2C1->SR1 &= ~AF;
        i2c_last_error = 2;
        return 0;
    }
    return 1;
}

static unsigned char Write(unsigned char data)
{
    i2c_last_error = 0;
    if (!Address(0))
        return 0;
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    if (!Wait(TXE, 4))
        return 0;
    I2C1->DR = data;
    if (!Wait(BTF, 5))
        return 0;
    I2C1->CR1 |= STOP;
    return 1;
}

static unsigned char Read(unsigned char *data)
{
    i2c_last_error = 0;
    if (!Address(1))
        return 0;

    I2C1->CR1 &= ~ACK;
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    I2C1->CR1 |= STOP;
    if (!Wait(RXNE, 6))
    {
        I2C1->CR1 |= ACK;
        return 0;
    }
    *data = I2C1->DR;
    I2C1->CR1 |= ACK;
    return 1;
}

static void LED(unsigned char on)
{
    PA->BSRR = on ? B(5) : B(21);
}

static void Blink(unsigned char count)
{
    while (count--)
    {
        LED(1);
        Delay(500);
        LED(0);
        Delay(500);
    }
}

static void Delay(unsigned int ms)
{
    unsigned int i;
    ST->LOAD = SYSCLK / 1000U - 1;
    ST->VAL = 0;
    ST->CTRL = B(0) | B(2);
    for (i = 0; i < ms; i++)
        while (!(ST->CTRL & B(16)))
        {
        }
    ST->CTRL = 0;
}
