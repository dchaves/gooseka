#ifndef __COMMONS_H
#define __COMMONS_H

#include <HardwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp32-hal.h>


#define TELEMETRY_RX_PIN 16
#define TELEMETRY_TX_UNUSED_PIN 17


#define LEFT_MOTOR_CHANNEL 18
#define PWM_FREQ 20000 // 20Khz (Max freq with 10 bits is 78.125KHz)
#define PWM_RESOLUTION 10 // 10 bits => 1024



#endif /* __COMMONS_H */
