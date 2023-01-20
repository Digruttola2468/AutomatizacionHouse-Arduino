//Librerias

/*Control Remoto*/
#include <IRremote.h>

#include <SPI.h>

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
#define contAux_4 0x1FE30CF
#define contAux_5 0x1FEB04F
#define contAux_6 0x1FE708F

#define contAux_VolPlus 0x1FE609F
#define contAux_VolRest 0x1FEA05F
#define contAux_CHPlus 0x1FEC03F
#define contAux_CHRest 0x1FE40BF
#define contAux_SCAN 0x1FE807F
#define contAux_MUTE 0x1FE48B7

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
#define Rele1               4
#define sensorTemperatura   3

//Control Remoto
IRrecv irrecv(SensorControlRemoto);
decode_results codigo;

//LCD
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7

//Sensor Temp Humd DHT11
DHT dht(sensorTemperatura, DHT11);

//Bluetooth
SoftwareSerial miBT(6, 5);

//Reloj
RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231

//Variables Globales
int humd = 0;
int temp = 0;
char DataBluetooth = '0';
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 2000;
bool isClock = false;
bool isTempHumd = false;

//Variables Reloj
int reloj[] = {2022, 1, 10, 18, 55};
int indexReloj = 0;
bool moodSettingClock = false;

//Variables Alarma
int moodSettingAlarma = false;
int alarma[] = {20,30}; //{hour,minutes}
int existsAlarm = false;
int indexAlarma = 0;


void setup() {
  //Inicializamos
  Serial.begin(9600);
  miBT.begin(38400);
  dht.begin();
  rtc.begin();
  SPI.begin();

  IrReceiver.begin(SensorControlRemoto, DISABLE_LED_FEEDBACK);     // inicializa recepcion de datos
  irrecv.enableIRIn();

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
  lcd.print("Presione Control");

  //Definimos las salidas
  pinMode(Rele1, OUTPUT);

  startMillis = millis();
}

void loop() {
  //Obtenemos los datos del DHT11
  temp = dht.readTemperature();
  humd = dht.readHumidity();

  currentMillis = millis();  //get the current time
  if (currentMillis - startMillis >= period){  //test whether the period has elapsed
    if (!moodSettingAlarma && !moodSettingClock) 
      if(isClock) mostrarFechaHorario();
      else if(isTempHumd) mostrarTempHumd();



      
        
    startMillis = currentMillis;
  }
  
  //------------------------------------------------------------

  if (irrecv.decode(&codigo)) {
    Serial.println(codigo.value, HEX);


    if (moodSettingClock) {

      if (codigo.value == contAux_VolPlus) reloj[indexReloj] += 1;

      if (codigo.value == contAux_VolRest) reloj[indexReloj] -= 1;

      if (codigo.value == contAux_CHPlus) {
        indexReloj++;
        if (indexReloj == 5) {
          indexReloj = 0;
        }
      }
      if (codigo.value == contAux_CHRest) {
        indexReloj--;
        if (indexReloj == -1) {
          indexReloj = 4;
        }
      }

      mostrarConfiguracionReloj(indexReloj);

      if (codigo.value ==  contAux_MUTE) {
        moodSettingClock = false;
        mostrarFechaHorario();
      }
      
      if (codigo.value == contAux_SCAN) {
        int anio = reloj[0];
        int meses = reloj[1];
        int dias = reloj[2];
        int horas = reloj[3];
        int minutos = reloj[4];
        rtc.adjust(DateTime(anio, meses, dias, horas, minutos));

        moodSettingClock = false;
        mostrarFechaHorario();
      }


    } else if(moodSettingAlarma){
      if (codigo.value == contAux_VolPlus) alarma[indexAlarma] += 1;

      if (codigo.value == contAux_VolRest) alarma[indexAlarma] -= 1;

      if (codigo.value == contAux_CHPlus) {
        indexAlarma++;
        if (indexAlarma == 1) {
          indexAlarma = 0;
        }
      }
      if (codigo.value == contAux_CHRest) {
        indexAlarma--;
        if (indexAlarma == -1) {
          indexAlarma = 1;
        }
      }

      mostrarConfiguracionAlarma(indexAlarma);
      
      if (codigo.value ==  contAux_MUTE) {
        moodSettingAlarma = false;
        mostrarFechaHorario();
      }
      if (codigo.value == contAux_SCAN) {
        
        mostrarPantalla("Se Guardo", 0, true);
        mostrarPantalla("Correctamente", 1, false);
        delay(2000);
        
        existsAlarm = true;
        moodSettingAlarma = false;
        mostrarFechaHorario();
      }
      
    } else {
      if (codigo.value == contAux_1) prenderApagarRele(Rele1);

      if (codigo.value == contAux_2) mostrarTempHumd();

      if (codigo.value == contAux_3) mostrarFechaHorario();

      if (codigo.value == contAux_4) {
        moodSettingClock = true;
        mostrarPantalla(" Configuracion ", 0, true);
        mostrarPantalla("De Fecha y Hora", 1, false);
      }
      if (codigo.value == contAux_5) {
        moodSettingAlarma = true;
        mostrarPantalla(" Setting Alarm ", 0, true);
        mostrarPantalla("Select Hour&Min", 1, false);
      }
    }

    irrecv.resume();
  }

  if (miBT.available()) {
    DataBluetooth = miBT.read();

    if (DataBluetooth == '1') prenderApagarRele(Rele1);

    if (DataBluetooth == '2') mostrarTempHumd();

    if (DataBluetooth == '3') mostrarFechaHorario();
  }

  if (existsAlarm) {
    DateTime fecha = rtc.now();
    if(fecha.hour() == alarma[0] && fecha.minute() == alarma[1]){
      prenderApagarRele(Rele1);
      
      existsAlarm = false;
    }
  }
}

