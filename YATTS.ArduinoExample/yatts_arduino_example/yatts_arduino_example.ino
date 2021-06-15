bool on = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial)
    ;
    
  digitalWrite(LED_BUILTIN, on);
}

void loop() {
  while (Serial.available() > 0) {
    if (Serial.read() == 69) {
      on = !on;
      digitalWrite(LED_BUILTIN, on);
    }
  }
}
