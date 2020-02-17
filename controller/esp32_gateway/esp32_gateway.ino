#include <SPI.h>
#include <LoRa.h>

#include "esp32_controller_defs.h"
#include "gooseka_structs.h"

#define STATE_SOF_1 0x00
#define STATE_SOF_2 0x01
#define STATE_FRAME 0x02

#define SOF_1 0xDE
#define SOF_2 0xAD

void init_radio() {
    // Set SPI LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

    // Setup LoRa transceiver module
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_BAND)) {
        while(1);
    }
    LoRa.setSyncWord(LORA_SYNCWORD);
}

void send_via_radio(uint8_t* payload, size_t size) {
    Serial.println("LoRa send");
    LoRa.beginPacket();
    LoRa.write(payload, size); 
    LoRa.endPacket();
}

int receive_radio_packet(uint8_t* buffer, int size) {
    uint8_t index;

    int packetSize = LoRa.parsePacket();
    if (packetSize == size) {
        Serial.print("INCOMING LORA ");
        Serial.println(LoRa.packetRssi());
        index = 0;
        while (LoRa.available() && index < size) {
            buffer[index] = LoRa.read();
            index++;
        }
    }

    return packetSize;
}

// CPU #1
void radio_receive_task(void* param) {
    uint8_t radio_buffer[sizeof(ESC_telemetry_t)];
    uint8_t index;

    while(1) {
        int packetSize = receive_radio_packet(radio_buffer, sizeof(ESC_telemetry_t));
        if (packetSize == sizeof(ESC_telemetry_t)) {
            // Send packet via USB
            Serial.write(SOF_1);
            Serial.write(SOF_2);
            Serial.write(radio_buffer, sizeof(ESC_telemetry_t));
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }     
    vTaskDelete(NULL);
}

// CPU #0
void USB_receive_task(void* param) {
    uint8_t state = STATE_SOF_1;
    uint8_t index;
    uint8_t buffer[sizeof(ESC_control_t)];
    memset(&buffer, 0, sizeof(ESC_control_t));

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
                        index = 0;
                    } else {
                        state = STATE_SOF_1;
                    }
                break;
                case STATE_FRAME:
                    Serial.println("FRAME");
                    if (index < sizeof(ESC_control_t) - 1) {
                        buffer[index] = incomingByte;
                        index++;
                    } else {
                        buffer[index] = incomingByte;
                        state = STATE_SOF_1;
                        send_via_radio(buffer, sizeof(ESC_control_t));
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

    // USB output
    Serial.begin(SERIAL_BAUDRATE);

    // Initialize radio interface
    init_radio();

    // Start USB receiver task
    xTaskCreatePinnedToCore(USB_receive_task, "USB_receiver", 10000, NULL, 1, NULL, 0);
    // Start LoRa receiver task
    xTaskCreatePinnedToCore(radio_receive_task, "radio_receiver", 10000, NULL, 1, NULL, 1);
}

void loop() {
    delay(2147483647L); // Delay forever
}
