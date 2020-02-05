#include <HardwareSerial.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <LoRa.h>

#include "gooseka_helpers.h"
#include "gooseka_structs.h"
#include "gooseka_defs.h"

HardwareSerial LEFT_ESC_serial(1);
HardwareSerial RIGHT_ESC_serial(2);

serial_buffer_t LEFT_serial_buffer;
serial_buffer_t RIGHT_serial_buffer;

ESC_telemetry_t telemetry;

bool LEFT_telemetry_complete;
bool RIGHT_telemetry_complete;

Servo LEFT_ESC_servo;
Servo RIGHT_ESC_servo;

uint8_t LEFT_duty;
uint8_t RIGHT_duty;

ESC_control_t control;

// CPU #1
// 0. Read incoming LoRa message
// 1. Set LEFT & RIGHT ESC duty cycle
void LoRa_receive_task(void* param) {
    uint8_t LoRa_buffer[sizeof(ESC_control_t)];
    uint8_t index;

    while(1) {
        int packetSize = LoRa.parsePacket();
        if (packetSize == sizeof(ESC_control_t)) {
            index = 0;
            while (LoRa.available()) {
                LoRa_buffer[index] = LoRa.read();
                index++;
            }
            memcpy(&control,LoRa_buffer,sizeof(ESC_control_t));
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

// CPU #0
// 0. Set LEFT & RIGHT ESC PWM
// 1. Read LEFT ESC telemetry
// 2. Read RIGHT ESC telemetry
// 3. If both telemetries are complete, send LoRa message
void ESC_control_task(void* param) {
    while (1) {
        LEFT_duty = control.left.duty;
        RIGHT_duty = control.right.duty;
        
        LEFT_ESC_servo.write(map(LEFT_duty,0,255,0,180));
        RIGHT_ESC_servo.write(map(RIGHT_duty,0,255,0,180));
        
        if(LEFT_ESC_serial.available()) {
            if(read_telemetry(&LEFT_ESC_serial, &LEFT_serial_buffer, &(telemetry.left))) {
                telemetry.left.duty = LEFT_duty;
                LEFT_telemetry_complete = true;
            }
        }

        if(RIGHT_ESC_serial.available()) {
            if(read_telemetry(&RIGHT_ESC_serial, &RIGHT_serial_buffer, &(telemetry.right))) {
                telemetry.right.duty = RIGHT_duty;
                RIGHT_telemetry_complete = true;
            }
        }

        if(LEFT_telemetry_complete && RIGHT_telemetry_complete) {
            LEFT_telemetry_complete = false;
            RIGHT_telemetry_complete = false;
            PRINT_TELEMETRY(&Serial, &telemetry);
            LoRa.beginPacket();
            LoRa.write((uint8_t *)&telemetry, sizeof(ESC_telemetry_t)); 
            LoRa.endPacket();
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

void setup() {
    // Initialize structs and arrays
    LEFT_telemetry_complete = false;
    RIGHT_telemetry_complete = false;
    LEFT_duty = 0;
    RIGHT_duty = 0;
    memset(&LEFT_serial_buffer,0,sizeof(serial_buffer_t));
    memset(&RIGHT_serial_buffer,0,sizeof(serial_buffer_t));
    memset(&telemetry,0,sizeof(ESC_telemetry_t));

    // Console output
    SERIAL_BEGIN(115200);

    // Telemetry serial lines
    LEFT_ESC_serial.begin(115200, SERIAL_8N1, LEFT_TELEMETRY_READ_PIN, LEFT_TELEMETRY_UNUSED_PIN);
    RIGHT_ESC_serial.begin(115200, SERIAL_8N1, RIGHT_TELEMETRY_READ_PIN, RIGHT_TELEMETRY_UNUSED_PIN);

    // Empty Rx Serial of garbage telemetry
    while(LEFT_ESC_serial.available()) {
        LEFT_ESC_serial.read();
    }
    while(RIGHT_ESC_serial.available()) {
        RIGHT_ESC_serial.read();
    }

    // Configure servos
    LEFT_ESC_servo.attach(LEFT_PWM_PIN, PWM_MIN, PWM_MAX);
    RIGHT_ESC_servo.attach(RIGHT_PWM_PIN, PWM_MIN, PWM_MAX);

    // Set SPI LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

    // Setup LoRa transceiver module
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_BAND)) {
        SERIAL_PRINTLN("LoRa init error.");
        while(1);
    }
    LoRa.setSyncWord(LORA_SYNCWORD);

    // Start ESC control task
    xTaskCreatePinnedToCore(ESC_control_task, "ESC_controller", 10000, NULL, 1, NULL, 0);
    // Start LoRa receiver task
    xTaskCreatePinnedToCore(LoRa_receive_task, "LoRa_receiver", 10000, NULL, 1, NULL, 1);
}

void loop() {
    delay(2147483647L); // Delay forever
}
