##Overview

At the base level, the IoDriver is nothing more than a PWM LED driver with some fancy software. The hardware is simple and very straightforward. There are three ICs on the board; the main ATmega32u4 microcontroller, a MAX14578 USB charge detector, and a TI bq24257 lithium-ion battery charger. The relatively high current to the LEDs is switched with FDN327N N-channel MOSFETs.

##General operation description

The LEDs are pulse width modulated to change their intensities and create different colors. The profiles store RGBW values, and we use a custom algorithm to generate the gradient sequence between each RGBW color (See color.cpp and component.cpp). The button is tied to an interrupt that increments the profile, as well as shut down and wake up the processor. Profiles are stored in EEPROM, and have a fairly simple structure.

###Power

The IoDriver is supplied with a 2400mAh lithium-ion cell. It will charge from any USB port, and can detect most types of dedicated chargers in order to use their max available current.

In sleep mode, the external 16MHz ceramic oscillator is powered off. USB communcation on the ATmega chip requires the external clock, which means if the device is plugged in in sleep mode, it will result in an "Unknown USB Device" error. The solution to this was to use the USB interrupt to wake up the system for USB communication. This means that the system is always on while it's plugged in, but once charging is done and the LED is turned off, total power consumption is very low.

####Charging

Using the MAX device to check what type of USB charger is connected was used to save program space, and to avoid writing the various algorithms needed to do that in the processor. When the IoDriver is plugged in, the TI charge controller starts up in standalone mode, with a charge current of 100mA, to prevent damaging an unknown charger. The TI device has an active-low power\_good output, which the main program checks at the start of each loop. When the power_good signal is detected, the IoDriver goes into charge mode. Within 100mS, the MAX device will have determined the charger type (how much current it can supply) and waits for the main processor to retrieve that information over the I2C bus. The processor waits 200mS then polls the MAX device, decides what the charge current should be, and sends that value to the TI device. The TI charge controller is internally limited to 2A, which allows a charge from completely dead in just over an hour.

While the IoDriver is in charge mode, it polls the charge controller for the charge status (either done, charging, or error), then sets the LED to a low-intensity color to reflect that. Amber for charging, green for done, and blue or putple for error. A request or incoming data from USB will interrupt this loop to send or receive profile information, or a color preview override.

Once charging is complete, the green LED will stay lit for 5 minutes, then turn off. The system is still powered on, so it will still immediately respond to any communication from the PC software. Additionally, since the internal voltage reference used to measure supply voltage is very innacurate (though very stable), the system will perform a calibration of the reference when charging is complete, and store this value in EEPROM. This gives a more accurate measurement of battery voltage, which could prevent it from shutting off prematurely.

Blue and purple are used for error indication to avoid confusion with the normal amber color. A purple error should only occur if the IoDriver is plugged in without a battery attached, and a blue error indicates any general charge error.

####Battery information

The ATmega device periodically checks the supply voltage, and when it falls below the BATTERY\_MINIMUM threshold (3.4V), it breaks to the blinkRed routine which, aptly, makes the LED blink red. When supply voltage falls below the BATTERY_CRITICAL threshold (3V), the processor disables the other two ICs and then goes into full shutdown mode, to save the battery from over-discharge.

In the event that the battery is deeply discharged, the TI charge controller will detect this, and go through an algorithm to recover the cell. In this state, the processor will disable the LED if the MAX device reports a charger with less than 1A supply current, in order to avoid overloading the charger, as it would be supplying all the current for the LEDs (over 500mA).

The processor does not check supply voltage while charging, so as to not bail into shutdown mode when it's plugged in.

Additionally, there is battery protection circuitry on a dedicated board attached to the cell which protects against over- and under-charge, as well as over-current situations.

###Other design notes

Because of a limitation or bug with the AVR libraries or the chip, the interrupt on PE6 (pin 1) doesn't wake the system from any sleep mode. To get around this, the button is actually connected to the SDA pin of the I2C bus. To prevent the button interfering with I2C communication, it is disconnected by a transtor when the system is plugged in. Since the I2C bus is only active when the system is plugged in, this seemed to be a reasonable workaround.

Unfortunately, the design got so complicated, and the board size reduced so much in the later revisions that a 4-layer board became necessary. This increases cost slightly, but results in a much cleaner layout.

The latest revisions of the board include footprints for both the TQFP and QFN packages, to accomidate whatever package happens to be in stock at the time.

####Expansion port

There is a port on the back of the IoDriver board that is intended for expansion modules. Currently planned are a bluetooth module (with accompanying iOS/Android apps), and an accelerometer module. Another possibility is a sound module, though there aren't currently any plans to make such a module unless there is significant demand for it. 

Essentially the expansion bus is just an SPI bus, with four chip select lines on the connector. Power to the expansion port is swiched with a MOSFET, to prevent the modules from consuming power when the system is supposed to be in sleep mode. Because of the body diode in the MOSFET, it's also possible to power the board from this connector for initial programming.

####Reset jumper

In some rare circumstances, corrupt or malformed information on the EEPROM can cause the IoDriver to crash when communicating with the PC software. Since this could possibly brick the device, there are two exposed pads on the board which will completely format all EEPROM and will recover the device. To format the device, you disconnect the battery, hold a bit of aluminum foil across the pads, then plug the battery back in. This formats __all__ EEPROM, including battery calibration data, so it's recommended to let the IoDriver recharge fully before using it again.
