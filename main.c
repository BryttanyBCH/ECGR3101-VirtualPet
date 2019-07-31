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

#define ADC_SEQUENCER 3

void drawTest();
void drawTest2();

double ADC0out = 0.0;

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

void initADC0(int channelNum){
    // Init ADC Module
    SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); // Set up clock freq for ADC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));

    // Configure ADC0 Sequencer
    ADCSequenceDisable(ADC0_BASE, ADC_SEQUENCER);
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);

    switch(channelNum) {
    case 4: //VertStick
        ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0, ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);
        break;
    case 7: //Accelerometer
        ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0, ADC_CTL_CH7 | ADC_CTL_IE | ADC_CTL_END);
        break;
    }
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCER);
}

typedef enum{TRIGGER, WAIT, RESULT} adc_states;

void taskReadADC0(){
    static adc_states state = TRIGGER;

    if(state == TRIGGER){
        ADCProcessorTrigger(ADC0_BASE, ADC_SEQUENCER);
        state = WAIT;
    } else if(state == WAIT){
        if(ADCIntStatus(ADC0_BASE, ADC_SEQUENCER, false)){
            state = RESULT;
        }
    } else if(state == RESULT){
        uint32_t pui32ADC0Value;
        ADCSequenceDataGet(ADC0_BASE, ADC_SEQUENCER, &pui32ADC0Value);
        ADC0out = (double)pui32ADC0Value;
        state = TRIGGER;
    }
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
    ST7735_FillScreen(0xF000);
}

void drawTest2()
{
    //Initialize LCD
    ST7735_InitR(INITR_REDTAB);
    ST7735_FillScreen(0x0F00);
}

int main(void)
{
    initMisc();
    initGPIO();
    initADC0(4);

    while(1){

        taskReadADC0();

        if(ADC0out > 3000.0){
            drawTest();
            ADC0out = 1900.0;
        }
        else if(ADC0out < 1000.0){
            drawTest2();
            ADC0out = 1900.0;
        }
    }
}

