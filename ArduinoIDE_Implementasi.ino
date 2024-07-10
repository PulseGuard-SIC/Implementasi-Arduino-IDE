#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_MLX90614.h>
#include <MAX30100_PulseOximeter.h>

const char* ssid = "CONFIDENTIAL (Wi-Fi Name/ID)";
const char* password = "CONFIDENTIAL (Wi-Fi Password)";
const char* serverName = "http://127.0.0.1:5000";

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;

#define REPORTING_PERIOD_MS 1000

uint32_t tsLastReport = 0;

void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");

    if (!mlx.begin())
    {
        Serial.println("Error connecting to MLX90614 sensor. Check wiring.");
        while (1);
    }

    if (!pox.begin())
    {
        Serial.println("MAX30100 was not found. Please check wiring/power.");
        while (1);
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);

    Serial.println("Sensors initialized successfully.");
}

void loop()
{
    double temp = mlx.readObjectTempC();
    Serial.print("Body Temperature: ");
    Serial.print(temp);
    Serial.println(" *C");

    pox.update();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
        float heartRate = pox.getHeartRate();
        float spo2 = pox.getSpO2();
        Serial.print("Heart rate: ");
        Serial.print(heartRate);
        Serial.print(" bpm / SpO2: ");
        Serial.print(spo2);
        Serial.println(" %");

        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;

            http.begin(http://127.0.0.1:5000 + "/sensor");
            http.addHeader("Content-Type", "application/json");

            String jsonPayload = "{\"temperature\": " + String(temp) + ", \"heart_rate\": " + String(heartRate) + ", \"spo2\": " + String(spo2) + "}";
            int httpResponseCode = http.POST(jsonPayload);

            if (httpResponseCode > 0)
            {
                String response = http.getString();
                Serial.println(httpResponseCode);
                Serial.println(response);
            }
            else
            {
                Serial.print("Error on sending POST: ");
                Serial.println(httpResponseCode);
            }

            http.end();
        }

        tsLastReport = millis();
    }

    delay(1000);
}