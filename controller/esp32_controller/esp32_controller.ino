#include <SPI.h>
#include <LoRa.h>

#include "esp32_controller_defs.h"
#include "gooseka_structs.h"

ESC_control_t control;

ESC_telemetry_t telemetry;

void LoRa_receive_task(void* param) {
    while(1) {
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            // Serial.print("Received byte 0x");
            // while (LoRa.available()) {
            //     uint8_t data = LoRa.read();
            //     Serial.print(data, HEX);
            // }
            // Serial.print(" with RSSI ");
            // Serial.println(LoRa.packetRssi());
            // Cast packet to struct
            // Send packet via USB
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }     
    vTaskDelete(NULL);
}

void USB_receive_task(void* param) {
    while(1){
        // Read commands from USB
        // Do Stuff
        if (Serial.available() > 0) {
            // Receive USB char
            // uint8_t incomingByte = Serial.read();
            // Serial.print("Sent byte 0x");
            // Serial.println(incomingByte, HEX);

            // IF received a complete msg
            // Send LoRa packet to receiver
            // LoRa.beginPacket();
            // LoRa.write(&incomingByte, 1); // MAX 255 bytes
            // LoRa.endPacket();
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

void setup() {
    // Initialize structs and arrays
    memset(&telemetry,0,sizeof(ESC_telemetry_t));

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
