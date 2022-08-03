/*
  objectives
  1. Check if a car is being detected, if no turn off green LED
  2. if yes, turn on the green LED and ask for the passowrd
  3. if the password is entered, turn on the dc motors and lit the red LED
  4. if the door has contact with the switches turn off the dc motor and red LED
  5. if the 2nd PIR is triggered, ask if want to close
  6. if user answer is yes then the dc will turn on (indicating the garage is closing)
   if user answer is no then do nothing

*/

#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

LiquidCrystal_I2C lcd(32, 16, 2);

// PASSWORD VARIABLES
char password[6] = "01234";
char userPassword[6];


// PIR variable
int PIRpin1 = 2;
int pirVal1;
int PIRpin2 = 12;
int pirVal2;

// IR variables
int IRpin = 3;
IRrecv IR(IRpin);

// LED variables
int greenLEDpin = 8;
int redLEDpin = 11;

// relay variable
int dc_relayPin = 7;

// contact switch (slide switch) variable
int csPin = 10;
int csVal;
int csPin2 = 9;
int csVal2;

// remote control debounce variable
unsigned long currentTime;
unsigned long prevDebounce = 0;
int debounceInterval = 50;

// closing delay variable
unsigned long closing_currentTime;
unsigned long closing_prevDebounce;

// closing delay variable
unsigned long closing_delay;
unsigned long closing_prevTime;

// Ask variables
String askA1 = "Close the Door?";

// Parameters
bool trigger;
char userChoice;
char remoteRead;
byte userPassCount = 0;
char ip;

// state variable
bool state = 0;
bool remoteState = true;
/***************************************************************************************/

void setup()
{
  Serial.begin(9600);
  pinMode(greenLEDpin, OUTPUT);
  pinMode(redLEDpin, OUTPUT);
  pinMode(dc_relayPin, OUTPUT);
  pinMode(csPin, INPUT);
  pinMode(csPin2, INPUT);
  pinMode(PIRpin1, INPUT);
  pinMode(PIRpin2, INPUT);
  pinMode(IRpin, INPUT);
  IR.enableIRIn();

  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setBacklight(0);
}

/***************************************************************************************/

void loop()
{
  currentTime = millis();
  Outside_PIRdetection();
  Inside_PIRdetection();
}

/***************************************************************************************/

void remoteControl()
{
  switch (IR.decodedIRData.decodedRawData)
  {
    case 0xFF00BF00:
      Serial.println("Power");
      break;

    case 0xFB04BF00:
      Serial.println("Rewind");
      userChoice = '<';
      break;

    case 0xF906BF00:
      Serial.println("Fast forward");
      userChoice = '>';
      break;

    case 0xF30CBF00:
      Serial.println("0");
      ip = '0';
      break;

    case 0xEF10BF00:
      Serial.println("1");
      ip = '1';
      break;

    case 0xEE11BF00:
      Serial.println("2");
      ip = '2';
      break;

    case 0xED12BF00:
      Serial.println("3");
      ip = '3';
      break;

    case 0xEB14BF00:
      Serial.println("4");
      ip = '4';
      break;

    case 0xEA15BF00:
      Serial.println("5");
      ip = '5';
      break;

    case 0xE916BF00:
      Serial.println("6");
      ip = '6';
      break;

    case 0xE718BF00:
      Serial.println("7");
      ip = '7';
      break;

    case 0xE619BF00:
      Serial.println("8");
      ip = '8';
      break;

    case 0xE51ABF00:
      Serial.println("9");
      ip = '9';
      break;
  }
}

/***************************************************************************************/

void AskPassword()
{
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");

  while (IR.decode())
  {
    if (currentTime - prevDebounce >= debounceInterval)
    {
      prevDebounce = currentTime;
      remoteControl();
      Store_userPass();
    }
    IR.resume();
  }
}

/***************************************************************************************/

void Store_userPass()
{
  // Grabs the user's input password and store it on the array userPassword
  userPassword[userPassCount] = ip;
  lcd.setCursor(userPassCount, 1);
  lcd.print(userPassword[userPassCount]);
  userPassCount++;
}

/***************************************************************************************/

