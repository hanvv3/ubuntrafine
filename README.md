# ubuntrafine
Ubuntu Gnome extension for MacOS-like LG Ultrafine 5k Display brightness control (for PERSONAL USE ONLY)

![Screenshot from 2024-04-03 21-19-48](https://github.com/hanvv3/ubuntrafine/assets/76027314/c76ec672-de76-4127-a6fe-871e5664460c)

* simple GUI
* Works with one single command `ufctl`
* `ufctl` is command line interface( cli ) interactive
* Can seamlessly work with Apple Magic Keyboard

-----------
## Before Use
`ufctl` requires `sudo` in order to operate "libusb". You can edit `/etc/sudoers` or add additional lines to `/etc/sudoers.d/local` to execute this command without `sudo`

## `ufctl` Usage

- `ufctl` enters cli mode
- `ufctl +` brightens by a step
- `ufctl -` dims the brightness by a step
- `ufctl get` gets the first Ultrafine display's brightness in percentage. ( this means you can skip the index number of your Ultrafine device if you have only one, not tested where you have more than one device )
- `ufctl set 0 {brightness}` sets the "first" (zero index) Ultrafine display's brightness to {brightness} in percentage.

## Support List:
* 24MD4KL
* 27MD5KL
* 27MD5KA

## Build
* cd .../ufctl
* mkdir build
* cd build
* cmake ..
* make







inspired by https://github.com/ycsos/LG-ultrafine-brightness
