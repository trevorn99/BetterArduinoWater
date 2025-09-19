#include <Wire.h>
#include "U8glib.h"
#include "RTClib.h"

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE); // I2C
RTC_DS1307 RTC;

// Moisture sensors
int moisture1 = A0;
int moisture2 = A1;
int moisture3 = A2;
int moisture4 = A3;

// Moisture values
int moisture1_value = 0;
int moisture2_value = 0;
int moisture3_value = 0;
int moisture4_value = 0;

// Sensor validity flags (1 = connected, 0 = not connected)
bool sensor_active[4] = {true, true, true, true}; // Modify based on connected sensors

// Water relays
int relay1 = 6;
int relay2 = 8;
int relay3 = 9;
int relay4 = 10;

// Water pump
int pump = 4;

// Button
int button = 12;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Pump and relay states (1: open, 0: close)
int pump_state_flag = 0;
int relay1_state_flag = 0;
int relay2_state_flag = 0;
int relay3_state_flag = 0;
int relay4_state_flag = 0;

// Display update optimization
int lastMoisture1 = -1, lastMoisture2 = -1, lastMoisture3 = -1, lastMoisture4 = -1;
unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 1000; // Update every 1 second

// Days of the week
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"};

// Good flower bitmap
unsigned char bitmap_good[] U8G_PROGMEM = {
  0x00, 0x42, 0x4C, 0x00, 0x00, 0xE6, 0x6E, 0x00, 0x00, 0xAE, 0x7B, 0x00, 0x00, 0x3A, 0x51, 0x00,
  0x00, 0x12, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x06, 0x40, 0x00, 0x00, 0x06, 0x40, 0x00,
  0x00, 0x04, 0x60, 0x00, 0x00, 0x0C, 0x20, 0x00, 0x00, 0x08, 0x30, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0xE0, 0x0F, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0xC1, 0x00, 0x00, 0x0E, 0x61, 0x00,
  0x00, 0x1C, 0x79, 0x00, 0x00, 0x34, 0x29, 0x00, 0x00, 0x28, 0x35, 0x00, 0x00, 0x48, 0x17, 0x00,
  0x00, 0xD8, 0x1B, 0x00, 0x00, 0x90, 0x1B, 0x00, 0x00, 0xB0, 0x09, 0x00, 0x00, 0xA0, 0x05, 0x00,
  0x00, 0xE0, 0x07, 0x00, 0x00, 0xC0, 0x03, 0x00
};

// Bad flower bitmap
unsigned char bitmap_bad[] U8G_PROGMEM = {
  0x00, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xE0, 0x0D, 0x00, 0x00, 0xA0, 0x0F, 0x00,
  0x00, 0x20, 0x69, 0x00, 0x00, 0x10, 0x78, 0x02, 0x00, 0x10, 0xC0, 0x03, 0x00, 0x10, 0xC0, 0x03,
  0x00, 0x10, 0x00, 0x01, 0x00, 0x10, 0x80, 0x00, 0x00, 0x10, 0xC0, 0x00, 0x00, 0x30, 0x60, 0x00,
  0x00, 0x60, 0x30, 0x00, 0x00, 0xC0, 0x1F, 0x00, 0x00, 0x60, 0x07, 0x00, 0x00, 0x60, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xC7, 0x1C, 0x00,
  0x80, 0x68, 0x66, 0x00, 0xC0, 0x33, 0x7B, 0x00, 0x40, 0xB6, 0x4D, 0x00, 0x00, 0xE8, 0x06, 0x00,
  0x00, 0xF0, 0x03, 0x00, 0x00, 0xE0, 0x00, 0x00
};

