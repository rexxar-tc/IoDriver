##Overview

At the base level, the IoDriver is nothing more than a PWM LED driver with some fancy software. The hardware is simple and very straightforward. There are three ICs on the board; the main ATmega device, a MAX14578 USB charge detector, and a TI bq24257 lithium-ion battery charger. The relatively high current to the LEDs is switched with a FDN327N N-channel MOSFET.

##General operation description

The LEDs are pulse width modulated to change their intensities and create different colors. The profiles store RGBW values, and we use a custom algorithm to generate the gradient sequence between each RGBW color (See color.cpp and component.cpp). The button is tied to an interrupt that increments the profile, as well as shut down and wake up the processor.

####Charging

Using the MAX device to check what type of USB charger is connected was used to save program space, and to avoid writing the various algorithms needed to do that in the processor. When the IoDriver is plugged in, the TI charge controller starts up in standalone mode, with a charge current of 2A ((FIX THIS)). The TI device has an active-low power_good output, which the main program checks at the start of each loop. When the power_good signal is detected, the IoDriver goes into charge mode. Within 100mS, the MAX device will have determined the charger type (how much current it can supply) and waits for the main processor to retreive that information over the I2C bus. The processor waits 200mS then polls the MAX device, decides what the charge current should be, and sends that value to the TI device. The TI charge controller is internally limited to 2A, which is just under 1C charge current, which allows a charge from completely dead in just over an hour.

While the IoDriver is in charge mode, it polls the charge controller for the charge status (either done, charging, or error), then sets the LED to a low-intensity color to reflect that. Amber for charging, green for done, and red for error. A request or incoming data from USB will interrupt this loop to send or recieve profile information, or a color preview override.

####Battery information

The ATmega device periodically checks the supply voltage, and when it falls below the BATTERY_MINIMUM threshold (3.4V), it breaks to the blinkRed routine which, aptly, makes the LED blink red. When supply voltage falls below the BATTERY_CRITICAL threshold (3V), the processor disables the other two ICs and then goes into full shutdown mode, to save the battery from over-discharge.

In the event that the battery is deeply discharged, the TI charge controller will detect this, and go through an algorithm to recover the cell. In this state, the processor will disable the LED if the MAX device reports a charger with less than 1A supply current, in order to avoid overloading the charger, as it would be supplying all the current for the LEDs (over 500mA).

The processor does not check supply voltage while charging, so as to not bail into shutdown mode when it's plugged in.
