// VFD Clock Demo mit ESP8266

// Auswahl der Anzeigeplatine
#define IV_17
// #define IV_22
// #define IV_7

// Kurze Tages-Namen für schnellere Anzeige
// #define SHORT_DAYS

#ifdef IV_17

  #define DIGIT_MAX 5
  #define SHOW_DATE
  #define SHOW_DAYSLEFT
  #define SHOW_MESSAGE
  #define DATE_INTERVAL 11
  #define DAYSLEFT_INTERVAL 60
  #define MESSAGE_INTERVAL 60
  #define MESSAGE "Heute ist ein guter Tag"

#else

  #define DIGIT_MAX 3
  #define SHOW_DATE
  #define SHOW_DAYSLEFT
  #define DATE_INTERVAL 9
  #define DAYSLEFT_INTERVAL 30

#endif

#define SET_BTN 0 // SET/PROG Button
#define INC_BTN 5 // INC Button
#define DEC_BTN 2 // DEC Button

#include <Strings.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#include <time.h>
#include <Ticker.h>  //Ticker Library

Ticker tick_timer;

char ssid[32] = "my_ssid";
char password[32] = "my_wlan_password";

// Zieldatum für Restzeit
const int dest_year = 2023;
const int dest_month = 7;
const int dest_day = 1;

int hours, minutes, seconds, seconds_old;
int week_day, date_day, date_month, date_year;
int days_left;
volatile int led_state;
byte displ_arr[18]; // ausreichend für 6 Digits je 3 Bytes

char weekday1[8] = {"SMDMDFS"};
char weekday2[8] = {"OOIIORA"};

#ifdef SHORT_DAYS
String day_names[7] = {"Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam"};
#else
String day_names[7] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
#endif

