#ifndef __GPS_H__
#define __GPS_H__

#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

// Structure dans laquelle stocker les données GPS (chaînes)
struct gpsData
{
	char latitude[10];	// En degrés, 6 décimales	: 12.456789
	char longitude[11];	// En degrés, 6 décimales	: 123.567891
	char altitude[8];	// En m, 2 décimales		: 1234.67
	char speed[7];		// En km/h					: 123.56
	char satellites[4];	// 99 satellites max :		: +23
	char hdop[5];		//							: +234
	char fixAge[7];		//							: +23456
	char date[11];		// 							: 12/45/7891
	char time[9];		//							: 12:45:78
	char dateAge[7];	//							: +23456
};

// Initialisation
int gpsSetup(struct gpsData*);

// Lit les données sur le port série, s'arrête au bout de n millisecondes
// Les données sont passées dans l'objet TinyGPS.
void gpsRead(TinyGPS*, SoftwareSerial, unsigned long);

// Met les données GPS en forme (chaînes de caractères) et les passe
// dans la structure gpsData.
// Les données sont mises à jour uniquement si elles sont toutes valides.
// retourne un booléen :
// true si tout va bien (les données sont valides)
// false si une des données est invalide, ou s'il n'y a pas le fix
bool gpsSetData(TinyGPS, struct gpsData*);


#endif
