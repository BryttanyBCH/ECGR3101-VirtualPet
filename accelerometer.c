/*
 * accelerometer.c
 *
 *  Created on: Jul 31, 2019
 *      Author: Superuser
 */
#include "accelerometer.h"
#define ADC_SEQUENCER 3

void initAccelerometer(void){
    // Init ADC Module
    SysCtlClockSet(SYSCTL_USE_PLL); // Set up clock freq for ADC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    // Configure Sequencer
    ADCSequenceDisable(ADC0_BASE, ADC_SEQUENCER);
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCER, 0, ADC_CTL_CH8 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCER);
}
