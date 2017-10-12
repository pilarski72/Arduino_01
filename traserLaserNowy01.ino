//Dyrektywy prekompilatora
//Biblioteki

#include <LiquidCrystal_I2C.h>
#include "PCF8574.h"
#include <AccelStepper.h>
#include <NewPing.h>
#include <math.h>

#define FULLSTEP 4
#define HALFSTEP 8

//definicje pinow Arduino

#define lewyPin1  10
#define lewyPin2  11
#define lewyPin3  12
#define lewyPin4  13

#define przodPin1  6
#define przodPin2  7
#define przodPin3  8
#define przodPin4  9

#define obrotPin1  A0
#define obrotPin2  A5
#define obrotPin3  4
#define obrotPin4  5

#define soniclewy A2
#define sonicprawy A1
#define MAX_DYSTANS 400

//definicje pinow expandera

#define zmiana 2
#define wlewo 7
#define wprawo 6

#define zderzakLpin 0
#define zderzakPpin 1

//Instancje klas urządzeń

AccelStepper lewy(HALFSTEP, lewyPin1, lewyPin2, lewyPin3, lewyPin4);
AccelStepper prawy(HALFSTEP, przodPin1, przodPin2, przodPin3, przodPin4);
AccelStepper obrot(HALFSTEP, obrotPin1, obrotPin2, obrotPin3, obrotPin4);
PCF8574 expander;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
NewPing sonar1(soniclewy, soniclewy, MAX_DYSTANS);
NewPing sonar2(sonicprawy, sonicprawy, MAX_DYSTANS);

float czulpodn[] = {0.1, 0.25, 0.5, 0.75, 1.0};
float czulobr[] = {1, 30, 45, 90, 180};
int intmotor = 0;
int czulosc = 0;
long czas = 0;
long czasstolu = 0;
long dlwcis = 0;
long lewytogo = 0;
long prawytogo = 0;
bool lewyzderzak = false;
bool prawyzderzak = false;
bool opuszczaniel = false;
bool opuszczaniep = false;
bool srodekrun = false;
bool lewystop = true;
bool prawystop = true;
bool obrotstop = true;
bool podnoszenie = false;
bool przechyllewo = false;
bool przechylprawo = false;
bool wyrownaj = false;
bool czekam = false;
bool reczny = false;
float katstolu = 0;

int przeczytane = 0;
int stage = 0;
byte dlrozkaz = 0;
int dlmsg = 0;
byte dlmsgbyte[2];
bool odczytane = false;
String inString0 = "";
String inString1 = "";
String rozkaz = "";
String msg = "";

//Definicje rozkazow arduino dla telefonu komorkowego

byte msgpodajx[24] = {(byte)'s', (byte)'t', (byte)'A', (byte)'r', (byte)'t', (byte)':', 5, 0, 2, (byte)'P', (byte)'o', (byte)'d', (byte)'a', (byte)'j', (byte)'X', (byte)':', (byte)':', (byte)'k', (byte)'o', (byte)'N', (byte)'i', (byte)'e', (byte)'c', (byte)'\n'};
byte msgpodajy[24] = {(byte)'s', (byte)'t', (byte)'A', (byte)'r', (byte)'t', (byte)':', 5, 0, 2, (byte)'P', (byte)'o', (byte)'d', (byte)'a', (byte)'j', (byte)'Y', (byte)':', (byte)':', (byte)'k', (byte)'o', (byte)'N', (byte)'i', (byte)'e', (byte)'c', (byte)'\n'};
byte msgpodajxy[25] = {(byte)'s', (byte)'t', (byte)'A', (byte)'r', (byte)'t', (byte)':', 5, 0, 3, (byte)'P', (byte)'o', (byte)'d', (byte)'a', (byte)'j', (byte)'X', (byte)'Y', (byte)':', (byte)':', (byte)'k', (byte)'o', (byte)'N', (byte)'i', (byte)'e', (byte)'c', (byte)'\n'};
byte msgkoniec[23] = {(byte)'s', (byte)'t', (byte)'A', (byte)'r', (byte)'t', (byte)':', 6, 0, 0, (byte)'K', (byte)'o', (byte)'n', (byte)'i', (byte)'e', (byte)'c', (byte)':', (byte)'k', (byte)'o', (byte)'N', (byte)'i', (byte)'e', (byte)'c', (byte)'\n'};

