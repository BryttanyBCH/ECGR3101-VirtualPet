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


//typedef enum{MENU, IDLE, WALK, FEED, PET} mode;
//
//int main2(void){  // main 2
//
//    PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
//    ST7735_InitR(INITR_REDTAB);
//    clearScreen();
//    printStart();
//    initButtons();
//    initTimer0();
//    initAnalog();
//    int select = 0;
//
//    static mode state = IDLE;
//    uint32_t stick = analogRead();
//
//    while(GPIOPinRead(CONTROLLER_PORT, BUTTON_1) != 0){                         //check for input from sw1 to start
//        printStart();
//    }
//    clearScreen();
//
//    while(1){
//        switch(state){
//
//        case IDLE :
//            while(GPIOPinRead(CONTROLLER_PORT, BUTTON_1) != 0){                         //check for input from sw1 to bring up menu
//                idle_animation(frame_count % 3);                                      //modded by amount of frames in animation
//            }
//            clearScreen();
//            state = MENU;
//            break;
//
//        case MENU :
//            printMenu(select);
//            if(analogRead() > 4000){
//                if(select == 0){
//                    select = 3;
//                }
//                else{
//                    select--;
//                }
//            }
//            break;
//
//    }
//}
//}