void checkPassword()
{
  if (userPassCount == 5)
  {
    if (!strcmp(userPassword, password))
    {
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("SUCCESS!");
      lcd.setCursor(0, 1);
      lcd.print("Correct");
      lcd.setCursor(8, 1);
      lcd.print("Password");
      delay(1000);
      Opening();
      lcd.clear();
    }
    else
    {
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Incorrect");
      lcd.setCursor(4, 1);
      lcd.print("Password");
      delay(1000);
      lcd.clear();
      reset_Parameters();
    }
  }
}

/***************************************************************************************/

void reset_Parameters()
{
  trigger = 0;
  userChoice = 0;
  remoteRead = 0;
  ip = 0;

  userPassCount = 0;
  for (int i = 0; i < 6; i++)
  {
    userPassword[i] = 0;
  }
}

/***************************************************************************************/

void Outside_PIRdetection()
{
  pirVal1 = digitalRead(PIRpin1);

  if (pirVal1 == 1)
  {
    lcd.setBacklight(1);
    digitalWrite(greenLEDpin, HIGH);

    AskPassword();
    checkPassword();
  }
  else
  {
    digitalWrite(greenLEDpin, LOW);
    reset_Parameters();
  }
}

/***************************************************************************************/

void contactSwitch()
{
  csVal = digitalRead(csPin);
  csVal2 = digitalRead(csPin2);
}

/***************************************************************************************/

void PIR_Readings()
{
  pirVal1 = digitalRead(PIRpin1);
  pirVal2 = digitalRead(PIRpin2);
}

/***************************************************************************************/

void Opening()
{
  contactSwitch();
  if (csVal == 0 && csVal2 == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Opening...");
    digitalWrite(dc_relayPin, HIGH);
    digitalWrite(redLEDpin, HIGH);
    while (csVal != 1) {contactSwitch();}
  }
  while (csVal == 1 && csVal2 == 1)
  {
    Opening();
  }

  if (csVal == 1 && csVal2 == 0)
  {
    PIR_Readings();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Garage is Open");
    digitalWrite(dc_relayPin, LOW);
    digitalWrite(redLEDpin, LOW);
    while (pirVal2 != 1) {PIR_Readings();}
  }
}

/***************************************************************************************/

void Inside_PIRdetection()
{
  PIR_Readings();

  if (pirVal1 == 1 && pirVal2 == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Car detected");
    trigger = 1;
    while (pirVal1 != 0) {PIR_Readings();}
  }
  if (pirVal2 == 1 && trigger == 1)
  {
    digitalWrite(greenLEDpin, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(askA1);
    lcd.setCursor(0, 1);
    lcd.print("[<]YES,[>]NO");
    while (IR.decode() == 0) {}
    Store_userAnswer();
  }
}

/***************************************************************************************/

void Store_userAnswer()
{
  closing_currentTime = millis();
  while (IR.decode())
  {
    if (closing_currentTime - closing_prevDebounce >= debounceInterval)
    {
      closing_prevDebounce = closing_currentTime;
      remoteControl();
      User_answer();
    }
    IR.resume();
  }
}

/***************************************************************************************/


void User_answer()
{
  while (userChoice == '<')
  {
    Closing();
  }
  while (userChoice == '>')
  {
    Store_userAnswer();
  }
}

/***************************************************************************************/

void Closing()
{
  contactSwitch();
  int ClosingTrig;
  
  if (csVal == 1 && csVal2 == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Closing...");
    digitalWrite(dc_relayPin, HIGH);
    digitalWrite(redLEDpin, HIGH);
    ClosingTrig = 1;
    while (csVal != 0) {contactSwitch();}
  }
  while (ClosingTrig == 1 && csVal2 == 0)
  {
    Closing();
  }
  if (ClosingTrig == 1 && csVal2 == 1)
  {
    PIR_Readings();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Garage is Close");
    digitalWrite(dc_relayPin, LOW);
    digitalWrite(redLEDpin, LOW);
    while (pirVal2 != 0) {PIR_Readings();}
    restart_program();
  }
}

/***************************************************************************************/

void restart_program()
{
  lcd.clear();
  lcd.setBacklight(0);
  reset_Parameters();
  while (pirVal1 != 1) {PIR_Readings();}
  loop();
}
