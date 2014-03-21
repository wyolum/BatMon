
#ifndef _BATMON_H_
#define _BATMON_H_


#define NUMSAMPLES 7    // Number of samples to collect whilst measuring the average analogue voltage

// Defines the different cutoff modes
#define CUTOFF_MODE_NONE      0    // Note sure what cutoff undefined means...
#define CUTOFF_MODE_12_2      1
#define CUTOFF_MODE_12_1      2
#define CUTOFF_MODE_12_0      3
#define CUTOFF_MODE_11_9      4
#define CUTOFF_MODE_11_8      5
#define CUTOFF_MODE_DEFAULT   CUTOFF_MODE_12_2

#define V_ENGINE_ON           13.0
#define HYSTERESIS            0.1

#define V_BATT_PIN            A0   // V_battery is connected to analog pin 0
#define BUTTON_PIN            A2   // Button is connected to analog pin 2
#define RELAY_PIN             A3   // Relay output pin is connected to analog pin 3

#define LED_GREEN             13
#define LED_RED               14

#define N_SAMPLE              5    // Average analog readings over 5 samples

#define LOOP_FREQ             10   // Frequency of the main loop, in Hz

// LOWDELAY is the seconds required for voltage to be low
// before cutoff (filtering transients)
#define LOWDELAY 10 * LOOP_FREQ

// HIGHDELAY is the seconds required for voltage to be OK
// before turning back on (filtering transients)
#define HIGHDELAY 3 * LOOP_FREQ

/* BatMonStates:
  ENGINE_RUNNING = Relay ON and VBatt > 13v (Engine is running)
  ENGINE_STOPPED = Relay ON and VBatt > Vcuttoff (Engine is off, but battery is ok)
  LOW_POWER_MODE = Relay OFF and VBatt < Vcuttoff, low power mode
 */
typedef enum BatMonState {
  ENGINE_RUNNING,
  ENGINE_STOPPED,
  LOW_POWER_MODE,
  UNINITIALISED
};

#endif // _BATMON_H_