float x0, x1, x;
float y0, y1, y;
float kat;
float gora, bok;
bool nax0 = false;
bool nax1 = false;
bool nay0 = false;
bool nay1 = false;
bool nax = false;
bool nay = false;
bool poziomowanie = false;
bool poziom = false;
bool poziomstart = false;
bool sprawdz = false;
bool pierwszy = true;
bool rusz = false;
bool rownoleg = false;
bool pozazasiegiem = false;
bool sprawdzanie = false;


void setup() {

  expander.begin(0x38);
  expander.pinMode(zderzakLpin, INPUT_PULLUP);
  expander.pinMode(zderzakPpin, INPUT_PULLUP);
  expander.pinMode(wlewo, INPUT_PULLUP);
  expander.pinMode(wprawo, INPUT_PULLUP);
  expander.pinMode(zmiana, INPUT_PULLUP);
  Serial1.begin(9600);

  lewy.setMaxSpeed(1000.0);
  lewy.setAcceleration(190.0);
  lewy.setSpeed(1000);

  prawy.setMaxSpeed(1000.0);
  prawy.setAcceleration(190.0);
  prawy.setSpeed(1000);

  obrot.setMaxSpeed(1000.0);
  obrot.setAcceleration(80.0);
  obrot.setSpeed(1000);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Start:"));
  lcd.setCursor(0, 1);
  lcd.print(F("Wcisnij przycisk"));

  while (expander.digitalRead(zmiana));
  lcd.clear();

  srodek();

}

void srodek() {
  lcd.print(F("ustawiam telefon"));
  lcd.setCursor(0, 1);
  lcd.print(F("w osi silnikow"));
  if (!zderzakL()) {
    lewy.runToNewPosition(100);
  }
  if (!zderzakP()) {
    prawy.runToNewPosition(100);
  }
  lewystop = false;
  lewy.move(-600);
  prawystop = false;
  prawy.move(-600);
  opuszczaniel = true;
  opuszczaniep = true;
  srodekrun = true;
  delay(200);
}

