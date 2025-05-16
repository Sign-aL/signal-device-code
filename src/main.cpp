#include <NimBLEDevice.h>

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_TX   "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // Notify

NimBLECharacteristic* txCharacteristic;

void setupBLE() {
  NimBLEDevice::init("FakeGloveBLE");
  NimBLEServer* server = NimBLEDevice::createServer();
  NimBLEService* service = server->createService(SERVICE_UUID);

  txCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_TX,
    NIMBLE_PROPERTY::NOTIFY
  );

  service->start();
  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();
}

void setup() {
  Serial0.begin(115200);  // Use Serial for Serial Monitor
  setupBLE();
  Serial0.println("🧤 Spelling Simulation Started!");
}

// Simulated letter list to spell out: I M H U N G R Y (with apostrophe and space)
char letters[] = { 'I', 'M', ' ', 'H', 'U', 'N', 'G', 'R', 'Y' };
int currentLetter = 0;

void loop() {
  // Simulate sensor values
  int flex[5];
  float accel[3];
  float gyro[3];

  for (int i = 0; i < 5; i++) {
    flex[i] = random(600, 1600);
  }

  accel[0] = random(-100, 100) / 100.0;
  accel[1] = random(-100, 100) / 100.0;
  accel[2] = random(0, 110) / 100.0;

  gyro[0] = random(-50, 50) / 100.0;
  gyro[1] = random(-50, 50) / 100.0;
  gyro[2] = random(-50, 50) / 100.0;

  // Select the current letter
  char letter = letters[currentLetter];

  // Format message
  String message = "Flex: [" + String(flex[0]) + ", " + String(flex[1]) + ", " +
                   String(flex[2]) + ", " + String(flex[3]) + ", " + String(flex[4]) + "]\n" +
                   "Accel: [" + String(accel[0], 2) + ", " + String(accel[1], 2) + ", " + String(accel[2], 2) + "]\n" +
                   "Gyro: [" + String(gyro[0], 2) + ", " + String(gyro[1], 2) + ", " + String(gyro[2], 2) + "]\n" +
                   "Detected Letter: " + String(letter);

  Serial0.println(message);
  Serial0.println("----------------------------");

  // Notify via BLE
  txCharacteristic->setValue(String(letter).c_str());
  txCharacteristic->notify();

  // Go to next letter in the sequence
  currentLetter = (currentLetter + 1) % (sizeof(letters) / sizeof(char));
  
  delay(1500);  // Wait before next letter
}