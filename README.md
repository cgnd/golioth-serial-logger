# golioth-serial-logger

This firmware enables logging serial data to the Golioth cloud.

> [!NOTE]
> This is currently a very rough prototype and should not be used in production.

## Hardware Setup

### [ESP32-C3-DevKitM-1](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)

1. Connect the target device's UART pins to the ESP32-C3-DevKitM-1 pins as follows:

   | Target device UART | ESP32-C3-DevKitM-1 UART |
   | ------------------ | ----------------------- |
   | `GND`              | `GND`                   |
   | `RXD`              | `GPIO7`                 |
   | `TXD`              | `GPIO6`                 |

2. Connect the ESP32-C3-DevKitM-1 USB port to your PC.

## Firmware Setup

### Set up a build environment

```sh
mkdir golioth-serial-logger-workspace
cd golioth-serial-logger-workspace
python -m venv golioth-serial-logger-workspace/.venv
source golioth-serial-logger-workspace/.venv/bin/activate
pip install -U pip wheel west
west init -m git@github.com:cgnd/golioth-serial-logger .
cd golioth-serial-logger/
west update
west blobs fetch hal_espressif
west zephyr-export
pip install -r ../zephyr/scripts/requirements.txt
```

### Build the firmware

```sh
west build -p -b esp32c3_devkitm --sysbuild app
```

### Flash the firmware
```sh
west flash --esp-device=/dev/<your_serial_port>
```

## Connect the logger to Golioth

Open a serial terminal connection to the USB-serial port on the ESP32-C3-DevKitM-1 (115200 8-N-1).

In the Zephyr console, configure the logger device settings as follows:

```
settings set wifi/ssid "<YOUR WIFI SSID>"
settings set wifi/psk "<YOUR WIFI PASSWORD>"
settings set golioth/psk-id <GOLIOTH-PSK-ID>
settings set golioth/psk <GOLIOTH-PSK>
```

Reboot the logger device:

```
kernel reboot cold
```

Wait for the logger device to connect to Golioth:

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.176,000] <inf> fs_nvs: 6 Sectors of 4096 bytes
[00:00:00.176,000] <inf> fs_nvs: alloc wra: 0, f88
[00:00:00.176,000] <inf> fs_nvs: data wra: 0, 94
[00:00:00.178,000] <inf> main: Starting Golioth Serial Logger App
[00:00:00.178,000] <inf> golioth_samples: Bringing up network interface
[00:00:00.178,000] <inf> golioth_samples: Waiting to obtain IP address
[00:00:00.178,000] <inf> golioth_wifi: Connecting to 'Wilson'
[00:00:05.134,000] <inf> net_dhcpv4: Received: 192.168.1.165
[00:00:05.135,000] <inf> golioth_mbox: Mbox created, bufsize: 1320, num_items: 10, item_size: 120
[00:00:05.584,000] <inf> golioth_coap_client_zephyr: Golioth CoAP client connected
[00:00:05.584,000] <inf> main: Golioth client connected
[00:00:05.584,000] <inf> golioth_coap_client_zephyr: Entering CoAP I/O loop
```

At this point, the logging device is ready for the target to start sending UART data.

## Usage

Each line of UART data sent by the target device will be automatically sent to the Golioth [logging service](https://docs.golioth.io/device-management/logging/) when a `\n` or `\r` is received from the target device.
