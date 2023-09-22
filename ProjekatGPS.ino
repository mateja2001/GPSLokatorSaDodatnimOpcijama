#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

volatile float gSirina = 0;
volatile float gDuzina = 0;
const int BUTTON_PIN = 7;
boolean pocetak = false;
boolean kraj=false;
boolean uToku=false;
unsigned long vreme = 0;
volatile float rastojanje = 0;

LiquidCrystal lcd(13, 12, 8, 7, 6, 5, 4);
TinyGPSPlus gps;
SoftwareSerial sUart(11, 10);

void setup()
{
  Serial.begin(9600);
  sUart.begin(9600);
  lcd.begin(40, 2);
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), taster, RISING);
  lcd.print("Waiting for GPS");
  obradiGpsSignal();
  ispisiKoordinate();

}

void taster() {
  static unsigned long poslPrekid=0;
  unsigned long trenPrekid=millis();
  if(trenPrekid-poslPrekid>200){
    if(!pocetak and !uToku){
      pocetak=true; 
    }else if(uToku and pocetak){
      uToku=false;
      pocetak=false;
     }
     else{
      kraj=true;
      uToku=false;
      }
  }
  poslPrekid=trenPrekid;
}



void obradiGpsSignal() {
  while (1) {
    sUart.listen();
    while (sUart.available() > 0) {
      gps.encode(sUart.read());
    }
    if (gps.location.isUpdated()) {
      gDuzina = gps.location.lng();
      gSirina = gps.location.lat();
      break;
    }
  }
}


void ispisiKoordinate() {
  lcd.clear();
  lcd.print("Lat: ");
  String gS = String(gSirina, 6);
  lcd.print(gS);
  lcd.setCursor(0, 1);
  lcd.print("Lng: ");
  String gD = String(gDuzina, 6);
  lcd.print(gD);
}

void ispisiRezultate(float brzina, unsigned long interval, float rastojanje) {
  lcd.clear();
  unsigned long seconds = interval/ 1000;
  unsigned long minutes = seconds / 60;
  lcd.print("Time:");
  lcd.print(minutes);
  lcd.print(":");
  lcd.print(seconds);
  delay(3000);
  lcd.clear();
  lcd.print("Dist:");
  String rst = String(rastojanje, 2);
  lcd.print(rst);
  lcd.print("m");
  lcd.setCursor(0,1);
  lcd.print("Speed: ");
  String v=String(brzina,2);
  lcd.print(v);
  lcd.print("m/s");
  }

void stanjeMerenja(){
   while(1){
       if(!uToku){
      unsigned long interval=millis()-vreme;
      unsigned long seconds = interval/ 1000;
      float brzina= (rastojanje*1.0)/seconds;
      ispisiRezultate(brzina,interval,rastojanje);
      uToku=true;
      return;
  }
 //potencijalno ako dugme nece da se prekine, onda ce se racunati samo kranja razdaljina kad se dugme natisne
  obradiGpsSignal();
  float oldlat=gSirina;
  float oldlong=gDuzina;
  delay(1000);
  obradiGpsSignal();
  double pom=TinyGPSPlus().distanceBetween(oldlat,oldlong,gSirina,gDuzina);
  //zbog greske koju pravi GPS pri mirovanju, mozda bi trebala biti jos malo veca
  if(pom>1){
  rastojanje+=pom;  
  }
 }
}

void loop()
{
if(pocetak and !uToku){
      uToku=true;
      vreme=millis();
      rastojanje=0;
      lcd.clear();
      lcd.print("In progress...");
      stanjeMerenja();
  }
  //sluzi za reset
if(kraj){
      kraj=false;
      obradiGpsSignal();
      ispisiKoordinate();
    }
}
