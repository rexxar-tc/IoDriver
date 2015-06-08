##06/07/15
-*Firmware version incremented to 0.5.1*
-*Hardware version incremented to G1*
-swapped pins 1 and 2
-moved CYC7C65213 Vcc pins from Vcc to Vbus. There was a latchup issue if the device was plugged into USB without a battery connected (no Vcc)
-added R10 on CYC7C65213; this is a current limiting resistor to prevent latchup
-RESET net wasn't properly connected
-deprecated recovery format functionality -- pin 24 (analog 1) is now free
