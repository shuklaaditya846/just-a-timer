#include "AiEsp32RotaryEncoder.h"
#include "bitmaps.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Buzzer response
#define BUZZER D0


#if defined(ESP8266)
#define ROTARY_ENCODER_A_PIN D6 //CLK
#define ROTARY_ENCODER_B_PIN D5 //DT
#define ROTARY_ENCODER_BUTTON_PIN D7
#else
#define ROTARY_ENCODER_A_PIN 32
#define ROTARY_ENCODER_B_PIN 21
#define ROTARY_ENCODER_BUTTON_PIN 25
#endif
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 8

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);


///////////////////////////////////////////////////////////////////////////////////////////

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


///////////////////
int hr = 00;
int mn = 00;
int sc = 00;

int a = 1;

bool hr_edit = true;
bool mn_edit = false;
bool sc_edit = false;
///////////////////



void rotary_onButtonClick()
{
  display.invertDisplay(true);
  delay(50);
  display.invertDisplay(false);
  a++;
  Serial.println(a);
  //  if (a == 1) { //hours
  //    rotaryEncoder.setEncoderValue(0);
  //    hr_edit = true;
  //    mn_edit = false;
  //    sc_edit = false;
  //  }
  if (a == 2) { //min
    rotaryEncoder.setEncoderValue(0);
    hr_edit = false;
    mn_edit = true;
    sc_edit = false;
  }
  else if (a == 3) { //sec
    rotaryEncoder.setEncoderValue(0);
    hr_edit = false;
    mn_edit = false;
    sc_edit = true;
  }
  else if (a == 4 && (hr + mn + sc) > 0 ) { //start countdown
    countdown();
    times_up();
    rotaryEncoder.setEncoderValue(0);
    hr = 0;
    mn = 0;
    sc = 0;
    a = 1;
    hr_edit = true;
    mn_edit = false;
    sc_edit = false;
  }
  else {
    a = 1;
    hr_edit = true;
    mn_edit = false;
    sc_edit = false;
  }
}




void rotary_loop()
{
  //dont print anything unless value changed
  if (hr_edit == true && rotaryEncoder.encoderChanged()) {
    hr = rotaryEncoder.readEncoder();
  }
  else if (mn_edit == true && rotaryEncoder.encoderChanged()) {
    mn = rotaryEncoder.readEncoder();
  }
  else if (sc_edit == true && rotaryEncoder.encoderChanged()) {
    sc = rotaryEncoder.readEncoder();
  }
}



void IRAM_ATTR readEncoderISR()
{
  rotaryEncoder.readEncoder_ISR();
}



//////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  bool circleValues = false;

  rotaryEncoder.setBoundaries(0, 59, false);
  rotaryEncoder.setAcceleration(10);
}




/////////////////////////////////////////////////////////////////


void loop() {
  rotary_loop();
  homescreen();
  if (digitalRead(ROTARY_ENCODER_BUTTON_PIN) == 0) {
    delay(300);
    if (digitalRead(ROTARY_ENCODER_BUTTON_PIN) == 0) {
      delay(2000);
      if (digitalRead(ROTARY_ENCODER_BUTTON_PIN) == 0) {
        help_menu();
      }
    }
    else {
      rotary_onButtonClick();
    }
  }
}



//////////////////////////////////////////////////////////////////////////////////

void help_menu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Press to cycle\nbetween hr, mn\nand sec");
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Press again to\nstart timer");
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Hold for 1.5 seconds\nto reset the\ntimer.");
  display.display();
  delay(3000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Hold longer to see\nthis help menu.");
  display.display();
  delay(3000);
  display.clearDisplay();
}


void homescreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.drawRoundRect(0, 0, 128, 32, 5, SSD1306_WHITE);
  display.setCursor(25, 23);
  display.setTextSize(1);
  display.print("hr     mn    sec");

  display.setTextSize(2);
  // hours
  display.setCursor(5, 5);
  display.print(hr);
  // minutes
  display.setCursor(48, 5);
  display.print(mn);
  // seconds
  display.setCursor(86, 5);
  display.print(sc);

  display.setTextSize(1);
  if (hr_edit == true) {
    display.setCursor(9, 22);
    display.write(24);
  } else if (mn_edit == true) {
    display.setCursor(52, 22);
    display.write(24);
  } else if (sc_edit == true) {
    display.setCursor(90, 22);
    display.write(24);
  }
  display.display();
}



void countdown() {
  int count = (60 * 60 * hr) + (60 * mn) + (sc);
  while (count > 0) {
    /* I was working with nodemcu (esp8266) and this rotating loop animation caused uC forced reset*/
    //    for (int z = 0; z <= 5; z++) {
    //      display.clearDisplay();
    //      display.drawBitmap(103, 2, bitmap_allArray[z], 25, 25, SSD1306_WHITE);
    //      display.setTextSize(2);
    //      display.setCursor(20, 8);
    //      display.print(count);
    //      display.print(" sec");
    //      display.display();
    //      delay(975);
    //      count--;
    //    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 8);
    display.print(count);
    display.print(" sec");
    display.display();
    delay(975);
    count--;
    ESP.wdtFeed();
  }
  display.clearDisplay();
}



void times_up() {
  display.setTextSize(2);
  while (digitalRead(ROTARY_ENCODER_BUTTON_PIN) == 1) {
    display.setCursor(20, 8);
    display.println("TIME'S UP");
    display.display();
    tone(BUZZER, 4000);
    delay(100);
    noTone(BUZZER);
    delay(100);
    tone(BUZZER, 4000);
    delay(100);
    noTone(BUZZER);
    delay(100);
    tone(BUZZER, 4000);
    delay(100);
    noTone(BUZZER);
    display.clearDisplay();
    display.display();
    delay(1000);
  }
  display.setTextSize(1);
  display.clearDisplay();
  return;
}
