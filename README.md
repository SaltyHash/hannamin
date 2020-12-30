# hannamin
A digital [theremin](https://en.wikipedia.org/wiki/Theremin) for my wife, Hannah.

## Hardware
- [PJRC Teensy 4.0 microcontroller](https://www.pjrc.com/store/teensy40.html) - This runs the show.
- [2x VL53L0X time-of-flight laser ranging sensor breakout](https://www.amazon.com/gp/product/B07Q5Y3G4C) - Senses the distance to the hands. One controls pitch, and the other controls volume.
- [Adafruit I<sup>2</sup>S Stereo Decoder - UDA1334A Breakout](https://www.adafruit.com/product/3678) - Converts the I<sup>2</sup>S digital audio data from the Teensy into an analog audio signal.
- [5V 3W stereo audio amplifier w/ volume control](https://www.amazon.com/gp/product/B07QTY7HXM) - Drives the speaker with the signal from the I<sup>2</sup>S decoder.
- [3W 4Ω speaker](https://www.amazon.com/gp/product/B01CHYIU26) - Jiggles the air molecules.
- [5V 3A voltage regulator](https://www.amazon.com/gp/product/B0758ZTS61) - Powers all this stuff, and allows the Hannamin to be powered by a standard 9V guitar pedal power supply.
