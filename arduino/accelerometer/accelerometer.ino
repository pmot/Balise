/* Copyright 2011 David Irvine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <LIS331.h>
#include <Wire.h>
LIS331 lis;

const int ledPin =  13;
volatile bool on = true;


void setup() {

  Wire.begin();
  // lis.i2cAddress = 25;

  pinMode(ledPin, OUTPUT);
  lis.setPowerStatus(LR_POWER_NORM);
  lis.setYEnable(true);

  // Registres
  // lis.writeReg(0x22, 0x00);
  // lis.writeReg(0x30, 0x2A);
  lis.writeReg(0x30, 0x08);
  lis.writeReg(0x32, 0x01);
  // lis.writeReg(0x33, 0x08);
  lis.writeReg(0x33, 0x06);

  Serial.begin(9600);
  // attachInterrupt(0, movment, RISING);
  attachInterrupt(0, movment, CHANGE);
  Serial.println("FIN INIT");

}


void movment()
{
  sei();
  int16_t y;
  lis.getYValue(&y);
  cli();
  Serial.print("Y Value: ");
  Serial.print(y);
  Serial.println(" milli Gs");
}

void loop() {
   // Nothing to do
}
