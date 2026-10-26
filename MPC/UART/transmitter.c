/* STM32F446RE USART1: PA9/PA10, LED PA5, button PC13 */

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
    volatile unsigned int SR, DR, BRR, CR1, CR2, CR3, GTPR;
} UART;

#define RCC ((RCC_Type *)0x40023800U)
#define PA ((GPIO *)0x40020000U)
#define PC ((GPIO *)0x40020800U)
#define U ((UART *)0x40011000U)
#define B(n) (1U << (n))

#define RXNE B(5)
#define TC B(6)
#define TXE B(7)
#define ORE B(3)
#define LIMIT 100000U
#define DELAY 4000U
#define PRESSED 1U

volatile unsigned char uart_last_error;

static void Init(void);
static unsigned char Send(unsigned char data);
static unsigned char Wait(unsigned int flag, unsigned char error);
static void LED(unsigned char on);
static void Blink(unsigned char count);
static void Delay(unsigned int ms);

int main(void)
{
    unsigned char last = 0, button, data;

    Init();
    LED(1);

    while (1)
    {
        button = (PC->IDR & B(13)) == 0;
        if (button != last)
        {
            LED(!button);
            Send(button);
            last = button;
        }

        if (U->SR & RXNE)
        {
            data = U->DR;
            if (data == PRESSED)
            {
                Blink(5);
                LED(0);
            }
            else if (!data)
            {
                LED(1);
            }
        }
        else if (U->SR & ORE)
        {
            (void)U->DR;
        }
        Delay(10);
    }
}

static void Init(void)
{
    RCC->AHB1ENR |= B(0) | B(2);

    PA->MODER = (PA->MODER & ~(3U << 10)) | B(10);
    PA->OTYPER &= ~B(5);
    PA->OSPEEDR |= 3U << 10;

    PC->MODER &= ~(3U << 26);
    PC->PUPDR &= ~(3U << 26);

    PA->MODER = (PA->MODER & ~(0xFU << 18)) | (0xAU << 18);
    PA->OTYPER &= ~(B(9) | B(10));
    PA->OSPEEDR |= 0xFU << 18;
    PA->PUPDR &= ~(0xFU << 18);
    PA->AFR[1] = (PA->AFR[1] & ~(0xFFU << 4)) | (0x77U << 4);

    RCC->APB2ENR |= B(4);
    U->BRR = 0x683U; /* 9600 baud at 16 MHz. */
    U->CR1 = B(13) | B(3) | B(2);
}

static unsigned char Wait(unsigned int flag, unsigned char error)
{
    unsigned int n = LIMIT;
    while (!(U->SR & flag))
        if (--n == 0)
        {
            uart_last_error = error;
            return 0;
        }
    return 1;
}

static unsigned char Send(unsigned char data)
{
    if (!Wait(TXE, 1))
        return 0;
    U->DR = data;
    if (!Wait(TC, 2))
        return 0;
    uart_last_error = 0;
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
    volatile unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < DELAY; j++)
        {
        }
}
