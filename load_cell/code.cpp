#include "HX711.h"

// Update these to match your actual wiring
#define LOADCELL_DOUT_PIN 4
#define LOADCELL_SCK_PIN  5

HX711 scale;

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 Thrust Test");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Calibration factor - you'll need to determine this
  // by placing a known weight and adjusting until reading matches
  scale.set_scale(2280.f); // placeholder, calibrate later
  scale.tare();            // zero out with no load

  Serial.println("Tare done. Place known weight to calibrate.");
}

void loop() {
  if (scale.is_ready()) {
    float grams = scale.get_units(10); // average of 10 readings
    Serial.print("Thrust: ");
    Serial.print(grams);
    Serial.println(" g");
  } else {
    Serial.println("HX711 not ready");
  }
  delay(200);
}