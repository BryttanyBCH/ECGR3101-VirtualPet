#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ST7735.h"
#include "PLL.h"
#include "sprite.h"
#include "menu.h"
#include "status.h"
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

double g_ADC0out = 0.0;

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

/*
 * Adjusts g_happiness by adding a SIGNED int (int32_t) to global g_happiness variable.
 * Upper limit is 100, lower limit is 0.
 * @param amount : (int32_t) --- the amount to increase or decrease (negative number) g_happiness by
 */
void increaseHappiness(int32_t amount){
    g_happiness += amount;
    if(g_happiness > 10){
        g_happiness = 10;
    }
    if(g_happiness < 0){
        g_happiness = 0;
    }
}

void increaseHunger(int32_t amount){
    g_hunger += amount;
    if(g_hunger > 10){
        g_hunger = 10;
    }
    if(g_hunger < 0){
        g_hunger = 0;
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
            ST7735_FillScreen(0xFFFF);
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
                //play animation, increase happiness
                ST7735_DrawBitmap(0, 128, fiv_one, 128, 128);
                int i; for(i = 0; i < 100; i++) {DelayWait10ms(1);}
                increaseHappiness(1);
                state = IDLE;
                break;
            case M_EXIT:
                ST7735_FillScreen(0xFFFF);
                state = IDLE;
                break;
            }
            break;
        case WALK:
            state = IDLE;
            break;
        }
    }
    DelayWait10ms(1);
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
        g_ADC0out = (double)pui32ADC0Value;
        state = TRIGGER;
    }
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

    while(state == MENU){
          taskReadADC0();

          if(g_ADC0out > 3000.0){
              //drawTest();
              menuSelectItem--;
              g_ADC0out = 1900.0;
              DelayWait10ms(1);
          }
          else if(g_ADC0out < 1000.0){
              //drawTest2();
              menuSelectItem++;
              g_ADC0out = 1900.0;
              DelayWait10ms(1);
          }
          printMenu(menuSelectItem);
      }
}

void gameState(){
while(1){

    if (g_hunger <= 0) {state = PERISH;}
    if (g_happiness <= 0) {state = RUN_AWAY;}

    switch(state){
        case START:
            //Display Start Screen
            printStart();
            break;
        case IDLE:
            idle_animation(g_frame_count % 4);
            break;
        case MENU:
            menuSelect();
            break;
        case EAT:
            ST7735_DrawBitmap(0, 128, for_thr, 128, 128);
            int i; for(i = 0; i < 100; i++) {DelayWait10ms(1);}
            increaseHunger(3);
            state = IDLE;
            break;
        case WALK:
            ST7735_DrawBitmap(0, 128, one_one, 128, 128);
            double lastVal, currentVal = 0.0;
            initADC0(7);
            g_ADC0out = 1900.0;
            while(state == WALK){
                idle_animation(g_frame_count % 4);
                taskReadADC0();
                currentVal = g_ADC0out;
                // check for motion threshold
                if (abs(lastVal - currentVal) > 500.00){
                    DelayWait10ms(25);
                    lastVal = currentVal;
                    ST7735_DrawBitmap(0, 128, fiv_one, 128, 128);
                    int i; for(i = 0; i < 50; i++) {DelayWait10ms(1);}
                    increaseHappiness(1);
                    increaseHunger(-1);
                }
            }
            break;
        case RUN_AWAY:
            ST7735_DrawBitmap(0, 128, one_fiv, 128, 128);
            for(i = 0; i < 200; i++) {DelayWait10ms(1);}
            while(1) {ST7735_FillScreen(0xFFFF);}
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
    initTimer1();

    ST7735_FillScreen(0xFFFF);
    gameState();
}

