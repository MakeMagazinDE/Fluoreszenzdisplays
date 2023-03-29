## VFD-Uhr/Display mit ESP8266 und PT6315

für russische Röhren mit separaten Adapterplatinen für die Röhren IV-11, IV-4/IV-17, IV-22 und das Pollin-Display
in vier- und sechsstelligen Versionen. Die Adapterplatinen besitzen beidseitige Steckleisten, so dass sie senkrecht oder parallel zur Hauptplatine 
montiert werden können.

* Schaltplan in *vfd_esp_main_06_schem.pdf*
* Bestückungspläne in *xxx_silk.pdf*
* Gerber-Files zur Platinenfertigung in ZIP-Files

Die Firmware für diese Version finden Sie im Unterverzeichnis *src*.

Bitte beachten: Die Treiber PT6311 bis PT6315 benötigen im Unterschied zum MAX6921 eine negative Spannung Vee, der Heizfaden muss auf Vee-Potential liegen. Wir haben hier zur Spannungsversorgung einen selbstgewickelten Übertrager auf EFD15-Kern ohne Luftspalt vorgesehen:

**Primär** 2x7 Windungen bifilar gewickelt, 0,4mm CuL

**Sekundär Anodenspannung** (über Spannungsverdoppler, siehe Schaltbild) 1x35 Windungen, 0,2mm CuL (25 Wdg. bei Pollin-Display)

**Sekundär Heizspannung** (FIL1/FIL2) 1x5 Windungen 0,4mm CuL

Der Übertrager benötigt den Transformator-Treiber SN6505A (SOT23-Gehäuse). 

Bei Beschaffungsproblemen können auch in Reihe geschaltete 1W-DC/DC-Wandler 5V/12V und 5V/15V verwendet werden. Die maximale Spannung Vee darf aber 40V nicht überschreiten. Für die Heizung der VFDs wird dann ein DC/DC-Wandler 5V/3,3V verwendet. Der Heizstromverbrauch der russischen Röhren ist relativ hoch, so dass dieser Wandler 2W leisten sollte. Für das Pollin-Display reicht eine 1W-Ausführung.
