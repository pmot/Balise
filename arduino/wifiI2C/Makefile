ARDUINO_DIR = ${HOME}/Arduino
# Répertoire des librairies externes
USER_LIB_PATH = ${ARDUINO_DIR}/libraries
# Répertoire des librairies internes
PROJECT_LIB_PATH = ../libraries
# Liste des librairies internes, externes et système
ARDUINO_LIBS = Wire WiFi_Shield-master wifi_scan_ap 
OBJDIR = ${HOME}/BaliseBuild
# Mettre le bon TAG
# BOARD_TAG = pro.menu.cpu.attiny
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
