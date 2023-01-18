//Librerias

/*Control Remoto*/
#include <IRremote.h>

/*Sensor de Tarjeta*/
#include <SPI.h>
#include <MFRC522.h>

/*Pantalla LCD*/
#include <Wire.h>                 // libreria de comunicacion por I2C
#include <LCD.h>                  // libreria para funciones de LCD
#include <LiquidCrystal_I2C.h>    // libreria para LCD por I2C

/*DHT sensor Temperatura y Humedad*/
#include <DHT.h>
#include <DHT_U.h>

/*RTC3231 Reloj y Fecha*/
//#include <Wire.h>    // incluye libreria para interfaz I2C
#include <RTClib.h>   // incluye libreria para el manejo del modulo RTC

/*Modulo Bluetooth HC-05*/
#include <SoftwareSerial.h>

//Botones del control remoto
#define contAux_1 0x1FE50AF
#define contAux_2 0x1FED827
#define contAux_3 0x1FEF807

/*Conect MFRC522
  SDA(SS) 10
  SCK     13
  MOSI    11
  MISO    12
  RST     9
  GND     GND
  3.3v    3.3v
*/
/*Conect LCD I2C
  SDA   A4
  SCL   A5
*/
/*Conect DHT11
  OUT -> PIN 3
*/
/*Conect ControlRemoto
  OUT -> PIN 2
*/
/*Conect RELE
  OUT -> PIN 4
*/
/*Conect HC-05 Bluetooth
  VCC --> 5v
  GND --> GND
  TXD --> 6
  RXD --> 5
*/
/*Conecction
  SDA --> A4
  SCL --> A5
  VCC --> 5v
  GND --> 0v
*/

//PINES
#define SensorControlRemoto 2           // sensor KY-022 a pin digital 2
#define RST_PIN             9
#define SS_PIN              10
#define Rele1               4
#define sensorTemperatura   3

byte LecturaUID[4];
byte Usuario1[4] = {0x6C, 0xF2, 0xDD, 0x2B};  //Tarjeta Usuario 1

IRrecv irrecv(SensorControlRemoto);
decode_results codigo;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // crear una instacia MFRC522

LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7

DHT dht(sensorTemperatura, DHT11);

SoftwareSerial miBT(6, 5);

RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231

//Variables Globales
int humd = 0;
int temp = 0;
char DataBluetooth = '0';

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(SensorControlRemoto, DISABLE_LED_FEEDBACK);     // inicializa recepcion de datos
  irrecv.enableIRIn();

  miBT.begin(38400);

  SPI.begin();
  mfrc522.PCD_Init();

  //Inicializamos DHT
  dht.begin();
  rtc.begin();

  //Establecer fecha y horario RTC3231
  rtc.adjust(DateTime(__DATE__, __TIME__));

  //Definimos funciones del LCD I2C
  lcd.setBacklightPin(3, POSITIVE); // puerto P3 de PCF8574 como positivo
  lcd.setBacklight(HIGH);   // habilita iluminacion posterior de LCD
  lcd.begin(16, 2);     // 16 columnas por 2 lineas para LCD 1602A
  lcd.clear();      // limpia pantalla

  lcd.setCursor(0, 0);
  lcd.print("QUE ONDA!");
  lcd.setCursor(0, 1);
  lcd.print("Precione Control");
  
  //Definimos las salidas
  pinMode(Rele1, OUTPUT);
}

void loop() {
  
  temp = dht.readTemperature();
  humd = dht.readHumidity();
  
  //------------------------------------------------------------
  if ( ! mfrc522.PICC_IsNewCardPresent() ) {
    //---
    if (irrecv.decode(&codigo)) {
      Serial.println(codigo.value, HEX);

      if (codigo.value == contAux_1)    // si codigo recibido es igual a Boton_1
        prenderApagarRele(Rele1);

      if (codigo.value == contAux_2)
        mostrarTempHumd();

      if (codigo.value == contAux_3)
        mostrarFechaHorario();

      irrecv.resume();
    }
    //---
    if (miBT.available()) {
      DataBluetooth = miBT.read();

      if (DataBluetooth == '1') prenderApagarRele(Rele1);

      if (DataBluetooth == '2') mostrarTempHumd();

      if (DataBluetooth == '3') mostrarFechaHorario();
    }
    //---
    return;
  }


  /*
    // Select one of the cards
    if (  mfrc522.PICC_ReadCardSerial()) {
    return;
    }*/


  for (byte i = 0; i < mfrc522.uid.size; i++)
    LecturaUID[i] = mfrc522.uid.uidByte[i];

  if ( comparaUID(LecturaUID, Usuario1)) {
    mostrarPantalla("Bienvenido", 0, true);
    mostrarPantalla("Ivan Di Gruttola", 1, false);

    delay(2000);

    mostrarTempHumd();
  }
  else
    mostrarPantalla("No te Conozco", 0, true);

  mfrc522.PICC_HaltA();
}


boolean comparaUID( byte lectura[], byte usuario[]) {
  for (byte i = 0 ; i < mfrc522.uid.size; i++)
    if (lectura[i] != usuario[i]) return (false);

  return (true);
}

void mostrarPantalla (String mensaje , int cursor, boolean clear) {
  if (clear)   lcd.clear();     // limpia pantalla

  lcd.setCursor(0, cursor);    // ubica cursor en columna 0 y linea 0
  lcd.print(mensaje);  // escribe el texto
}

void mostrarTempHumd() {
  lcd.clear();
  lcd.setCursor(0, 0);    // ubica cursor en columna 0 y linea 0
  lcd.print("Temp: ");  // escribe el texto
  lcd.print(temp);
  lcd.print("*C");
  lcd.setCursor(0, 1);
  lcd.print("Humd: ");  // escribe el texto
  lcd.print(humd);
  lcd.print(" %");
}

void prenderApagarRele(int pinRele) {
  digitalWrite(pinRele, !digitalRead(pinRele));
  //Serial.println(digitalRead(pinRele));
}

void mostrarFechaHorario() {
  DateTime fecha = rtc.now();

  lcd.clear();

  lcd.setCursor(0, 0);    // ubica cursor en columna 0 y linea 0
  lcd.print(fecha.day());  // escribe el texto
  lcd.print("/");
  lcd.print(fecha.month());  // escribe el texto
  lcd.print("/");
  lcd.print(fecha.year());  // escribe el texto

  lcd.setCursor(0, 1);
  lcd.print(fecha.hour());  // escribe el texto
  lcd.print(":");
  lcd.print(fecha.minute());
  lcd.print(":");
  lcd.print(fecha.second());

/*
  Serial.print(fecha.day());     // funcion que obtiene el dia de la fecha completa
  Serial.print("/");       // caracter barra como separador
  Serial.print(fecha.month());     // funcion que obtiene el mes de la fecha completa
  Serial.print("/");       // caracter barra como separador
  Serial.print(fecha.year());      // funcion que obtiene el año de la fecha completa
  Serial.print(" ");       // caracter espacio en blanco como separador
  Serial.print(fecha.hour());      // funcion que obtiene la hora de la fecha completa
  Serial.print(":");       // caracter dos puntos como separador
  Serial.print(fecha.minute());      // funcion que obtiene los minutos de la fecha completa
  Serial.print(":");       // caracter dos puntos como separador
  Serial.println(fecha.second());    // funcion que obtiene los segundos de la fecha completa
*/
}
