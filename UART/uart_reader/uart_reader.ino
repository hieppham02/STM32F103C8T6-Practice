HardwareSerial STM32Serial(2);

void setup() {
  Serial.begin(115200);
  STM32Serial.begin(115200, SERIAL_8N1, 16, 17);
}

void loop() {
  while (Serial.available()) {
    STM32Serial.write(Serial.read());
  }

  while (STM32Serial.available()) {
    Serial.write(STM32Serial.read());
  }
}