ARDUINO_DIR = ${HOME}/Arduino
# Répertoire des librairies externes
USER_LIB_PATH = ${ARDUINO_DIR}/libraries
# Répertoire des librairies internes, relatif au projet core
PROJECT_LIB_PATH = ../libraries
# Liste des librairies internes, externes et système
ARDUINO_LIBS = Wire SoftwareSerial TinyGPS-13 LIS331/src WiFi_Shield-master \
			   wifi_scan_ap gps sample_lib accelerometer GSMM95
OBJDIR = ${HOME}/BaliseBuild
BOARD_TAG = pro.menu.cpu.16MHzatmega328
# Optimisé pour la taille du code par défaut
# level 2 ou 3 ne font rien gagner en occupation de la SRAM
# OPTIMIZATION_LEVEL=s		
ARDUINO_PORT = /dev/ttyUSB*
AVRDUDE = ${AVR_TOOLS_DIR}/bin/avrdude
AVRDUDE_CONF = ${AVR_TOOLS_DIR}/etc/avrdude.conf
AVRDUDE_ARD_PROGRAMMER = arduino
AVRGCC_OPT = -O2
AVRDUDE_OPTS = -q
ISP_PROG = avrispmkII
ISP_PORT = usb
include ../../build/arduino/Arduino.mk
