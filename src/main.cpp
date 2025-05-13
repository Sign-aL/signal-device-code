// /********************************************************************
//  *  GLV‑HUB firmware  –  ESP32‑S3 + GY‑91  + 5 flex sensors
//  *  • Publishes sensor frames over BLE Nordic UART Service (NUS)
//  *  • Frame rate: 50 Hz   (every 20 ms)
//  *  • Packet: {"flex":[1234,…],"imu":[ax,ay,az,gx,gy,gz]}\n
//  *
//  *  PlatformIO:
//  *    board      = esp32-s3-devkitc-1
//  *    framework  = arduino
//  *    lib_deps   =
//  *      adafruit/Adafruit BMP280 Library@^2.6.8
//  *      asukiaaa/MPU9250_asukiaaa@^1.5.13
//  *      h2zero/NimBLE-Arduino@^2.2.3
//  *      bblanchon/ArduinoJson@^7.4.1
//  ********************************************************************/
// #include <Wire.h>
// #include <MPU9250_asukiaaa.h>
// #include <Adafruit_BMP280.h>
// #include <NimBLEDevice.h>
// #include <ArduinoJson.h>

// /* ---------------- pin map ----------------------------------------- */
// constexpr uint8_t SDA_PIN  = 20;
// constexpr uint8_t SCL_PIN  = 21;
// const uint8_t flexPin[5]   = {4, 5, 6, 7, 8};      // ADC1‑0…4
// /* ------------------------------------------------------------------ */

// /* ---------------- sensor objects ---------------------------------- */
// MPU9250_asukiaaa mpu;
// Adafruit_BMP280  bmp;

// /* ---------------- BLE UUIDs (Nordic UART) ------------------------- */
// static NimBLECharacteristic* txChar = nullptr; // 6E400003‑...
// const char* DEVICE_NAME = "GLV-HUB";
// static bool deviceConnected = false;

// /* ---------------- BLE callbacks ----------------------------------- */
// class MyServerCallbacks : public NimBLEServerCallbacks {
//   void onConnect(NimBLEServer*)   /* no 'override' to silence mismatch */ {
//     deviceConnected = true;
//   }
//   void onDisconnect(NimBLEServer*) {
//     deviceConnected = false;
//   }
// };

// void setupBLE() {
//   NimBLEDevice::init(DEVICE_NAME);
//   NimBLEDevice::setSecurityAuth(false, false, false);

//   auto* server = NimBLEDevice::createServer();
//   server->setCallbacks(new MyServerCallbacks());

//   NimBLEService* nus = server->createService(
//       "6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

//   txChar = nus->createCharacteristic(
//       "6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
//       NIMBLE_PROPERTY::NOTIFY);

//   nus->start();
//   NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
//   adv->addServiceUUID(nus->getUUID());
//   adv->setAppearance(0x03C0);   // generic glove (optional)
//   adv->start();
// }

// /* ---------------- helpers ----------------------------------------- */
// void startSensors() {
//   Wire.begin(SDA_PIN, SCL_PIN, 400000);

//   mpu.setWire(&Wire);
//   mpu.beginAccel();
//   mpu.beginGyro();

//   bmp.begin(0x76);              // not sent but keeps BMP idle current low
//   analogReadResolution(12);
//   analogSetAttenuation(ADC_11db);
// }

// StaticJsonDocument<256> doc;

// /* ---------------- main -------------------------------------------- */
// void setup() {
//   Serial.begin(115200);
//   startSensors();
//   setupBLE();
// }

// void loop() {
//   static uint32_t last = 0;
//   if (millis() - last < 20) return;   // 50 Hz
//   last = millis();

//   /* ------------ read sensors ---------------- */
//   int flex[5];
//   for (int i = 0; i < 5; ++i) flex[i] = analogRead(flexPin[i]);

//   mpu.accelUpdate();  mpu.gyroUpdate();

//   /* ------------ build JSON ------------------ */
//   doc.clear();
//   auto fArr = doc.createNestedArray("flex");
//   for (int v : flex) fArr.add(v);

//   auto imu = doc.createNestedArray("imu");
//   imu.add(mpu.accelX()); imu.add(mpu.accelY()); imu.add(mpu.accelZ());
//   imu.add(mpu.gyroX());  imu.add(mpu.gyroY());  imu.add(mpu.gyroZ());

//   /* ------------ transmit -------------------- */
//   if (deviceConnected && txChar) {
//     char buf[256];
//     size_t n = serializeJson(doc, buf);
//     buf[n++] = '\n';                    // newline delimiter for the phone
//     txChar->setValue(reinterpret_cast<uint8_t*>(buf), n);
//     txChar->notify();
//   }
// }