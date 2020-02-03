#include <HardwareSerial.h>
#include <ESP32Servo.h>

#define MOTOR_POLES 14  

#define LEFT_PWM_PIN 25
#define LEFT_TELEMETRY_READ_PIN 35
#define LEFT_TELEMETRY_UNUSED_PIN 17

#define RIGHT_PWM_PIN 23
#define RIGHT_TELEMETRY_READ_PIN 34
#define RIGHT_TELEMETRY_UNUSED_PIN 13

#define PWM_MIN 1040
#define PWM_MAX 1960

typedef struct __attribute__((packed)){
    int16_t temperature;
    int16_t voltage;
    int16_t current;
    int16_t power;
    int16_t erpm;
} ESC_telemetry_t;

ESC_telemetry_t LEFT_ESC_telemetry[5]; // Temperature, Voltage, Current, used mAh, eRpM
ESC_telemetry_t RIGHT_ESC_telemetry[5]; // Temperature, Voltage, Current, used mAh, eRpM
uint8_t temperature = 0;
float voltage = 0;
uint32_t current = 0;
uint32_t erpm = 0;
uint32_t rpm = 0;
uint32_t kv = 0;
HardwareSerial LEFT_ESC_serial(1);
HardwareSerial RIGHT_ESC_serial(2);
uint8_t LEFT_received_bytes = 0;
static uint8_t LEFT_serial_buffer[10];
uint8_t RIGHT_received_bytes = 0;
static uint8_t RIGHT_serial_buffer[10];
uint8_t dutyCycle = 0;
Servo LEFT_ESC_servo;
Servo RIGHT_ESC_servo;

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

    Serial.println("Timestamp, Temperature, Voltage, Current, Power, RPM");
}

void read_telemetry(HardwareSerial* serial, uint8_t* serialBuf, uint8_t* receivedBytes, ESC_telemetry_t* telemetry) {
    SerialBuf[*receivedBytes] = (*serial).read();
    (*receivedBytes)++;
    if((*receivedBytes) > 9){ // transmission TELEMETRY_
        (*receivedBytes) = 0;
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
        ESC_telemetry[0] = SerialBuf[0]; // temperatureTELEMETRY_
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

void loop() {
    char command = '0';
    if(Serial.available()) {
        command = Serial.read();
        if(command == '+') {
            dutyCycle = min(dutyCycle + 32, 255);
        } else if (command == '-') {
            dutyCycle = max(dutyCycle - 32, 0);
        }
    }
    
    LEFT_ESC_servo.write(map(dutyCycle,0,255,0,180));
    RIGHT_ESC_servo.write(map(dutyCycle,0,255,0,180));
    
    if(LEFT_ESC_serial.available()){
       
    }


    if(RIGHT_ESC_serial.available()){
        
    }
}
