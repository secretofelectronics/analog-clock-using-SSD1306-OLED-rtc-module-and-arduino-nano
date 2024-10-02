#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

// Define OLED screen width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define CENTER_X 64    // Center X of the screen
#define CENTER_Y 32    // Center Y of the screen
#define RADIUS 30      // Radius of the analog clock

// Define GPIO pins for the buttons
int BUTTON_HOUR=2;
int BUTTON_MINUTE=3;
int BUTTON_SET=4;

// Create an SSD1306 display object connected to I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Create RTC object
RTC_DS3231 rtc;

// Variables to hold time for setting
int setHour = 0;
int setMinute = 0;
bool settingTime = false;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  
  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize the buttons
  pinMode(BUTTON_HOUR, INPUT);    // Button to increase hour
  pinMode(BUTTON_MINUTE, INPUT);  // Button to increase minute
  pinMode(BUTTON_SET, INPUT);     // Button to set time
}

void loop() {
  if (!settingTime) {
    // If not setting time, get the current time from the RTC
    DateTime now = rtc.now();
    
    // Clear the display buffer
    display.clearDisplay();

    // Draw the analog clock face
    drawClockFace();

    // Draw hour, minute, and second hands
    drawHands(now.hour(), now.minute(), now.second());

    // Display everything on the OLED
    display.display();
    
    // Check if we are entering time-setting mode
    if (digitalRead(BUTTON_SET) == LOW) {
      settingTime = true;
      setHour = now.hour();    // Load current hour
      setMinute = now.minute();// Load current minute
      delay(200); // Debounce delay
    }
  } else {
    // Time setting mode
    adjustTime();
  }

  // Wait for 100 ms
  delay(100);
}

// Function to draw the clock face
void drawClockFace() {
  // Draw outer circle (clock outline)
  display.drawCircle(CENTER_X, CENTER_Y, RADIUS, SSD1306_WHITE);

  // Draw the clock's 12-hour markers
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 * PI / 180;  // Convert degree to radians
    int x1 = CENTER_X + cos(angle) * (RADIUS - 2);  // Inner point (near center)
    int y1 = CENTER_Y + sin(angle) * (RADIUS - 2);
    int x2 = CENTER_X + cos(angle) * (RADIUS);  // Outer point (clock border)
    int y2 = CENTER_Y + sin(angle) * (RADIUS);
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);  // Draw hour marker line
  }
}

// Function to draw the hour, minute, and second hands
void drawHands(int hour, int minute, int second) {
  // Adjust hour for 12-hour format
  if (hour > 12) hour -= 12;

  // Draw hour hand (shortest)
  float hour_angle = (hour * 30 + minute * 0.5) * PI / 180;  // Hour in degrees, add minute fraction
  drawHand(hour_angle, RADIUS - 15, SSD1306_WHITE);  // Shorter length for the hour hand

  // Draw minute hand (longer)
  float minute_angle = minute * 6 * PI / 180;  // Minute in degrees
  drawHand(minute_angle, RADIUS - 5, SSD1306_WHITE);  // Longer length for the minute hand

  // Draw second hand (longest and thin)
  float second_angle = second * 6 * PI / 180;  // Second in degrees
  drawHand(second_angle, RADIUS, SSD1306_WHITE);  // Longest length for the second hand
}

// Function to calculate and draw clock hands
void drawHand(float angle, int length, int color) {
  int x = CENTER_X + cos(angle - PI / 2) * length;  // Adjust angle for OLED coordinate system
  int y = CENTER_Y + sin(angle - PI / 2) * length;
  display.drawLine(CENTER_X, CENTER_Y, x, y, color);  // Draw hand from center to end point
}

// Function to adjust and set the time using buttons
void adjustTime() {
  // Clear display
  display.clearDisplay();
  
  // Display the time being set
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.print("Set Time:");
  display.setCursor(10, 30);
  display.print(setHour);
  display.print(":");
  display.print(setMinute < 10 ? "0" : "");
  display.print(setMinute);
  display.display();

  // Adjust hour if button is pressed
  if (digitalRead(BUTTON_HOUR) == LOW) {
    setHour++;
    if (setHour >= 24) setHour = 0;  // Wrap around to 0 after 23
    delay(200);  // Debounce delay
  }

  // Adjust minute if button is pressed
  if (digitalRead(BUTTON_MINUTE) == LOW) {
    setMinute++;
    if (setMinute >= 60) setMinute = 0;  // Wrap around to 0 after 59
    delay(200);  // Debounce delay
  }

  // Set the time when the "set" button is pressed again
  if (digitalRead(BUTTON_SET) == LOW) {
    rtc.adjust(DateTime(2023, 1, 1, setHour, setMinute, 0));  // Set new time to RTC
    settingTime = false;  // Exit time-setting mode
    delay(200);  // Debounce delay
  }
}