// Elecrow Logo
static unsigned char bitmap_logo[] U8G_PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xE0,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x04,0xF8,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x08,0xFE,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x10,0x1F,0xE0,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xB0,0x07,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xE0,0x03,0x00,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0xC0,0x00,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x80,0x01,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x60,0x23,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0xC7,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x9E,0x0F,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x3C,0xFE,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x70,0x78,0xF8,0x7F,0xF0,0x9F,0x07,0xFE,0x83,0x0F,0xFF,0x00,0x77,0x3C,0x18,0x1C,
  0x70,0xF0,0xE1,0x3F,0xF1,0x9F,0x07,0xFE,0xE1,0x1F,0xFF,0xC3,0xF7,0x3C,0x38,0x0C,
  0x70,0xE0,0x87,0x8F,0xF1,0xC0,0x07,0x1E,0x70,0x3C,0xCF,0xE3,0xE1,0x7D,0x3C,0x0E,
  0x70,0xD0,0x1F,0xC0,0xF1,0xC0,0x03,0x1F,0x78,0x3C,0xCF,0xE3,0xE1,0x7D,0x3C,0x06,
  0xF0,0xB0,0xFF,0xF1,0xF0,0xC0,0x03,0x0F,0x78,0x3C,0xCF,0xF3,0xE0,0x7B,0x3E,0x06,
  0xF0,0x60,0xFF,0xFF,0xF0,0xC6,0x03,0xEF,0x3C,0x80,0xEF,0xF1,0xE0,0x7B,0x3E,0x03,
  0xF0,0xE1,0xFC,0xFF,0xF8,0xCF,0x03,0xFF,0x3C,0x80,0xFF,0xF0,0xE0,0x7B,0x7B,0x01,
  0xE0,0xC3,0xF9,0x7F,0x78,0xC0,0x03,0x0F,0x3C,0x80,0xF7,0xF1,0xE0,0xF9,0xF9,0x01,
  0xE0,0x83,0xE3,0x7F,0x78,0xE0,0x03,0x0F,0x3C,0xBC,0xE7,0xF1,0xE0,0xF9,0xF9,0x00,
  0xC0,0x0F,0x8F,0x3F,0x78,0xE0,0x81,0x0F,0x3C,0x9E,0xE7,0xF1,0xE0,0xF1,0xF8,0x00,
  0x80,0x3F,0x1E,0x00,0x78,0xE0,0x81,0x07,0x38,0x9E,0xE7,0xF1,0xF0,0xF0,0x78,0x00,
  0x80,0xFF,0xFF,0x00,0xF8,0xEF,0xBF,0xFF,0xF8,0xCF,0xE7,0xE1,0x7F,0x70,0x70,0x00,
  0x00,0xFF,0xFF,0x0F,0xF8,0xEF,0xBF,0xFF,0xE0,0xC3,0xE3,0x81,0x1F,0x70,0x30,0x00,
  0x00,0xFC,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xF8,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0xE0,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void setup() {
  // Display initial Elecrow logo
  u8g.firstPage();
  do {
    draw_elecrow();
  } while (u8g.nextPage());
  delay(2000);

  // Initialize I2C, RTC, and Serial
  Wire.begin();
  RTC.begin();
  Serial.begin(9600);

  // Declare relay pins as output
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Declare pump as output
  pinMode(pump, OUTPUT);

  // Declare button as input
  pinMode(button, INPUT);
}

void loop() {
  read_value();
  water_flower();

  // Button debouncing
  int reading = digitalRead(button);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Update display only if values have changed or interval has passed
    if (millis() - lastDisplayUpdate >= displayInterval ||
        lastMoisture1 != moisture1_value || lastMoisture2 != moisture2_value ||
        lastMoisture3 != moisture3_value || lastMoisture4 != moisture4_value) {
      u8g.firstPage();
      do {
        if (reading == HIGH) {
          drawTH();
          drawflower();
        } else {
          drawtime();
          u8g.drawStr(8, 55, "www.elecrow.com");
        }
      } while (u8g.nextPage());
      lastDisplayUpdate = millis();
      lastMoisture1 = moisture1_value;
      lastMoisture2 = moisture2_value;
      lastMoisture3 = moisture3_value;
      lastMoisture4 = moisture4_value;
    }
  }
  lastButtonState = reading;
}

