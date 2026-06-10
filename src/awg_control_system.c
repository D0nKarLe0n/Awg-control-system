#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- PIN DEFINITIONS (ESP32-C3) ---
#define DHTPIN_RADIATOR 3   // Radiator temperature sensor
#define DHTPIN_ROOM     2   // Ambient temperature/humidity sensor
#define PELTIER_PIN     1   // TEC1-12706 MOSFET control
#define FAN_PWM_PIN     0   // PWM output for Delta fan
#define FAN_FG_PIN      5   // Tachometer (Frequency Generator) input

#define DHTTYPE DHT22

// --- PERIPHERALS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DHT dhtRadiator(DHTPIN_RADIATOR, DHTTYPE);
DHT dhtRoom(DHTPIN_ROOM, DHTTYPE);

// --- GLOBAL VARIABLES ---
volatile int fanPulseCount = 0;
unsigned long lastCalculateTime = 0;

// Hardware interrupt for tachometer (FG signal)
void IRAM_ATTR fanPulseISR() {
    fanPulseCount++;
}

// Simple approximation of the Dew Point
float calculateDewPoint(float temperature, float humidity) {
    return temperature - ((100.0 - humidity) / 5.0);
}

// Calculate RPM based on FG pulses (2 pulses per revolution for standard fans)
int calculateRPM() {
    int rpm = (fanPulseCount * 60) / 2;
    fanPulseCount = 0;
    return rpm;
}

// Calculate PWM duty cycle based on radiator temperature
int getFanPWM(float radiatorTemp) {
    // Base 50% duty cycle, adding 10% for every 5°C above 30°C
    int dutyCycle = 50 + ((radiatorTemp - 30.0) / 5.0) * 10; 
    dutyCycle = constrain(dutyCycle, 50, 100);
    return map(dutyCycle, 0, 100, 0, 255);
}

void setup() {
    Serial.begin(115200);
    
    // I2C Init (SDA: 8, SCL: 4)
    Wire.begin(8, 4);

    dhtRadiator.begin();
    dhtRoom.begin();

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED allocation failed!");
        while(true);
    }
    display.clearDisplay();
    display.display();

    pinMode(PELTIER_PIN, OUTPUT);
    digitalWrite(PELTIER_PIN, LOW);
    
    pinMode(FAN_FG_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FAN_FG_PIN), fanPulseISR, FALLING);
    
    lastCalculateTime = millis();
}

void loop() {
    if (millis() - lastCalculateTime >= 2000) { // 2-second control loop
        lastCalculateTime = millis();
        
        float radiatorTemp = dhtRadiator.readTemperature();
        float roomTemp = dhtRoom.readTemperature();
        float roomHumidity = dhtRoom.readHumidity();

        if (isnan(radiatorTemp) || isnan(roomTemp) || isnan(roomHumidity)) {
            Serial.println("Sensor read error!");
            return;
        }

        // 1. Thermodynamic Calculation
        float dewPoint = calculateDewPoint(roomTemp, roomHumidity);

        // 2. Thermal Management (Fan Control)
        int fanPWM = getFanPWM(radiatorTemp);
        analogWrite(FAN_PWM_PIN, fanPWM);

        // 3. Condensation Control (Peltier Logic)
        // Ensure the hot side doesn't overheat while maintaining cold side below dew point
        if (radiatorTemp > dewPoint + 2.0) {
            digitalWrite(PELTIER_PIN, HIGH);
        } else {
            digitalWrite(PELTIER_PIN, LOW);
        }

        int rpm = calculateRPM();

        // 4. UI Update
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        
        display.setCursor(0, 0);
        display.printf("Rad: %.1fC  Room: %.1fC", radiatorTemp, roomTemp);
        
        display.setCursor(0, 10);
        display.printf("Dew: %.1fC  Hum: %.0f%%", dewPoint, roomHumidity);
        
        display.setCursor(0, 20);
        display.printf("Fan: %d%%  RPM: %d", (fanPWM * 100 / 255), rpm);
        
        display.display();
    }
}