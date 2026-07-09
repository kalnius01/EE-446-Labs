  #include <Arduino_HS300x.h>
  #include <Arduino_BMI270_BMM150.h>
  #include <Arduino_APDS9960.h>

  float p_temp = 0.0;
  float p_rh = 0.0;
  float p_mag = 0.0;
  int p_clear = 0;
  bool initial = true;
  unsigned long eventMarker = 0;
  const unsigned long cooldown = 2000;
  
  void setup() {
    Serial.begin(115200);
    delay(1500);

    // Temp and Humidity
    if (!HS300x.begin()) {
      Serial.println("Failed to initialize humidity/temperature sensor.");
      while (1) {
        delay(10);
      }
    }

    // Brightness
    if (!APDS.begin()) {
      Serial.println("Failed to initialize APDS9960 sensor.");
      while (1) {
        delay(10);
      }
    }

    // Magnetic
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU.");
      while (1) {
        delay(10);
      }
  }

  }

  void loop() {

    //Temp and Humidity
    float temp = HS300x.readTemperature();
    float rh = HS300x.readHumidity();

    // Ambient brightness
    int r, g, b, c;
    if (APDS.colorAvailable()) {
      APDS.readColor(r, g, b, c);
    }

    // Magnetic
    float x, y, z, mag;
    if (IMU.magneticFieldAvailable()) {
      IMU.readMagneticField(x, y, z);
      mag = sqrt(x*x + y*y + z*z);
    }

    if (initial) {
      p_temp = temp;
      p_rh = rh;
      p_mag = mag;
      p_clear = c;
      initial = false;
    }
    
    int humid_jump, temp_rise, mag_shift, light_or_color_change;

    if ((temp - p_temp) > 0.2) {
      temp_rise = 1;
    } else {
      temp_rise = 0;
    }

    if ((rh - p_rh) > 10) {
      humid_jump = 1;
    } else {
      humid_jump = 0;
    }

    if (abs(mag - p_mag) > 50) {
      mag_shift = 1;
    } else {
      mag_shift = 0;
    }

    if ((c - p_clear) > 100) {
      light_or_color_change = 1;
    } else {
      light_or_color_change = 0;
    }

    Serial.print("raw,rh=");
    Serial.print(rh);
    Serial.print(",temp=");
    Serial.print(temp);
    Serial.print(",mag=");
    Serial.print(mag);
    Serial.print(",r=");
    Serial.print(r);
    Serial.print(",g=");
    Serial.print(g);
    Serial.print(",b=");
    Serial.print(b);
    Serial.print(",clear=");
    Serial.println(c);

    Serial.print("flags,humid_jump=");
    Serial.print(humid_jump);
    Serial.print(",temp_rise=");
    Serial.print(temp_rise);
    Serial.print(",mag_shift=");
    Serial.print(mag_shift);
    Serial.print(",light_or_color_change=");
    Serial.println(light_or_color_change);

    unsigned long curTime = millis();

    if (curTime - eventMarker > cooldown) {
      if (humid_jump == 1 || temp_rise == 1) {
        Serial.println("event,BREATH_OR_WARM_AIR_EVENT");
        eventMarker = curTime;
      } else if (mag_shift == 1) {
        Serial.println("event,MAGNETIC_DISTURBANCE_EVENT");
        eventMarker = curTime;
      } else if (light_or_color_change == 1) {
        Serial.println("event,LIGHT_OR_COLOR_CHANGE_EVENT");
        eventMarker = curTime;
      } else {
        Serial.println("event,BASELINE_NORMAL");
      }
    } else {
      Serial.println("event,BASELINE_NORMAL");
    }

    p_temp = temp;
    p_rh = rh;
    p_mag = mag;
    p_clear = c;

    delay(500);
  }


