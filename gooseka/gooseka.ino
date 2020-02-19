#include <HardwareSerial.h>
#include <ESP32Servo.h>
#include <SPI.h>
#include <LoRa.h>

#include "gooseka_helpers.h"
#include "gooseka_structs.h"
#include "gooseka_defs.h"

QueueHandle_t control_queue;
QueueHandle_t telemetry_queue;

void init_radio() {
    // Set SPI LoRa pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

    // Setup LoRa transceiver module
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if(!LoRa.begin(LORA_BAND)) {
        DEBUG_PRINTLN("LoRa init error.");
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
    uint8_t index = 0;

    int packetSize = LoRa.parsePacket();
    if (packetSize == size) {
        while (LoRa.available() && index < size) {
            buffer[index] = LoRa.read();
            index++;
        }
    }

    return packetSize;
}

// CPU #1
// 0. Read incoming message
// 1. Set LEFT & RIGHT ESC duty cycle
void radio_receive_task(void* param) {
    uint8_t radio_buffer[sizeof(ESC_control_t)];
    uint8_t index;
    ESC_control_t control;
    uint32_t last_received_millis;
    Servo LEFT_ESC_servo;
    Servo RIGHT_ESC_servo;
    uint32_t last_sent_millis;
    ESC_telemetry_t telemetry;

    memset(&telemetry,0,sizeof(ESC_telemetry_t));
    memset(&control,0,sizeof(ESC_control_t));
    last_sent_millis = millis();
    
    // Configure servos
    LEFT_ESC_servo.attach(LEFT_PWM_PIN, PWM_MIN, PWM_MAX);
    RIGHT_ESC_servo.attach(RIGHT_PWM_PIN, PWM_MIN, PWM_MAX);

    while(1) {
        int packetSize = receive_radio_packet(radio_buffer, sizeof(ESC_control_t));
        // DEBUG_PRINTLN(packetSize);
        if (packetSize == sizeof(ESC_control_t)) {
            memcpy(&control, radio_buffer, sizeof(ESC_control_t));
            last_received_millis = millis();
            xQueueSend(control_queue, &control, 0);
            
            DEBUG_PRINT("Received commands: ");
            DEBUG_PRINT(control.left.duty);
            DEBUG_PRINT(",");
            DEBUG_PRINT(control.right.duty);
            DEBUG_PRINT(",");
            DEBUG_PRINTLN(LoRa.packetRssi());
        } else if(millis() - last_received_millis > RADIO_IDLE_TIMEOUT) {
            DEBUG_PRINTLN("OUT OF RANGE");
            last_received_millis = millis();
            control.left.duty = 0;
            control.right.duty = 0;
        }
        
        LEFT_ESC_servo.write(map(control.left.duty,0,255,0,180));
        RIGHT_ESC_servo.write(map(control.right.duty,0,255,0,180));

        if(millis() - last_sent_millis > LORA_SLOWDOWN) {
            // Send enqueued msgs
            last_sent_millis = millis();
            xQueueReceive(telemetry_queue, &telemetry, 0);
            send_via_radio((uint8_t *)&telemetry, sizeof(ESC_telemetry_t));
            //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            DEBUG_TELEMETRY(&Serial, &telemetry);
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

// CPU #0
// 0. Set LEFT & RIGHT ESC PWM
// 1. Read LEFT ESC telemetry
// 2. Read RIGHT ESC telemetry
// 3. If both telemetries are complete, send message
void ESC_control_task(void* param) {
    ESC_control_t control;
    serial_buffer_t LEFT_serial_buffer;
    serial_buffer_t RIGHT_serial_buffer;
    ESC_telemetry_t telemetry;
    bool LEFT_telemetry_complete;
    bool RIGHT_telemetry_complete;
    HardwareSerial LEFT_ESC_serial(1);
    HardwareSerial RIGHT_ESC_serial(2);

    // Initialize structs and arrays
    LEFT_telemetry_complete = false;
    RIGHT_telemetry_complete = false;
    memset(&LEFT_serial_buffer,0,sizeof(serial_buffer_t));
    memset(&RIGHT_serial_buffer,0,sizeof(serial_buffer_t));
    memset(&telemetry,0,sizeof(ESC_telemetry_t));
    memset(&control,0,sizeof(ESC_control_t));

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

    while (1) {
        xQueueReceive(control_queue, &control, 0);
        
        if(LEFT_ESC_serial.available()) {
            // DEBUG_PRINTLN("LEFT AVAILABLE");
            if(read_telemetry(&LEFT_ESC_serial, &LEFT_serial_buffer, &(telemetry.left))) {
                telemetry.left.duty = control.left.duty;
                LEFT_telemetry_complete = true;
                // DEBUG_PRINTLN("LEFT TELEMETRY");
            }
        }

        if(RIGHT_ESC_serial.available()) {
            // DEBUG_PRINTLN("RIGHT AVAILABLE");
            if(read_telemetry(&RIGHT_ESC_serial, &RIGHT_serial_buffer, &(telemetry.right))) {
                telemetry.right.duty = control.right.duty;
                RIGHT_telemetry_complete = true;
                // DEBUG_PRINTLN("RIGHT TELEMETRY");
            }
        }

        if(LEFT_telemetry_complete && RIGHT_telemetry_complete) {
            LEFT_telemetry_complete = false;
            RIGHT_telemetry_complete = false;    
            // DEBUG_TELEMETRY(&Serial, &telemetry);
            xQueueSend(telemetry_queue, &telemetry, 0);
        }
        vTaskDelay(1); // Without this line watchdog resets the board
    }
    vTaskDelete(NULL);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    // Console output
    DEBUG_BEGIN(115200);

    // Init radio interface
    init_radio();

    // Init control msg queue
    control_queue = xQueueCreate(QUEUE_SIZE, sizeof(ESC_control_t));

    // Init telemetry msg queue
    telemetry_queue = xQueueCreate(QUEUE_SIZE, sizeof(ESC_telemetry_t));

    // Start ESC control task
    xTaskCreatePinnedToCore(ESC_control_task, "ESC_controller", 10000, NULL, 1, NULL, 0);
    // Start LoRa receiver task
    xTaskCreatePinnedToCore(radio_receive_task, "radio_receiver", 100000, NULL, 1, NULL, 1);
}

void loop() {
    delay(2147483647L); // Delay forever
}
