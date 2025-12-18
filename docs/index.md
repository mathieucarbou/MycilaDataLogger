# MycilaDataLogger

ESP32 firmware to connect an ESP32 to an existing ESP32 to view the Serial logs including crashes and stack traces remotely through a web interface

[![](https://mathieu.carbou.me/MycilaDataLogger/screenshot.png)](https://mathieu.carbou.me/MycilaDataLogger/screenshot.png)

[![Latest Release](https://img.shields.io/github/release/mathieucarbou/MycilaDataLogger.svg)](https://GitHub.com/mathieucarbou/MycilaDataLogger/releases/)
[![GPLv3 license](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.txt)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)
[![Build](https://github.com/mathieucarbou/MycilaDataLogger/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/MycilaDataLogger/actions/workflows/ci.yml)
[![GitHub latest commit](https://badgen.net/github/last-commit/mathieucarbou/MycilaDataLogger)](https://GitHub.com/mathieucarbou/MycilaDataLogger/commit/)

Table of contents

- 📥 [Downloads](#downloads)
- ⚙️ [Installation](#installation)
- 🔌 [Wiring](#wiring)
- 👀 [Usage](#usage)

## Downloads

Releases (firmware binaries) are available in the GitHub releases page:

[https://github.com/mathieucarbou/MycilaDataLogger/releases](https://github.com/mathieucarbou/MycilaDataLogger/releases)

Naming convention:

- `MycilaDataLogger-BOARD.FACTORY.bin` — first-time / factory flash image
- `MycilaDataLogger-BOARD.OTA.bin` — OTA update image

FYI, Supported ESP32 boards with Ethernet support:

- [OLIMEX ESP32-PoE](https://docs.platformio.org/en/stable/boards/espressif32/esp32-poe.html)
- [OLIMEX ESP32-GATEWAY](https://docs.platformio.org/en/stable/boards/espressif32/esp32-gateway.html)
- [Wireless-Tag WT32-ETH01 Ethernet Module](https://docs.platformio.org/en/stable/boards/espressif32/wt32-eth01.html)
- [T-ETH-Lite ESP32 S3](https://github.com/Xinyuan-LilyGO/LilyGO-T-ETH-Series/)
- [Waveshare ESP32-S3 ETH Board](https://www.waveshare.com/wiki/ESP32-S3-ETH)

## Installation

First-time flash (factory image):

```bash
esptool.py erase_flash
esptool.py write_flash 0x0 MycilaDataLogger-BOARD.FACTORY.bin
```

## Wiring

| Board                 | RX (connected to TX0 on remote ESP32) | TX (connected to RX0 on remote ESP32) | WiFi | Ethernet |
| :-------------------- | :-----------------------------------: | :-----------------------------------: | :--: | :------: |
| denky_d4              |                  22                   |                  21                   |  ✅  |    ❌    |
| esp32-c3-devkitc-02   |                  18                   |                  19                   |  ✅  |    ❌    |
| esp32-c6-devkitc-1    |                   4                   |                   5                   |  ✅  |    ❌    |
| esp32-gateway         |                  16                   |                  17                   |  ✅  |    ✅    |
| esp32-poe             |                  35                   |                  33                   |  ✅  |    ✅    |
| esp32-s2-saola-1      |                  18                   |                  17                   |  ✅  |    ❌    |
| esp32-s3-devkitc-1    |                  16                   |                  17                   |  ✅  |    ❌    |
| esp32dev              |                  16                   |                  17                   |  ✅  |    ❌    |
| esp32s3box            |                  19                   |                  20                   |  ✅  |    ❌    |
| lilygo_eth_lite_s3    |                  17                   |                  18                   |  ✅  |    ✅    |
| nodemcu-32s           |                  16                   |                  17                   |  ✅  |    ❌    |
| tinypico              |                   4                   |                  25                   |  ✅  |    ❌    |
| waveshare_esp32s3_eth |                  19                   |                  20                   |  ✅  |    ✅    |
| wemos_d1_mini32       |                  16                   |                  17                   |  ✅  |    ❌    |
| wipy3                 |                   4                   |                  25                   |  ✅  |    ❌    |
| wt32-eth01            |                   5                   |                  17                   |  ✅  |    ✅    |

- Connect GND to GND of the remote ESP32
- Connect 5V input pint to the remote ESP32 5V or VIN pin
- Connect RX/TX as per table above

[![](https://mathieu.carbou.me/MycilaDataLogger/schema.jpeg)](https://app.cirkitdesigner.com/project/6c610b27-b877-42f8-ba96-a26dd2e3c492?view=interactive_preview)

**Notes**:

- For now, you can only read the remote ESP32 TX0 logs, not send data to it.
  So the TX pin connection (going to RX0 on remote ESP32) is optional.

## Usage

On first boot the device creates an access point to configure WiFi or stay in AP mode.

Then it will keep your preferences and connect to your WiFi network automatically on next boots (or stay in AP mode as chosen).

The project includes mDNS support so you should be able to access it at `http://DataLogger.local/` in WiFi or ETH mode.

On macOS, teh Discovery app can help you find mDNS devices on your network:
![](https://mathieu.carbou.me/MycilaDataLogger/mdns.png)

### Links

- **Serial Console**: `http://<device-ip>/`

- **Factory reset**: `http://<device-ip>/reset`

- **Restart**: `http://<device-ip>/restart`

- **Firmware update (OTA)**: `http://<device-ip>/update`

### Terminal Streaming

You can also directly stream the ESP32 logs to a terminal by using `websocat`:

```bash
websocat ws://<device-ip>/webserialws
```

[![](https://mathieu.carbou.me/MycilaDataLogger/websocat.png)](https://mathieu.carbou.me/MycilaDataLogger/websocat.png)

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
