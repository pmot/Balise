#ifndef __GPS_H__
#define __GPS_H__

#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

struct gpsData {
	
};


// Lit les données sur le port série, s'arrête au bout de n millisecondes
// Les données sont passées dans l'objet TinyGPS.
void gpsRead(TinyGPS, SoftwareSerial, unsigned long);

// Met les données GPS en forme (chaînes de caractères) et les passe
// dans la structure gpsData.
// retourne un entier :
// 1 si tout va bien (les données sont valides)
// 0 si une des données est invalide, ou s'il n'y a pas le fix
int getGpsData(struct gpsData*);

#endif