void read_value() {
  const int min_valid_reading = 360; // Updated minimum valid ADC reading
  const int max_valid_reading = 600; // Updated maximum valid ADC reading
  const int num_samples = 20; // Number of samples for averaging
  const int sample_delay = 10; // Delay between samples in ms
  const int max_valid_range = 10; // Maximum allowed range for connected sensors

  // Sensor A0
  float total1 = 0;
  int min1 = 1023, max1 = 0;
  for (int i = 0; i < num_samples; i++) {
    int reading = analogRead(moisture1);
    total1 += reading;
    if (reading < min1) min1 = reading;
    if (reading > max1) max1 = reading;
    delay(sample_delay);
  }
  float average1 = total1 / num_samples;
  int range1 = max1 - min1;
  if (average1 >= min_valid_reading && average1 <= max_valid_reading && range1 <= max_valid_range) {
    moisture1_value = map(average1, 600, 360, 0, 100);
    if (moisture1_value < 0) moisture1_value = 0;
    sensor_active[0] = true;
  } else {
    moisture1_value = 100; // Safe value to prevent watering
    sensor_active[0] = false;
  }
  Serial.print("A0 Avg: "); Serial.print(average1); Serial.print(" Range: "); Serial.print(range1); Serial.print(" Active: "); Serial.println(sensor_active[0]);

  // Sensor A1
  float total2 = 0;
  int min2 = 1023, max2 = 0;
  for (int i = 0; i < num_samples; i++) {
    int reading = analogRead(moisture2);
    total2 += reading;
    if (reading < min2) min2 = reading;
    if (reading > max2) max2 = reading;
    delay(sample_delay);
  }
  float average2 = total2 / num_samples;
  int range2 = max2 - min2;
  if (average2 >= min_valid_reading && average2 <= max_valid_reading && range2 <= max_valid_range) {
    moisture2_value = map(average2, 600, 360, 0, 100);
    if (moisture2_value < 0) moisture2_value = 0;
    sensor_active[1] = true;
  } else {
    moisture2_value = 100;
    sensor_active[1] = false;
  }
  Serial.print("A1 Avg: "); Serial.print(average2); Serial.print(" Range: "); Serial.print(range2); Serial.print(" Active: "); Serial.println(sensor_active[1]);

  // Sensor A2
  float total3 = 0;
  int min3 = 1023, max3 = 0;
  for (int i = 0; i < num_samples; i++) {
    int reading = analogRead(moisture3);
    total3 += reading;
    if (reading < min3) min3 = reading;
    if (reading > max3) max3 = reading;
    delay(sample_delay);
  }
  float average3 = total3 / num_samples;
  int range3 = max3 - min3;
  if (average3 >= min_valid_reading && average3 <= max_valid_reading && range3 <= max_valid_range) {
    moisture3_value = map(average3, 600, 360, 0, 100);
    if (moisture3_value < 0) moisture3_value = 0;
    sensor_active[2] = true;
  } else {
    moisture3_value = 100;
    sensor_active[2] = false;
  }
  Serial.print("A2 Avg: "); Serial.print(average3); Serial.print(" Range: "); Serial.print(range3); Serial.print(" Active: "); Serial.println(sensor_active[2]);

  // Sensor A3
  float total4 = 0;
  int min4 = 1023, max4 = 0;
  for (int i = 0; i < num_samples; i++) {
    int reading = analogRead(moisture4);
    total4 += reading;
    if (reading < min4) min4 = reading;
    if (reading > max4) max4 = reading;
    delay(sample_delay);
  }
  float average4 = total4 / num_samples;
  int range4 = max4 - min4;
  if (average4 >= min_valid_reading && average4 <= max_valid_reading && range4 <= max_valid_range) {
    moisture4_value = map(average4, 600, 360, 0, 100);
    if (moisture4_value < 0) moisture4_value = 0;
    sensor_active[3] = true;
  } else {
    moisture4_value = 100;
    sensor_active[3] = false;
  }
  Serial.print("A3 Avg: "); Serial.print(average4); Serial.print(" Range: "); Serial.print(range4); Serial.print(" Active: "); Serial.println(sensor_active[3]);
}

void water_flower() {
  static unsigned long lastRelayTime = 0;
  const unsigned long relayDelay = 50;

  if (millis() - lastRelayTime < relayDelay) return;
  lastRelayTime = millis();

  // Sensor 1
  if (sensor_active[0] && moisture1_value < 30) {
    digitalWrite(relay1, HIGH);
    relay1_state_flag = 1;
    if (pump_state_flag == 0) {
      digitalWrite(pump, HIGH);
      pump_state_flag = 1;
    }
  } else if (sensor_active[0] && moisture1_value > 55) {
    digitalWrite(relay1, LOW);
    relay1_state_flag = 0;
  }

  // Sensor 2
  if (sensor_active[1] && moisture2_value < 30) {
    digitalWrite(relay2, HIGH);
    relay2_state_flag = 1;
    if (pump_state_flag == 0) {
      digitalWrite(pump, HIGH);
      pump_state_flag = 1;
    }
  } else if (sensor_active[1] && moisture2_value > 55) {
    digitalWrite(relay2, LOW);
    relay2_state_flag = 0;
  }

  // Sensor 3
  if (sensor_active[2] && moisture3_value < 30) {
    digitalWrite(relay3, HIGH);
    relay3_state_flag = 1;
    if (pump_state_flag == 0) {
      digitalWrite(pump, HIGH);
      pump_state_flag = 1;
    }
  } else if (sensor_active[2] && moisture3_value > 55) {
    digitalWrite(relay3, LOW);
    relay3_state_flag = 0;
  }

  // Sensor 4
  if (sensor_active[3] && moisture4_value < 30) {
    digitalWrite(relay4, HIGH);
    relay4_state_flag = 1;
    if (pump_state_flag == 0) {
      digitalWrite(pump, HIGH);
      pump_state_flag = 1;
    }
  } else if (sensor_active[3] && moisture4_value > 55) {
    digitalWrite(relay4, LOW);
    relay4_state_flag = 0;
  }

  // Turn off pump only if all active relays are off
  bool all_relays_off = true;
  for (int i = 0; i < 4; i++) {
    if (sensor_active[i] && (i == 0 && relay1_state_flag == 1 ||
                             i == 1 && relay2_state_flag == 1 ||
                             i == 2 && relay3_state_flag == 1 ||
                             i == 3 && relay4_state_flag == 1)) {
      all_relays_off = false;
      break;
    }
  }
  if (all_relays_off) {
    digitalWrite(pump, LOW);
    pump_state_flag = 0;
  }
}

