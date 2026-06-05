void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32-S3 funcionando correctamente.");
  Serial.println("Firmware cargado con exito.");
}

void loop() {
  Serial.println("Ping desde ESP32-S3...");
  delay(1000);
}