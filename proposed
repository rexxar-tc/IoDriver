IoDriver
========
##About the IoDriver

The IoDriver is a custom, open-source RGBW LED driver for model lightsabers. Here you'll find source code, schematics, PCB files, firmware, binaries, documentation, and various other text files. The IoDriver is also available, assembled at [our Tindie store](https://www.tindie.com/products/rexxar/iodriver/). 

###Full Feature List

If you've come here from Tindie, you've seen a short list of what we thought were the most important features. Here's everything:


- Cross-platform software
- Up to 5 completely customizable profiles
- Up to 4 hours of runtime
- Full-spectrum RGBW color selection
- Integrated USB charging
- Expansion modules including bluetooth, sound, and accelerometer
- Choice of red, black, or silver buttons
- Easy installation-- no soldering necessary
- All profile data is stored on the driver

###Installation

It's fairly easy to install the IoDriver. First, you need to pull out any existing electronics in your saber. The easiest way I've found is to cut off the battery holder, then pry out the button and cut that out as well. The LED housing should just unscrew, allowing you to remove the existing LED.

Everything on the IoDriver has a connector, so you just need to fit the LED then the button, and plug it into the board. Easy. You may have to get a bit creative to fit all the wires into the LED housing, but it should work. It will take some force to put the new switch in, but with some gentle persuasion, it should go smoothly.

###What's All This Profile Jazz?

You get to define up to five profiles, with 20 color steps, and settable transition times between each. You can chose transistion effects as well. Currently there is the default gradient, solid colors, and a strobe effect. You can name each profile; you get 31 characters. All profile data, including names, are stored on board, so they will appear exactly as you set them no matter where you load up the software. You can also store as many profiles as you like locally on your computer.

When your profile is selected on your saber, it starts with the first color, then runs through a gradient to the next color. How fast this happens is based on the delay time you set. When it reaches the last color, you can choose to have it either cycle backwards through the profile, or just start over at the beginning.

###Why RGB_W_ LEDs?

RGB covers most of the color spectrum, but it's hard to get very light colors, and it's impossible to get true white. The separate red green and blue LEDs don't blend that well, so 'white' comes out as three colored spots or streaks. Having a white LED lets you get a true white, or pastel colors, if that suits you.

###Buttons

The switches specified in the BOM (and that are provided on Tindie) are meant to be pressed into sabers with a 7/16” opening. They're a shielded switch with red, black, or silver buttons. 7/16” seems to be a common size, so that's what I've selected these switches for. If your saber has a different size opening, you'll need to find your own switch. You need a momentary, normally-open (or normally-off) type.

###Software

The PC-side software is written in Perl, which works naively on Linux, and ports easily to Windows. It *should* be Mac compatible, though that's not been tested yet.

The software is fairly simple: You pick a profile number, 1-5, name it, pick your first color, transistion time and effect, then you pick a second color, and a third, etc. There is a preview button so you can see what your selected color looks like on your saber. You can also re-number your profiles, which changes the order in which they cycle through.

###Expansion Modules

Currently planned are a bluetooth module, accelerometer, and due to large demand, a sound expansion. None are available right now, but will be released in that order. Modules plug into the port on the back of the board, then stack on top of each other. The modules are screwed into mounting posts to keep them from coming loose.

Other modules are possible; feature requests can be posted on our GitHub [issue page] (https://github.com/rexxar-tc/IoDriver/issues).

###Other Details

The IoDriver comes with a 2400mAh lithium ion cell. There's circuitry on the board that will detect most chargers, and will charge at up to 2A, given a powerful enough charger. Most smartphone chargers are capable of supplying 1A or more. At a charge rate of 2A, the battery will charge from completely dead in just over an hour, and you get over 4 hours of use.

##That's It

That's about all you need to know about the IoDriver. If you're interested in building your own, or just want the nitty-gritty details, there's more documentation in the DOC folder. There's also schematics and Gerber files for getting your own PCBs made up.

You can contact us at [IoDriver@onionstorm.net](mailto:IoDriver@onionstorm.net)
