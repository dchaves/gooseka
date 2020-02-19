#ifndef GOOSEKA_STRUCTS_H
#define GOOSEKA_STRUCTS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t timestamp;
    uint16_t temperature;
    uint16_t voltage;
    uint16_t current;
    uint16_t power;
    uint16_t erpm;
    uint8_t duty;
} ESC_oneside_telemetry_t;

typedef struct __attribute__((packed)) {
    ESC_oneside_telemetry_t left;
    ESC_oneside_telemetry_t right;
} ESC_telemetry_t;

typedef struct __attribute__((packed)) {
    uint8_t turbo: 1;
    uint8_t red_led: 1;
    uint8_t green_led: 1;
    uint8_t blue_led: 1;
    uint8_t reserved: 4;
} ESC_control_flags_t;

typedef struct __attribute__((packed)) {
    uint8_t duty;
    ESC_control_flags_t flags;
} ESC_oneside_control_t;

typedef struct __attribute__((packed)) {
    ESC_oneside_control_t left;
    ESC_oneside_control_t right;
} ESC_control_t;

#endif /* GOOSEKA_STRUCTS_H */