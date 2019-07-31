/*
 * accelerometer.h
 *
 *  Created on: Jul 31, 2019
 *      Author: Superuser
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

void initAccelerometer(void);

#endif /* ACCELEROMETER_H_ */
