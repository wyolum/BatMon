BatMon
======

Arduino compatible car battery monitor with relay controlled output

BatMon is designed to prevent a car battery from going flat when used with a 12v device, like an dashboard camera, operating when the engine is switched off.  BatMon monitor's the car's battery and switches off the accessory (camera) once the voltage goes below a set threshold for a period of time.

Physical setup:
BatMon is connected to a constant 12V supply, typically from the car's fuse box and ground.  There is a single 12V output controlled by a and ground.  There is one SPST momentary switch and a bi-colour red/green LED.  

User interface:
Upon power-up BatMon will blink Red/Green rapidly for two second followed by a series of slow amber blinks corresponding to the saved setpoint in non-volitile memory.  

1 Amber = Cuttoff at 12.2V
2 Amber = Cuttoff at 12.1V
3 Amber = Cuttoff at 12.0V
4 Amber = Cuttoff at 11.9V
5 Amber = Cuttoff at 11.8V

If no previous cutoff point has been set it will default to 1, or 12.2 volts.

BatMon will now enter normal operational mode, Green LED and SSR (solid state relay) active.  

To change the set point the user must press and hold the mode switch for three second.  The LED's will flash red/green rapidly for two second.  The user then presses the button the number of times corresponding to the requested mode.  After three seconds with no button input the LED will flash N slow amber pulses confirming the selected mode and return to [green] normal mode.  

Example - switch to mode 3, cutoff at 12.0V:
Press and hold button for three seconds.  LED's flash red/green for two seconds.  Press button three times.  3 second delay.  Led flashes 3 times amber slowly, then turns green.

If the user attempts to enter an invalid choice, like 6 presses for mode 6, the LED's will flash red/green for two seconds and then wait for the user to re-enter their mode choice.  If no entry is made within 3 seconds the led will flash the previous set point code and resume normal mode.

If the user makes a mistake they may hold down the button for three seconds at any time to restart the set point sequence.

Normal mode operates as a state machine:
 0 = Relay ON and VBatt > 13v (Engine is running)
 1 = Relay ON and VBatt > Vcuttoff (Engine is off, but battery is ok)
 2 = Relay ON and VBatt < Vcuttoff, time less than LOWDELAY
 3 = Relay OFF and VBatt < Vcuttoff, low power mode
 4 = Relay OFF and VBatt > 13v, (Car started, but time less than HIGHDELAY)    

When the engine is running BatMon will be in normal mode 0, Green LED on constantly.

When the engine is not running BatMon will be in normal mode 1, green LED blinking slowly.

Once the battery dips below the threshold it will be in normal mode 2, amber LED blinking slowly. The voltage must remain below the threshold for LOWDELAY or revert back to normal mode 1.  The reason for this is to accomodate high current drain accessories present when a car shuts down, like headlights, wipers and the radio.  This also prevents the device from switching off too early in vehicles equiped with start/stop engines, like a prius.  LOWDELAY is typically 2-5 minutes.

Once the voltage has been below the thresholf for LOWDELAY BatMon enters normal mode 3, LED slow blinking red.  The relay is switched off and the Arduino enters low power mode.

Once the voltage goes above 13V (the car has been started) BatMon will enter normal mode 4, LED solid amber.  Once the device has been in mode 4 for longer than HIGHDELAY (typically 3-5 seconds) it will switch to normal mode 0, LED constant green.