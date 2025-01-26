# Controller module

The controller module consists of a magnetic reed sensor, a push button, a buzzer and an ESP32 module.
The ESP32 module is the controller module of a BLE network and is read to receive signal from the device modules.
Whenever a device module sends a signal, the controller module fires the buzzer if the system's state is in ARM
mode (ARM mode is the mode where all residents are away from the residence and the last to leave arms the alarm by pushing 
the push button). By the time a user arms the alarm, a timer is triggered, so that any door change state doesn't 
fire the alarm by accident and the user has got time to leave the residence. When a user return to the residence he has
got 20 seconds to Disarm the alarm by pressing the push button so that the buzzer doesn't ring after the 20 second interval.
