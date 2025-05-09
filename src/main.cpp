/********************************************************************
 *  Smart‑glove hub  ‑  ESP32‑S3 + GY‑91 + 5 flex sensors
 *  – prints IMU + barometer + flex ADCs at 20 Hz
 *  – hot‑plugs the GY‑91 (re‑scans I²C every 3 s)
 *
 *  platformio.ini  →  lib_deps =
 *      asukiaaa/MPU9250_asukiaaa @ ^1.5.13
 *      adafruit/Adafruit BMP280 Library @ ^2.6.8
 ********************************************************************/

#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>

/* ── user‑configurable pins ───────────────────────────────────── */
constexpr uint8_t SDA_PIN  = 20;   // I²C data
constexpr uint8_t SCL_PIN  = 21;   // I²C clock
constexpr uint8_t flexPin[5] = {4, 5, 6, 7, 8};  // ADC inputs
/* ─────────────────────────────────────────────────────────────── */

MPU9250_asukiaaa mpu;
Adafruit_BMP280  bmp;

bool mpuOk = false, bmpOk = false;
uint32_t lastRetry = 0;
constexpr uint32_t RETRY_MS  = 3000;
constexpr uint32_t PRINT_MS  = 50;     // 20 Hz
uint32_t lastPrint = 0, bootMs = 0;

/* ─────────────────────────────────────────────────────────────── */
void tryAttachSensors() {
  /* ----- IMU / mag ------------------------------------------- */
  if (!mpuOk) {
    mpu.setWire(&Wire);
    uint8_t id;
    if (mpu.readId(&id) == 0) {
      mpu.beginAccel();
      mpu.beginGyro();
      mpu.beginMag();
      mpuOk = true;
      Serial.printf("✔ MPU‑9250 found (WHO_AM_I 0x%02X)\n", id);
    }
  }

  /* ----- BMP‑280 --------------------------------------------- */
  if (!bmpOk) {
    bmpOk = bmp.begin(0x76) || bmp.begin(0x77);
    if (bmpOk) {
      bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                      Adafruit_BMP280::SAMPLING_X2,
                      Adafruit_BMP280::SAMPLING_X16,
                      Adafruit_BMP280::FILTER_X16,
                      Adafruit_BMP280::STANDBY_MS_500);
      Serial.println("✔ BMP‑280 found.");
    }
  }
}
/* ─────────────────────────────────────────────────────────────── */
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Smart‑glove hub (ESP32‑S3) ===");
  Wire.begin(SDA_PIN, SCL_PIN, 100000);          // start at 100 kHz
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  bootMs = millis();
  tryAttachSensors();
}
/* ─────────────────────────────────────────────────────────────── */
void loop() {
  uint32_t now = millis();

  /* ----- periodic print -------------------------------------- */
  if (now - lastPrint >= PRINT_MS) {
    lastPrint = now;
    Serial.printf("[%6lu ms] ", now - bootMs);

    /* IMU block */
    if (mpuOk) {
      mpu.accelUpdate();
      mpu.gyroUpdate();
      Serial.printf("Ax %.2f Ay %.2f Az %.2f | ",
                    mpu.accelX(), mpu.accelY(), mpu.accelZ());
      Serial.printf("Gx %.1f Gy %.1f Gz %.1f | ",
                    mpu.gyroX(), mpu.gyroY(), mpu.gyroZ());
    } else {
      Serial.print("IMU‑WAIT | ");
    }

    /* Barometer block */
    if (bmpOk) {
      float T = bmp.readTemperature();
      float P = bmp.readPressure() / 100.0F;
      if (T > -40 && T < 85 && P > 300 && P < 1100) {
        Serial.printf("T %.1fC P %.0fhPa | ", T, P);
      } else {
        Serial.print("BARO‑ERR | ");
      }
    } else {
      Serial.print("BARO‑WAIT | ");
    }

    /* Flex sensors */
    for (int i = 0; i < 5; ++i) {
      int raw = analogRead(flexPin[i]);
      Serial.printf("F%d %d ", i, raw);
    }
    Serial.println();
  }

  /* ----- retry I²C attach every 3 s --------------------------- */
  if (now - lastRetry >= RETRY_MS) {
    lastRetry = now;
    if (!mpuOk || !bmpOk) {
      Serial.println("Scanning I²C for missing sensors …");
      tryAttachSensors();
    }
  }
}