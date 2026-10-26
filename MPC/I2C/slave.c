/* STM32F446RE I2C1 slave: PB8/PB9, LED PA5, button PC13 */

typedef struct {
    volatile unsigned int MODER, OTYPER, OSPEEDR, PUPDR;
    volatile unsigned int IDR, ODR, BSRR, LCKR, AFR[2];
} GPIO;
typedef struct {
    volatile unsigned int CR, PLLCFGR, CFGR, CIR, AHB1RSTR, AHB2RSTR;
    volatile unsigned int AHB3RSTR;
    unsigned int r0;
    volatile unsigned int APB1RSTR, APB2RSTR;
    unsigned int r1[2];
    volatile unsigned int AHB1ENR, AHB2ENR, AHB3ENR;
    unsigned int r2;
    volatile unsigned int APB1ENR, APB2ENR;
} RCC_Type;
typedef struct {
    volatile unsigned int CR1, CR2, OAR1, OAR2, DR;
    volatile unsigned int SR1, SR2, CCR, TRISE, FLTR;
} I2C;
typedef struct { volatile unsigned int CTRL, LOAD, VAL, CALIB; } ST_Type;

#define RCC ((RCC_Type *)0x40023800U)
#define PA  ((GPIO *)0x40020000U)
#define PB  ((GPIO *)0x40020400U)
#define PC  ((GPIO *)0x40020800U)
#define I2C1 ((I2C *)0x40005400U)
#define ST  ((ST_Type *)0xE000E010U)
#define B(n) (1U << (n))

#define PE B(0)
#define ACK B(10)
#define RESET B(15)
#define ADDR B(1)
#define STOPF B(4)
#define RXNE B(6)
#define TXE B(7)
#define AF B(10)
#define TRA B(2)

#define SLAVE 0x30U
#define TIMEOUT 100000U
#define SYSCLK 16000000U
#define PRESSED 1U

volatile unsigned char i2c_slave_last_error;

static void GPIO_Init(void);
static void I2C_Init(void);
static unsigned char Wait(unsigned int flag);
static void LED(unsigned char on);
static void Blink(unsigned char count);
static void Delay(unsigned int ms);

int main(void)
{
    unsigned char last = 0, own = 0, button, command, ok;
    unsigned int status;

    GPIO_Init();
    I2C_Init();
    LED(1);

    while (1) {
        button = (PC->IDR & B(13)) == 0;
        if (button != last)
            LED(!button);
        last = own = button;

        if (I2C1->SR1 & ADDR) {
            (void)I2C1->SR1;
            status = I2C1->SR2;

            if (status & TRA) {
                ok = Wait(TXE);
                if (ok) {
                    I2C1->DR = own;
                    ok = Wait(AF);
                    if (I2C1->SR1 & AF)
                        I2C1->SR1 &= ~AF;
                }
                i2c_slave_last_error = ok ? 0 : 3;
            } else {
                ok = Wait(RXNE);
                command = ok ? I2C1->DR : 0;
                if (ok)
                    ok = Wait(STOPF);

                if (ok) {
                    (void)I2C1->SR1;
                    I2C1->CR1 |= PE;
                    i2c_slave_last_error = 0;
                    if (command == PRESSED) {
                        Blink(5);
                        LED(0);
                    } else if (command == 0) {
                        LED(1);
                    }
                } else {
                    i2c_slave_last_error = 1;
                }
            }
            if (i2c_slave_last_error)
                I2C_Init();
        }
        Delay(10);
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
    I2C1->OAR1 = B(14) | (SLAVE << 1);
    I2C1->CCR = 80;
    I2C1->TRISE = 17;
    I2C1->CR1 = PE | ACK;
}

static unsigned char Wait(unsigned int flag)
{
    unsigned int n = TIMEOUT;
    while (!(I2C1->SR1 & flag))
        if (--n == 0)
            return 0;
    return 1;
}

static void LED(unsigned char on)
{
    PA->BSRR = on ? B(5) : B(21);
}

static void Blink(unsigned char count)
{
    while (count--) {
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
        while (!(ST->CTRL & B(16))) {}
    ST->CTRL = 0;
}
