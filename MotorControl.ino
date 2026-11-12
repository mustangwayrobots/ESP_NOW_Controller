/**
 * ==============================================================================
 * MotorControl.ino - Arcade Drive Mixing, Trim, Ramping, and Weapon Control
 * ==============================================================================
 */

// Reference variables from ESP_NOW_Controller.ino
extern unsigned long lastSendTime;
extern int currentSendInterval;
extern unsigned long lastSendAckTime;

void CalculateMotorDriverValues(int rawX, int rawY) {
  // 1. Retrieve settings from the named list (see SettingId in ConfigData.h)
  int maxSpeed    = currentSettings[SET_MAX_SPEED];
  int forwardTrim = currentSettings[SET_FORWARD_TRIM];
  int reverseTrim = currentSettings[SET_REVERSE_TRIM];
  int steerRate   = currentSettings[SET_STEER_RATE];
  int steerExpo   = currentSettings[SET_STEER_EXPO];
  int spinSpeed   = currentSettings[SET_SPIN_SPEED];
  int motorADir   = currentSettings[SET_MOTOR_A_DIR];
  int motorBDir   = currentSettings[SET_MOTOR_B_DIR];
  int motorSwap   = currentSettings[SET_MOTOR_SWAP];
  int weaponOnVal = currentSettings[SET_WEAPON_ON];
  int deadzone    = currentSettings[SET_DEADZONE];
  int centerX     = currentSettings[SET_ZERO_X];   // stick center, calibrated in the menu
  int centerY     = currentSettings[SET_ZERO_Y];
  if (deadzone < 50) deadzone = 50; // Enforce safe minimum deadzone for high-count ADCs

  // 2. Axis Mapping (Swapped RawX/RawY based on physical board wiring)
  int throttle = -(rawX - centerX);
  int steering = rawY - centerY;

  // 3. Strict Center Deadzone
  if (abs(throttle) < deadzone) throttle = 0;
  if (abs(steering) < deadzone) steering = 0;

  bool isActivelyDriving = (throttle != 0 || steering != 0);

  int leftPWM = 0;
  int rightPWM = 0;

  // 4. Spin-on-Demand (if SpinButton is held)
  if (SpinButton) {
    int spinPwm = map(spinSpeed, 0, 100, 0, 255);
    if (steering > 0) {
      leftPWM = spinPwm;
      rightPWM = -spinPwm;
    } else if (steering < 0) {
      leftPWM = -spinPwm;
      rightPWM = spinPwm;
    } else {
      leftPWM = 0;
      rightPWM = 0;
    }
  } else {
    // 5. Arcade Drive Mixing with Steering Expo & Rate
    float throttleScaled = (throttle / 1800.0) * (maxSpeed * 255.0 / 100.0);
    
    float steeringNorm = steering / 1800.0;
    float cubicSteering = steeringNorm * steeringNorm * steeringNorm;
    float blendedSteering = (steeringNorm * (1.0 - (steerExpo / 100.0))) + (cubicSteering * (steerExpo / 100.0));
    
    float steeringScaled = blendedSteering * (maxSpeed * 255.0 / 100.0) * (steerRate / 100.0);

    float rawLeft = throttleScaled + steeringScaled;
    float rawRight = throttleScaled - steeringScaled;

    // 6. Directional Trim (C-07 Fix: Only applied when actively driving)
    if (isActivelyDriving) {
      if (throttle >= 0) {
        if (forwardTrim > 0) rawLeft -= forwardTrim;
        else if (forwardTrim < 0) rawRight -= abs(forwardTrim);
      } else {
        if (reverseTrim > 0) rawLeft -= reverseTrim;
        else if (reverseTrim < 0) rawRight -= abs(reverseTrim);
      }
    }

    leftPWM = constrain((int)rawLeft, -255, 255);
    rightPWM = constrain((int)rawRight, -255, 255);
  }

  // 7. Jitter Floor (values below 15 become 0)
  if (abs(leftPWM) < 15) leftPWM = 0;
  if (abs(rightPWM) < 15) rightPWM = 0;

  // 8. Convert Sign-Magnitude to DRV8833 Dual-Pin PWM Inputs (A1/A2, B1/B2)
  int a1 = 0, a2 = 0, b1 = 0, b2 = 0;

  if (leftPWM > 0) {
    a1 = leftPWM;
    a2 = 0;
  } else if (leftPWM < 0) {
    a1 = 0;
    a2 = abs(leftPWM);
  }

  if (rightPWM > 0) {
    b1 = rightPWM;
    b2 = 0;
  } else if (rightPWM < 0) {
    b1 = 0;
    b2 = abs(rightPWM);
  }

  // 9. Motor Direction Reversal & Channel Swapping
  if (motorADir == 1) { int temp = a1; a1 = a2; a2 = temp; }
  if (motorBDir == 1) { int temp = b1; b1 = b2; b2 = temp; }

  if (motorSwap == 1) {
    SendingData.motor_A1 = b1;
    SendingData.motor_A2 = b2;
    SendingData.motor_B1 = a1;
    SendingData.motor_B2 = a2;
  } else {
    SendingData.motor_A1 = a1;
    SendingData.motor_A2 = a2;
    SendingData.motor_B1 = b1;
    SendingData.motor_B2 = b2;
  }

  // 10. Weapon Control Logic
  if (UpButton) {
    SendingData.weapon = map(spinSpeed, 0, 100, 0, 180);
  } else {
    SendingData.weapon = weaponOnVal;
  }
}
