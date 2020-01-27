#include <SPI.h>
#include <LoRa.h>

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

// When a packet is received, print its RSSI and contents.
void receive_task(void* param) {
    //Do stuff
    while(1) {
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            Serial.print("Received byte 0x");
            while (LoRa.available()) {
                uint8_t data = LoRa.read();
                Serial.print(data, HEX);
            }
            Serial.print(" with RSSI ");
            Serial.println(LoRa.packetRssi());
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }     
    vTaskDelete(NULL);
}

// CPU #0
// Perform cycle:
// 1. Read commands from USB
// 2. Send commands via LoRa
void control_task(void* param) {
    while(1){
        // Read commands from USB
        // Do Stuff
        if (Serial.available() > 0) {
            uint8_t incomingByte = Serial.read();
            Serial.print("Sent byte 0x");
            Serial.println(incomingByte, HEX);
            // Send LoRa packet to receiver
            LoRa.beginPacket();
            LoRa.write(&incomingByte, 1); // MAX 255 bytes
            LoRa.endPacket();
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

// Init LoRa comms
void setup() {
    //Initialize Serial comms
    Serial.begin(115200);
    Serial.println("Starting LoRa tester...");

    //SPI LoRa pins
    SPI.begin(SCK, MISO, MOSI, SS);
    //setup LoRa transceiver module
    LoRa.setPins(SS, RST, DIO0);
    if(!LoRa.begin(BAND)) {
        Serial.println("LoRa init error.");
        while(1);
    }
    LoRa.setSyncWord(0xCA);
    Serial.println("LoRa Initializing OK!");
    //Puts the radio in continuous receive mode
    //LoRa.receive();
    //LoRa.onReceive(onReceive);

    // Init Serial Port listener task
    xTaskCreatePinnedToCore(control_task, "Control", 10000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(receive_task, "Receive", 10000, NULL, 1, NULL, 1);
}

void loop() {
    delay(2147483647L);
}
