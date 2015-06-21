ARDUINO_DIR = ${HOME}/Arduino
# Répertoire des librairies externes
USER_LIB_PATH = ${ARDUINO_DIR}/libraries
# Répertoire des librairies internes, relatif au projet core
PROJECT_LIB_PATH = ../libraries
# Liste non ordonnée des librairies internes, externes et système
ARDUINO_LIBS = Wire SoftwareSerial TinyGPS-13 LIS331/src WiFi_Shield-master wifi_scan_ap gps sample_lib
OBJDIR = ${HOME}/BaliseBuild
BOARD_TAG = pro.menu.cpu.16MHzatmega328
ARDUINO_PORT = /dev/ttyUSB*
AVRDUDE_OPTS = -q  # -DDEBUG_TO_CONSOLE
ISP_PROG = avrispmkII
ISP_PORT = usb
include ../../build/arduino/Arduino.mk
