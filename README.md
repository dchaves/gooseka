![alt text](https://66.media.tumblr.com/a73649e201017207355dc937401faa7b/tumblr_niqa2eefpw1tu022ro6_500.png "Gooseka")

# Gooseka, el del puño cerrado
Prototipo de coche solar de bajo coste.

## Lista de materiales
### Coche
* 2x Motores brushless - [Racerstar BR2830 850KV 2-4S](https://www.banggood.com/es/Racerstar-BR2830-850KV-2-4S-Brushless-Motor-For-RC-Airplane-p-1128160.html?rmmds=detail-left-hotproducts__5&cur_warehouse=CN)
* 2x ESC BLHeli32 - [Razor32 V2 35A BLheli_32 3-6S](https://www.banggood.com/es/Razor32-V2-35A-BLheli_32-3-6S-DShot1200-ESC-w-RGB-LED-Current-Sensor-Bidirectional-for-RC-Drone-p-1398234.html?rmmds=search&cur_warehouse=CN)
* 1x Controlador panel solar - [MPPT 5A](https://www.banggood.com/es/MPPT-5A-Solar-Panel-Regulator-Controller-Battery-Charging-9V-12V-24V-Automatic-Switch-p-1307801.html?rmmds=search&cur_warehouse=CN) 
* 2x Ruedas - [Neumáticos de goma para HSP HPI Tamiya 1/10](https://www.banggood.com/es/4PCS-12mm-Hub-Wheel-Rims-Rubber-Tires-for-HSP-HPI-Tamiya-1-10-On-road-Drift-Rc-Car-Parts-p-1378475.html?rmmds=search&cur_warehouse=CN)
* 2x Rodamientos - [Rodamiento de bola](https://banggood.com/es/58-Inch-Transfer-Bearing-Unit-Conveyor-Roller-Wheel-Mounted-Ball-Bearing-p-1011925.html?akmClientCountry=ES&rmmds=detail-bottom-alsolike)

### OBU (On Board Unit)
* 1x CPU con radio LoRa - [LILYGO® TTGO ESP32 SX1276 LoRa 868MHz](https://www.banggood.com/es/LILYGO-TTGO-2Pcs-ESP32-SX1276-LoRa-868MHz-bluetooth-WI-FI-Lora-Internet-Antenna-Development-Board-For-Arduino-p-1295045.html?rmmds=search&cur_warehouse=CN)

### RSU (Road Side Unit)
* 1x CPU con radio LoRa - [LILYGO® TTGO ESP32 SX1276 LoRa 868MHz](https://www.banggood.com/es/LILYGO-TTGO-2Pcs-ESP32-SX1276-LoRa-868MHz-bluetooth-WI-FI-Lora-Internet-Antenna-Development-Board-For-Arduino-p-1295045.html?rmmds=search&cur_warehouse=CN)

### Otros componentes
* 1x PC con SO Linux (Debian o similar) y Docker v19.03.6
* 1x Mando USB con controles analógicos (p.ej. PS3 SixAxis)

## Estructura del código
El código se organiza en tres submódulos, organizados de la siguiente forma:

| Repositorio | Estado | Descripción |
|-------------|:------:|-------------|
| [Controller](https://github.com/dchaves/gooseka-controller) | ![Python](https://github.com/dchaves/gooseka-controller/workflows/Python/badge.svg?branch=master) | Controlador del coche solar. Se encarga de traducir el input de usuario (mando) a órdenes para el coche, que envía por puerto serie (USB). |
| [RSU](https://github.com/dchaves/gooseka-rsu) | ![Build](https://github.com/dchaves/gooseka-rsu/workflows/Build/badge.svg?branch=master) | Gateway radio. Recibe mensajes via puerto serie (USB) y los envía via radio. |
| [OBU](https://github.com/dchaves/gooseka-obu) | ![Build](https://github.com/dchaves/gooseka-obu/workflows/Build/badge.svg?branch=master) | Unidad de control empotrada para el coche solar. |