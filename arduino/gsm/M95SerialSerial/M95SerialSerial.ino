volatile char c;

void setup() {
  Serial.begin(9600);
  Serial.println("Yep!");

  digitalWrite(8, HIGH);
  delay(5000);
  digitalWrite(8, LOW);
  delay(2000);
  digitalWrite(8, HIGH);
  delay(5000);

  digitalWrite(15, HIGH);
  
  Serial.println("Yip!");
  
  
  Serial3.begin(19200);
  Serial3.flush();
  for (int i=0; i < 3; i++)
  {
  Serial3.print("A");
  delay(500);
  Serial3.print("T");
  delay(500);
  Serial3.print("\r");
  delay(500);
  }
  Serial3.print("AT+IPR=19200\r");

  while(Serial3.available())
  {
    c = Serial3.read();
  }  
  
  Serial.println("Init done");  
}

void loop() {

  if(Serial.available())
  {
    c = Serial.read();
    Serial3.print(c);
  }

  if(Serial3.available())
  {
    c = Serial3.read();
    Serial.print(c);   
  }
}
