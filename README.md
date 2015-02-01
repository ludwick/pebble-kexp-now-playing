# KEXP Now Playing Pebble Watch App

This repository has a simple Pebble watch app to show current KEXP radio song.

Currently it is built using [CloudPebble](https://cloudpebble.net).

## Usage

Run the app. It should load the current song. Press the center button and it will refresh (if the song has changed).

## Limitations

This version uses a hidden HTTP request the main KEXP player uses to update its playlist and extracts the Artist, Track and Album information from that. This may break at any time and is an egregious hack.

Stuff that is busted:

* Most HTML entities don't work.
* Unicode support is pretty busted.
* No scrolling of long strings.
* KEXP feed data sometimes has malformed UTF-8 in the encoded JSON. So it looks ugly on the watch too.
* Air breaks are not handled very well, depending on when the app was started and when you try to refresh.

