# VFD-Uhr/Display mit ATmega328 und MAX6921

Version mit ATmega328 (im Schaltbild evt. falsch als ATmega8 bezeichnet) für das Pollin-Display, 
jedoch mit MAX6921 als Treiberbaustein in SMD-Technik

* Schaltplan in *vfd_avr_4094_2_schem.pdf*
* Bestückungsplan in *vfd_avr_4094_2_silk.pdf*
* Gerber-Files zur Platinenfertigung im ZIP-File

Bei der Kompilierung Compiler-Schalter {$DEFINE NEW_PCB} für MAX6921 setzen. Mit {$DEFINE SENSOR_BTN} können kapazitive Sensor-Tasten mit positivem Ausgangsimpuls angeschlossen werden, dazu sind auf der Platine 4 dreipolige Steckverbinder vorgesehen.
