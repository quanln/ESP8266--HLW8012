# ESP-pow

First, i'm sorrry because my english skill is bad. This repository help to monitor voltage, current and active power using ESP8285 and chip HLW8012 with ``Arduino IDE`` and ``Blynk app``

- Use ESP8285 and chip HLW8012 link (https://iotmaker.vn/esp-pow-thiet-bi-do-dien.html)
- monitor on Blynk app 
- Arduino IDE to write source code .

**STEPS**

1. Add library, before flash program, you must be connect GPIO 0 to GND. (can use module USB to UART with chip CP2102)

2. flash with ``esp-pow.ino``. After flash done, access Access Point ``TUANPM_id-chip`` to enter ``wifi manager``, connect to SSID of your router.

3. Connect Input of module ESP-pow with AC-voltage, in output, we will connect with devices (light, fan, motor, ...)

4. we can see the value of voltage , current via blynk app. 



 
