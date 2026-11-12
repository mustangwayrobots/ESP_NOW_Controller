/**
 * ==============================================================================
 * MotorControl.ino - Arcade Drive Mixing, Trim, and Weapon Control
 * ==============================================================================
 * Takes the two raw stick readings and fills in SendingData (the packet for the car).
 * Called every loop; it does not send anything itself.
 */

// Moves a motor command toward its target, but never speeds up faster than maxStep.
// Slowing down or stopping is instant; reversing stops first, then speeds up.
static float applyAccelLimit(float current, float target, float maxStep) {
  if (current != 0 && ((target > 0) != (current > 0))) current = 0;  // reversing: stop first
  if (fabs(target) <= fabs(current)) return target;                  // slowing down: no limit
  float diff = target - current;                                     // speeding up: limited
  if (diff > maxStep) diff = maxStep;
  else if (diff < -maxStep) diff = -maxStep;
  return current + diff;
}

void CalculateMotorDriverValues(int rawThrottle, int rawSteer) {
  // 1. Retrieve settings from the named list (see SettingId in ConfigData.h)
  int maxSpeed       = currentSettings[SET_MAX_SPEED];
  int forwardTrim    = currentSettings[SET_FORWARD_TRIM];
  int reverseTrim    = currentSettings[SET_REVERSE_TRIM];
  int steerRate      = currentSettings[SET_STEER_RATE];
  int steerExpo      = currentSettings[SET_STEER_EXPO];
  int spinSpeed      = currentSettings[SET_SPIN_SPEED];
  int motorADir      = currentSettings[SET_MOTOR_A_DIR];
  int motorBDir      = currentSettings[SET_MOTOR_B_DIR];
  int motorSwap      = currentSettings[SET_MOTOR_SWAP];
  int weaponOnVal    = currentSettings[SET_WEAPON_ON];
  int weaponOffVal   = currentSettings[SET_WEAPON_OFF];
  int deadzone       = currentSettings[SET_DEADZONE];
  int centerThrottle = currentSettings[SET_ZERO_THROTTLE];  // stick centers, calibrated in the menu
  int centerSteer    = currentSettings[SET_ZERO_STEER];
  if (deadzone < DEADZONE_MIN) deadzone = DEADZONE_MIN;     // Enforce safe minimum deadzone

  // 2. Stick position relative to center. Pushing the stick forward LOWERS the
  //    throttle reading on this board, hence the minus sign.
  int throttle = -(rawThrottle - centerThrottle);
  int steering = rawSteer - centerSteer;

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
    //    Stick positions are turned into -1.0 ... +1.0 and clamped, so a reading
    //    beyond STICK_FULL_RANGE can never push the math past full scale.
    float fullScale    = maxSpeed * 255.0 / 100.0;
    float throttleNorm = constrain(throttle / (float)STICK_FULL_RANGE, -1.0f, 1.0f);
    float steeringNorm = constrain(steering / (float)STICK_FULL_RANGE, -1.0f, 1.0f);

    float throttleScaled = throttleNorm * fullScale;

    float cubicSteering = steeringNorm * steeringNorm * steeringNorm;
    float blendedSteering = (steeringNorm * (1.0 - (steerExpo / 100.0))) + (cubicSteering * (steerExpo / 100.0));
    float steeringScaled = blendedSteering * fullScale * (steerRate / 100.0);

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

  // 7. Acceleration limit (ACCEL RATE): a gentle start keeps the car straight when
  //    it takes off, which makes the trim test easier to judge. 100% = no limit.
  int accelRate = currentSettings[SET_ACCEL_RATE];
  static float rampLeft = 0, rampRight = 0;
  static unsigned long lastRampTime = 0;
  unsigned long now = millis();
  unsigned long dt = now - lastRampTime;
  lastRampTime = now;
  if (dt > ACCEL_MAX_DT_MS) dt = ACCEL_MAX_DT_MS;

  if (accelRate >= 100) {
    rampLeft = leftPWM;
    rampRight = rightPWM;
  } else {
    float rampMs  = (100 - accelRate) * (float)ACCEL_FULL_RAMP_MS / 100.0;
    float maxStep = 255.0 * dt / rampMs;
    rampLeft  = applyAccelLimit(rampLeft,  leftPWM,  maxStep);
    rampRight = applyAccelLimit(rampRight, rightPWM, maxStep);
  }
  leftPWM  = (int)rampLeft;
  rightPWM = (int)rampRight;

  // 8. Jitter Floor (tiny values just make the motors buzz, so they become 0)
  if (abs(leftPWM) < MOTOR_MIN_PWM) leftPWM = 0;
  if (abs(rightPWM) < MOTOR_MIN_PWM) rightPWM = 0;

  // 9. Convert Sign-Magnitude to DRV8833 Dual-Pin PWM Inputs (A1/A2, B1/B2)
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

  // 10. Motor Direction Reversal & Channel Swapping
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

  // 11. Weapon: UP held = WEAPON ON angle, released = WEAPON OFF angle
  SendingData.weapon = UpButton ? weaponOnVal : weaponOffVal;
}
