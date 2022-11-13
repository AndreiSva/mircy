# mircy

A very minimalistic (and only sort of functional) curses-based IRC client.

## Building & Running

To build mircy you'll need a working installation of meson and ninja. You can generate the build files by running `meson build --release` and then `ninja -C build'. This will build mircy inside the `./build` directory.

## Caveats

The client has only been tested on OpenBSD and FreeBSD and there's probably (definitely) a bunch of bugs. There's also not a lot of functionality and only the bare minimum of the irc protocol has been implemented.
