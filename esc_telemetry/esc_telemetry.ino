#include <HardwareSerial.h>
#include <ESP32Servo.h>

#include "gooseka_helpers.h"
#include "gooseka_structs.h"
#include "gooseka_defs.h"

typedef struct {
    uint8_t received_bytes;
    uint8_t buffer[SERIAL_BUFFER_SIZE];
} serial_buffer_t;

HardwareSerial LEFT_ESC_serial(1);
HardwareSerial RIGHT_ESC_serial(2);

serial_buffer_t LEFT_serial_buffer;
serial_buffer_t RIGHT_serial_buffer;

ESC_telemetry_t telemetry;
bool LEFT_telemetry_complete =  false;
bool RIGHT_telemetry_complete =  false;

Servo LEFT_ESC_servo;
Servo RIGHT_ESC_servo;

uint8_t LEFT_duty = 0;
uint8_t RIGHT_duty = 0;

void setup() {
    // Console output
    Serial.begin(115200);

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
}

bool read_telemetry(HardwareSerial* serial, serial_buffer_t* serial_buffer, ESC_oneside_telemetry_t* telemetry) {
    serial_buffer->buffer[serial_buffer->received_bytes] = serial->read();
    serial_buffer->received_bytes++;

    if(serial_buffer->received_bytes > 9){ // transmission completed
        serial_buffer->received_bytes = 0;
        uint8_t crc8 = get_crc8(serial_buffer->buffer, 9); // get the 8 bit CRC
        
        if(crc8 != serial_buffer->buffer[9]) { // CRC ERROR                
            // Empty Rx Serial of garbage telemtry
            while(serial->available()) {
                serial->read();
            }
            return false; // transmission failure 
        }

        // compute the received values
        telemetry->timestamp = millis();
        telemetry->temperature = serial_buffer->buffer[0]; // temperature
        telemetry->voltage = (serial_buffer->buffer[1]<<8) | serial_buffer->buffer[2]; // voltage
        telemetry->current = (serial_buffer->buffer[3]<<8) | serial_buffer->buffer[4]; // Current
        telemetry->power = (serial_buffer->buffer[5]<<8) | serial_buffer->buffer[6]; // used mA/h
        telemetry->erpm = (serial_buffer->buffer[7]<<8) | serial_buffer->buffer[8]; // eRpM *100

        return true;
    }
    return false;
}

void loop() {
    // TODO RECEIVE COMMANDS VIA LORA
    /************** REPLACE ***********************/
    char command = '0';
    if(Serial.available()) {
        command = Serial.read();
        if(command == '+') {
            LEFT_duty = min(LEFT_duty + 32, 255);
            RIGHT_duty = min(RIGHT_duty + 32, 255);
        } else if (command == '-') {
            LEFT_duty = max(LEFT_duty - 32, 0);
            RIGHT_duty = max(RIGHT_duty - 32, 0);
        }
    }
    /************** REPLACE END ********************/
    
    LEFT_ESC_servo.write(map(LEFT_duty,0,255,0,180));
    RIGHT_ESC_servo.write(map(RIGHT_duty,0,255,0,180));
    
    if(LEFT_ESC_serial.available()){
       if(read_telemetry(&LEFT_ESC_serial, &LEFT_serial_buffer, &(telemetry.left))) {
           telemetry.left.duty = LEFT_duty;
           LEFT_telemetry_complete = true;
       }
    }

    if(RIGHT_ESC_serial.available()){
        if(read_telemetry(&RIGHT_ESC_serial, &RIGHT_serial_buffer, &(telemetry.right))) {
           telemetry.right.duty = RIGHT_duty;
           RIGHT_telemetry_complete = true;
       }
    }

    if(LEFT_telemetry_complete && RIGHT_telemetry_complete) {
        LEFT_telemetry_complete = false;
        RIGHT_telemetry_complete = false;
        print_telemetry(&Serial, &telemetry);
        // TODO: SEND TELEMETRY VIA LORA
    }
}
