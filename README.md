# KEXP Now Playing Pebble Watch App

This repository has a simple Pebble watch app to show current KEXP radio song.

Currently it is built using [CloudPebble](https://cloudpebble.net).

You can see a picture of it in action!

![KEXP Now Playing App in action](https://farm8.staticflickr.com/7325/16227719299_bd9bc50895_n.jpg).

## Usage

Run the app. It should load the current song. Press the center button and it will refresh (if the song has changed).

## Limitations

This version uses the [official API](http://cache.kexp.org/cache/docs) however
it is very basic in that it uses only the "latest" play request and just shows
dashes if some field is missing. The egregious hack has been removed. It now
also puts the text in a scroll layer nad in theory will allow seeing all text.

Stuff that is busted:

* Most HTML entities probably don't work.
* Unicode support is pretty busted (unless magically the JSON gotten via the API
  copied into char[] and displayed "just works" which I highly doubt).
* Air breaks are not handled very well, depending on when the app was started and when you try to refresh.

