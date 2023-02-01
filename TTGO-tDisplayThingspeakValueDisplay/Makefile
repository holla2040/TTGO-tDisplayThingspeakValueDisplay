IP=192.168.0.183



bin:
	arduino-cli compile --build-path /tmp/arduino-build -b esp32:esp32:esp32doit-devkit-v1 .

flash:
	make ota

flashSerial:
	- pkill -9 -f microcom
	arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32doit-devkit-v1 /tmp/arduino-build --input-dir /tmp/arduino-build

ota:
	python3 "/home/holla/.arduino15/packages/esp32/hardware/esp32/2.0.6/tools/espota.py" -r -i $(IP) -p 3232 --auth=admin -f "/tmp/arduino-build/TTGO-tDisplayThingspeakValueDisplay.ino.bin"

