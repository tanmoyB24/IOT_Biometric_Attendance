//Author: Tanmoy Bhowmik
//Dept. of Physics, SUST


#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_Fingerprint.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

// Pin definitions
#define Finger_Rx D5
#define Finger_Tx D6
#define ATTEND_BUTTON D7  // GPIO13
// WiFi credentials
const char *ssid = "LAPTOP-0MASE292 4922";
const char *password = "9S728j*2";

// Google Script Deployment ID - you'll need to replace this with your new script ID after deployment
const char *GScriptId = "AKfycbzw_vZy7p04ZIjhNaozpzxGMXCUZzC1WbJUMLH24rxvCst3-Dc53txcsvT9N-HYlrkg";

// Google Sheets setup
const char *host = "script.google.com";
const int httpsPort = 443;
String url = String("/macros/s/") + GScriptId + "/exec";

// OLED display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Fingerprint sensor setup
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Global variables
int u = 0;
int v = 0;
int count = 0;

uint8_t readnumber()
{
  uint8_t num = 0;
  while (num == 0)
  {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll(uint8_t id)
{
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }

  Serial.println("Place same finger again");
  p = -1;
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

void enrollFingerprint()
{
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  uint8_t id = readnumber();
  if (id == 0)
  { // ID #0 not allowed
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!getFingerprintEnroll(id))
    ;
}

// Updated function to mark attendance with the new column-based system
void updateAttendance(String userName, String studentId)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    HTTPClient http;

    // Construct the full URL
    String fullUrl = "https://" + String(host) + url;

    // Begin the HTTP request
    client.setInsecure();        // Ignore SSL certificate validation
    http.begin(client, fullUrl); // Use WiFiClientSecure with HTTPClient
    http.addHeader("Content-Type", "application/json");

    // Build the payload with the new column-based format
    String payload = "{\"command\": \"column_attendance\", \"sheet_name\": \"Attendance\", "
                     "\"student_id\": \"" +
                     studentId + "\", "
                                 "\"student_name\": \"" +
                     userName + "\", "
                                "\"status\": \"present\"}";

    Serial.println("Publishing attendance data...");
    Serial.println(payload);

    // Send the POST request
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    }
    else
    {
      Serial.println("Error publishing data. HTTP Response code: " + String(httpResponseCode));
    }
    http.end(); // Free resources
  }
  else
  {
    Serial.println("WiFi not connected");
  }
}

int getFingerprintID()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1;

  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  return finger.fingerID;
}