void mostrarPantalla (String mensaje , int cursor, boolean clear) {
  if (clear)   lcd.clear();     // limpia pantalla

  lcd.setCursor(0, cursor);    // ubica cursor en columna 0 y linea 0
  lcd.print(mensaje);  // escribe el texto
}

void mostrarConfiguracionReloj(int indexReloj) {
  String thisString = "";
  //Year
  if (indexReloj == 0) {
    mostrarPantalla("Year", 0, true);

    thisString = String(reloj[indexReloj]);
    mostrarPantalla(thisString, 1, false);
  }
  //Mounth
  if (indexReloj == 1) {
    mostrarPantalla("Mounth", 0, true);

    if ( reloj[indexReloj] > 12 )reloj[indexReloj] = 1;
    if ( reloj[indexReloj] < 1 )reloj[indexReloj] = 12;

    thisString = String(reloj[indexReloj]);
    mostrarPantalla(thisString, 1, false);
  }
  //Day
  if (indexReloj == 2) {
    mostrarPantalla("Day", 0, true);

    if ( reloj[indexReloj] > 31 ) reloj[indexReloj] = 1;
    if ( reloj[indexReloj] < 1 )reloj[indexReloj] = 31;

    thisString = String(reloj[indexReloj]);
    mostrarPantalla(thisString, 1, false);
  }
  //Hour
  if (indexReloj == 3) {
    mostrarPantalla("Hour", 0, true);

    if ( reloj[indexReloj] > 23 ) reloj[indexReloj] = 0;
    if ( reloj[indexReloj] < 0 )reloj[indexReloj] = 23;

    thisString = String(reloj[indexReloj]);
    mostrarPantalla(thisString, 1, false);
  }
  //Minutos
  if (indexReloj == 4) {
    mostrarPantalla("Minutes", 0, true);

    if ( reloj[indexReloj] > 59 ) reloj[indexReloj] = 0;
    if ( reloj[indexReloj] < 0 )reloj[indexReloj] = 59;

    thisString = String(reloj[indexReloj]);
    mostrarPantalla(thisString, 1, false);
  }
}

void mostrarTempHumd() {
  isClock = false;
  isTempHumd = true;
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

void mostrarConfiguracionAlarma (int indexAlarma) {
  String thisString = "";
  if (indexAlarma == 0) {
    mostrarPantalla("Hour", 0, true);

    if ( alarma[indexAlarma] > 23 ) alarma[indexAlarma] = 0;
    if ( alarma[indexAlarma] < 0 ) alarma[indexAlarma] = 23;

    thisString = String(alarma[indexAlarma]);
    mostrarPantalla(thisString, 1, false);
  }
  //Minutos
  if (indexAlarma == 1) {
    mostrarPantalla("Minutes", 0, true);

    if ( alarma[indexAlarma] > 59 ) alarma[indexAlarma] = 0;
    if ( alarma[indexAlarma] < 0 )  alarma[indexAlarma] = 59;

    thisString = String(alarma[indexAlarma]);
    mostrarPantalla(thisString, 1, false);
  }
}

void prenderApagarRele(int pinRele) {
  digitalWrite(pinRele, !digitalRead(pinRele));
  //Serial.println(digitalRead(pinRele));
}

void mostrarFechaHorario() {
  isClock = true;
  isTempHumd = false;
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
