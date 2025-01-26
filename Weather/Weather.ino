#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "Dhole_weather_icons32px.h"

// 64 X 32 
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANEL_CHAIN 1
MatrixPanel_I2S_DMA *dma_display = nullptr;

const char *ssid = "Your_SSID";                            // WIFI ID
const char *password = "Your_SSID_Password";               // WIFI PASSWORD
const String apiKey = "Your_OpenWeather_API_Key";          // OpenWeatherMap API key
const String cityID = "5815135";                           // City ID for your location int this case Washington, US  https://openweathermap.org/city/5815135
const String units = "imperial";                           // Use "metric" for Celsius, "imperial" for Fahrenheit
const String url = "http://api.openweathermap.org/data/2.5/weather?id=" + cityID + "&units=" + units + "&appid=" + apiKey;

bool awaitingArrivals = true;
bool arrivalsRequested = false;
WiFiClient client;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long previousMillis = 0;


// Local Weather
String Temp = "";  // Temperature
String Humi = "";  // Humidity
String WfEn = "";  // Weather

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);

// Color values ​​are entered as values ​​between 0 and 255.
uint16_t colorWheel(uint8_t pos) {
  if (pos < 85) {
    return dma_display->color565(pos * 3, 255 - pos * 3, 0);
  } else if (pos < 170) {
    pos -= 85;
    return dma_display->color565(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return dma_display->color565(0, pos * 3, 255 - pos * 3);
  }
}

// WIFI Icon
const char wifi_image1bit[] PROGMEM = {
  0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0x01, 0x00,
  0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x1f,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x07, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x80, 0xff, 0x00, 0x00,
  0xff, 0x01, 0x00, 0x00, 0xc0, 0x1f, 0x00, 0x00, 0xf8, 0x03, 0x00, 0x00, 0xe0, 0x0f, 0x00,
  0x00, 0xf0, 0x07, 0x00, 0x00, 0xf0, 0x03, 0xf0, 0x0f, 0xc0, 0x0f, 0x00, 0x00, 0xe0, 0x01,
  0xff, 0xff, 0x80, 0x07, 0x00, 0x00, 0xc0, 0xc0, 0xff, 0xff, 0x03, 0x03, 0x00, 0x00, 0x00,
  0xe0, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0xf0, 0x1f, 0x00, 0x00, 0x00,
  0x00, 0xfc, 0x01, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x3e, 0x00, 0x00,
  0x00, 0x00, 0x38, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x10, 0xe0, 0x07, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00
};

// Function the generate the Weather Icon
void drawXbm565(int x, int y, int width, int height, const char *xbm, uint16_t color = 0xffff) {
  if (width % 8 != 0) {
    width = ((width / 8) + 1) * 8;
  }
  for (int i = 0; i < width * height / 8; i++) {
    unsigned char charColumn = pgm_read_byte(xbm + i);
    for (int j = 0; j < 8; j++) {
      int targetX = (i * 8 + j) % width + x;
      int targetY = (8 * i / (width)) + y;
      if (bitRead(charColumn, j)) {
        dma_display->drawPixel(targetX, targetY, color);
      }
    }
  }
}

/* Load icon */
int current_icon = 0;
static int num_icons = 22;

static char icon_name[22][30] = {
  "cloud_moon_bits",
  "cloud_sun_bits",
  "clouds_bits",
  "cloud_wind_moon_bits",
  "cloud_wind_sun_bits",
  "cloud_wind_bits",
  "cloud_bits",
  "lightning_bits",
  "moon_bits",
  "rain0_sun_bits",
  "rain0_bits",
  "rain1_moon_bits",
  "rain1_sun_bits",
  "rain1_bits",
  "rain2_bits",
  "rain_lightning_bits",
  "rain_snow_bits",
  "snow_moon_bits",
  "snow_sun_bits",
  "snow_bits",
  "sun_bits",
  "wind_bits"
};

static char *icon_bits[22] = { cloud_moon_bits,
                               cloud_sun_bits,
                               clouds_bits,
                               cloud_wind_moon_bits,
                               cloud_wind_sun_bits,
                               cloud_wind_bits,
                               cloud_bits,
                               lightning_bits,
                               moon_bits,
                               rain0_sun_bits,
                               rain0_bits,
                               rain1_moon_bits,
                               rain1_sun_bits,
                               rain1_bits,
                               rain2_bits,
                               rain_lightning_bits,
                               rain_snow_bits,
                               snow_moon_bits,
                               snow_sun_bits,
                               snow_bits,
                               sun_bits,
                               wind_bits };

void setup() {
  Serial.begin(115200);  // Start serial communication
  connectWiFi();         // WIFI Connection
  timeClient.begin();
  timeClient.setTimeOffset(-18000);  // Time offset for EST (UTC-5)

  // Matrix Setup
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,  // module width
    PANEL_RES_Y,  // module height
    PANEL_CHAIN   // Chain length
  );

  //mxconfig.gpio.e = -1;
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::FM6124;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90);  //0-255
  dma_display->clearScreen();

  dma_display->fillScreen(dma_display->color444(0, 1, 0));

  // WIFI Logo
  for (int r = 0; r < 255; r++) {
    drawXbm565(0, 0, 64, 32, wifi_image1bit, dma_display->color565(r, 0, 0));
    delay(10);
  }

  delay(2000);
  getData();
  delay(3000);
  dma_display->clearScreen();
}

