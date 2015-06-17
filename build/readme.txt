Build
=====

Contient les outils pour construire et téléverser le code, se placer dans ce répertoire, puis : 
Chaque plateforme dispose de son fichier de configuration.

	Exemple : config.ARDUINO

Linux
-----
Requis :
	- arduino-mk
	- make
	- avrdude
	- python-serial

Commande :
	./build.sh

Arduino
=======

	Fichier de configuration : config.ARDUINO

	PLATFORM_DIR="../arduino/core"

	PLATFORM_DIR Contient le Makefile pour construire le projet.
