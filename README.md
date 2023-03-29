
![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH

![Aufmacher](https://github.com/MakeMagazinDE/Fluoreszenzdisplays/blob/main/aufm_gh.JPG)

# Fluoreszenzdisplays

Retro oder Mehrwert? Vakuum-Fluoreszenzdisplays (VFDs) sind hell, blickwinkelunabhängig und lassen sich hervorragend ablesen – man findet sie deshalb oft in hochwertigen Messgeräten, Musikelektronik und HiFi-Equipment. Wir zeigen in **Make: 2/23**, wie man sie einsetzt und was bei der Ansteuerung zu beachten ist.

Unser Beispielprojekt ist eine Uhr mit einigen Sonderfunktionen und netter Animation der Ziffernanzeige, kompiliert für den ATmega328.

Die Firmware für die Versionen mit ATmega328 wurde mit dem (inzwischen kostenlosen) [AVRco Pascal](https://www.e-lab.de/downloads/AVRco/rev4/index.html) entwickelt, hierzu die Projektdatei *VFD_CLOCK.ppro* importieren. Kompilerschalter {$IFDEF} legen fest, ob ein MAX6921 oder 4094-Schieberegister verwendet werden sollen. Das Gehäuse für die Versionen mit Pollin-Display besteht aus lasergeschnittenen Plexiglasscheiben mit 3mm Stärke (siehe DXF- oder CorelDraw-Dateien).

Die Platinenlayouts wurden mit EasyPC Version 22 erstellt, die exportierten Gerber-Files zur Platinenfertigung sind in den ZIPs enthalten. Es stehen drei verschiedene Versionen zur Verfügung:

* **vfd_pcb_atmega_4094** ist die Grundversion wie im Heft beschrieben mit ATmega328 (im Schaltbild falsch als ATmega8 bezeichnet) für das Pollin-Display, diskret aufgebaut mit 4049-Schieberegistern, UDN2981-Treibern und (größtenteils) bedrahteten Bauteilen
* **vfd_pcb_atmega_max6921** mit ATmega328 für das Pollin-Display, jedoch mit MAX6921 als Treiberbaustein in SMD-Technik
* **vfd_pcb_pt6315_esp8266** ist eine neue Version in SMD-Technik mit ESP8266 und PT6315 mit separaten Adapterplatinen für die Röhren IV-11, IV-4/IV-17 und IV-22 in vier- und sechsstelligen Versionen. Die Firmware für Arduino-IDE finden Sie im Unterverzeichnis *src*.

Beachten Sie die Schalt- und Bestückungspläne in den jeweiligen Verzeichnissen (PDF).
