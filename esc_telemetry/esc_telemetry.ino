#include <HardwareSerial.h>
#include <ESP32Servo.h>

#define MOTOR_POLES 14  
#define LEFT_PWM_CHANNEL 0
#define LEFT_PWM_PIN 25
#define PWM_FREQ 24000
#define PWM_RESOLUTION 10
#define TELEMETRY_READ_PIN 35
#define TELEMETRY_UNUSED_PIN 17
#define PWM_MIN 1040
#define PWM_MAX 1960

int16_t ESC_telemetry[5]; // Temperature, Voltage, Current, used mAh, eRpM
uint8_t temperature = 0;
float voltage = 0;
uint32_t current = 0;
uint32_t erpm = 0;
uint32_t rpm = 0;
uint32_t kv = 0;
HardwareSerial ESC_left(1);
uint8_t receivedBytes = 0;
static uint8_t SerialBuf[10];
uint8_t dutyCycle = 0;
Servo ESC_left_PWM;

uint8_t update_crc8(uint8_t crc, uint8_t crc_seed){
  uint8_t crc_u, i;
  crc_u = crc;
  crc_u ^= crc_seed;
  for ( i=0; i<8; i++) crc_u = ( crc_u & 0x80 ) ? 0x7 ^ ( crc_u << 1 ) : ( crc_u << 1 );
  return (crc_u);
}

uint8_t get_crc8(uint8_t *Buf, uint8_t BufLen){
  uint8_t crc = 0, i;
  for( i=0; i<BufLen; i++) crc = update_crc8(Buf[i], crc);
  return (crc);
}

void setup() {
    Serial.begin(115200);
    ESC_left.begin(115200, SERIAL_8N1, TELEMETRY_READ_PIN, TELEMETRY_UNUSED_PIN);

    // Empty Rx Serial of garbage telemetry
    while(ESC_left.available()) {
        ESC_left.read();
    }

    // if ((rmt_send = rmtInit(LEFT_PWM_PIN, true, RMT_MEM_64)) == NULL) {
    //     Serial.println("init sender failed\r\n");
    // }

    // float realTick = rmtSetTick(rmt_send, 12.5); // 12.5ns sample rate
    // Serial.printf("rmt_send tick set to: %fns\r\n", realTick);

    // xTaskCreatePinnedToCore(dshot_task, "DSHOT", 10000, NULL, 1, NULL, 0);

    // configure LED PWM functionalitites
    // ledcSetup(LEFT_PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    
    // attach the channel to the GPIO to be controlled
    // ledcAttachPin(LEFT_PWM_PIN, LEFT_PWM_CHANNEL);

    ESC_left_PWM.attach(LEFT_PWM_PIN, PWM_MIN, PWM_MAX);

    Serial.println("Timestamp, Temperature, Voltage, Current, Power, RPM");
}

// void dshot_task(void* params) {
//     while(1){
//         dshot_output(dshot50);
//         vTaskDelay(1); // Without this line watchdog resets the board
//     }
//     vTaskDelete(NULL);
// }

// void dshot_output(uint16_t value) {
//     uint16_t packet;
//     packet = (value << 1) | 1; // Telemetry ON
//     int csum = 0;
//     int csum_data = packet;
//     for (int i = 0; i < 3; i++) {
//         csum ^=  csum_data;
//         csum_data >>= 4;
//     }
//     csum &= 0xf;
//     packet = (packet << 4) | csum;

//     // durations are for dshot1200
//     // https://www.speedgoat.com/products/dshot
//     for (int i = 0; i < 16; i++) {
//         if (packet & 0x8000) {
//               dshotPacket[i].level0 = 1;
//               dshotPacket[i].duration0 = 50; // T1H = 50 * 12.5 = 625ns
//               dshotPacket[i].level1 = 0;
//               dshotPacket[i].duration1 = 17; // T1L = 17 * 12.5 = 212.5 ~ 208ns = 833 - 625
//           } else {
//               dshotPacket[i].level0 = 1;
//               dshotPacket[i].duration0 = 25; // T0H = 25 * 12.5 = 312.5 ~ 313ns
//               dshotPacket[i].level1 = 0;
//               dshotPacket[i].duration1 = 42; // T0L 42 * 12.5 = 525 ~ 520ns
//           }
//         packet <<= 1;
//     }
    
//     rmtWrite(rmt_send, dshotPacket, 16);
// }

void loop() {
    char command = '0';
    if(Serial.available()) {
        command = Serial.read();
        if(command == '+') {
            dutyCycle = min(dutyCycle + 32, 255);
        } else if (command == '-') {
            dutyCycle = max(dutyCycle - 32, 0);
        }
        // Serial.println(dutyCycle);
        // Serial.println(map(dutyCycle,0,255,0,180));
    }
    // ledcWrite(LEFT_PWM_CHANNEL, map(dutyCycle,0,255,50,100));
    ESC_left_PWM.write(map(dutyCycle,0,255,0,180));
    
    if(ESC_left.available()){
        SerialBuf[receivedBytes] = ESC_left.read();
        receivedBytes++;
        if(receivedBytes > 9){ // transmission complete
            receivedBytes = 0;
            uint8_t crc8 = get_crc8(SerialBuf, 9); // get the 8 bit CRC
          
            if(crc8 != SerialBuf[9]) { // CRC ERROR                
                // Empty Rx Serial of garbage telemtry
                while(ESC_left.available()) {
                    ESC_left.read();
                }
                Serial.print(millis()); 
                Serial.print(","); 
                Serial.println("NaN, NaN, NaN, NaN, NaN");
                return; // transmission failure 
            }

            // compute the received values
            ESC_telemetry[0] = SerialBuf[0]; // temperature
            ESC_telemetry[1] = (SerialBuf[1]<<8)|SerialBuf[2]; // voltage
            ESC_telemetry[2] = (SerialBuf[3]<<8)|SerialBuf[4]; // Current
            ESC_telemetry[3] = (SerialBuf[5]<<8)|SerialBuf[6]; // used mA/h
            ESC_telemetry[4] = (SerialBuf[7]<<8)|SerialBuf[8]; // eRpM *100

            Serial.print(millis()); 
            Serial.print(","); 
          //      Serial.print("Temperature (deg.): ");
                Serial.print(ESC_telemetry[0] / 100.0); 
                Serial.print(",");   
          //      Serial.print("Voltage (V): ");
                Serial.print(ESC_telemetry[1] / 100.0); 
                Serial.print(",");   
          //      Serial.print("Current (A): ");
                Serial.print(ESC_telemetry[2] / 10.0); 
                Serial.print(","); 
          //      Serial.print("Power (mAh): ");
                Serial.print(ESC_telemetry[3]); 
                Serial.print(","); 
          //      Serial.print("RPM : ");
                Serial.println(ESC_telemetry[4] * 100 / (MOTOR_POLES / 2)); 
        }
    }
}
