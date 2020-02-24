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


class MySerialComm(object):

    def send_packet(self, duty_left, duty_right):
        """ Send the packet with motor duties """

        message_to_send = struct.pack('<BBBBBB', SOF_1, SOF_2, duty_left, 0, duty_right, 0)
        self.serial_port.write(message_to_send)

    def receive_telemmetry(self):
        """ Receive telemetry """

        telemetry = {}
        
        while (self.serial_port.in_waiting > 0):
            received_byte = struct.unpack('B',serial_port.read())[0]
            # print(received_byte)
            if (self.state == STATE_SOF_1):
                # print("SOF_1")
                if (received_byte == SOF_1):
                    self.state = STATE_SOF_2
                    continue
            elif (self.state == STATE_SOF_2):
                # print("SOF_2")
                if (received_byte == SOF_2):
                    buffer_index = 0
                    buffer = bytearray(TELEMETRY_SIZE_BYTES)
                    self.state = STATE_FRAME
                    continue
                else:
                    self.state = STATE_SOF_1
                    continue
            elif (self.state == STATE_FRAME):
                # print("FRAME")
                if (buffer_index < TELEMETRY_SIZE_BYTES - 1):
                    buffer[buffer_index] = received_byte
                    buffer_index += 1
                    continue
                else:
                    buffer[buffer_index] = received_byte
                    self.state = STATE_SOF_1
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
                    continue

        return telemetry
                
    def __init__(self, serial_port, serial_rate, radio_idle_timeout):
        """ Initialization """

        self.state = STATE_SOF_1
        self.serial_port = serial.Serial(serial_port, serial_rate)
        self.radio_idle_timeout = radio_idle_timeout
        

