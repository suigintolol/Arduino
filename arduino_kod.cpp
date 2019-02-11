#include <Ultrasonic.h>         //библиотека для работы с дальнометром
#include <TimeHelpers.h>           //библиотека для работы с таймерами 
#include "DHT.h"                  //библиотека для работы с датчиком температуры и влажности воздуха
#include <LiquidCrystal_I2C.h>     //дисплей
#include <SPI.h>             //библиотека для работы с SPI
#include <Ethernet.h>        //библиотека для работы с Ethernet 
#include <SD.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Устанавливаем дисплей

int gg = 0 ;
String StringMZDVV;
String StringMS;
String StringPMAXDDV;
String StringPMINDDV;
String StringMDDV;

#define  T_sbordanih  _SEC_(30)
#define  T_poliva  _MIN_(30)
#define  T_sveta  _MIN_(30)
#define  T_kolvody  _MIN_(30)

#define  DVV  A1   //пин Датчика влажности воды
#define  PDVV  6   //пин Питания Датчика влажности воды
int ZDVV;      //Значение Датчика влажности воды
int MZDVV  =  700 ;  //Значение Датчика влажности воды при которой будет происходить полив
#define  N  8    //пин Насоса


#define  S  A0     //пин фоторезистора   (Sвет)
int ZS;            //Значение оСвещёности
#define  R  9      //пин реле   
int MS  =  800 ;  //Значение фоторезистора при котором будет происходить переключение реле

#define  TiV  7   //пин дачитка темературы и влажности воздуха
int ZV;      //Значение  влажности воздуха
int ZT;      //Значение температуры
DHT dht(TiV, DHT11);

#define  DT  10   //пин Trig  дальнометра 
#define  DE  11   //пин Echo дальнометра
#define  OS  12   //пин оповещающего светодиода
float DDV ;        //дистанция до воды
int PMAXDDV  =  5 ;      //Значение 100% наполненности (В СМ)
int PMINDDV  =  25 ;      //Значение 0% наполненности  (В СМ)
int PDDV ;                  //Значение % наполненности  (В СМ)
int MDDV  =  30;       //Значение при котором будет гореть оповещающий светодиод, сообщая что воды мало   (В %)
Ultrasonic ultrasonic(DT, DE);

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };    // MAC АДРЕС
IPAddress ip(192, 168, 0, 101);  // IP АДРЕС
String readString = String(30);
EthernetServer server(80);      //инициализация библиотеки Ethernet server library
boolean newInfo = 0; //переменная для новой информации

