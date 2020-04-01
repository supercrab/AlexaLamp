# AlexaLamp

This project was to create an Alexa voice controlled lamp with adjustable brightness. The lamp can also controlled using a rotary encoder with built in push switch.

![Lamp looking cool](images/lamp2.jpg)

## Components

Here is a list of the essential components you will need.  Shop around for the components as you can get some good deals if you buy from China/Banggood.

### Essential

* [Echo Dot](https://www.amazon.co.uk/Echo-Dot-3rd-Gen-Charcoal/dp/B07PJV3JPR) - for testing or final use but not mandatory as the lamp has a standalone mode
* [Wemos D1 Mini](https://wiki.wemos.cc/products:d1:d1_mini)
* [HLK-PM01 AC-DC 220V to 5v mini power supply module](http://www.hlktech.net/product_detail.php?ProId=54)
* [Robotdyn AC Light Dimmer Module](https://robotdyn.com/ac-light-dimmer-module-1-channel-3-3v-5v-logic-ac-50-60hz-220v-110v.html)
* [Rotary encoder with push switch](https://uk.pi-supply.com/products/rotary-encoder-push-switch) or [KY-040 rotary encoder module](https://uk.banggood.com/KY-040-Rotary-Decoder-Encoder-Module-AVR-PIC-p-914010.html)
* [Vintage E27 bulb holder ceramic](https://www.ebay.co.uk/itm/Vintage-E27-Bulb-Holder-Ceramic-Industrial-Lamp-Lighting-Antique-Retro-Edison/253903670767) (you could use a bayonet holder if you wanted)
* [MF-A03 control knob](https://www.google.com/search?q=MF-a03)
* [2 core twisted cable](https://www.ebay.co.uk/itm/2-Core-Twist-Braided-Fabric-Cable-Vintage-Electric-Wire-Flexible-Lighting-Cord/312959065511)
* 240v/mains plug 

### Optional

These depend on how you are going to mount your bulb and what finish you fancy for your enclosure.

* [M10 hollow threaded rod](https://lightingspares.co.uk/collections/threaded-rods)
* [M10 threaded nut](https://lightingspares.co.uk/products/oaks-oa16-metal-10mm-threaded-nut)
* [M10 penny washer](https://www.toolstation.com/penny-washer/p90716)
* [Liberon based wood dye dark oak](https://www.screwfix.com/p/liberon-ethanol-based-wood-dye-dark-oak-250ml/3102f)
* [Liberon superior Danish oil clear](https://www.screwfix.com/c/decorating/wood-oil/cat850452)
* [Felt pads](https://www.toolstation.com/felt-gard-felt-pad-set/p95753)

![Project components](images/components.jpg)

## Schematic

![Project schematic](images/schematic.png)

## Wooden Enclosure

* The enclosure is made up of 2 parts: a wooden block (roughly 4 inches by 3 inches) and a planed wood base that screws into the block and locks the electronics away
* The wooden block is hollowed out so that there is enough space for all the components
* The components are arrange inside the cavity **ensuring that wires carrying 240v were kept as short as possible and away from the Wemos and dimmer control board.  It was found that brightness control was affected if the high voltage wires got too close**
* A cloth was used to apply 3 coats of [Liberon based wood dye dark oak](https://www.screwfix.com/p/liberon-ethanol-based-wood-dye-dark-oak-250ml/3102f) to give some colour.  You can also use [Liberon based wood dye light oak](https://www.screwfix.com/p/liberon-ethanol-based-wood-dye-light-oak-250ml/8322F) or mix them both dyes to create a new shade
* A cloth was used to apply 3 coats of [Liberon superior Danish oil clear](https://www.screwfix.com/c/decorating/wood-oil/cat850452) which gave the wood a slight shine and made it slightly waterproof
* Once I certain everything worked, the wooden base was screwed into the block and [felt pads](https://www.toolstation.com/felt-gard-felt-pad-set/p95753) were used to hide the screws 

![Project enclosure](images/enclosure.png)

## Instructions

* Build the circuit on a breadboard **be careful with the 240v side - turn it off when not in use**
* Download and install the following Arduino libraries
	* [Fauxmoesp (Xose Pérez, Ben Hencke)](https://github.com/simap/fauxmoesp)
	* [Encoder (Paul Stoffregen)](https://github.com/PaulStoffregen/Encoder)
	* [WifiManager (tzapu)](https://github.com/tzapu/WiFiManager)
	* [JC_Button (Jack Christensen)](https://github.com/JChristensen/JC_Button)
	* [Dimmable-Light-Arduino (Fabiano Riccardi)](https://github.com/fabiuz7/Dimmable-Light-Arduino)
* Download the AlexaLamp code, compile and upload code to Wemos D1
* Test everything works as expected (see below for how to setup wifi and change modes)
* Make the circuit permenant by installing into your wooden block or whatever space you have.

## How to use

The lamp has 2 modes; Alexa (default) and standalone.  

To change the mode:

* Turn off the 240v supply to the lamp
* Hold the rotary encoder push switch down
* Turn on 240v supply
* Wait until the lamp starts to slowly flash
* Release the push switch
* The number of flashes seen will tell you which mode the lamp is in.  1 flash indicates standalone and 2 flashes for Alexa voice controlled mode.

### Alexa mode (default)

* When the lamp is turned on for the first time it will create an unsecured wifi hotspot called Lamp
* You can use your phone to connect to the hotspot and enter your wifi network credentials
* Once your credentials are verified, the hotspot will disappear and your wifi settings will be stored on the device
* The lamp will now connect to your network and you will be able to control the lamp using the rotary encoder
* Say "Alexa, discover devices" or go to the Alexa app and hit "Add Device", "Other", "Discover devices"
* Your lamp is now ready!

**If you connect the lamp to the mains and it cannot connect to a network using its stored settings, it will create a new hotspot and wait for new credentials.  While it is in this state the rotary encoder will not work until the new wifi credentials are entered.  You can still change the lamp's mode using the method described above.**

### Standalone mode

* This mode does not connect to any wifi networks or create any hotspots
* Plain old lamp
* Rotary encoder works straight away

### Alexa voice controls

Once setup, you can use different ways to invoke Alexa to modify the lamp's state:

* Alexa, turn lamp on
* Alexa, turn on lamp
* Alexa, set lamp to fifty (50 means 50% of brightness)
* Alexa, set lamp to one hundred (100% brightness)

![Lamp looking cool](images/lamp1.jpg)
![Lamp looking cool](images/lamp3.jpg)

### Admin web server

* There is a secret, backdoor web server that lists device information such as uptime, brightness, lamp state and such (handy for debugging)
* You can use a network discovery program to find the IP address and point your browser to it on port 81, e.g. `http:\\192.168.1.20:81` 

# Credits

Developed on a WeMos D1 mini clone, using Arduino IDE 1.6.5 with ESP8266 Core 2.3.0

This project would not have been possible without the kind development of others on GitHub!  Click on a name to see the authors' GitHub page

* [Fauxmoesp (Xose Pérez, Ben Hencke)](https://github.com/simap)
* [Encoder (Paul Stoffregen)](https://github.com/PaulStoffregen)
* [WifiManager (tzapu)](https://github.com/tzapu)
* [JC_Button (Jack Christensen)](https://github.com/JChristensen)
* [Dimmable-Light-Arduino (Fabiano Riccardi)](https://github.com/fabiuz7)

# Futre Improvements

* Web form to change config
* OTA updates
