
Ce répertoire contient de la documentation décrivant l'implémentation sur MCU Arduino Pro.

Communication avec le MCU Arduino Pro
=====================================
Interfaces
----------
- 1 UART
- 1 I2C
- 1 SPI
- n SoftwareSerial via les pin numériques (heu digital)

Communication avec le module GSM/GPRS
=====================================
TODO
----
Interface à identifier : UART ?

Communication avec les modules GPS et WiFi
==========================================
La communication avec les modules GPS et WiFI est réalisées avec SoftwareSerial.
SoftwareSerial ne peut lire que sur un seul port à la fois.
Les données envoyées sur un port SoftwareSerial qui n'est pas en écoute sont perdues.

Il faut définir la priorité de lecture.
Le temps d'accroche GPS est d'environ 1mn.
Pendant la période d'accroche du module GPS, il est donc possible d'affecter l'écoute au module WiFi.

TODO
----
Identifier d'autres possibilités de communication : I2C, SPI pour ces modules.

WARNING
-------
SoftwareSerial utilise des vecteurs d'interruption (à identifier).
Cette librairie peut donc entraîner des effets de bords avec d'autres librairies (à identifier).
Attention à l'utilisation de ces vecteurs.

TIPS : (à définir).

Module Accéléromètre
====================
Chip LIS331H
Datasheet : (à définir)
Application note : (à définir)



Module WiFi
===========











TODO :

Créer la documentation qui décrit la collecte des données.
Ex:
Polling GPS via softwareserial toutes les 5s.
Si on perd le fix, on poll 40s plus tard (temps d'accroche environ).
En attendant on poll le wifi.
