##Overview

At the base level, the IoDriver is nothing more than a PWM LED driver with some fancy software. The hardware is relatively simple and straightforward. There are three major ICs on the board; the main ATmega328(p) microcontroller, a CYC7C65213 USB to serial converter, and a TI BQ24257 lithium-ion battery charger. In addition there is a 74HC164 shift register to provide chip select lines for expansion modules and a CAT25160 16kB EEPROM. The relatively high current to the LEDs is switched with FDN327N N-channel MOSFETs.

##General operation description

The LEDs are pulse width modulated to change their intensities and create different colors. The profiles store RGBW values, and we use a custom algorithm to generate the gradient sequence between each RGBW color (See color.cpp and component.cpp). The button is tied to an interrupt that increments the profile, as well as shut down and wake up the processor. Profiles are stored in EEPROM, and have a fairly simple structure.


####Charging

The Cypress USB to serial converter has the capability to detect when the device is plugged into a high-current USB port. When the IoDriver is plugged in, the TI charge controller starts up in standalone mode, with a charge current of 100mA, to prevent damaging an unknown charger. The TI device has an active-low power\_good output, which the main program checks at the start of each loop. When the power_good signal is detected, the IoDriver goes into charge mode. Within 100mS, the Cypress device will have determined the charger type (how much current it can supply) and toggles the states of two of its GPIO pins. The processor waits 200mS then reads the status of these GPIO lines, decides what the charge current should be, and sends that value to the TI device over the I2C bus. The TI charge controller is internally limited to 2A, which allows a charge from completely dead in just over an hour.

While the IoDriver is in charge mode, it polls the charge controller for the charge status (either done, charging, or error), then sets the LED to a low-intensity color to reflect that. Amber for charging, green for done, and blue or putple for error. A request or incoming data from USB will interrupt this loop to send or receive profile information, or a color preview override.

Once charging is complete, the green LED will stay lit for 5 minutes, then turn off. The system is still powered on, so it will still immediately respond to any communication from the PC software. Additionally, since the internal voltage reference used to measure supply voltage is very innacurate (though very stable), the system will perform a calibration of the reference when charging is complete, and store this value in EEPROM. This gives a more accurate measurement of battery voltage, which could prevent it from shutting off prematurely.

Blue and purple are used for error indication to avoid confusion with the normal amber color. A purple error should only occur if the IoDriver is plugged in without a battery attached, and a blue error indicates any general charge error.

####Battery information

The ATmega device periodically checks the supply voltage, and when it falls below the BATTERY\_MINIMUM threshold (3.4V), it breaks to the blinkRed routine which, aptly, makes the LED blink red. When supply voltage falls below the BATTERY_CRITICAL threshold (3V), the processor disables the charge controller, turns off power to the expansion bus, and then goes into full shutdown mode, to save the battery from over-discharge. In the event that the battery is deeply discharged, the TI charge controller can detect this, and will go through an algorithm to attempt to recover the cell.

The processor does not check supply voltage while charging, so as to not bail into shutdown mode when it's plugged in.

Additionally, there is battery protection circuitry on a dedicated board attached to the cell which protects against over- and under-charge, as well as over-current situations.

###Other design notes

The latest revision (G) saw the switch from the ATmega32u4 to the ATmega328(p) processor, as well as replacing the expensive Maxim USB charge detector IC with a USB to serial converter that has that functionality built in. This reduced both cost and complexity, and allowed us to go from a 4 layer PCB back to a much cheaper 2 layer board. Part of the motivation for this was the unpredictable stock of the ATmega32u4; we had to resort to sourcing the chips from ebay because all of the regular distributors were sold out. In addition, the ATmega328 processors cost about half of the 32u4.

Revision G also marks a major change in overall board design. All of the large parts have been put on the top side of the board, allowing for easier machine assembly.

####Expansion port

There is a port on the front of the IoDriver board that is intended for expansion modules. Currently planned are a bluetooth module (with accompanying iOS/Android apps), an accelerometer module, and a sound board.

Essentially the expansion bus is just an SPI bus, with four chip select lines on the connector. Power to the expansion port is swiched with a MOSFET to prevent the modules consuming power when the system is supposed to be in sleep mode. Because of the body diode in the MOSFET, it's also possible to power the board from this connector for initial programming.

####Format jumper

In some rare circumstances, corrupt or malformed information on the EEPROM can cause the IoDriver to crash when communicating with the PC software. Since this could possibly brick the device, there are two exposed pads on the board which will completely format all EEPROM and will recover the device. To format the device, you disconnect the battery, hold a bit of aluminum foil across the pads, then plug the battery back in. The LED will be lit red throughout the processes, whcich takes about a minute. This formats __all__ EEPROM, including battery calibration data, so it's recommended to let the IoDriver recharge fully before using it again.
