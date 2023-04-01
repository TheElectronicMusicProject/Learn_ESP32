# _ATTENTION!_

I used a JDY-23 instead a JDY-31.
My PC and smartphone doesn't support this device (connection to the bluetooth device fails every time I try).
Download and install "Serial Bluetooth Terminal" from the Play Store and connect using it.

## CONNECTIONS

    * VCC to 5V
    * GND to ground
    * TXD to GPIO21
    * RXD to GPIO19


## UPDATE
With JDY-31 you must use the `1234` password to connect to the module, then see bluetooth properties on your PC.
Find the *outcoming* COM port and select this COM from the ESP-IDF terminal.
Then you'll see the output log inside your terminal.

    * VCC to 3.3V
    * GND to ground
    * TXD to GPIO19
    * RXD to GPIO21