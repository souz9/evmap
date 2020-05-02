# evmap
Remap keys for X/Wayland (over evdev)

# My personal case

```
Win+Left  -> Home
Win+Right -> End
Win+Up    -> PgUp
Win+Down  -> PgDw
```

If you need some other mapping, please edit the mapping table in [main.c](main.c).

# Dependencies

- libevdev

# How to Build / Install

You need [Meson](https://mesonbuild.com/) for building from the source code.

```bash
$ meson --buildtype=release build
$ cd build
$ ninja
$ ninja install
```

# How to Run

Run `evmap` with an input device specified. Look for input devices in `/dev/input/`. Use the input device that references your keyboard.

*Example:*

```bash
$ evmap /dev/input/by-id/usb-Logitech_USB_Receiver-if02-event-kbd
```

`root` access is required for grabbing an input device!

Or, you can set the proper access rights to your input device (grabbed device) and to the input device with remapped keys (`/dev/uinput`).
