#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"
#include "PLL.h"
#include "inc/tm4c123gh6pm.h"
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"

#define ADC_SEQUENCER 3

#define CONTROLLER_PORT     GPIO_PORTD_BASE
#define BUTTON_1            GPIO_PIN_6
#define BUTTON_2            GPIO_PIN_7
#define analog_vert         GPIO_PIN_3

uint32_t frame_count = 0;

void DelayWait10ms(uint32_t n){
  uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
      time--;
    }
    n--;
  }
}

void timer0IntHandler(){                                                    //Interrupt for frame timer
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    if(frame_count > 0xFFFFFFF0){
        frame_count = 0;
    }
    frame_count++;
}

void initButtons(){                                                         //initialze the two buttons
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    GPIOPinTypeGPIOInput(CONTROLLER_PORT, BUTTON_1 | BUTTON_2);
}

void initAnalog(){                                                          //initialize analog stick as analog input (ADC0)
    SysCtlClockSet(SYSCTL_USE_PLL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);                            //already taken care of in initButtons
    //while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    GPIOPinTypeADC(CONTROLLER_PORT, analog_vert);

    ADCSequenceDisable(ADC0_BASE, ADC_SEQUENCER);
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0, ADC_CTL_CH8 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCER);
}

void initTimer0(){                                                          //initialize timer for frames
    SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    IntMasterEnable();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 4000000);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer0IntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    TimerEnable(TIMER0_BASE,TIMER_A);
}

typedef enum{TRIGGER, WAIT, RESULT} adc_states;
uint32_t analogRead(){
    static adc_states state = TRIGGER;
    uint32_t adcVal;


    if(state == TRIGGER){
        ADCProcessorTrigger(ADC0_BASE, ADC_SEQUENCER);
        state = WAIT;
    }
    else if(state == WAIT){
        if(ADCIntStatus(ADC0_BASE, ADC_SEQUENCER, false)){
            state = RESULT;
        }
    }
    else if(state == RESULT){
        ADCSequenceDataGet(ADC0_BASE, ADC_SEQUENCER, &adcVal);
        state = TRIGGER;
    }
    return adcVal;
}

