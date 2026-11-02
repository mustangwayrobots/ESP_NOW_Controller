/**
 * ==============================================================================
 * MotorControl.ino - Clean Arcade Drive, Spin-on-Demand, and Independent Trim
 * ==============================================================================
 */
void CalculateMotorDriverValues(int rawX, int rawY) {
  // --- 1. AXIS MAPPING ---
  int throttle = -rawY; 
  int steering = rawX;

  // --- 2. EXTRACT CONFIGURATION SETTINGS (Name-Based Lookups) ---
  int MaxSpeed    = map(getSetting("MAX SPEED"), 0, 100, 0, 255); 
  int ForwardTrim = getSetting("FORWARD TRIM");                      
  int ReverseTrim = getSetting("REVERSE TRIM");                      
  int rampStep    = map(getSetting("ACCEL RATE"), 0, 100, 5, 127); 
  int DeadZone    = getSetting("JOYSTICK DEADZONE");                 
  
  float steerRatePct = getSetting("STEER RATE") / 100.0; 
  float steerExpoPct = getSetting("STEER EXPO") / 100.0; 
  
  int spinSpeed = map(getSetting("SPIN SPEED"), 0, 100, 0, 255);

  // --- 3. APPLY DEADZONE ---
  if (abs(throttle) < DeadZone) throttle = 0;
  if (abs(steering) < DeadZone) steering = 0;

  int leftDriveSpeed = 0;
  int rightDriveSpeed = 0;

  // --- 4. SPIN-ON-DEMAND VS NORMAL ARCADE DRIVE ---
  if (SpinButton) {
    if (abs(steering) > DeadZone) {
      if (steering > 0) {
        leftDriveSpeed  = spinSpeed;
        rightDriveSpeed = -spinSpeed;
      } else {
        leftDriveSpeed  = -spinSpeed;
        rightDriveSpeed = spinSpeed;
      }
    } else {
      leftDriveSpeed  = 0;
      rightDriveSpeed = 0;
    }
  } else {
    int throttleVal = map(throttle, -1023, 1023, -MaxSpeed, MaxSpeed);

    float normSteer = steering / 1023.0;
    float cubicSteer = normSteer * normSteer * normSteer;
    float blendedSteer = ((1.0 - steerExpoPct) * normSteer) + (steerExpoPct * cubicSteer);

    int maxSteeringCap = (int)(MaxSpeed * steerRatePct);
    int steeringVal = (int)(blendedSteer * maxSteeringCap);

    leftDriveSpeed  = throttleVal + steeringVal;
    rightDriveSpeed = throttleVal - steeringVal;

    // --- INDEPENDENT DIRECTIONAL TRIM ---
    int activeTrim = ForwardTrim;
    if (throttleVal < 0) {
      activeTrim = ReverseTrim; 
    }

    if (activeTrim > 0) {
      leftDriveSpeed = constrain(leftDriveSpeed - activeTrim, -255, 255);
    } else if (activeTrim < 0) {
      rightDriveSpeed = constrain(rightDriveSpeed + activeTrim, -255, 255);
    }
  }

  leftDriveSpeed  = constrain(leftDriveSpeed, -255, 255);
  rightDriveSpeed = constrain(rightDriveSpeed, -255, 255);

  // --- 5. TRANSIENT INERTIA SMOOTHING ---
  static int prevLeft = 0;
  static int prevRight = 0;
  
  leftDriveSpeed  = constrain(leftDriveSpeed, prevLeft - rampStep, prevLeft + rampStep);
  rightDriveSpeed = constrain(rightDriveSpeed, prevRight - rampStep, prevRight + rampStep);
  
  prevLeft = leftDriveSpeed;
  prevRight = rightDriveSpeed;

  // --- 6. JITTER FLOOR ---
  if (abs(leftDriveSpeed) < 15)  leftDriveSpeed = 0;
  if (abs(rightDriveSpeed) < 15) rightDriveSpeed = 0;

  // --- 7. Convert Wheel Speed to PWM PIN Values ---
  int a1 = 0, a2 = 0, b1 = 0, b2 = 0;

  if (leftDriveSpeed > 0) {
    a1 = leftDriveSpeed;  
    a2 = 0;               
  } else if (leftDriveSpeed < 0) {
    a1 = 0;               
    a2 = -leftDriveSpeed; 
  }

  if (rightDriveSpeed > 0) {
    b1 = rightDriveSpeed; 
    b2 = 0;               
  } else if (rightDriveSpeed < 0) {
    b1 = 0;               
    b2 = -rightDriveSpeed;
  }

  SendingData.motor_A1 = a1;
  SendingData.motor_A2 = a2;
  SendingData.motor_B1 = b1;
  SendingData.motor_B2 = b2;

  // --- 8. HARDWARE CONFIGURATION OVERRIDES (Name-Based Lookups) ---
  int tempHolding;
  if (getSetting("MOTOR \"A\" DIRECTION") != 0){ 
    tempHolding = SendingData.motor_A1;
    SendingData.motor_A1 = SendingData.motor_A2;
    SendingData.motor_A2 = tempHolding;
  }
  if (getSetting("MOTOR \"B\" DIRECTION") != 0){ 
    tempHolding = SendingData.motor_B1;
    SendingData.motor_B1 = SendingData.motor_B2;
    SendingData.motor_B2 = tempHolding;
  }
  if (getSetting("MOTOR A-B RIGHT-LEFT") != 0){ 
    tempHolding = SendingData.motor_A1;
    SendingData.motor_A1 = SendingData.motor_B1; 
    SendingData.motor_B1 = tempHolding; 
    tempHolding = SendingData.motor_A2;
    SendingData.motor_A2 = SendingData.motor_B2; 
    SendingData.motor_B2 = tempHolding; 
  }

  // --- 9. WEAPON STATE HANDLING (Name-Based Lookups) ---
  if(UpButton == false) SendingData.weapon = getSetting("WEAPON ON VALUE"); 
  if(UpButton == true) SendingData.weapon = map(getSetting("SPIN SPEED"), 0, 100, 0, 127); 

  SendData(); 
}