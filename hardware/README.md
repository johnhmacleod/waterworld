This folder contains details of the hardware we used

The Flow meter is a simple hall effect sensor with 3 connections

+V <br>
-V <br>
Sense

You can usually run these from 3.3V

![Flow Sensor](flowsensor.jpg)
![Flow](flow.jpg)



The pH meter recommended needs 5V to operate.  Using the NodeMCU board recommended allows you to provide power from the VU pin on the board which provides 5V from the USB connection. Note that not all NodeMCU boards have this available. Pin 3 is usually NC.

The resistor divider network is to step down the 5V coming back from the board to 3.3V suitable for the ESP8266. Each resistor is 1K (3.3V is 2/3 of 5V, conveniently!)

![pH](pH.jpg)
![pH Sensor](phsensor.jpg)
