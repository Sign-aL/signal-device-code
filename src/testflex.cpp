#include <Arduino.h>

const int flexPin = 13; // GPIO13 connected to the voltage divider
int flexValue = 0;

void setup() {
  Serial0.begin(115200);
  delay(1000); // Allow time for serial monitor to initialize
}

void loop() {
  flexValue = analogRead(flexPin); // Read the analog value
  Serial0.print("Flex Sensor Value: ");
  Serial0.println(flexValue);
  delay(500); // Wait for half a second before the next reading
}