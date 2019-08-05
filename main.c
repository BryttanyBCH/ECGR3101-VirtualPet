#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ST7735.h"
#include "PLL.h"
#include "sprite.h"
#include "menu.h"
#include "driverlib/timer.h"
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

#define CONTROLLER_PORT     GPIO_PORTD_BASE
#define BUTTON_1            GPIO_PIN_6
#define BUTTON_2            GPIO_PIN_2
#define analog_vert         GPIO_PIN_3

void drawTest();
void drawTest2();

double ADC0out = 0.0;
uint8_t hunger = 0;
uint8_t happiness = 100;
uint32_t frame_count = 0;

typedef enum {M_FEED, M_WALK, M_PET, M_EXIT} menu_item;
menu_item menuSelectItem = M_FEED;

typedef enum {START, IDLE, MENU, EAT, WALK, PET, RUN_AWAY, PERISH} game_state;
game_state state = START;

void DelayWait10ms(uint32_t n){
    uint32_t volatile time;
    while(n){
        time = 727240*2/91;  // 10msec
        while(time){time--;}
        n--;
    }
}

//For the buttons
void PortDIntHandler()
{
    uint32_t status = 0;
    volatile uint32_t ui32Loop;

    status = GPIOIntStatus(GPIO_PORTD_BASE, true);
    GPIOIntClear(GPIO_PORTD_BASE, status);

    if((status & GPIO_INT_PIN_6) == GPIO_INT_PIN_6)
    {
        switch(state){
        case START:
            state = IDLE;
            break;
        case IDLE:
            state = MENU;
            break;
        }
    }
    else if ((status & GPIO_INT_PIN_2) == GPIO_INT_PIN_2)
    {
        switch(state){
        case MENU:
            //select menu item
            switch(menuSelectItem){
            case M_FEED:
                state = EAT;
                break;
            case M_WALK:
                state = WALK;
                break;
            case M_PET:
                state = PET;
                break;
            case M_EXIT:
                state = IDLE;
                break;
            }
            break;
        case WALK:
            //stop walking
            break;
        }
    }
}

void timer0IntHandler()
{                                                    //Interrupt for frame timer
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    if(frame_count > 0xFFFFFFF0){
        frame_count = 0;
    }
    frame_count++;
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

void initTimer0()
{                   //initialize timer for frames
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

void initMisc()
{
    //Set system clock to 80 MHz
    PLL_Init(Bus80MHz);
    ST7735_InitR(INITR_REDTAB);
}

void menuSelect()
{
    initADC0(4);
    ST7735_FillScreen(0xFFFF);
    printMenu(menuSelectItem);

    while(1){
          taskReadADC0();

          if(ADC0out > 3000.0){
              //drawTest();
              menuSelectItem--;
              ADC0out = 1900.0;
          }
          else if(ADC0out < 1000.0){
              //drawTest2();
              menuSelectItem++;
              ADC0out = 1900.0;
          }
          printMenu(menuSelectItem);
      }
}

void gameState(){
while(1){

    switch(state){
        case START:
            //Display Start Screen
            printStart();
            //Clear vars from previous game

            break;
        case IDLE:
            ST7735_DrawBitmap(0, 128, one_one, 128, 128);
            break;
        case MENU:
            menuSelect();
            break;
        case EAT:
            ST7735_DrawBitmap(0, 128, for_thr, 128, 128);
            break;
        case WALK:
            ST7735_DrawBitmap(0, 128, fiv_one, 128, 128);
            break;
        case RUN_AWAY:
            ST7735_DrawBitmap(0, 128, one_fiv, 128, 128);
            break;
        case PERISH:
            ST7735_DrawBitmap(0, 128, nin_six, 128, 128);
            break;
        default:
            state = START;
            break;
    }
}
}

int main(void)
{
    initMisc();
    initGPIO();
    initTimer0();

    ST7735_FillScreen(0xFFFF);
    gameState();
}