String month_names[12] = {"Jan", "Febr", "Mrz", "Apr", "Mai", "Juni", "Juli", "Aug", "Sept", "Okt", "Nov", "Dez"};
String val_str = "000";
int dayspermonth_arr[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int time_offset;

#define MY_TIMEZONE "CET-1CEST,M3.5.0/02,M10.5.0/03" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define MY_NTP_SERVER "de.pool.ntp.org"
time_t now;                         // this is the epoch
tm ptm;                             // the structure tm holds time information in a more convient way

#ifdef IV_17
// 16 Segmente laut Datenblatt IV-17
// Bit 0  Seg 2   DP links
// Bit 1  Seg 3   e
// Bit 2  Seg 4   d1
// Bit 3  Seg 5   m
// Bit 4  Seg 6   l
// Bit 5  Seg 7   k
// Bit 6  Seg 8   d2
// Bit 7  Seg 9   c
// Bit 8  Seg 10  DP rechts
// Bit 9  Seg 13  b
// Bit 10 Seg 14  a2
// Bit 11 Seg 15  g2
// Bit 12 Seg 16  j
// Bit 13 Seg 17  i
// Bit 14 Seg 18  h
// Bit 15 Seg 19  g1
// Bit 16 Seg 20  a1
// Bit 17 Seg 21  f
// Einige Sonderzeichen, Ziffern 0..9 und Buchstaben A..Z
const int32 letter2segm[96] = {0x00000, 0x00000, 0x02000, 0x00000, 0x3ACD4, 0x00000, 0x00000, 0x02000, // ab #32 = Blank bis "'"
                               0x02450, 0x12014, 0x00000, 0x0A810, 0x00001, 0x08800, 0x00100, 0x01008, // bis "/", einige Sonderzeichen nicht darstellbar
                               0x316CE, 0x01280, 0x10E4C, 0x11CC4, 0x2A810, 0x38464, 0x38CC6, 0x11410, // #48 ="0"
                               0x38EC6, 0x38EC4, 0x00000, 0x00000, 0x04008, 0x08844, 0x01020, 0x00000, // bis #57 ="9", "<=>"
                               // Großbuchstaben "@, A..Z":
                               0x32E46, 0x38E82, 0x12ED4, 0x30446, 0x126D4, 0x38446, 0x38402, 0x30CC6,
                               0x28A82, 0x12454, 0x002C6, 0x29022, 0x20046, 0x25282, 0x242A2, 0x306C6,
                               0x38E02, 0x306E6, 0x38E22, 0x14CC4, 0x12410, 0x202C6, 0x2100A, 0x202AA,
                               0x05028, 0x05010, 0x1144C, 0x02450, 0x02010, 0x12014, 0x01200, 0x00044,
                               // Kleinbuchstaben "'", "A..Z" als Großbuchstaben
                               0x02000, 0x38E82, 0x12ED4, 0x30446, 0x126D4, 0x38446, 0x38402, 0x30CC6,
                               0x28A82, 0x12454, 0x002C6, 0x29022, 0x20046, 0x25282, 0x242A2, 0x306C6,
                               0x38E02, 0x306E6, 0x38E22, 0x14CC4, 0x12410, 0x202C6, 0x2100A, 0x202AA,
                               0x05028, 0x05010, 0x1144C, 0x02450, 0x02010, 0x12014, 0x240A0, 0x00000};
#else
// Siebensegment-Anzeige
// Off	Off	-	0
// Bit 0	Pin 1	dpr	1
// Bit 1	Pin 2	f	2
// Bit 2	Pin 3	d	4
// Bit 3	Pin 4	c	8
// Bit 4	Pin 7	a	16
// Bit 5	Pin 8	b	32
// Bit 6	Pin 10	e	64
// Bit 7	Pin 11	g	128
const int32 letter2segm[96] = {0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, // ab #32 = Blank bis "'"
                               0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00004, 0x00000, 0x00000, // bis "/", nur "-"
                               0x000FA, 0x0000A, 0x000DC, 0x0009E, 0x0002E, 0x000B6, 0x000F6, 0x0001A, // #48 =" 0"
                               0x000FE, 0x000BE, 0x00000, 0x00000, 0x00000, 0x00006, 0x00000, 0x00000, // bis #57 = "9" und "="

                               0x000FC, 0x0007E, 0x000E6, 0x000C4, 0x000CE, 0x000F4, 0x00074, 0x000F2, // @, A..F, H, I, J, L, O, P, R, T, U, Y darstellbar
                               0x0006E, 0x0000A, 0x0008A, 0x00000, 0x000E0, 0x0007A, 0x00046, 0x000C6,
                               0x0007C, 0x00000, 0x00044, 0x000B6, 0x000E4, 0x000C2, 0x00000, 0x00000,
                               0x00000, 0x000AE, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00002,

                               0x00000, 0x0007E, 0x000E6, 0x000C4, 0x000CE, 0x000F4, 0x00074, 0x000F2, // A..F, H, I, J, L, O, P, R, T, U, Y darstellbar
                               0x0006E, 0x0000A, 0x0008A, 0x00000, 0x000E0, 0x0007A, 0x00046, 0x000C6,
                               0x0007C, 0x00000, 0x00044, 0x000B6, 0x000E4, 0x000C2, 0x00000, 0x00000,
                               0x00000, 0x000AE, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00002};
#endif

// Start Credentials im "EEPROM"; Adresse 0 manchmal korrumpiert
#define EEPROM_CRED 128

// ##############################################################################

/* Load WLAN credentials from "EEPROM" */
void loadCredentials() {
  EEPROM.begin(256);
  EEPROM.get(EEPROM_CRED + 0, ssid);
  EEPROM.get(EEPROM_CRED + 32, password);
  if (EEPROM.read(EEPROM_CRED + 64) != 0x55) {
    ssid[0] = 0;
    password[0] = 0;
  }
  EEPROM.end();
}

/* Store WLAN credentials to "EEPROM" */
void saveCredentials() {
  EEPROM.begin(256);
  EEPROM.put(EEPROM_CRED + 0, ssid);
  EEPROM.put(EEPROM_CRED + 32, password);
  EEPROM.write(EEPROM_CRED + 64, 0x55);
  EEPROM.commit();
  EEPROM.end();
}

int count_days(int day, int month, int year)
{
  int temp_days, i;
  temp_days = 0;
  year = year % 1000;
  for (i = 0; i < year; i++)
  {
    temp_days += 365;
    if (i % 4 == 0)
    {
      temp_days++;
    }
  }
  for (i = 1; i < month; i++)
  {
    temp_days += dayspermonth_arr[i - 1];
  }
  if ((month > 2) && (year % 4 == 0))
  {
    temp_days++;
  }
  temp_days += day - 1;
  return temp_days;
}

// ##############################################################################

void spi_wr_cmd(byte cmd)
{
  digitalWrite(16, LOW);
  SPI.transfer(cmd & 0xFF);
  digitalWrite(16, HIGH);
}

void spi_wr_disp(byte addr, byte data)
{
  spi_wr_cmd(0x44); // Command 2, Write Display, Fixed
  digitalWrite(16, LOW);
  SPI.transfer(addr | 0xC0); // Command 3
  SPI.transfer(data);
  digitalWrite(16, HIGH);
}

void spi_wr_led(byte data)
{
  digitalWrite(16, LOW);
  SPI.transfer(0x45); // Command 2 LED
  SPI.transfer(data);
  digitalWrite(16, HIGH);
}

void spi_send_displ_arr()
{
  spi_wr_cmd(0x40); // Command 2, Write Display, Incremented
  digitalWrite(16, LOW);
  SPI.transfer(0xC0); // Command 3 ab Adresse 0
  for (int i = 0; i < sizeof displ_arr; i++)
    SPI.transfer(displ_arr[i]);
  digitalWrite(16, HIGH);
}

void set_display_mode() {
#ifdef IV_17
  spi_wr_cmd(0x02); // 6 digits, 22 Segments (18 benutzt)
#endif
#ifdef IV_7
  spi_wr_cmd(0x00); // 4 digits, 22 Segments (7 benutzt)
#endif
#ifdef IV_22
  spi_wr_cmd(0x00); // 4 digits, 24 Segments (7 benutzt)
#endif
  spi_wr_cmd(0x8F); // Display ON
}

void set_digit(int pos, char letter, int dp_left, int dp_right)
{
  int arr_ptr = pos * 3;
  int32 segments = letter2segm[letter - 32];
#ifdef IV_17
  if (dp_left)  {
    segments = segments | 0x0001;
  }
  if (dp_right) {
    segments = segments | 0x0100;
  }
#endif
#ifdef IV_22
  if (dp_right) {
    segments = segments | 0x0001;
  }
#endif

  displ_arr[arr_ptr] = segments & 0xFF;
  displ_arr[arr_ptr + 1] = (segments >> 8) & 0xFF;
  displ_arr[arr_ptr + 2] = (segments >> 16) & 0xFF;
}

void divmod_10(int number, int &n1, int &n10)
{
  n1 = number % 10;
  n10 = number / 10;
}

#ifndef IV_17

void set_weekday_chars(int pos, int day_of_week)
{
  char letter = weekday1[day_of_week];
  set_digit(pos, letter, 0, 0); // -64 -1 +11
  letter = weekday2[day_of_week];
  set_digit(pos + 1, letter, 0, 0);
}

#endif

void set_number(int pos, int number, int dp_right)
{
  int number1, number10;
  divmod_10(number, number1, number10);
  set_digit(pos, number10 + 48, 0, 0);
  set_digit(pos + 1, number1 + 48, 0, dp_right);
}

// ##############################################################################

void display_time()
{
  set_number(0, hours, 0);
  set_number(2, minutes, 0);
  set_number(4, seconds, 0);
  spi_send_displ_arr();
}

void clear_disp()
{
  for (int i = 0; i < sizeof displ_arr; i++)
    displ_arr[i] = 0x00;
  spi_send_displ_arr();
}

#ifdef IV_17

void insert_shift_left(char letter)
{
  int arr_ptr;
  for (int i = 0; i < DIGIT_MAX; i++) {
    arr_ptr = i * 3;  // Drei Bytes pro Ziffer
    displ_arr[arr_ptr] = displ_arr[arr_ptr + 3];
    displ_arr[arr_ptr + 1] = displ_arr[arr_ptr + 4];
    displ_arr[arr_ptr + 2] = displ_arr[arr_ptr + 5];
  }
  set_digit(DIGIT_MAX, letter, 0, 0);
  spi_send_displ_arr();
}


#else

void display_date() {
#ifdef IV_17
  set_weekday_chars(0, week_day);
#endif
  set_number(DIGIT_MAX - 3, date_day, 1);
  set_number(DIGIT_MAX - 1, date_month, 1);
  spi_send_displ_arr();
}

#endif

void static_message(String msg) {
  clear_disp();
  for (int i = 0; (i < msg.length()) && (i <= DIGIT_MAX); i++) {
    set_digit(i, msg.charAt(i), 0, 0);
  }
  spi_send_displ_arr();
}

#ifdef IV_17
void running_message(String msg, int speed) {
  clear_disp();
  for (int i = 0; i < msg.length(); i++) {
    delay(speed);
    insert_shift_left(msg.charAt(i));
  }
  for (int i = 0; i < 6; i++) {
    delay(speed);
    insert_shift_left(' ');
  }

}
#endif

// ##############################################################################

void spkr_tick(int duration) {
  digitalWrite(4, HIGH);
  delayMicroseconds(duration);
  digitalWrite(4, LOW);
}

void clock_tick() {
  led_state = !led_state;
  if (led_state) {
   spkr_tick(40);
  } else {
   spkr_tick(20);
  }
}

void spkr_beep(int duration) {
  for (int i = 0; i < duration; i++) {
    digitalWrite(4, HIGH);
    delayMicroseconds(1000);
    digitalWrite(4, LOW);
    delayMicroseconds(2000);
  }
}

// ##############################################################################

bool start_wps() {
  // Startet die WPS-Konfiguration
  bool wpsSuccess = WiFi.beginWPSConfig();
  if (wpsSuccess) {
      // Muss nicht immer erfolgreich heißen! Nach einem Timeout ist die SSID leer
      String newSSID = WiFi.SSID();
      if(newSSID.length() == 0) {
        // Nur wenn eine SSID gefunden wurde waren wir erfolgreich
        wpsSuccess = false;
      }
  }
  return wpsSuccess;
}

bool connect_wps() {
#ifdef IV_17
  running_message("WPS am Router aktivieren - danach INC Button nochmals druecken", 125);
#else
  static_message("ROUT");
#endif
  int i = 0;
  while ((digitalRead(INC_BTN)!=0) && (i < 100)) {
    delay(500);
    set_number(DIGIT_MAX - 1, i, 0);
    spi_send_displ_arr();
    spi_wr_led(i);
    i++;
  }
  if (start_wps()) {
    WiFi.SSID().toCharArray(ssid, WiFi.SSID().length()+1);
    WiFi.psk().toCharArray(password, WiFi.psk().length()+1);
    saveCredentials();
#ifdef IV_17
    running_message("WPS OK - " + String(ssid), 125);
#else
    static_message("0000");
    spkr_beep(50);
#endif
  } else {
#ifdef IV_17
    while (true) {
      running_message("Keine Verbindung mit WPS", 125);
    }
#else
  static_message("ERR ");
#endif
  Serial.println("Keine Verbindung mit WPS");
  }
}

void setup() {
  int i;
  Serial.begin(57600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, LSBFIRST, SPI_MODE3));
  pinMode(16, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(SET_BTN, INPUT_PULLUP); // SET Button
  pinMode(DEC_BTN, INPUT_PULLUP); // DEC Button
  pinMode(INC_BTN, INPUT_PULLUP); // INC Button
  led_state = 0;
  tick_timer.attach(0.5, clock_tick);
  set_display_mode();
  clear_disp();
  spkr_beep(100);

  loadCredentials(); // SSID und Password laden, falls bekannt
  Serial.println("VFD Clock #1.5 by cm@make-magazin.de");
#ifdef IV_17
  Serial.println("IV-17/IV-4 tubes, 6 digits");
#endif
#ifdef IV_7
  Serial.println("IV-7 tubes, 4 digits");
#endif
#ifdef IV_22
  Serial.println("IV-22 tubes, 4 or 2x4 digits");
#endif
  WiFi.mode(WIFI_STA);
#ifdef IV_17
  running_message("Natz-Clock 1.05", 125);
  running_message("Um WLAN zu aendern, INC Button druecken", 125);
  delay(1000);
#else
  static_message("INC-");
  delay(2000);
#endif
  if (digitalRead(INC_BTN) == LOW) {
    connect_wps();
  } else {
    // Wir versuchen eine Anmeldung mit gespeicherten Zugangsdaten
#ifdef IV_17
    running_message("WLAN anmelden - " + String(ssid), 125);
#else
    static_message("--00");
#endif
    WiFi.begin(ssid, password);
  }

  i = 0;
  while ((WiFi.status() != WL_CONNECTED) && (i < 50)){
    delay(250);
    set_number(DIGIT_MAX - 1, i, 0);
    spi_send_displ_arr();
    spi_wr_led(i);
    i++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    connect_wps();
  }
  configTime(MY_TIMEZONE, MY_NTP_SERVER); // --> Here is the IMPORTANT ONE LINER needed in your sketch!
}


// ##############################################################################

void loop() {
//  timeClient.update();
  time(&now);                       // read the current time
  localtime_r(&now, &ptm);           // update the structure tm with the current time
  seconds_old = seconds;
  seconds = ptm.tm_sec;
  if (led_state) {
    spi_wr_led(0);
  } else {
    spi_wr_led(15);
  }
  if (seconds_old != seconds) {
    week_day = ptm.tm_wday;
    date_day = ptm.tm_mday;
    date_month = ptm.tm_mon + 1;
    date_year = ptm.tm_year - 100;  // seit 1900, letzte beiden Ziffern
    hours = ptm.tm_hour;
    minutes = ptm.tm_min;

#ifdef SHOW_DAYSLEFT
    // Alle 30 Sekunden ein wichtiger Hinweis ;-)
    if (seconds % DAYSLEFT_INTERVAL == 0) {
      days_left = count_days(dest_day, dest_month, dest_year) - count_days(date_day, date_month, date_year);
  #ifdef IV_17
      val_str = String(days_left);
      running_message("NUR NOCH " + val_str + " TAGE BIS ZUR RENTE ", 125);
  #else
      spi_wr_led(15);
      set_number(DIGIT_MAX - 1, days_left % 100, 0);
      set_digit(DIGIT_MAX-2, (days_left / 100) + 48, 0, 0);
      set_digit(DIGIT_MAX-3, '-', 0, 0);
      spi_send_displ_arr();
      delay(1500);
  #endif
    }
#endif

#ifdef SHOW_MESSAGE
    // Alle 30 Sekunden ein anderer wichtiger Hinweis ;-)
    if (seconds % MESSAGE_INTERVAL == MESSAGE_INTERVAL / 2) {
      running_message(MESSAGE, 125);
    }
#endif


#ifdef SHOW_DATE
    if (seconds % DATE_INTERVAL == 0) {
#ifdef IV_17
      running_message(day_names[week_day] + "," + String(date_day) + " " + month_names[date_month - 1], 125);
#else
      spi_wr_led(15);
      display_date(); // Kurzform ohne Laufschrift
      delay(1500);
#endif
    } else {
      display_time();
    }
#else
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    display_time();
#endif
  }
  delay(50);
}
