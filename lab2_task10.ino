  #include <Arduino_APDS9960.h>
  #include <PDM.h>
  #include <Arduino_BMI270_BMM150.h>

  short sampleBuffer[256];
  volatile int samplesRead = 0;

  void onPDMdata() {
    int bytesAvailable = PDM.available();
    PDM.read(sampleBuffer, bytesAvailable);
    samplesRead = bytesAvailable / 2;
  }

  void setup() {
    Serial.begin(115200);
    delay(1500);

    // Audio
    PDM.onReceive(onPDMdata);
    if (!PDM.begin(1, 16000)) {
      Serial.println("Failed to start PDM microphone.");
      while (1) {
      delay(10);
      }
    }

    //Brightness
    if (!APDS.begin()) {
      Serial.println("Failed to initialize APDS9960 sensor.");
      while (1) {
      delay(10);
      }
    }

    //motion
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU.");
      while (1) {
      delay(10);
      }
    }

  }

  void loop() {

    //Audio output: Level
    int level;
    if (samplesRead) {
      long sum = 0;
      for (int i = 0; i < samplesRead; i++) {
        sum += abs(sampleBuffer[i]);
      }
      level = sum / samplesRead;
      samplesRead = 0;
    }

    // Ambient brightness: clear
    int r, g, b, c;
    if (APDS.colorAvailable()) {
      APDS.readColor(r, g, b, c);
    }

    // Physical motion: totMov
    float x, y, z;
    float totMov;
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
      totMov = x + y + z;
    }

    // Presence: proximity
    static int proximity;
    if (APDS.proximityAvailable()) {
      proximity = APDS.readProximity();
    }

    int sound, dark, moving, near;

    if (level > 500) {
      sound = 1;
    } else {
      sound = 0;
    }

    if (c > 50) {
      dark = 0;
    } else {
      dark = 1;
    }

    if (totMov > 1.5 || totMov < -1.5) {
      moving = 1;
    } else {
      moving = 0;
    }

    if (proximity < 200) {
      near = 1;
    } else {
      near = 0;
    }

    Serial.print("raw,mic=");
    Serial.print(level);
    Serial.print(",clear=");
    Serial.print(c);
    Serial.print(",motion=");
    Serial.print(totMov);
    Serial.print(",prox=");
    Serial.println(proximity);

    Serial.print("flags,sound=");
    Serial.print(sound);
    Serial.print(",dark=");
    Serial.print(dark);
    Serial.print(",moving=");
    Serial.print(moving);
    Serial.print(",near=");
    Serial.println(near);

    if (sound == 0 && dark == 0 && moving == 0 && near == 0) {
      Serial.println("state,QUIET_BRIGHT_STEADY_FAR");
    } else if (sound == 1 && dark == 0 && moving == 0 && near == 0) {
      Serial.println("state,NOISY_BRIGHT_STEADY_FAR");
    } else if (sound == 0 && dark == 1 && moving == 0 && near == 1) {
      Serial.println("state,QUIET_DARK_STEADY_NEAR");
    } else if (sound == 1 && dark == 0 && moving == 1 && near == 1) {
      Serial.println("state,NOISY_BRIGHT_MOVING_NEAR");
    } else {
      Serial.println("state,UNDEFINED");
    }

    delay(500);
  }

