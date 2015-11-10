##11/10/15
-*Hardware version incremented to H*
-*Added hardware support for accent LEDs* (user request)
-Added connector for buttons with built-in LEDs
-Added support for kill-key hard power switch. This simply disables the SMPS, which reduces power use to negligible levels
-We've officially used all available GPIO

##07/04/15
-*Firmware version incrementerd to 0.5.2*
-*Hardware version incremented to G2*
-Connected D+/D- pins on bq24257 for possible future use--currently unused in firmware
-Fixed major firmware issue where device would not wake from sleep mode, and had to be power cycled
-Fixed condition with bq24257 where it could not recover from completely dead battery--also can now be powered by USB with no battery connected
##06/07/15
-*Firmware version incremented to 0.5.1*
-*Hardware version incremented to G1*
-swapped pins 1 and 2
-moved CYC7C65213 Vcc pins from Vcc to Vbus. There was a latchup issue if the device was plugged into USB without a battery connected (no Vcc)
-added R10 on CYC7C65213; this is a current limiting resistor to prevent latchup
-RESET net wasn't properly connected
-deprecated recovery format functionality -- pin 24 (analog 1) is now free