void clearScreen(){
    ST7735_FillScreen(0xFFFF);
}
void printStart() {                                                         //start screen

    ST7735_DrawCharS(20, 10, 'P', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(37, 10, 'r', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(54, 10, 'e', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(71, 10, 's', ST7735_Color565(0, 0,255), 0xFFFF, 3);
    ST7735_DrawCharS(88, 10, 's', ST7735_Color565(0, 0, 255), 0xFFFF, 3);

    ST7735_DrawCharS(32, 40, 'S', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(49, 40, 'W', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(66, 40, ' ', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(83, 40, '1', ST7735_Color565(0, 0, 255), 0xFFFF, 3);

    ST7735_DrawCharS(42, 70, 'T', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(59, 70, 'o', ST7735_Color565(0, 0, 255), 0xFFFF, 3);

    ST7735_DrawCharS(20, 100, 'S', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(37, 100, 't', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(54, 100, 'a', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    ST7735_DrawCharS(71, 100, 'r', ST7735_Color565(0, 0,255), 0xFFFF, 3);
    ST7735_DrawCharS(88, 100, 't', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
}

void printMenu(int sel){

    switch(sel){

    case 0 :                                                                    //highlighted over feed
        ST7735_DrawCharS(30, 10, 'M', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(47, 10, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(64, 10, 'N', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(81, 10, 'U', ST7735_Color565(255, 0, 0), 0xFFFF, 3);

        ST7735_DrawCharS(10, 40, '>', ST7735_Color565(255, 0, 0), 0xFFFF, 2);   //highlight character
        ST7735_DrawCharS(30, 40, 'F', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(40, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(66, 40, 'd', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 60, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   //turn highlight character off
        ST7735_DrawCharS(30, 60, 'W', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 60, 'a', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 60, 'l', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(64, 60, 'k', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 80, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 80, 'P', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 80, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(51, 80, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 100, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 100, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 100, 'x', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 100, 'i', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(62, 100, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        break;

    case 1 :                                                                    //highlighted over Walk
        ST7735_DrawCharS(30, 10, 'M', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(47, 10, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(64, 10, 'N', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(81, 10, 'U', ST7735_Color565(255, 0, 0), 0xFFFF, 3);

        ST7735_DrawCharS(10, 40, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 40, 'F', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(40, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(66, 40, 'd', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 60, '>', ST7735_Color565(255, 0, 0), 0xFFFF, 2);   //highlight character
        ST7735_DrawCharS(30, 60, 'W', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 60, 'a', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 60, 'l', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(64, 60, 'k', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 80, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 80, 'P', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 80, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(51, 80, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 100, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 100, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 100, 'x', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 100, 'i', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(62, 100, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        break;

    case 2 :
        ST7735_DrawCharS(30, 10, 'M', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(47, 10, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(64, 10, 'N', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(81, 10, 'U', ST7735_Color565(255, 0, 0), 0xFFFF, 3);

        ST7735_DrawCharS(10, 40, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 40, 'F', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(40, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(66, 40, 'd', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 60, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 60, 'W', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 60, 'a', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 60, 'l', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(64, 60, 'k', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 80, '>', ST7735_Color565(255, 0, 0), 0xFFFF, 2);   //highlight character
        ST7735_DrawCharS(30, 80, 'P', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 80, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(51, 80, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 100, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 100, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 100, 'x', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 100, 'i', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(62, 100, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        break;

    case 3 :
        ST7735_DrawCharS(30, 10, 'M', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(47, 10, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(64, 10, 'N', ST7735_Color565(255, 0, 0), 0xFFFF, 3);
        ST7735_DrawCharS(81, 10, 'U', ST7735_Color565(255, 0, 0), 0xFFFF, 3);

        ST7735_DrawCharS(10, 40, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 40, 'F', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(40, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 40, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(66, 40, 'd', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 60, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 60, 'W', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 60, 'a', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 60, 'l', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(64, 60, 'k', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 80, '>', ST7735_Color565(255, 255, 255), 0xFFFF, 2);   // turn highlight character off
        ST7735_DrawCharS(30, 80, 'P', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 80, 'e', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(51, 80, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);

        ST7735_DrawCharS(10, 100, '>', ST7735_Color565(255, 0, 0), 0xFFFF, 2);   //highlight character
        ST7735_DrawCharS(30, 100, 'E', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(42, 100, 'x', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(53, 100, 'i', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        ST7735_DrawCharS(62, 100, 't', ST7735_Color565(255, 0, 0), 0xFFFF, 2);
        break;
    }
}

void idle_animation(uint32_t frame){                                        // idle animation

    if(frame == 0){
        ST7735_DrawCharS(42, 70, '1', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    }
    else if(frame == 1){
        ST7735_DrawCharS(42, 70, '2', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    }
    else {
        ST7735_DrawCharS(42, 70, '3', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    }

    ST7735_DrawCharS(3, 120, 'S', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(9, 120, 'W', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(15, 120, ' ', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(21, 120, '1', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(27, 120, '=', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(33, 120, '>', ST7735_Color565(0, 0, 0), 0xFFFF, 1);

    ST7735_DrawCharS(39, 120, 'M', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(45, 120, 'e', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(51, 120, 'n', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(57, 120, 'u', ST7735_Color565(0, 0, 0), 0xFFFF, 1);

}

typedef enum{MENU, IDLE, WALK, FEED, PET} mode;

int main2(void){  // main 2

    PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
    ST7735_InitR(INITR_REDTAB);
    clearScreen();
    printStart();
    initButtons();
    initTimer0();
    initAnalog();
    int select = 0;

    static mode state = IDLE;
    uint32_t stick = analogRead();

    while(GPIOPinRead(CONTROLLER_PORT, BUTTON_1) != 0){                         //check for input from sw1 to start
        printStart();
    }
    clearScreen();

    while(1){
        switch(state){

        case IDLE :
            while(GPIOPinRead(CONTROLLER_PORT, BUTTON_1) != 0){                         //check for input from sw1 to bring up menu
                idle_animation(frame_count % 3);                                      //modded by amount of frames in animation
            }
            clearScreen();
            state = MENU;
            break;

        case MENU :
            printMenu(select);
            if(analogRead() > 4000){
                if(select == 0){
                    select = 3;
                }
                else{
                    select--;
                }
            }
            break;

    }
}
}