void loop() {

  czas = millis();
  if (!expander.digitalRead(wlewo))
    sprbutton(wlewo);

  if (!expander.digitalRead(wprawo))
    sprbutton(wprawo);

  if (!expander.digitalRead(zmiana))
    sprbutton(zmiana);

  if (!czekam) {
    runmotors();
    if (!opuszczaniel && !opuszczaniep && srodekrun) {    // gdy silniki zejdą na sam dół do zderzaków
      nax0 = true;
      wyslijDoTelefonu(msgpodajx, sizeof(msgpodajx));
      nay0 = true;
      wyslijDoTelefonu(msgpodajy, sizeof(msgpodajy));
      lewystop = false;
      lewy.move(500);                                     // podnieś silniki max w górę
      prawystop = false;
      prawy.move(500);
      srodekrun = false;
      podnoszenie = true;
      delay(200);
    }
    if (podnoszenie && lewystop && prawystop) {           // gdy silniki dojdą do samej góry
      podnoszenie = false;
      nax1 = true;
      wyslijDoTelefonu(msgpodajx, sizeof(msgpodajx));
      nay1 = true;
      wyslijDoTelefonu(msgpodajy, sizeof(msgpodajy));
      lewystop = false;
      lewy.move(-250);                                    // opuść silniki do połowy
      prawystop = false;
      prawy.move(-250);
      wyrownaj = true;
    }
    if (wyrownaj && lewystop && prawystop) {              // gdy silniki zatrzymają się w połowie
      wyrownaj = false;
      x = (x1 - x0) / 2.4;
      y = (y1 - y0) / 2.4;
      kat = atan2(x, y) * 57.296;                        // obliczanie kata pod jakim jest telefon
      katstolu = kat;
      katstolu = nowykat(-90);
      poziomowanie = true;
      poziomstart = true;
      obracaj(kat);                                      // obracanie telefonu w osie silników
    }
    if (poziomowanie && poziomstart && obrotstop && lewystop && prawystop) {    // rozpoczynanie poziomowania - odczytanie danych z telefonu
      poziomstart = false;
      nax = true;
      wyslijDoTelefonu(msgpodajx, sizeof(msgpodajx));
      nay = true;
      wyslijDoTelefonu(msgpodajy, sizeof(msgpodajy));
      rusz = true;
    }
    if (poziomowanie && !poziom && !nax && !nay && lewystop && prawystop && obrotstop && rusz && !sprawdzanie) { // obliczanie i wykonanie ruchu silnikami
      rusz = false;
      gora = -y * 222;
      bok = -x * 111;
      lcd.clear();
      lcd.print(F("Poziomuje..."));
      lcd.setCursor(0, 1);
      lcd.print(F("x= "));
      lcd.print(x);
      lcd.print(F(" y= "));
      lcd.print(y);
      long lm = gora - bok;
      long pm = gora + bok;
      if ( czypozazas(lm, pm)) {
        pozazasiegiem = true;
        poziomowanie = false;
        return;
      }
      lewy.move(int (lm));
      prawy.move(int (pm));
      lewystop = false;
      prawystop = false;
      sprawdz = true;
    }
    if (poziomowanie && (sprawdz || sprawdzanie) && lewystop && prawystop) { // sprawdzenie po wykonaniu ruchu
      sprawdz = false;                          
      if (pierwszy) {                                       // jeżeli sprawdzenie wywołane pierwszy raz - odczytuje dane z telefonu
        poziomstart = true;
        pierwszy = false;
        sprawdzanie = true;
      } else {                                              // jeżeli sprawdzanie wywołane drugi raz - ma już dane z telefonu
        sprawdzanie = false;
        if (abs(x) <= 0.1 && abs(y) <= 0.1) {               // sprawdza czy jest poziom jezeli nie wraca do poziomowania
          poziom = true;
          poziomowanie = false;
          pierwszy = true;
          lcd.clear();
          lcd.print(F("Poziomowanie"));
          lcd.setCursor(0, 1);
          lcd.print(F("zakonczone."));
          wyslijDoTelefonu(msgkoniec, sizeof(msgkoniec));
          obrotstop = false;
          obrot.setCurrentPosition(0);
          obracaj(-kat);
          //lirownoleg = true;
        } else {
          pierwszy = true;
          poziomstart = true;
        }
      }
    }
    if (poziom && !rownoleg && obrotstop) {           // gdy osągnął poziom - rozpoczyna ustawianie sołu - powtarza do osiągnięcia równoległości
      float pozycjastolu = poz_hor();
      lcd.clear();
      lcd.print(F("Ustawianie stolu"));
      lcd.setCursor(0, 1);
      lcd.print(F("kat: "));
      lcd.print(pozycjastolu);
      lcd.print(F(" st."));
      if (abs (pozycjastolu) <= 0.1) {
        if (pierwszy) {
          pierwszy = false;
          czasstolu = millis();
        } else if (millis()-czasstolu > 1000) {
          pierwszy = true;
          czasstolu = 0;
          rownoleg = true;
        }
      } else {
        obracaj(-pozycjastolu);
      }
    }
    if (reczny && obrotstop && lewystop && prawystop) {   // zakończenie ręcznego ustawiania
      reczny = false;
      lcd.setCursor(0, 1);
      lcd.print(F("OK.             "));
    }
  }
  if (odczytane) {
    odczytane = false;
    wiadomosc ();
  }
  if (Serial1.available()) {
    serialFromBt();
  }
  if (pozazasiegiem) {
    pozazasiegiem = false;
    lcd.clear();
    lcd.print(F("Blad poziomow."));
    lcd.setCursor(0, 1);
    lcd.print(F("Poza zasiegiem."));
  }
}


void zmintmotor() {
  switch (intmotor) {
    case 0:
      intmotor = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Zmi. poruszania"));
      lcd.setCursor(0, 1);
      lcd.print(F("Wyb.Przechylanie" ));
      break;
    case 1:
      intmotor = 2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Zmi. poruszania"));
      lcd.setCursor(0, 1);
      lcd.print(F("Wyb.obrot stolem" ));
      break;
    case 2:
      intmotor = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Zmi. poruszania"));
      lcd.setCursor(0, 1);
      lcd.print(F("Wyb. podnoszenie" ));
      break;
  }
}