// Function to add attendance
void addAttendance(int fingerprintID)
{
  String userName;
  String studentId;

  switch (fingerprintID)
  {
  case 1:
    userName = "Arik";
    studentId = "1";
    break;
  case 2:
    userName = "NNN";
    studentId = "2";
    break;
  case 3:
    userName = "33";
    studentId = "3";
    break;
  case 4:
    userName = "NNN";
    studentId = "4";
    break;
  case 5:
    userName = "NNN";
    studentId = "5";
    break;
  case 6:
    userName = "NNN";
    studentId = "6";
    break;
  case 7:
    userName = "NNN";
    studentId = "7";
    break;
  case 8:
    userName = "NNN";
    studentId = "8";
    break;
  case 9:
    userName = "Alpha";
    studentId = "9";
    break;
  case 10:
    userName = "Beta";
    studentId = "10";
    break;
  case 11:
    userName = "Gamma";
    studentId = "11";
    break;
  case 12:
    userName = "Delta";
    studentId = "12";
    break;
  case 13:
    userName = "Epsilon";
    studentId = "13";
    break;
  case 14:
    userName = "Zeta";
    studentId = "14";
    break;
  case 15:
    userName = "Eta";
    studentId = "15";
    break;
  case 16:
    userName = "Theta";
    studentId = "16";
    break;
  case 17:
    userName = "Iota";
    studentId = "17";
    break;
  case 18:
    userName = "Kappa";
    studentId = "18";
    break;
  case 19:
    userName = "Spiderman";
    studentId = "19";
    break;
  case 20:
    userName = "Lambda";
    studentId = "20";
    break;
  case 21:
    userName = "Mu";
    studentId = "21";
    break;
  case 22:
    userName = "Nu";
    studentId = "22";
    break;
  case 23:
    userName = "Xi";
    studentId = "23";
    break;
  case 24:
    userName = "Sazid Hassan";
    studentId = "24";
    break;
  case 25:
    userName = "Omicron";
    studentId = "25";
    break;
  case 26:
    userName = "Pi";
    studentId = "26";
    break;
  case 27:
    userName = "Rho";
    studentId = "27";
    break;
  case 28:
    userName = "Sigma";
    studentId = "28";
    break;
  case 29:
    userName = "Tau";
    studentId = "29";
    break;
  case 30:
    userName = "Upsilon";
    studentId = "30";
    break;
  case 31:
    userName = "Phi";
    studentId = "31";
    break;
  case 32:
    userName = "Chi";
    studentId = "32";
    break;
  case 33:
    userName = "33";
    studentId = "33";
    break;
  case 34:
    userName = "Psi";
    studentId = "34";
    break;
  case 35:
    userName = "Omega";
    studentId = "35";
    break;
  case 36:
    userName = "Sirat";
    studentId = "36";
    break;
  case 37:
    userName = "Zion";
    studentId = "37";
    break;
  case 38:
    userName = "Foysah Hassan";
    studentId = "38";
    break;
  case 39:
    userName = "Apex";
    studentId = "39";
    break;
  case 40:
    userName = "Blitz";
    studentId = "40";
    break;
  case 41:
    userName = "Abeer";
    studentId = "41";
    break;
  case 42:
    userName = "Cipher";
    studentId = "42";
    break;
  case 43:
    userName = "Dusk";
    studentId = "43";
    break;
  case 44:
    userName = "Echo";
    studentId = "44";
    break;
  case 45:
    userName = "Frost";
    studentId = "45";
    break;
  case 46:
    userName = "Glitch";
    studentId = "46";
    break;
  case 47:
    userName = "Havoc";
    studentId = "47";
    break;
  case 48:
    userName = "Inferno";
    studentId = "48";
    break;
  case 49:
    userName = "Jinx";
    studentId = "49";
    break;
  case 50:
    userName = "Kraken";
    studentId = "50";
    break;
  case 51:
    userName = "Lynx";
    studentId = "51";
    break;
  case 52:
    userName = "Maverick";
    studentId = "52";
    break;
  case 53:
    userName = "Nebula";
    studentId = "53";
    break;
  case 54:
    userName = "Oblivion";
    studentId = "54";
    break;
  case 55:
    userName = "Phantom";
    studentId = "55";
    break;
  case 56:
    userName = "Quasar";
    studentId = "56";
    break;
  case 57:
    userName = "Raven";
    studentId = "57";
    break;
  case 58:
    userName = "Shadow";
    studentId = "58";
    break;
  case 59:
    userName = "Tempest";
    studentId = "59";
    break;
  case 60:
    userName = "Umbra";
    studentId = "60";
    break;
  case 61:
    userName = "Vortex";
    studentId = "61";
    break;
  case 62:
    userName = "Wraith";
    studentId = "62";
    break;
  case 63:
    userName = "Xenon";
    studentId = "63";
    break;
  case 64:
    userName = "Zephyr";
    studentId = "64";
    break;
  case 65:
    userName = "Ymir1";
    studentId = "65";
    break;
  case 66:
    userName = "Ymir2";
    studentId = "66";
    break;
  case 67:
    userName = "Ymir3";
    studentId = "67";
    break;   
   case 68:
    userName = "Ymir4";
    studentId = "68";
    break; 
  case 69:
    userName = "Ymir";
    studentId = "69";
    break;
  case 70:
    userName = "Ymir";
    studentId = "70";
    break;   
  default:
    Serial.println("Unknown fingerprint ID");
    return;
  }

  // Update Google Sheets with the new column-based format
  updateAttendance(userName, studentId);

  // Display welcome message
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Welcome");
  display.setCursor(0, 35);
  display.print(userName);
  display.display();

  delay(1000); // Show the welcome message for 2 seconds
}

void enrollMode()
{
  Serial.println("Entering Enroll Mode...");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Enroll Mode");
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Follow instructions");
  display.setCursor(0, 45);
  display.print("on serial monitor");
  display.display();

  while (true)
  {
    enrollFingerprint();
  }
}

void attendanceMode()
{
  Serial.println("Entering Attendance Mode...");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Attendance");
  display.setCursor(0, 30);
  display.print("Mode");
  display.display();
  delay(1000);

  while (true)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Place");
    display.setCursor(0, 30);
    display.print("Finger...");
    display.display();

    // Wait for a fingerprint to be detected
    int fingerprintID = -1;
    while (fingerprintID == -1)
    {
      fingerprintID = getFingerprintID();
      delay(50); // Add a small delay to avoid spamming the sensor
    }

    // Fingerprint found, add attendance
    addAttendance(fingerprintID);
    delay(1000); // Delay before next scan
  }
}

