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

uint32_t last_received_millis;
uint32_t last_sent_millis;


void init_radio() {
    // Set SPI LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

    // Setup LoRa transceiver module
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_BAND)) {
        SERIAL_PRINTLN("LoRa init error.");
        while(1);
    }
    LoRa.setSyncWord(LORA_SYNCWORD);
}

void send_via_radio(uint8_t* payload, size_t size) {
    LoRa.beginPacket();
    LoRa.write(payload, size); 
    LoRa.endPacket();
}

int receive_radio_packet(uint8_t* buffer, int size) {
    uint8_t index;

    int packetSize = LoRa.parsePacket();
    if (packetSize == size) {
        index = 0;
        while (LoRa.available() && index < size) {
            buffer[index] = LoRa.read();
            index++;
        }
    }

    return packetSize;
}

// CPU #1
// 0. Read incoming LoRa message
// 1. Set LEFT & RIGHT ESC duty cycle
void radio_receive_task(void* param) {
    uint8_t radio_buffer[sizeof(ESC_control_t)];
    uint8_t index;

    while(1) {
        int packetSize = receive_radio_packet(radio_buffer, sizeof(ESC_control_t));
        if (packetSize == sizeof(ESC_control_t)) {
            memcpy(&control, radio_buffer, sizeof(ESC_control_t));
            last_received_millis = millis();
            SERIAL_PRINT("Received commands: ");
            SERIAL_PRINT(control.left.duty);
            SERIAL_PRINT(",");
            SERIAL_PRINTLN(control.right.duty);
            SERIAL_PRINT(",");
            SERIAL_PRINTLN(LoRa.packetRssi());
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
        if(millis() - last_received_millis > RADIO_IDLE_TIMEOUT) {
            SERIAL_PRINTLN("OUT OF RANGE");
            last_received_millis = millis();
            control.left.duty = 0;
            control.right.duty = 0;
        }
        LEFT_duty = control.left.duty;
        RIGHT_duty = control.right.duty;
        
        LEFT_ESC_servo.write(map(LEFT_duty,0,255,0,180));
        RIGHT_ESC_servo.write(map(RIGHT_duty,0,255,0,180));
        
        if(LEFT_ESC_serial.available()) {
            // SERIAL_PRINTLN("LEFT AVAILABLE");
            if(read_telemetry(&LEFT_ESC_serial, &LEFT_serial_buffer, &(telemetry.left))) {
                telemetry.left.duty = LEFT_duty;
                LEFT_telemetry_complete = true;
                // SERIAL_PRINTLN("LEFT TELEMETRY");
            }
        }

        if(RIGHT_ESC_serial.available()) {
            // SERIAL_PRINTLN("RIGHT AVAILABLE");
            if(read_telemetry(&RIGHT_ESC_serial, &RIGHT_serial_buffer, &(telemetry.right))) {
                telemetry.right.duty = RIGHT_duty;
                RIGHT_telemetry_complete = true;
                // SERIAL_PRINTLN("RIGHT TELEMETRY");
            }
        }

        if(LEFT_telemetry_complete && RIGHT_telemetry_complete) {
            LEFT_telemetry_complete = false;
            RIGHT_telemetry_complete = false;
            if(millis() - last_sent_millis > LORA_SLOWDOWN) {
                PRINT_TELEMETRY(&Serial, &telemetry);
                last_sent_millis = millis();
                send_via_radio((uint8_t *)&telemetry, sizeof(ESC_telemetry_t)); 
            }
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
    last_received_millis = millis();
    last_sent_millis = millis();
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

    // Init radio interface
    init_radio();

    // Start ESC control task
    xTaskCreatePinnedToCore(ESC_control_task, "ESC_controller", 10000, NULL, 1, NULL, 0);
    // Start LoRa receiver task
    xTaskCreatePinnedToCore(radio_receive_task, "radio_receiver", 10000, NULL, 1, NULL, 1);
}

void loop() {
    delay(2147483647L); // Delay forever
}
