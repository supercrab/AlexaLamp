# AlexaLamp

This project was to create an Alexa voice controlled lamp with adjustable brightness.  The lamp can also be illuminated and brightness adjusted using a rotary encoder with push switch.

![Lamp looking cool](images/lamp2.jpg)

## Components

* Wemos D1 Mini
* HLK-PM01 AC-DC 220V to 5v mini power supply module
* Robotdyn AC Light Dimmer Module (https://robotdyn.com/ac-light-dimmer-module-1-channel-3-3v-5v-logic-ac-50-60hz-220v-110v.html)
* KY-040 rotary encoder with push switch
* Bulb holder and fittings
* MF-A03 control knob
* Power cord and plug 

![Components](images/components.jpg)

## Electronic Schematic

![Schematic](images/schematic.png)

## Instructions

* Build the circuit on a breadboard 
* Download the code, compile and upload code to Wemos D1
* Test everything works as expected
* Take a wooden block and hollow insides to fit components
* Arrange all the components into the cavity but **ensure that wires carrying 240v are kept as short as possible and away from the Wemos and dimmer control board as brightness control can be affected**

## How to use

The lamp has 2 modes: Alexa (default) and standalone.  To change the mode, power cycle the 240v supply whilst holding the rotary encoder push switch down until the lamp slowly flashes.  The number of flashes seen will tell you which mode the lamp is in.  1 flash indicates standalone and 2 flashes for Alexa voice controlled mode.

### Alexa mode (default)

* When the lamp is turned on for the first time it will create an unsecured wifi hotspot called Lamp
* You can use your phone to connect to the hotspot and enter your wifi network credentials
* Once your credentials are verified, the hotspot will disappear and your settings will be stored
* The lamp will now connect to your network and you will be able to control the lamp using the rotary encoder
* Say "Alexa, discover devices" or go to the Alexa app, hit "Add Device", "Other", "Discover devices"
* If the lamp cannot connect to a network using its stored settings, it will create a new hotspot and wait for the new credentials (when not connected to wifi the rotary encoder will not work)

### Standalone mode

* This mode does not connect to any wifi networks or create any hotspots
* Plain old lamp
* Rotary encoder works straight away

## Alexa voice controls

Once setup, you can use different ways to invoke Alexa to modify the lamp's state:

* Alexa, turn lamp on
* Alexa, turn on lamp
* Alexa, set lamp to fifty (50 means 50% of brightness)
* Alexa, set lamp to one hundred (100% brightness)

![Lamp looking cool](images/lamp1.jpg)
![Lamp looking cool](images/lamp3.jpg)