void draw_elecrow(void) {
  u8g.setFont(u8g_font_gdr9r);
  u8g.drawStr(8, 55, "www.elecrow.com");
  u8g.drawXBMP(0, 5, 128, 32, bitmap_logo);
}

void drawtime(void) {
  int x = 5;
  DateTime now = RTC.now();
  if (!RTC.isrunning()) {
    u8g.setFont(u8g_font_6x10);
    u8g.setPrintPos(5, 20);
    u8g.print("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  } else {
    u8g.setFont(u8g_font_7x13);
    u8g.setPrintPos(x, 11);
    u8g.print(now.year(), DEC);
    u8g.setPrintPos(x + 80, 11);
    u8g.print(daysOfTheWeek[now.dayOfTheWeek()]);
    u8g.setPrintPos(x + 28, 11);
    u8g.print("/");
    u8g.setPrintPos(x + 33, 11);
    u8g.print(now.month(), DEC);
    if (now.month() < 10) x -= 7;
    u8g.setPrintPos(x + 47, 11);
    u8g.print("/");
    u8g.setPrintPos(x + 53, 11);
    u8g.print(now.day(), DEC);
    u8g.setFont(u8g_font_8x13);
    x = 35;
    u8g.setPrintPos(x, 33);
    u8g.print(now.hour(), DEC);
    if (now.hour() < 10) x -= 7;
    u8g.setPrintPos(x + 15, 33);
    u8g.print(":");
    u8g.setPrintPos(x + 21, 33);
    u8g.print(now.minute(), DEC);
    if (now.minute() < 10) x -= 7;
    u8g.setPrintPos(x + 36, 33);
    u8g.print(":");
    u8g.setPrintPos(x + 42, 33);
    u8g.print(now.second(), DEC);
  }
}

void drawflower(void) {
  if (sensor_active[0]) {
    u8g.drawXBMP(0, 0, 32, 30, moisture1_value < 30 ? bitmap_bad : bitmap_good);
  }
  if (sensor_active[1]) {
    u8g.drawXBMP(32, 0, 32, 30, moisture2_value < 30 ? bitmap_bad : bitmap_good);
  }
  if (sensor_active[2]) {
    u8g.drawXBMP(64, 0, 32, 30, moisture3_value < 30 ? bitmap_bad : bitmap_good);
  }
  if (sensor_active[3]) {
    u8g.drawXBMP(96, 0, 32, 30, moisture4_value < 30 ? bitmap_bad : bitmap_good);
  }
}

void drawTH(void) {
  char moisture_value_temp[5] = {0};
  u8g.setFont(u8g_font_7x14);

  // Sensor A0
  u8g.setPrintPos(9, 60);
  u8g.print("A0");
  if (sensor_active[0]) {
    if (moisture1_value >= 100) moisture1_value = 100;
    itoa(moisture1_value, moisture_value_temp, 10);
    u8g.drawStr(moisture1_value < 10 ? 14 : 7, 45, moisture_value_temp);
    u8g.setPrintPos(23, 45);
    u8g.print("%");
  } else {
    u8g.drawStr(7, 45, "N/A");
  }

  // Sensor A1
  u8g.setPrintPos(41, 60);
  u8g.print("A1");
  if (sensor_active[1]) {
    if (moisture2_value >= 100) moisture2_value = 100;
    itoa(moisture2_value, moisture_value_temp, 10);
    u8g.drawStr(moisture2_value < 10 ? 46 : 39, 45, moisture_value_temp);
    u8g.setPrintPos(54, 45);
    u8g.print("%");
  } else {
    u8g.drawStr(39, 45, "N/A");
  }

  // Sensor A2
  u8g.setPrintPos(73, 60);
  u8g.print("A2");
  if (sensor_active[2]) {
    if (moisture3_value >= 100) moisture3_value = 100;
    itoa(moisture3_value, moisture_value_temp, 10);
    u8g.drawStr(moisture3_value < 10 ? 14 + 64 : 7 + 64, 45, moisture_value_temp);
    u8g.setPrintPos(23 + 64, 45);
    u8g.print("%");
  } else {
    u8g.drawStr(7 + 64, 45, "N/A");
  }

  // Sensor A3
  u8g.setPrintPos(105, 60);
  u8g.print("A3");
  if (sensor_active[3]) {
    if (moisture4_value >= 100) moisture4_value = 100;
    itoa(moisture4_value, moisture_value_temp, 10);
    u8g.drawStr(moisture4_value < 10 ? 14 + 96 : 7 + 96, 45, moisture_value_temp);
    u8g.setPrintPos(23 + 96, 45);
    u8g.print("%");
  } else {
    u8g.drawStr(7 + 96, 45, "N/A");
  }
}
