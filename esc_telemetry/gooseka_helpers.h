#ifndef GOOSEKA_HELPERS_H
#define GOOSEKA_HELPERS_H

#include <HardwareSerial.h>
#include "gooseka_structs.h"
#include "gooseka_defs.h"

uint8_t update_crc8(uint8_t crc, uint8_t crc_seed) {
    uint8_t crc_u, i;
    crc_u = crc;
    crc_u ^= crc_seed;
    for ( i=0; i<8; i++) crc_u = ( crc_u & 0x80 ) ? 0x7 ^ ( crc_u << 1 ) : ( crc_u << 1 );
    return (crc_u);
}

uint8_t get_crc8(uint8_t *Buf, uint8_t BufLen) {
    uint8_t crc = 0, i;
    for( i=0; i<BufLen; i++) crc = update_crc8(Buf[i], crc);
    return (crc);
}

void print_telemetry(HardwareSerial *serial, ESC_telemetry_t *telemetry) {
    serial->print("LEFT,");
    serial->print(telemetry->left.timestamp); 
    serial->print(","); 
    serial->print(telemetry->left.temperature / 100.0); 
    serial->print(",");
    serial->print(telemetry->left.voltage / 100.0); 
    serial->print(","); 
    serial->print(telemetry->left.current / 10.0); 
    serial->print(","); 
    serial->print(telemetry->left.power); 
    serial->print(","); 
    serial->println(telemetry->left.erpm * 100 / (MOTOR_POLES / 2)); 

    serial->print("RIGHT,");
    serial->print(telemetry->right.timestamp); 
    serial->print(","); 
    serial->print(telemetry->right.temperature / 100.0); 
    serial->print(",");
    serial->print(telemetry->right.voltage / 100.0); 
    serial->print(","); 
    serial->print(telemetry->right.current / 10.0); 
    serial->print(","); 
    serial->print(telemetry->right.power); 
    serial->print(","); 
    serial->println(telemetry->right.erpm * 100 / (MOTOR_POLES / 2)); 
}

#endif /* GOOSEKA_HELPERS_H */