void clearAllFingerprints()
{
  Serial.println("Are you sure you want to clear all fingerprints? (Y/N)");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Clear ALL?");
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Type Y/N on serial");
  display.display();

  while (!Serial.available())
    ;
  char confirmation = Serial.read();
  if (confirmation == 'Y' || confirmation == 'y')
  {
    Serial.println("Clearing all fingerprints...");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Clearing...");
    display.display();

    uint8_t p = finger.emptyDatabase();
    if (p == FINGERPRINT_OK)
    {
      Serial.println("All fingerprints cleared successfully!");

      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 10);
      display.print("All Prints");
      display.setCursor(0, 30);
      display.print("Cleared!");
      display.display();
    }
    else
    {
      Serial.println("Failed to clear fingerprints.");

      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 10);
      display.print("Clear");
      display.setCursor(0, 30);
      display.print("Failed!");
      display.display();
    }
  }
  else
  {
    Serial.println("Clear operation canceled.");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Operation");
    display.setCursor(0, 30);
    display.print("Canceled");
    display.display();
  }
  delay(1000);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("System initialized");

  // Initialize OLED display
  display.begin(0x3D);
  delay(1000);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Starting...");
  display.display();

  Wire.begin();
  pinMode(ATTEND_BUTTON, INPUT_PULLUP);  // Setup button with internal pull-up

  display.begin(0x3C, true);  // Initialize OLED with I2C address 0x3C
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Booting...");
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Connecting");
  display.setCursor(0, 30);
  display.print("to WiFi...");
  display.display();

  int wifiCounter = 0;
  while (WiFi.status() != WL_CONNECTED && wifiCounter < 20) // Timeout after 20 seconds
  {
    delay(1000);
    Serial.print(".");
    wifiCounter++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println('\n');
    Serial.println("Connection established!");
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("WiFi");
    display.setCursor(0, 30);
    display.print("Connected!");
    display.display();
  }
  else
  {
    Serial.println('\n');
    Serial.println("WiFi connection failed!");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("WiFi Fail!");
    display.setCursor(0, 30);
    display.print("Continuing...");
    display.display();
  }
  delay(2000);

  // Initialize fingerprint sensor
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Init");
  display.setCursor(0, 30);
  display.print("Sensor...");
  display.display();

  finger.begin(57600);
  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Sensor");
    display.setCursor(0, 30);
    display.print("Found!");
    display.display();
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.print("Sensor");
    display.setCursor(0, 30);
    display.print("Not Found!");
    display.display();

    while (1)
    {
      delay(1000);
    }
  }
  delay(2000);

  finger.getTemplateCount();
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print("Stored");
  display.setCursor(0, 30);
  display.print("Prints: ");
  display.print(finger.templateCount);
  display.display();

  if (finger.templateCount == 0)
  {
    Serial.println("Sensor doesn't contain any fingerprint data. Please enroll a fingerprint.");
  }
  else
  {
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  }
  delay(2000);

  // Prompt user to select mode
  Serial.println("Select mode:");
  Serial.println("1. Enroll Mode");
  Serial.println("2. Attendance Mode");
  Serial.println("3. Clear All Fingerprints");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Select Mode:");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("1. Enroll Mode");
  display.setCursor(0, 35);
  display.print("2. Attendance Mode");
  display.setCursor(0, 50);
  display.print("3. Clear All Prints");
  display.display();
}

void loop()
{
  // Show default menu on OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Select Mode:");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("1. Enroll Mode");
  display.setCursor(0, 35);
  display.print("2. Attendance Mode");
  display.setCursor(0, 50);
  display.print("3. Clear All Prints");
  display.display();

  // Wait for user input via Serial or button
  while (true)
  {
    // Button press triggers Attendance Mode (physical mode selection)
    if (digitalRead(ATTEND_BUTTON) == LOW)
    {
      delay(200);  // Debounce delay
      attendanceMode();
      break;
    }

    // Serial input mode selection
    if (Serial.available())
    {
      char mode = Serial.read();
      if (mode == '1')
      {
        enrollMode();
        break;
      }
      else if (mode == '2')
      {
        attendanceMode();
        break;
      }
      else if (mode == '3')
      {
        clearAllFingerprints();
        break;
      }
      else
      {
        Serial.println("Invalid choice. Enter 1, 2, or 3.");
      }
    }

    delay(100);
  }
}