void loop() {
  drawText();
}

void getData() {
  HTTPClient http;
  Serial.print("Requesting URL: ");
  Serial.println(url);  // Print the URL to the Serial Monitor
  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String payload = http.getString();

    // Print the raw JSON payload to the Serial Monitor
    Serial.println("Received JSON payload:");
    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // temperature
    Temp = String(doc["main"]["temp"].as<float>());
    // humidity
    Humi = String(doc["main"]["humidity"].as<int>());
    // Weather Description
    WfEn = doc["weather"][0]["main"].as<String>();

    // Print parsed values to Serial Monitor
    Serial.print("Temperature: ");
    Serial.println(Temp);
    Serial.print("Humidity: ");
    Serial.println(Humi);
    Serial.print("Weather: ");
    Serial.println(WfEn);
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void connectWiFi() {
  Serial.println();
  Serial.println();
  Serial.println("Connecting to ");
  Serial.print(ssid);
  WiFi.hostname("Name");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void drawText() {
  static unsigned long lastUpdateTime = 0;     // Store the last update time for the 1000 ms delay
  static unsigned long lastWeatherUpdate = 0;  // Store the last time getData() was called
  static bool colonVisible = true;             // State variable for colon visibility
  const unsigned long interval = 600000;       // Interval for 10 minutes (600,000 milliseconds)
  const unsigned long shortInterval = 1000;    // Interval for the short delay (1000 milliseconds)

  unsigned long currentMillis = millis();

  // Check if 10 minutes have passed to update weather data
  if (currentMillis - lastWeatherUpdate >= interval) {
    lastWeatherUpdate = currentMillis;
    getData();  // Call getData() every 10 minutes
  }

  // Check if 1000 milliseconds have passed to update the display
  if (currentMillis - lastUpdateTime >= shortInterval) {
    lastUpdateTime = currentMillis;

    colonVisible = !colonVisible;  // Toggle colon visibility   

    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    String formattedTime = timeClient.getFormattedTime();
    int Hour = timeClient.getHours();
    int SetHour = 0;
    int Minute = timeClient.getMinutes();
    String mm;
    String APM = "";
    String hh;
    if (Minute < 10) {
      mm = "0";
    } else {
      mm = "";
    }
    if (Hour < 10) {
      hh = "0";
    } else {
      hh = "";
    }

    int Temp_int = Temp.toInt();
    int Humi_int = Humi.toInt();
    
    Serial.println(String("시간 ") + hh + Hour + ":" + mm + Minute);
    Serial.println("온도 " + Temp);
    Serial.println("습도 " + Humi);
    Serial.print("날씨 " + WfEn);
    Serial.println("-----------------");

    dma_display->setTextSize(1);  // size 1 ~ 8 까지 설정
    dma_display->setTextWrap(false);

    dma_display->fillRect(34, 3, 35, 7, dma_display->color444(0, 0, 0));
    dma_display->setCursor(34, 3);
    dma_display->setTextColor(dma_display->color444(0, 15, 0));
    dma_display->println(hh + Hour);

    // Draw the blinking colon
    if (colonVisible) {
      dma_display->setTextColor(dma_display->color444(15, 0, 0));
      dma_display->setCursor(45, -1);      
      dma_display->println(".");
      dma_display->setCursor(45, 2);      
      dma_display->println(".");
    } else {
      dma_display->fillRect(45, -1, 6, 7, dma_display->color444(0, 0, 0));  // Clear the colon
      dma_display->fillRect(45, 2, 6, 7, dma_display->color444(0, 0, 0));   // Clear the colon
    }
    
    dma_display->setTextColor(dma_display->color444(0, 15, 0));
    dma_display->setCursor(51, 3);
    dma_display->println(mm + Minute);

    dma_display->fillRect(38, 13, 35, 7, dma_display->color444(0, 0, 0));
    dma_display->setTextColor(dma_display->color444(15, 0, 0));
    dma_display->setCursor(38, 13);
    dma_display->println(Temp_int);
    dma_display->setCursor(48, 8);
    dma_display->println(".");
    dma_display->setCursor(53, 13);
    dma_display->println("F");

    dma_display->fillRect(38, 22, 35, 7, dma_display->color444(0, 0, 0));
    dma_display->setCursor(38, 22);
    dma_display->setTextColor(dma_display->color444(0, 0, 15));
    dma_display->print(Humi_int);
    dma_display->setCursor(52, 22);
    dma_display->println("%");

    // Clear
    // Cloudy
    // Mostly Cloudy
    // Shower 
    // Rain
    // Snow

    /*----------Icons for Weather Conditions-------*/

    dma_display->fillRect(0, 0, 32, 32, dma_display->color444(0, 0, 0));
    if (WfEn == "Clear" && 6 <= Hour <= 18) {
      drawXbm565(0, 0, 32, 32, icon_bits[20]);
    } else if (WfEn == "Clear" && (19 <= Hour <= 18 || Hour < 6)) {
      drawXbm565(0, 0, 32, 32, icon_bits[8]);
    }

    else if (WfEn == "Clouds" && 6 <= Hour <= 18) {
      drawXbm565(0, 0, 32, 32, icon_bits[1]);
    } else if (WfEn == "Clouds" && (19 <= Hour <= 18 || Hour < 6)) {
      drawXbm565(0, 0, 32, 32, icon_bits[3]);
    }

    else if (WfEn == "Few Clouds" && 6 <= Hour <= 18) {
      drawXbm565(0, 0, 32, 32, icon_bits[4]);
    } else if (WfEn == "Few Clouds" && (19 <= Hour <= 18 || Hour < 6)) {
      drawXbm565(0, 0, 32, 32, icon_bits[3]);
    }

    else if ((WfEn == "Rain") && 6 <= Hour <= 18) {
      drawXbm565(0, 0, 32, 32, icon_bits[12]);
    } else if ((WfEn == "Rain") && (19 <= Hour <= 18 || Hour < 6)) {
      drawXbm565(0, 0, 32, 32, icon_bits[11]);
    }

    else if (WfEn == "Snow" && 6 <= Hour <= 18) {
      drawXbm565(0, 0, 32, 32, icon_bits[18]);
    } else if (WfEn == "Snow" && (19 <= Hour <= 18 || Hour < 6)) {
      drawXbm565(0, 0, 32, 32, icon_bits[17]);
    }

    else {
    }
  }
}
