from inputs import get_gamepad
from inputs import devices
import struct
import serial
import time
import json 

STATE_SOF_1 = 0x00
STATE_SOF_2 = 0x01
STATE_FRAME = 0x02

SOF_1 = 0xDE
SOF_2 = 0xAD

TELEMETRY_SIZE_BYTES = 30
RADIO_IDLE_TIMEOUT = 5000

serial_port = serial.Serial('/dev/ttyUSB0', 115200)

def millis():
    return int(round(time.time() * 1000))

def main():
    state = STATE_SOF_1
    ready_to_send = True
    duty_left = 0
    duty_right = 0
    last_sent_millis = 0

    while 1:
        events = get_gamepad()
        for event in events:
            # print(event.code, event.state)
            if (event.code == "ABS_Z"):
                duty_left = event.state
            if (event.code == "ABS_RZ"):
                duty_right = event.state
            # Send serial port message
        if(millis() - last_sent_millis > RADIO_IDLE_TIMEOUT/2):
            ready_to_send = True
        if (ready_to_send):
            message_to_send = struct.pack('<BBBBBB', SOF_1, SOF_2, duty_left, 0, duty_right, 0)
            serial_port.write(message_to_send)
            # print("Sending message: " + str(message_to_send))
            last_sent_millis = millis()
            ready_to_send = True
        while (serial_port.in_waiting > 0):
            received_byte = struct.unpack('B',serial_port.read())[0]
            # print(received_byte)
            if (state == STATE_SOF_1):
                # print("SOF_1")
                if (received_byte == SOF_1):
                    state = STATE_SOF_2
                    continue
            elif (state == STATE_SOF_2):
                # print("SOF_2")
                if (received_byte == SOF_2):
                    buffer_index = 0
                    buffer = bytearray(TELEMETRY_SIZE_BYTES)
                    state = STATE_FRAME
                    continue
                else:
                    state = STATE_SOF_1
                    continue
            elif (state == STATE_FRAME):
                # print("FRAME")
                if (buffer_index < TELEMETRY_SIZE_BYTES - 1):
                    buffer[buffer_index] = received_byte
                    buffer_index += 1
                    continue
                else:
                    buffer[buffer_index] = received_byte
                    state = STATE_SOF_1
                    # print ('SIZE ' + str(struct.calcsize('!LHHHHHBLHHHHHB')))
                    received_list = struct.unpack('<LHHHHHBLHHHHHB',buffer)
                    telemetry = {
                        "left": {
                            "timestamp": received_list[0],
                            "temperature": received_list[1],
                            "voltage": received_list[2],
                            "current": received_list[3],
                            "power": received_list[4],
                            "erpm": received_list[5],
                            "duty": received_list[6]
                        },
                        "right": {
                            "timestamp": received_list[7],
                            "temperature": received_list[8],
                            "voltage": received_list[9],
                            "current": received_list[10],
                            "power": received_list[11],
                            "erpm": received_list[12],
                            "duty": received_list[13]
                        }
                    }
                    # Send data to mqtt
                    print("Received: " + json.dumps(telemetry, indent = 4))
                    ready_to_send = True
                    continue

if __name__ == "__main__":
    main()