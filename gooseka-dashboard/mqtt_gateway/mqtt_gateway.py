import json
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

INFLUXDB_ADDRESS = 'influxdb'
INFLUXDB_USER = ''
INFLUXDB_PASSWORD = ''
INFLUXDB_DATABASE = 'gooseka'
INFLUXDB_PORT = 8086

MQTT_ADDRESS = 'mosquitto'
MQTT_PORT = 1883
MQTT_USER = ''
MQTT_PASSWORD = ''
MQTT_TOPIC = '/gooseka/#'

influxdb_client = InfluxDBClient(host=INFLUXDB_ADDRESS, port=INFLUXDB_PORT, retries=20)

def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)

def _send_sensor_data_to_influxdb(sensor_data):
    try:
        json_body = [
            {
                'time': sensor_data['left']['timestamp'],
                'measurement': 'left',
                'fields': {
                    'temperature': sensor_data['left']['temperature'],
                    'voltage': sensor_data['left']['voltage'],
                    'current': sensor_data['left']['current'],
                    'power': sensor_data['left']['power'],
                    'erpm': sensor_data['left']['erpm'],
                    'duty': sensor_data['left']['duty']
                }
            },
            {
                'time': sensor_data['right']['timestamp'],
                'measurement': 'right',
                'fields': {
                    'temperature': sensor_data['right']['temperature'],
                    'voltage': sensor_data['right']['voltage'],
                    'current': sensor_data['right']['current'],
                    'power': sensor_data['right']['power'],
                    'erpm': sensor_data['right']['erpm'],
                    'duty': sensor_data['right']['duty']
                }
            },
        ]
    except ValueError:
        return
    # print(json.dumps(json_body,indent=4))
    influxdb_client.write_points(points=json_body, database=INFLUXDB_DATABASE, protocol='json', time_precision='ms')

def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    # print(msg.topic + '\t' + str(msg.payload.decode('utf-8')))
    try:
        sensor_data = json.loads(msg.payload.decode('utf-8'))
    except ValueError:
        return
    _send_sensor_data_to_influxdb(sensor_data)

if __name__ == '__main__':
    # print('MQTT to InfluxDB bridge')
    influxdb_client.create_database(INFLUXDB_DATABASE)

    mqtt_client = mqtt.Client()
    # mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, MQTT_PORT)
    mqtt_client.loop_forever()