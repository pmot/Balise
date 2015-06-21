#ifndef __CORE_PARAMETERS_H__
#define __CORE_PARAMETERS_H__

#define WIFI_TX  4
#define WIFI_RX  5
#define GPS_TX   6
#define GPS_RX   7

/*
 * Accéléromètre
 * 
  CTRL_REG1 :
    PM2/PM1/PM0 : 110  (10Hz)
    DR : 00
    Zen/Yen/Xen : 010  (Zen disabled, Y enabled, X disabled)
*/
#define CTRL_REG1_ADDR  0x20
#define CTRL_REG2_VALUE 0xC2

/*
  CTRL_REG2 :
    TODO
*/
//#define CTRL_REG2_ADDR  0x21
//#define CTRL_REG2_VALUE 0x00

void movment();

#endif