void zmczulosc() {
  switch (czulosc) {
    case 0:
      czulosc = 1;
      break;
    case 1:
      czulosc = 2;
      break;
    case 2:
      czulosc = 3;
      break;
    case 3:
      czulosc = 4;
      break;
    case 4:
      czulosc = 0;
      break;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Zmiana czulosci"));
  lcd.setCursor(0, 1);
  lcd.print(F("pod/obr:" ));
  lcd.print(czulpodn[czulosc]);
  lcd.print(F("/"));
  lcd.print(czulobr[czulosc]);
}

void sprbutton(int ktory) {
  while (!expander.digitalRead(ktory));
  dlwcis = millis() - czas;
  if (dlwcis < 100) {
    czas = millis();
    return;
  }
  switch (ktory) {
    case wlewo:
      switch (intmotor) {
        case 0:
          wdol(czulpodn[czulosc]);
          break;
        case 1:
          pochyllewo(czulpodn[czulosc]);
          break;
        case 2:
          obracaj(czulobr[czulosc]);
          break;
      }
      break;
    case wprawo:
      switch (intmotor) {
        case 0:
          wgore(czulpodn[czulosc]);
          break;
        case 1:
          pochylprawo(czulpodn[czulosc]);
          break;
        case 2:
          obracaj(-czulobr[czulosc]);

          break;
      }
      break;
    case zmiana:
      if (poziomowanie) {
        poziomowanie = false;
        lcd.clear();
        lcd.print(F("Poziomowanie"));
        lcd.setCursor(0, 1);
        lcd.print(F("przerwane."));
      }
      else if (poziom && !rownoleg){
        rownoleg = true;
      }
      else if (dlwcis < 500) {
        zmczulosc();
      }
      else if (dlwcis >= 500) {
        zmintmotor();
      }
      break;
    default:
      break;
  }
}


void runmotors() {
  if (lewy.distanceToGo() != 0 && !lewyzderzak) {
    lewy.run();
    lewytogo = lewy.distanceToGo();
    if (!zderzakL()) {
      lewy.setCurrentPosition(0);
      lewy.setSpeed(1000);
      lewyzderzak = true;
      prawy.move(0);
      lewy.move(50);
    }
  }
  else if (lewy.distanceToGo() != 0 && lewyzderzak)  {
    lewy.run();
    lewytogo = lewy.distanceToGo();
    delay(10);
    if (zderzakL()) {
      lewy.setCurrentPosition(0);
      lewy.move(0);
      lewy.setSpeed(1000);
      prawy.move(prawytogo);
      lewyzderzak = false;
      opuszczaniel = false;
    }
  }
  if (prawy.distanceToGo() != 0 && !prawyzderzak) {
    prawy.run();
    prawytogo = prawy.distanceToGo();
    if (!zderzakP()) {
      prawy.setCurrentPosition(0);
      prawy.setSpeed(1000);
      prawyzderzak = true;
      lewy.move(0);
      prawy.move(50);
    }
  }
  else if (prawy.distanceToGo() != 0 && prawyzderzak)  {
    prawy.run();
    prawytogo = prawy.distanceToGo();
    delay(10);
    if (zderzakP()) {
      prawy.setCurrentPosition(0);
      prawy.setSpeed(1000);
      prawy.move(0);
      lewy.move(lewytogo);
      prawyzderzak = false;
      opuszczaniep = false;
    }
  }

  if (obrot.distanceToGo() != 0) {
    obrotstop = false;
    obrot.run();
  }

  if (lewy.distanceToGo() == 0) {
    lewystop = true;
  }

  if (prawy.distanceToGo() == 0) {
    prawystop = true;
  }

  if (obrot.distanceToGo() == 0) {
    obrotstop = true;
  }
}

void pochyllewo(float ile) {
  float *xy = funkcja(nowykat(90));
  long lm = long(ile * xy[0]*222);
  long pm = long(ile * xy[1]*222);
  if (czypozazas(lm, pm)) {
    pozazasiegiem = true;
    return;
  }
  reczny = true;
  lcd.clear();
  lcd.print(F("Przechylam lewo"));
  lcd.setCursor(0, 1);
  lcd.print(F("o: "));
  lcd.print(ile);
  lcd.print(F(" st."));
  lewystop = false;
  lewy.move(lm);
  prawystop = false;
  prawy.move(pm);
}

void pochylprawo(float ile) {
  float *xy = funkcja(nowykat(-90));

  long lm = long(ile * xy[0]*222);
  long pm = long(ile * xy[1]*222);
  if (czypozazas(lm, pm)) {
    pozazasiegiem = true;
    return;
  }
  reczny = true;
  lcd.clear();
  lcd.print(F("Przechylam prawo"));
  lcd.setCursor(0, 1);
  lcd.print(F("o: "));
  lcd.print(ile);
  lcd.print(F(" st."));
  lewystop = false;
  lewy.move(lm);
  prawystop = false;
  prawy.move(pm);
}

void wgore(float ile) {
  float *xy = funkcja(katstolu);
  long lm = long(ile * xy[0]*222);
  long pm = long(ile * xy[1]*222);
  if (czypozazas(lm, pm)) {
    pozazasiegiem = true;
    return;
  }
  reczny = true;
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(F("Podnosze o "));
  lcd.print(ile);
  lcd.print(F(" st."));
  lewystop = false;
  lewy.move(lm);
  prawystop = false;
  prawy.move(pm);
}

void wdol(float ile) {
  float *xy = funkcja(katstolu);
  long lm = long(-ile * xy[0]*222);
  long pm = long(-ile * xy[1]*222);
  if (czypozazas(lm, pm)) {
    pozazasiegiem = true;
    return;
  }
  reczny = true;
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(F("Obnizam o "));
  lcd.print(ile);
  lcd.print(F(" st."));
  lewystop = false;
  lewy.move(lm);
  prawystop = false;
  prawy.move(pm);
}

void obracaj(float ilestopni) {
  katstolu = nowykat(ilestopni);
  reczny = true;
  lcd.clear();
  ilestopni > 0 ? lcd.print(F("Obracam lewo")) : lcd.print(F("Obracam prawo")) ;
  lcd.setCursor(0, 1);
  lcd.print(F("o: "));
  lcd.print(abs(ilestopni));
  lcd.print(F(" st."));
  obrotstop = false;
  obrot.move(int (11.805 * ilestopni));
}

void serialFromBt() {
  byte inbyte;
  switch (stage) {
    case 0:
      inbyte = Serial1.read();
      if ((char) inbyte == 's') {
        inString1 += (char) inbyte;
        stage = 1;
        przeczytane = 1;
      } else {
        przeczytane = 0;
        inString1 = "";
      }
      break;
    case 1:
      inbyte = Serial1.read();
      inString1 += (char) inbyte;
      przeczytane += 1;
      if (przeczytane == 6) {
        if (inString1 == "stArt:") {
          stage = 2;
        } else {
          stage = 0;
        }
        przeczytane = 0;
        inString1 = "";
      }
      break;
    case 2:
      inbyte = Serial1.read();
      dlrozkaz = inbyte;
      stage = 3;
      break;
    case 3:
      inbyte = Serial1.read();
      dlmsgbyte[przeczytane] = inbyte;
      przeczytane += 1;
      if (przeczytane == 2) {
        dlmsg = dlmsgbyte[0] * 256 + dlmsgbyte[1];
        przeczytane = 0;
        stage = 4;
      }
      break;
    case 4:
      inbyte = Serial1.read();
      rozkaz += (char) inbyte;
      przeczytane += 1;
      if (przeczytane == dlrozkaz) {
        przeczytane = 0;
        stage = 5;
      }
      break;
    case 5:
      if (dlmsg > 0) {
        inbyte = Serial1.read();
        msg += (char) inbyte;
        przeczytane += 1;
        if (przeczytane == dlmsg) {
          przeczytane = 0;
          stage = 6;
        }
      } else {
        stage = 6;
        przeczytane = 0;
      }
      break;
    case 6:
      inbyte = Serial1.read();
      inString1 += (char) inbyte;
      przeczytane += 1;
      if (przeczytane == 7) {
        if (inString1 == ":koNiec") {
          stage = 0;
          odczytane = true;
        } else {
          stage = 0;
        }
        przeczytane = 0;
        inString1 = "";
      }
      break;
  }
}

void wiadomosc() {
  msg.replace(",", ".");
  if (rozkaz == "X:") {
    if (nax0) {
      x0 = msg.toFloat();
      nax0 = false;
    }
    if (nax1) {
      x1 = msg.toFloat();
      nax1 = false;
    }
    if (nax) {
      x = msg.toFloat();
      nax = false;
    }
  } else if (rozkaz == "Y:") {
    if (nay0) {
      y0 = msg.toFloat();
      nay0 = false;
    }
    if (nay1) {
      y1 = msg.toFloat();
      nay1 = false;
    }
    if (nay) {
      y = msg.toFloat();
      nay = false;
    }
  } else if (rozkaz == "XY:") {
    if (nax0 && nay0) {
      x0 = msg.substring(0, msg.indexOf("\n")).toFloat();
      y0 = msg.substring(msg.indexOf("\n") + 1).toFloat();
      nax0 = false;
      nay0 = false;
    }
    if (nax1 && nay1) {
      x1 = msg.substring(0, msg.indexOf("\n")).toFloat();
      y1 = msg.substring(msg.indexOf("\n") + 1).toFloat();
      nax1 = false;
      nay1 = false;
    }
    if (nax && nay) {
      x = msg.substring(0, msg.indexOf("\n")).toFloat();
      y = msg.substring(msg.indexOf("\n") + 1).toFloat();
      nax = false;
      nay = false;
    }
  } else if (rozkaz == "OBROC:") {
    obracaj(msg.toFloat());

  } else if (rozkaz == "PODNIES:") {
    float ile = msg.toFloat();
    if (ile > 0) {
      wgore(ile);
    } else {
      wdol(-ile);
    }

  } else if (rozkaz == "POCHYL:") {
    float ile = msg.toFloat();
    if (ile > 0) {
      pochylprawo(ile);
    } else {
      pochyllewo(-ile);
    }
  } else if (rozkaz == "ZERUJ:") {
    poziomowanie = true;
    poziomstart = true;
    poziom = false;
    rownoleg = false;
    if (katstolu >90 && katstolu <= 270 ){
      obracaj(katstolu-90);
    } else if (katstolu <= 90) {
      obracaj(-90 + katstolu);
    } else {
      obracaj(-90 - 360 + katstolu);
    }
  } else if (rozkaz == "STOP:") {
    poziomowanie = false;
    poziomstart = false;
    poziom = true;
    rownoleg = true;
  } else if (rozkaz == "ROWNOL:") {
    poziomowanie = false;
    poziomstart = false;
    poziom = true;
    rownoleg = false;
  }
  rozkaz = "";
  msg = "";
  czekam = false;
}

void wyslijDoTelefonu(byte data[], int sizedata) {
  czekam = true;
  delay(500);
  Serial1.write(data, sizedata);
  if (data == msgkoniec) {
    czekam = false;
  }
}

bool zderzakL() {
  return expander.digitalRead(zderzakLpin);
}

bool zderzakP() {
  return expander.digitalRead(zderzakPpin);
}

bool ifzderzak() {
  return (expander.digitalRead(zderzakLpin) || expander.digitalRead(zderzakPpin));
}

float poz_hor() {
  delay(200);
  float dist1 = sonar1.ping_median() / 58.77;
  delay(10);
  float dist2 = sonar2.ping_median() / 58.77;
  return atan2(dist2 - dist1, 55.3) * 180 / 3.14;
}

bool czypozazas(long lm, long pm) {
  long l = lewy.currentPosition();
  long p = prawy.currentPosition();
  return ( l + lm > 500 || l + lm < 0 || p + pm > 500 || p + pm < 0);
}

float* funkcja (float kat) {
  float*  xy = new float[2]; 
  if (kat >=0 && kat < 60) {
    xy[0] = 1;
    xy[1] = -kat/60+1;
  } else if (kat >=60 && kat < 120) {
    xy[0] = -kat/60+2;
    xy[1] = -kat/60+1;
  } else if (kat >=120 && kat < 180) {
    xy[0] = -kat/60+2;
    xy[1] = -1;
  } else if (kat >=180 && kat < 240) {
    xy[0] = -1;
    xy[1] = kat/60-4;
  } else if (kat >=240 && kat < 300) {
    xy[0] = kat/60-5;
    xy[1] = kat/60-4;
  } else if (kat >=300 && kat < 360) {
    xy[0] = kat/60-5;
    xy[1] = 1;
  }


  return xy;

  
}

float nowykat (float ilestopni) {
  if (katstolu-ilestopni >= 360) {
    return katstolu-ilestopni-360;
  } else if (katstolu-ilestopni < 0) {
    return 360 + katstolu - ilestopni;
  }
  return katstolu-ilestopni;
}



