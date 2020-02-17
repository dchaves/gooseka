#include <SPI.h>
#include <LoRa.h>

#include "esp32_controller_defs.h"
#include "gooseka_structs.h"

#define STATE_SOF_1 0x00
#define STATE_SOF_2 0x01
#define STATE_FRAME 0x02

#define SOF_1 0xDE
#define SOF_2 0xAD

typedef struct {
    uint8_t index;
    uint8_t buffer[sizeof(ESC_control_t)];
} USB_buffer_t;

typedef struct {
    uint8_t index;
    uint8_t buffer[sizeof(ESC_telemetry_t)];
} LoRa_buffer_t;

USB_buffer_t USB_buffer;

ESC_telemetry_t telemetry;

uint8_t state;

void LoRa_receive_task(void* param) {
    uint8_t LoRa_buffer[sizeof(ESC_telemetry_t)];
    uint8_t index;

    while(1) {
        int packetSize = LoRa.parsePacket();
        if (packetSize == sizeof(ESC_telemetry_t)) {
            Serial.print("INCOMING LORA ");
            Serial.println(LoRa.packetRssi());
            index = 0;
            while (LoRa.available() && index < sizeof(ESC_telemetry_t)) {
                LoRa_buffer[index] = LoRa.read();
                index++;
            }
            // Send packet via USB
            // Serial.write(SOF_1);
            // Serial.write(SOF_2);
            // Serial.write(LoRa_buffer, sizeof(ESC_telemetry_t));
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }     
    vTaskDelete(NULL);
}

void USB_receive_task(void* param) {
    while(1){
        if (Serial.available() > 0) {
            // Receive USB char
            uint8_t incomingByte = Serial.read();
            Serial.println(incomingByte, HEX);
            switch(state) {
                case STATE_SOF_1:
                    Serial.println("SOF_1");
                    if(incomingByte == SOF_1) {
                        state = STATE_SOF_2;
                    }
                break;
                case STATE_SOF_2:
                    Serial.println("SOF_2");
                    if(incomingByte == SOF_2) {
                        state = STATE_FRAME;
                        USB_buffer.index = 0;
                    } else {
                        state = STATE_SOF_1;
                    }
                break;
                case STATE_FRAME:
                    Serial.println("FRAME");
                    if (USB_buffer.index < sizeof(ESC_control_t) - 1) {
                        USB_buffer.buffer[USB_buffer.index] = incomingByte;
                        USB_buffer.index++;
                    } else {
                        USB_buffer.buffer[USB_buffer.index] = incomingByte;
                        Serial.println("LoRa send");
                        state = STATE_SOF_1;
                        LoRa.beginPacket();
                        LoRa.write(USB_buffer.buffer, sizeof(ESC_control_t)); // MAX 255 bytes
                        LoRa.endPacket();
                    }
                break;
                default:
                break;
            }
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

void setup() {
    // Initialize structs and arrays
    memset(&telemetry,0,sizeof(ESC_telemetry_t));
    state = STATE_SOF_1;
    memset(&USB_buffer, 0, sizeof(USB_buffer_t));

    // USB output
    Serial.begin(SERIAL_BAUDRATE);

    // Set SPI LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

    // Setup LoRa transceiver module
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_BAND)) {
        while(1);
    }
    LoRa.setSyncWord(LORA_SYNCWORD);

    // Start USB receiver task
    xTaskCreatePinnedToCore(USB_receive_task, "USB_receiver", 10000, NULL, 1, NULL, 0);
    // Start LoRa receiver task
    xTaskCreatePinnedToCore(LoRa_receive_task, "LoRa_receiver", 10000, NULL, 1, NULL, 1);
}

void loop() {
    delay(2147483647L); // Delay forever
}
