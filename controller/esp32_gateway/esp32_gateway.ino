#include <SPI.h>
#include <LoRa.h>

#include "esp32_controller_defs.h"
#include "gooseka_structs.h"
#include "esp32_controller_helpers.h"

#define STATE_SOF_1 0x00
#define STATE_SOF_2 0x01
#define STATE_FRAME 0x02

#define SOF_1 0xDE
#define SOF_2 0xAD

QueueHandle_t control_queue;

void init_radio() {
    // Set SPI LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

    // Setup LoRa transceiver module
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_BAND)) {
        DEBUG_PRINTLN("LoRa initialization error");
        while(1);
    }

    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSyncWord(LORA_SYNCWORD);
}

void send_via_radio(uint8_t* payload, size_t size) {
    LoRa.beginPacket();
    LoRa.write(payload, size); 
    if(LoRa.endPacket() == 1) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}

int receive_radio_packet(uint8_t* buffer, int size) {
    uint8_t index;

    int packetSize = LoRa.parsePacket();
    if (packetSize == size) {
        // DEBUG_PRINT("INCOMING LORA ");
        // DEBUG_PRINTLN(LoRa.packetRssi());
        index = 0;
        while (LoRa.available() && index < size) {
            buffer[index] = LoRa.read();
            index++;
        }
    }

    return packetSize;
}

int16_t radio_rssi() {
    return LoRa.packetRssi();
}

// CPU #1
void radio_receive_task(void* param) {
    uint8_t radio_buffer[sizeof(ESC_telemetry_t)];
    uint8_t index;
    ESC_control_t control;
    long last_sent_millis = 0;

    memset(&control,0,sizeof(ESC_control_t));

    while(1) {
        int packetSize = receive_radio_packet(radio_buffer, sizeof(ESC_telemetry_t));
        if (packetSize == sizeof(ESC_telemetry_t)) {
            // Send packet via USB
            Serial.write(SOF_1);
            Serial.write(SOF_2);
            Serial.write(radio_buffer, sizeof(ESC_telemetry_t));
            // Serial.write(radio_rssi());
        }

        if(millis() - last_sent_millis > LORA_SLOWDOWN) {
            // Send enqueued radio msgs
            xQueueReceive(control_queue, &control, 0);
            send_via_radio((uint8_t *)&control, sizeof(ESC_control_t));
            last_sent_millis = millis();
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
            // DEBUG_PRINTLN(incomingByte, HEX);
            switch(state) {
                case STATE_SOF_1:
                    // DEBUG_PRINTLN("SOF_1");
                    if(incomingByte == SOF_1) {
                        state = STATE_SOF_2;
                    }
                break;
                case STATE_SOF_2:
                    // DEBUG_PRINTLN("SOF_2");
                    if(incomingByte == SOF_2) {
                        state = STATE_FRAME;
                        index = 0;
                    } else {
                        state = STATE_SOF_1;
                    }
                break;
                case STATE_FRAME:
                    // DEBUG_PRINTLN("FRAME");
                    if (index < sizeof(ESC_control_t) - 1) {
                        buffer[index] = incomingByte;
                        index++;
                    } else {
                        buffer[index] = incomingByte;
                        state = STATE_SOF_1;
                        xQueueSend(control_queue, buffer, 0);
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
    pinMode(LED_BUILTIN, OUTPUT);

    // USB output
    Serial.begin(SERIAL_BAUDRATE);

    // Init control msg queue
    control_queue = xQueueCreate(QUEUE_SIZE, sizeof(ESC_control_t));

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
