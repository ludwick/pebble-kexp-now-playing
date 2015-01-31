# KEXP Now Playing Pebble Watch App

This repository has a simple Pebble watch app to show current KEXP radio song.

Currently it is built using [CloudPebble](https://cloudpebble.net). 

## Usage

Run the app. It should load the current song. Press the center button and it will refresh (if the song has changed).

## Limitations

This version uses a hidden HTTP request the main KEXP player uses to update its playlist and extracts the Artist, Track and Album information from that. 

It does not handle:

* Very many HTML entities.
* Unicode support is pretty busted.
* No scrolling.

