# Stahl Secure Storage Device

The Stahl USB dongle provides secure long-term storage for about 2000 characters of text. Simply plug the device into your computer and open any serial communications program that supports VT100 emulation (PuTTY, Tera Term, Minicom, screen, etc.) in order to access the built-in text editor.

![Stahl](https://github.com/machdyne/stahl/blob/70c873443f540fe50ba8bddf3de0b9b58c02abc7/stahl.png)

This repo contains schematics, firmware and a 3D-printable case.

Find usage instructions and more information on the [Stahl product page](https://machdyne.com/product/stahl-secure-storage-device/).

## Disclaimer

We consider the device "secure" in that, in theory the data should be preserved longer than with some other mediums. Please do not rely solely on the included password-based encryption for sensitive data. Please keep multiple copies of your important data.

## Firmware

The firmware can be updated over DFU by pressing CTRL-Y in the text editor to enter DFU mode. You can also enter DFU mode by shorting B1 when power is applied.

You can then use the following scripts to program the latest firmware:

```
$ cd firmware/images
$ sudo bash ../scripts/DFU-PROG.sh
$ sudo bash ../scripts/DFU-RESET.sh
```

You can build the firmware from source, if you have the avr-gcc toolchain installed:

```
$ cd firmware/stahl
$ make
```

## ISP Header

Stahl has an AVR ISP header that can be used to restore the DFU bootloader. Use this interface with caution as it is possible to "brick" the MCU if the fuses are set incorrectly.

| Pin | Signal |
| --- | ------ |
| 1 | MOSI |
| 2 | MISO |
| 3 | SCK |
| 4 | RST |
| 5 | GND |

If using a USBASP programmer, you can restore the bootloader with:

```
$ cd firmware/images
$ ../scripts/PROG.sh
```

This header also provides SPI access to the FRAM if you short B2, allowing the FRAM contents to be recovered, if for example the MCU were to become inoperable.

## License

This project, and the LUFA USB library that it uses, are released under the MIT license.
