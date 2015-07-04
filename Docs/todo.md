##ToDo
Include prebuilt firmware binaries
General code cleanup
Optimization of SPI EEPROM routines

##Bugs:
-GUI program will fail and disconnect when polling for connection status [Workaround: set polling period to 10 minutes]
-Sometimes color will freeze when plugging device in, pressing the button will switch it back to the charging color. (can't seem to reproduce?)
-Preview profile can sometimes get corrupted during transmission.
-Writing a single byte to the second EEPROM under specific conditions will fail with no warning.


##Fixed bugs:
-LED can sometimes stress SMPS to the point of overload, thermal shutdown.
Fix: changed LED supply back to battery voltage.
-Battery voltage drops due to LED current, causes premature shutdown [Workaround: battery voltage checking is disabled for now, relying on protection board]
Fix: Workaround caused Li protection circuit to isolate battery; charging circuit couldn't recover. Now averaging voltage over 10 seconds to account for voltage droop.
-Profile preview function in GUI doesn't work
Fix: program now actually checks if there's a preview profile coming in
-When resuming from sleep, it will switch to the next profile
Fix: program now checks if we're resuming from sleep; disables profile advance
-Device doesn't wake when plugged in while asleep.
Fix: Moved PowerGood signal to interrupt pin.

##Issues:
-Opening serial port causes a reset. This is by design, but GUI can't cope [Workaround: removed C4; this means firmware is not user updateable] May include manual reset button later to allow user updates.
