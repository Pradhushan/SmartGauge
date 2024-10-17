int measure() {
  currentTime = millis();

  if (currentTime - lastSampleTime >= samplePeriod) {
    // Read sensor value
    int distance = getDistance();

    // Filter sensor value
    if (distance >= acceptableMin && distance <= acceptableMax) {
      values[currentIndex] = distance;
      currentIndex = (currentIndex + 1) % maxValues;
      if (valuesCount < maxValues) {
        valuesCount++;
      }
    }

    // Calculate and print mode
    int approxDist = calculateMode();
    /* if (approxDist == NO_VALID_DATA) {
      Serial.println("No valid data");
    } else {
      // Serial.print("Mode value: ");
      Serial.println(approxDist);
    } */
    
             lastSampleTime = currentTime;
  }
}

int getDistance() {
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo pin
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in centimeters
  int distance = duration * 0.034 / 2;

  return distance;
}

int calculateMode() {
  if (valuesCount == 0) return NO_VALID_DATA;  // Return a special value if no valid data

  int mode = values[0];
  int maxCount = 0;

  for (int i = 0; i < valuesCount; i++) {
    int count = 0;
    for (int j = 0; j < valuesCount; j++) {
      if (values[j] == values[i]) {
        count++;
      }
    }
    if (count > maxCount) {
      maxCount = count;
      mode = values[i];
    }
  }

  return mode;
}
