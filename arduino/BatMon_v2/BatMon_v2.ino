/*
Carduino BatMon_v2 - An Arduino powered battery monitor for your car
 Version 0.3, March 2014
 
 david@wyolum.com - http://forum.wyolum.com/forumdisplay.php?fid=13
 
 BatMon monitors the voltage of your car battery (V_batt) while powering an external 12V load.   
 If your car battery drops below a set threshold (V_cuttoff) the load will be switched off until 
 the car battery rises above a set voltage, V_recovery.  BatMon can also be configured to automatically 
 switch off power to the load after a period of time (Time_cuttoff) once the car has been switched off. 
 
 Assume that the maximum voltage is 15V. At this maximum voltage we want to see no 
 more than 4.0V on an analog arduino pin.  The voltage divider looks like this:
 
 Vbatt +<--- R1 --- Analog_measurement --- R2 ---||| Ground
 
 R1 + R2 = 150kOhms        to minimise current draw
 R2 = (4V / 15V) * 150kOhms = 40kOhms
 R1 = 110kOhms
 
 Press and hold the mode button for three seconds at any time to re-enter mode selection mode.
 Press the button the number of times corresponding to the mode you wish to select.
 For example, to enter mode three, press the button three times.
 After a short pause with no input the LED will flash the corresponding number of times
 to confirm the mode and return to normal operation.
 The selected mode is held in non-volitile memory and will remain the same until changed.
 
 Set 0 : V_cuttoff is undefined, default to 12.2V = Set 1
 Set 1 : V_cuttoff = 12.2V
 Set 2 : V_cuttoff = 12.1V
 Set 3 : V_cuttoff = 12.0V
 Set 4 : V_cuttoff = 11.9V
 Set 5 : V_cuttoff = 11.8V
 
 */

#include <Time.h>
#include <TimerOne.h>
#include "BatMon.h"

// Cutoff voltages depending on the mode
const float v_cutoffs[6] = { 0, 12.2, 12.1, 12.0, 11.9, 11.8 };

int readings[NUMSAMPLES];
int readingindex = 0;

int cutoff_mode = CUTOFF_MODE_DEFAULT;
float V_cutoff = v_cutoffs[CUTOFF_MODE_DEFAULT];
float V_engineON = V_ENGINE_ON;    // Voltage above which the engine is running, battery charging

BatMonState currentState = UNINITIALISED;
BatMonState newState = UNINITIALISED;
int timeInNewState = 0;

int globalTime = 0;

void setup()
{
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // turn the relay on during startup or reboot
  
  // Check the mode memory in eprom, set V_cuttoff to this value
  
  FastFlash();
  BlinkModeNumber(); 
  
  // TJS: fill up readings so there is something to average over
  for(int i=0; i < N_SAMPLE; i++){
    readings[i] = analogRead(V_BATT_PIN);
  }
  
  // Setup the timer to execute the BatMon code at a frequency of LOOP_FREQ Hz
  Timer1.initialize(1000000 / LOOP_FREQ);
  Timer1.attachInterrupt(batMonLoop);
}

void loop()
{
  // Do whatever you want to in the loop()
}

void batMonLoop() {
  float V_batt = averagedRead() * 0.0184; // 5.0 / 1024.0 * ((R1+R2) / R2);

  globalTime++;

  // Check the state of the button (assuming button pressed forces pin low?)
  if(digitalRead(BUTTON_PIN) == LOW) {
    getUserInput();
  }

  processStateMachine(V_batt);
}

// This function is called at LOOP_FREQ Hz
void processStateMachine(float volts)
{
  BatMonState tempState = newState;
  
  newState = getcurrentState(volts);
  
  // Update if we are switching states
  if(newState != currentState) {
    
    if(newState == tempState) {
      timeInNewState++;
    } else {
      timeInNewState = 0;
    }
    // Whether we are switching state or not is determined in switchState()
    switchState();
    
  } else {
    timeInNewState = 0;
  }
  
  // Display the state we are in on the LED
  blinkState();
}

void getUserInput() {
  while(1) {
    // Get the beginning time
    
    // Wait for the button to be released
    while(digitalRead(BUTTON_PIN) == LOW) { }
    
    // Check how long the button has been pressed for
  }
}

