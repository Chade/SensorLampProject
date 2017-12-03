# README #

This is a Arduino-IDE conform repo for sensor testing.

### What do I need ###

* Arduino Mega 2560
* Datalogging-Shield with RTC DS1307 and SD-card (e.g. Adafruit Data Logging Shield 1141)

### Analog Sensors ###

* Resistive graphite fork sensor (TODO: List types and supplier)
* Resistive PCB fork sensor (TODO: List types and supplier)

### Digital Sensors ###

* Capacitive PCB sensor (TODO: List type and supplier)
* Air humidity sensor SHT10 (TODO: List supplier)
* Air humidity sensor DHT22 (TODO: List supplier)

### How do I get set up? ###

#### Arduino IDE ####

* Checkout Repository in Arduino-IDE workspace (e.g. ~/Arduino)
* Open .ino file in Arduino-IDE
* Configure for your board and hardware
* Compile and upload sketch

#### Eclipse ####

* Go to 'Help -> Eclipse Marketplace'
* Search for 'Arduino'
* Select 'Arduino Eclipse plugin named Sloeber V4' to install
* Follow instructions and restart Eclipse
* Got to 'File -> Open Projects from File System...' and select the sketch you want to open.
* Configure for your board and hardware
* Compile and upload sketch

### Contribution guidelines ###

* Please adhere to Arduino-IDE coding rules
* Use Arduino .ino Files for new sketches
* Assemble all neccessary libraries in 'libraries' folder.
