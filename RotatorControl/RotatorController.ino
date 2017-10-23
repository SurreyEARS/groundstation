
// The pins use control the relays
const int UPPIN = 4;
const int DOWNPIN = 5;
const int LEFTPIN = 7;
const int RIGHTPIN = 6;

// The pins for the two sensors
const int AZIMUTHPIN = 23;
const int ELEVATIONPIN = 24;

// The pins for the four buttons
const int UPBUTTON = A5;
const int DOWNBUTTON = A4;
const int LEFTBUTTON = A3; // 'CW' on the box
const int RIGHTBUTTON = A2; // 'CCW' on the box
const int PWR = 3;

// Scale Factor (Check this is correct values)
// In degrees * 100
const long AZIMUTHSCALE = 232;
const long ELEVATIONSCALE = 568;
const long AZIMUTHMAX = 45000; // TODO check this is the real maz
const long AZIMUTHMIN = -9000; // TODO Check this is the real minimum
const long ELEVATIONMAX = 18000;
const long ELEVATIONMIN = 9000; // TODO Find out the real minimum
const long TOLERANCE = 100;

long azimuthOffset = 325;
long elevationOffset = 0;

long azimuthCurrent = 0;
long elevationCurrent = 0;
long azimuthGoal = 0;
long elevationGoal = 0;
long azimuthLastGoal = 0;
long elevationLastGoal = 0;

bool azimuthStop = false;
bool elevationStop = false;

char storedSerial[] = {};

void setup() {
  // Set the pin modes
  pinMode(UPPIN, OUTPUT);
  pinMode(DOWNPIN, OUTPUT);
  pinMode(LEFTPIN, OUTPUT);
  pinMode(RIGHTPIN, OUTPUT);
  pinMode(PWR, OUTPUT);
  pinMode(UPBUTTON, INPUT);
  pinMode(DOWNBUTTON, INPUT);
  pinMode(LEFTBUTTON, INPUT);
  pinMode(RIGHTBUTTON, INPUT);
  pinMode(AZIMUTHPIN, INPUT);
  pinMode(ELEVATIONPIN, INPUT);

  // Set the relays to low
  digitalWrite(UPPIN, LOW);
  digitalWrite(DOWNPIN, LOW);
  digitalWrite(LEFTPIN, LOW);
  digitalWrite(RIGHTPIN, LOW);

  // Sets STATUS/PWR to high
  // Required for the buttons to work
  digitalWrite(PWR, HIGH);

  Serial.begin(9600);
  
  Serial.println("EARS Ground Station Control");
}

void loop() {
  elevationLastGoal = elevationGoal;
  azimuthLastGoal = azimuthGoal;

  if(Serial.available()) {
    serialUpdate();
  }
  
  // Check sensors
  azimuthCurrent = ((analogRead(AZIMUTHPIN) * 10000) / AZIMUTHSCALE) - azimuthOffset;
  elevationCurrent = ((analogRead(ELEVATIONPIN) * 10000) / ELEVATIONSCALE) - elevationOffset;

  buttonCheck();

  if(azimuthGoal!=azimuthLastGoal && azimuthStop) {
    azimuthStop = false;
  }
  if(elevationGoal!=elevationLastGoal && elevationStop) {
    elevationStop = false;
  }
  
  if(!azimuthStop) {
    azimuthUpdate();
  }
  if(!elevationStop) {
    elevationUpdate();
  }
}

void azimuthUpdate(){
  long posDiff = azimuthCurrent - azimuthGoal;

  if(abs(posDiff) < TOLERANCE) {
    stopCtrl(LEFTPIN);
    stopCtrl(RIGHTPIN);
    azimuthStop = true;
  }

  // Go the short way around
  if(posDiff > 18000) {
    posDiff -= 180;
  } else if(posDiff < -18000) {
    posDiff += 180;
  }

  if(posDiff < 0) {
    if(azimuthCurrent<=AZIMUTHMIN+TOLERANCE || azimuthCurrent<=AZIMUTHMIN-TOLERANCE) {
      stopCtrl(LEFTPIN);
      // TODO Report this somewhere
      azimuthStop = true;
      Serial.println("Stop azimuth");
    } else {
      stopCtrl(RIGHTPIN);
      startCtrl(LEFTPIN);
    }
  } else {
    if(azimuthCurrent>=AZIMUTHMAX+TOLERANCE || azimuthCurrent>=AZIMUTHMAX-TOLERANCE) {
      stopCtrl(RIGHTPIN);
      // TODO Report this somewhere
      azimuthStop = true;
      Serial.println("Stop azimuth");
    } else {
      stopCtrl(LEFTPIN);
      startCtrl(RIGHTPIN);
    }
  }
}

void elevationUpdate() {
  long posDiff = elevationCurrent - elevationGoal;

  if(abs(posDiff) < TOLERANCE) {
    stopCtrl(UPPIN);
    stopCtrl(DOWNPIN);
    elevationStop = true;
  }
  
  if(posDiff < 0) {
    if(elevationCurrent<=ELEVATIONMIN+TOLERANCE || elevationCurrent<=ELEVATIONMIN-TOLERANCE) {
      stopCtrl(DOWNPIN);
      // TODO Report this somewhere
      elevationStop = true;
      Serial.println("Stop elevation");
    } else {
      stopCtrl(UPPIN);
      startCtrl(DOWNPIN);
    } 
  } else {
    if(elevationCurrent>=ELEVATIONMAX+TOLERANCE || elevationCurrent>=ELEVATIONMAX-TOLERANCE) {
      stopCtrl(UPPIN);
      // TODO Report this somewhere
      elevationStop = true;
      Serial.println("Stop elevation");
    } else {
      stopCtrl(DOWNPIN);
      startCtrl(UPPIN);
    } 
  }
}

void buttonCheck() {
  if(!digitalRead(UPBUTTON)) {
    elevationGoal = elevationCurrent + TOLERANCE + 100;
  } else if(!digitalRead(DOWNBUTTON)) { 
    elevationGoal = elevationCurrent - TOLERANCE - 100;
  }

  if(!digitalRead(LEFTBUTTON)) {
    azimuthGoal = azimuthCurrent - TOLERANCE - 100;
  } else if(!digitalRead(RIGHTBUTTON)) {
    azimuthGoal = azimuthCurrent + TOLERANCE + 100;
  }
}

void serialUpdate() {
  String stringInput = Serial.readStringUntil("/n");
  char input[8];
  stringInput.toCharArray(input, 8);
  
  Serial.println(input);
}

void startCtrl(byte pin) {
  digitalWrite(pin, HIGH);
}

void stopCtrl(byte pin) {
  digitalWrite(pin, LOW);
}
