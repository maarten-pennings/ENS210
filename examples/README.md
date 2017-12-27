# ENS210 examples
There are several examples of increasing complexity.


## ens210simple
A simple demo sketch that prints measurement data from them ENS210,
relative humidity and temperature.

This is a bare minimum example.
It tries to not use any other libraries (it only uses `Serial` to print results, and `Wire` for I2C).
The ENS210 library is used without error error handling.


## ens210full
A sketch, that is simple like ens210simple in that it uses no other libraries.
But it does employ full error handling and it demos all conversions (Kelvin, Celsius, Fahrenheit).


## ens210thingspeak
A real application, not just a simple demo.
Reads temperature and humidity from the ENS210, and uploads that to a ThingSpeak channel.

This application uses several (standard) libraries, like WiFi and ThingSpeak.
It also requires you to create a ThingSpeak account and create an appropriate channel to receive the data.

This has only been tested on ESP8266.


## ens210websockets
An application that demos how to "push" measurement data (from ENS210) to web clients (using websockets).
The data appears in a simple list in the browser.
Uses several libraries: web server, websocket server, dns server, flash file system.

This has only been tested on ESP8266.


## ens210wsgraph
This application is an extension of the previous.
Data no longer appears in a list, but is shown in a graph, using the 
excellent [smoothie](http://smoothiecharts.org/) library.

This has only been tested on ESP8266.

(end of doc)
