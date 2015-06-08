##Overview

At the base level, the IoDriver is nothing more than a PWM LED driver with some fancy software. The hardware is relatively simple and straightforward. There are three major ICs on the board; the main ATmega328(p) microcontroller, a CYC7C65213 USB to serial converter, and a TI BQ24257 lithium-ion battery charger. In addition there is a Micrel MIC2876 5v boost converter, and a CAT25160 16kB EEPROM. The relatively high current to the LEDs is switched with FDN327N N-channel MOSFETs.

##General operation description

The LEDs are pulse width modulated to change their intensities and create different colors. The profiles store RGBW values, and we use a custom algorithm to generate the gradient sequence between each RGBW color (See color.cpp and component.cpp). The button is tied to an interrupt that increments the profile, as well as shut down and wake up the processor. Profiles are stored in EEPROM, and have a fairly simple structure.


####Charging

The Cypress USB to serial converter has the capability to detect when the device is plugged into a high-current USB port. When the IoDriver is plugged in, the TI charge controller starts up in standalone mode, with a charge current of 100mA, to prevent damaging an unknown charger. The TI device has an active-low power\_good output, which the main program checks at the start of each loop. When the power_good signal is detected, the IoDriver goes into charge mode. Within 100mS, the Cypress device will have determined the charger type (how much current it can supply) and toggles the states of two of its GPIO pins. The processor waits 200mS then reads the status of these GPIO lines, decides what the charge current should be, and sends that value to the TI device over the I2C bus. The TI charge controller is internally limited to 2A, which allows a charge from completely dead in just over an hour.

While the IoDriver is in charge mode, it polls the charge controller for the charge status (either done, charging, or error), then sets the LED to a low-intensity color to reflect that. Amber for charging, green for done, and blue or putple for error. A request or incoming data from USB will interrupt this loop to send or receive profile information, or a color preview override.

During charging, the LED will stay lit for about 5 minutes, then shut off. Once charging is complete, it will come back on green for another five minutes. The system is still powered on, so it will still immediately respond to any communication from the PC software.

Blue and purple are used for error indication to avoid confusion with the normal amber color. A purple error should only occur if the IoDriver is plugged in without a battery attached, and a blue error indicates any general charge error.

####Battery information

The ATmega device periodically checks the supply voltage, and when it falls below the BATTERY\_MINIMUM threshold (3.2V), it will blink red for about 10 seconds, then go into shutdown mode. The device can be woken back up, but a flag is raised in memory, locking it into low power mode until a charger is connected. In the event that the battery is deeply discharged, the TI charge controller can detect this, and will go through an algorithm to attempt to recover the cell.

The processor does not check supply voltage while charging, so as to not bail into shutdown mode when it's plugged in.

Additionally, there is battery protection circuitry on a dedicated board attached to the cell which protects against over- and under-charge, as well as over-current situations.

###Other design notes

The latest revision (G) saw the switch from the ATmega32u4 to the ATmega328(p) processor, as well as replacing the expensive Maxim USB charge detector IC with a USB to serial converter that has that functionality built in. This reduced both cost and complexity, and allowed us to go from a 4 layer PCB back to a much cheaper 2 layer board. Part of the motivation for this was the unpredictable stock of the ATmega32u4; we had to resort to sourcing the chips from ebay because all of the regular distributors were sold out. In addition, the ATmega328 processors cost about half of the 32u4.

Revision G also marks a major change in overall board design. All of the large parts have been put on the top side of the board, allowing for easier machine assembly.

Older hardware versions had an issue where the switching FETs weren't fully turning on. Changing all of the digital circuitry to run on a 5V rail provided by the boost converter solved this problem. Because of the high current requirements, the LED supply is still taken straight from the battery.

####Expansion connectors

There are two connectors on the front of the IoDriver board that are intended for expansion modules. Currently planned are a bluetooth module (with accompanying iOS/Android apps), an accelerometer module, and a sound board.

The large connector is just an SPI bus, with two chip select lines. It also carries all power to the expansion modules, as well as acting as the first-time ISP programmer for the ATmega328. Power to the expansion port is swiched with a MOSFET to prevent the modules consuming power when the system is supposed to be in sleep mode. Because of the body diode in the MOSFET, it's also possible to power the board from this connector for initial programming.

The second connector carries the I2C and RS232 Rx/Tx lines.