// Determines what BatMonState the car is currently in depending on the battery voltage
BatMonState getcurrentState(float volts) {
  if(volts > V_ENGINE_ON + HYSTERESIS) {
    return ENGINE_RUNNING;
  } else if(volts <= V_ENGINE_ON - HYSTERESIS && volts > v_cutoffs[cutoff_mode] + HYSTERESIS) {
    return ENGINE_STOPPED;
  } else if(volts <= v_cutoffs[cutoff_mode] - HYSTERESIS) {
    return LOW_POWER_MODE;
  } else {
    // If none of the former states has been detected it is because we are in a 'gray zone'
    // determine by HYSTERESIS. We don't switch states in these zones, which is why
    // currentState is returned.
    return currentState;
  }
}

void switchState() {
  // Based on newState and timeInNewState, determine if we are switching states
  switch(newState) {
    
  case ENGINE_RUNNING:
    if(timeInNewState >= HIGHDELAY) {
      // The engine has been on for HIGHDELAY. We assume that it is running
      currentState = newState;
      Serial.println("Entered new state: engine running");
    }
    break;
  
  case ENGINE_STOPPED:
    // We immediately switch to to engine stopped without delay
    currentState = newState;
    Serial.println("Entered new state: engine stopped");
    break;
    
  case LOW_POWER_MODE:
    if(timeInNewState >= LOWDELAY) {
      // The voltage has been below low_power cutoff for LOWDELAY. We
      // assume the battery is low.
      currentState = newState;
      Serial.println("Entered new state: low power mode");
    }
    break;
    
  default:
    // Do nothing
    break;
  }
}

// Blinks depending on the state
void blinkState() {
  // Display the current mode on the LEDs
  if(currentState == ENGINE_RUNNING || newState == ENGINE_RUNNING) {
    digitalWrite(LED_GREEN, HIGH);  // turn on green LED
    if(currentState != ENGINE_RUNNING) {
      // We must be starting up! Make the LED amber
      digitalWrite(LED_RED, HIGH);
    } else {
      digitalWrite(LED_RED, LOW);
    }
  } else {
    int redLed, greenLed;
    
    if(currentState == ENGINE_STOPPED) {
      greenLed = HIGH;
    }
    
    if(newState == LOW_POWER_MODE || currentState == ENGINE_STOPPED) {
      redLed = LOW;
    }
    
    // This is a dity trick to make the LED blink at LOOP_FREQ / 32 Hz.
    // The alternative would be to do divisions by 10 of globalTime
    // which are costly on an AVR.
    if((globalTime & 0x001F) < 0x0010) {
      // For the first 16 loop()s the LED is on
      digitalWrite(LED_GREEN, greenLed);
      digitalWrite(LED_RED, redLed);
    } else {
      // For the next 16 loop()s the LED is off
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
    }
  }
}

void FastFlash() {
  // Quickly blink the LEDs between red and green to show we
  // are about to display the current mode
  for(int i = 0; i < 10; i++) {
    digitalWrite(LED_GREEN, HIGH);  // turn on green LED
    digitalWrite(LED_RED, LOW);     // turn off red LED
    delay(100);                     // LEDs flashing at 5Hz
    
    digitalWrite(LED_GREEN, LOW);   // and now the other way round
    digitalWrite(LED_RED, HIGH);
    delay(100);
  }
}

void BlinkModeNumber() {
  Serial.print("Blink ");
  Serial.println(cutoff_mode);
  
  for (int i=0; i < cutoff_mode; i++){
    digitalWrite(LED_GREEN, HIGH);  // turn on both LEDs for amber light
    digitalWrite(LED_RED, HIGH);
    delay(500);                     // wait for a second
    
    digitalWrite(LED_GREEN, LOW);   // turn off both LEDs
    digitalWrite(LED_RED, LOW);
    delay(500);                     // wait for a second
  }
}

float averagedRead()   // Return the average reading from the V_BATT_PIN.  Take NUMSAMPLES readings and average.
{
  int averageValue=0;
  readings[readingindex] = analogRead(V_BATT_PIN);
  readingindex = (readingindex + 1) % NUMSAMPLES;
  for (int i=0; i < NUMSAMPLES; i++){
    averageValue = readings[i] + averageValue;
  }
  return(averageValue / NUMSAMPLES);
}
