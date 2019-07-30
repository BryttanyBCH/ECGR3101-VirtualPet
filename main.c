#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ST7735.h"
#include "PLL.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

void drawTest();
void drawTest2();

void DelayWait10ms(uint32_t n){
    uint32_t volatile time;
    while(n){
        time = 727240*2/91;  // 10msec
        while(time){time--;}
        n--;
    }
}

void PortDIntHandler()
{
    uint32_t status = 0;
    volatile uint32_t ui32Loop;

    status = GPIOIntStatus(GPIO_PORTD_BASE, true);
    GPIOIntClear(GPIO_PORTD_BASE, status);

    if((status & GPIO_INT_PIN_6) == GPIO_INT_PIN_6){
      drawTest();
    }else if ((status & GPIO_INT_PIN_2) == GPIO_INT_PIN_2){
      drawTest2();
    }
    DelayWait10ms(100);
    DelayWait10ms(100);
}

void initGPIO()
{
    // Enable the GPIO port D
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    // Enable Buttons S1 and S2 as input
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_2);
    // Enable pull-up resistor
    GPIOPadConfigSet(GPIO_PORTD_BASE,GPIO_PIN_6|GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);

    // Configure interrupt
    GPIOIntTypeSet(GPIO_PORTD_BASE,GPIO_PIN_6|GPIO_PIN_2,GPIO_FALLING_EDGE);
    GPIOIntRegister(GPIO_PORTD_BASE, PortDIntHandler);
    // Enable the interrupt
    GPIOIntEnable(GPIO_PORTD_BASE,GPIO_PIN_6|GPIO_PIN_2);
}

void initMisc()
{
    //Set system clock to 80 MHz
    PLL_Init(Bus80MHz);
}

void drawTest()
{
    //Initialize LCD
    ST7735_InitR(INITR_REDTAB);
    ST7735_FillScreen(0x1211);
}

void drawTest2()
{
    //Initialize LCD
    ST7735_InitR(INITR_REDTAB);
    ST7735_FillScreen(0x7171);
}

int main(void)
{
    initGPIO();
    initMisc();
}

