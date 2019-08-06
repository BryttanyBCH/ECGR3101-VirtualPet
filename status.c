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
#define BUTTON_2            GPIO_PIN_2
#define analog_vert         GPIO_PIN_3

extern int32_t g_hunger = 100;
extern int32_t g_happiness = 100;
extern uint32_t g_frame_count = 0;

void timer0IntHandler(){                                                    //Interrupt for frame timer
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    if(g_frame_count > 0xFFFFFFF0){
        g_frame_count = 0;
    }
    g_frame_count++;
}

void timer1IntHandler(){
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    g_hunger--;
    g_happiness--;
}

void initTimer0(){                                                          //initialize timer for frames
    SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    IntMasterEnable();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 8000000);
    TimerIntRegister(TIMER0_BASE, TIMER_A, timer0IntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);
    TimerEnable(TIMER0_BASE,TIMER_A);
}

void initTimer1(){
    //SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    //IntMasterEnable();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));
    TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, 0xFFFFFFFF);
    TimerIntRegister(TIMER1_BASE, TIMER_A, timer1IntHandler);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER1A);
    TimerEnable(TIMER1_BASE,TIMER_A);
}

void idle_animation(uint32_t frame) {                                        // idle animation

    if(frame == 0){
        ST7735_DrawCharS(42, 70, '1', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    }
    else if(frame == 1){
        ST7735_DrawCharS(42, 70, '2', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    }
    else {
        ST7735_DrawCharS(42, 70, '3', ST7735_Color565(0, 0, 255), 0xFFFF, 3);
    }

    ST7735_DrawCharS(3, 5, 'H', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(9, 5, 'a', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(15, 5, 'p', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(21, 5, 'p', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(27, 5, 'i', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(33, 5, 'n', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(39, 5, 'e', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(45, 5, 's', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(51, 5, 's', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(57, 5, ':', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(63, 5, ' ', ST7735_Color565(0, 0, 0), 0xFFFF, 1);

    ST7735_DrawCharS(3, 15, 'H', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(9, 15, 'u', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(15, 15, 'n', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(21, 15, 'g', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(27, 15, 'e', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(33, 15, 'r', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(39, 15, ':', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
    ST7735_DrawCharS(45, 15, ' ', ST7735_Color565(0, 0, 0), 0xFFFF, 1);

    switch(g_happiness){

    case 0:
        ST7735_DrawCharS(69, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 1:
        ST7735_DrawCharS(69, 5, '1', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 2:
        ST7735_DrawCharS(69, 5, '2', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 3:
        ST7735_DrawCharS(69, 5, '3', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 4:
        ST7735_DrawCharS(69, 5, '4', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 5:
        ST7735_DrawCharS(69, 5, '5', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 6:
        ST7735_DrawCharS(69, 5, '6', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 7:
        ST7735_DrawCharS(69, 5, '7', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 8:
        ST7735_DrawCharS(69, 5, '8', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 9:
        ST7735_DrawCharS(69, 5, '9', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;

    case 10:
        ST7735_DrawCharS(69, 5, '1', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(75, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(81, 5, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        ST7735_DrawCharS(87, 5, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
        break;
    }

    switch(g_hunger){

        case 0:
            ST7735_DrawCharS(51, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 1:
            ST7735_DrawCharS(51, 15, '1', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 2:
            ST7735_DrawCharS(51, 15, '2', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 3:
            ST7735_DrawCharS(51, 15, '3', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 4:
            ST7735_DrawCharS(51, 15, '4', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 5:
            ST7735_DrawCharS(51, 15, '5', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 6:
            ST7735_DrawCharS(51, 15, '6', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 7:
            ST7735_DrawCharS(51, 15, '7', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 8:
            ST7735_DrawCharS(51, 15, '8', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 9:
            ST7735_DrawCharS(51, 15, '9', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;

        case 10:
            ST7735_DrawCharS(51, 15, '1', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(57, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(63, 15, '0', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            ST7735_DrawCharS(69, 15, '%', ST7735_Color565(0, 0, 0), 0xFFFF, 1);
            break;
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