void setup()
{
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  dht.begin();
  pinMode(PDVV, OUTPUT);
  pinMode(N, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(OS, OUTPUT);

  lcd.init();
  lcd.backlight();// Включаем подсветку дисплея

  lcd.setCursor(0, 0);
  lcd.print("VP=");
  lcd.setCursor(8, 0);
  lcd.print("Svet=");
  lcd.setCursor(0, 1);
  lcd.print("Vlj=  %");
  lcd.setCursor(8, 1);
  lcd.print("Temp=  C");
  digitalWrite(R, HIGH);
  digitalWrite(N, HIGH);
}

void loop()
{
  DO_EVERY(T_sbordanih, {sbordanih() ;   });
  DO_EVERY(T_poliva, {poliv();   });
  DO_EVERY(T_sveta, {svet();   });
  DO_EVERY(T_kolvody, {kolvody();   });

  ezernet(& MZDVV, & MS, &PMAXDDV, &PMINDDV, &MDDV);
}

void sbordanih()
{
  //получение влажности почвы
  digitalWrite(PDVV, HIGH);      //включение питания датчика
  delay(10);
  ZDVV = analogRead(DVV); //
  Serial.println("Влажность почвы: " + String(ZDVV));   //вывод влажности почвы в консоль
  delay(10);
  digitalWrite(PDVV, LOW);      //вылючение питания датчика
  //получение влажности почвы

  //получение данных освещёности
  ZS = analogRead(S);
  Serial.println("Освещённость: " + String(ZS));
  //получение данных освещёности

  //получение влажности воздуха и температуры
  ZV = dht.readHumidity();               // Считываем влажность
  ZT = dht.readTemperature();            // Считываем температуру
  if (isnan(ZV) || isnan(ZT)) {                // Проверка, удачно ли прошло считывание.
    Serial.println("Ошибка датчика температуры и влажности воздуха");
    return;
  }
  Serial.println("Влажность: " + String(ZV) + "%\t" + "Температура: " + String(ZT) + "*C ");
  //получение влажности воздуха и температуры

  //получение дистанции до воды
  DDV =  ultrasonic.Ranging(CM); // дистанция в см
  PDDV = abs( ( DDV - PMAXDDV ) / ((PMINDDV - PMAXDDV) * 0.01) - 100 );
  Serial.println("Уровень жидкости в резервуаре: " + String(PDDV) + "%");
  //получение дистанции до воды

  //вывод на дисплей
  lcd.setCursor(3, 0);
  lcd.print(ZDVV);
  lcd.setCursor(13, 0);
  lcd.print(ZS);
  lcd.setCursor(4, 1);
  lcd.print(ZV);
  lcd.setCursor(13, 1);
  lcd.print(ZT);
  //вывод на дисплей
}

void poliv()              // если значение влажности почвы меньше установленой то переключается 2 реле
{
  if (ZDVV > MZDVV) {
    digitalWrite(N, LOW);
    delay(3000);
    digitalWrite(N, HIGH);
  }
  else {
    digitalWrite(N, HIGH);
  }
}

void svet()
{
  if (ZS < MS) {
    digitalWrite(R, LOW); // если значение освещёности меньше установленой то переключается 1 реле
  }
  else {
    digitalWrite(R, HIGH);
  }
}

void kolvody()
{
  if (MDDV > PDDV) {
    digitalWrite(OS, HIGH); // если уровень воды меньше установленой то включается свотодиод
  }
  else {
    digitalWrite(OS, LOW);
  }
}

void ezernet(int * MZDVV, int * MS, int * PMAXDDV, int * PMINDDV, int * MDDV)
{
  // =============Создаем клиентское соединение====================
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          //store characters to string
          readString.concat( c);
        }

        Serial.print( c);
        /*
          if (newInfo && c == ' '){
          newInfo = 0; }
          if (c == '$'){
          newInfo = 1; }
          if (newInfo == 1){

          if (c == '1'){
          Serial.println (c);
          Serial.println ("Включить");
          digitalWrite (13, HIGH); }
          if (c == '2'){
          Serial.println (c);
          Serial.println ("Выключить");
          digitalWrite (13, LOW);}
          } */

        if (c == '\n' ) {
          // =============Формируем HTML-страницу==========================
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<head> ");
          client.println("<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> ");
          client.println("<title> Avtor Denis Beketov </title>");
          client.println("</head> ");
          client.println("<body");
          client.println("<hr />");
          client.println("<h1> Avtor Denis Beketov </h1>");

          client.println("<h2>Отслеживание данных</h2>");
          client.println("<table border=\"0\">");            //открытие тега таблицы без рамки

          client.println("<tr> <th>Название</th> <th>Значение</th> </tr>");

          client.println("<tr> <td>Влажность почвы:</td> <td>"); client.println(ZDVV); client.println("</td> </tr>");
          client.println("<tr> <td>Освещённость:</td> <td>"); client.println(ZS); client.println("</td> </tr>");
          client.println("<tr> <td>Влажность:</td> <td>"); client.println(ZV); client.println("% </td> </tr>");
          client.println("<tr> <td>Температура:</td> <td>"); client.println(ZT); client.println("*C </td> </tr>");
          client.println("<tr> <td>Уровень жидкости <br> в резервуаре </td> <td>"); client.println(PDDV); client.println("% </td> </tr>");

          client.println("</table>");                    //закрытие тега таблицы

          /*/==============Ручное управление======================
            client. print ("<a href=\"/$1\"><button>Включить</button></a>");
            client. print ("<a href=\"/$2\"><button>Выключить</button></a>");
          */

          client.println("<h2>Отслеживание настроек</h2>");
          client.println("<table border=\"0\">");            //открытие тега таблицы без рамки

          client.println("<tr> <th>Название</th> <th>Значение</th> </tr>");

          client.println("<tr> <td>Порог влажности почвы</td> <td>"); client.println(*MZDVV); client.println("</td> </tr>");
          client.println("<tr> <td>Порог освещения</td> <td>"); client.println(*MS); client.println("</td> </tr>");
          client.println("<tr> <td>100% заполненности бака(в СМ)<td>"); client.println(*PMAXDDV); client.println("% </td> </tr>");
          client.println("<tr> <td>0% заполненности бака(в СМ)</td> <td>"); client.println(*PMINDDV); client.println("*% </td> </tr>");
          client.println("<tr> <td>Порог заполненности бака (в %)</td> <td>"); client.println(*MDDV); client.println("% </td> </tr>");

          client.println("</table>");                    //закрытие тега таблицы
          //==============Настройки======================
          client.println("<h3>Изменение настроек</h3>");
          client.println("<form method=get name=ptemp>");
          client.println("<label>Порог влажности почвы</label><input maxlength=20 name=MZDVV type=text value="">");
          client.println("<br> "); //перенос на след. строчку
          client.println("<label>Порог освещения</label><input maxlength=20 name=MS type=text value="">");
          client.println("<br> "); //перенос на след. строчку
          client.println("<label>100% наполненности бака(в СМ)</label><input maxlength=20 name=PMAXDDV type=text value="">");
          client.println("<br> "); //перенос на след. строчку
          client.println("<label>0% наполненности бака(в СМ)</label><input maxlength=20 name=PMINDDV type=text value="">");
          client.println("<br> "); //перенос на след. строчку
          client.println("<label>Порог наполненности бака (в %)</label><input maxlength=20 name=MDDV type=text value="">");
          client.println("<br> "); //перенос на след. строчку
          client.println("<input type=submit value=Изменить настройки>");
          client.println("</form>");

          //обработка входных данных
          for (int i = 0; i < readString.length();) {
            if (readString.substring(i).startsWith("MZDVV=")) {
              i += 6; // пропускаем "time="
              int j = i; // запоминаем начало числа
              while ((i < readString.length()) && ((readString.charAt(i) >= 0x30) && (readString.charAt(i) <= 0x39))) {
                ++i;
              }
              StringMZDVV = readString.substring(j, i);
            }  else if (readString.substring(i).startsWith("MS=")) {
              i += 3;
              int j = i; // запоминаем начало числа
              while ((i < readString.length()) && ((readString.charAt(i) >= 0x30) && (readString.charAt(i) <= 0x39))) {
                ++i;
              }
              StringMS = readString.substring(j, i);
            }  else if (readString.substring(i).startsWith("PMAXDDV=")) {
              i += 8;
              int j = i; // запоминаем начало числа
              while ((i < readString.length()) && ((readString.charAt(i) >= 0x30) && (readString.charAt(i) <= 0x39))) {
                ++i;
              }
              StringPMAXDDV = readString.substring(j, i);
            }  else if (readString.substring(i).startsWith("PMINDDV=")) {
              i += 8;
              int j = i; // запоминаем начало числа
              while ((i < readString.length()) && ((readString.charAt(i) >= 0x30) && (readString.charAt(i) <= 0x39))) {
                ++i;
              }
              StringPMINDDV = readString.substring(j, i);
            }  else if (readString.substring(i).startsWith("MDDV=")) {
              i += 5;
              int j = i; // запоминаем начало числа
              while ((i < readString.length()) && ((readString.charAt(i) >= 0x30) && (readString.charAt(i) <= 0x39))) {
                ++i;
              }
              StringMDDV = readString.substring(j, i);
            }
            ++i;
          }
          //обработка входных данных

          //изменение переменных настроек
          *MZDVV = StringMZDVV.toInt();
          *MS = StringMS.toInt();
          *PMAXDDV = StringPMAXDDV.toInt();
          *PMINDDV = StringPMINDDV.toInt();
          *MDDV = StringMDDV.toInt();

          //изменение переменных настроек

          client.println("</body></html>"); //закрытие  тега HTML
          readString = "";
          client.stop();
        }
      }
    }
  }
  delay(1000);
}
