## Cutie-pie Wallpaper Changer aka Waifu Slideshow

Slideshow generator for KDE.

This program takes a text file containing file locations of you're waifu and creates optionally multiplied images at regular intervals, suitable for feeding into GNOME, KDE or any other desktop environment that can show a folder as a slideshow.  Think of a folder containing scaled and cropped images that is generated at run-time yet takes zero disk space by default. (due to clever use of /dev/shm.)

The inspriation of this program was KDE3's feature of applying overlays to wallpapers before display.  This can lazily be reproduced by feeding an image set to imagemagick and a copious amount of hard disk space.

## Usage

The program creates folders in /dev/shm/qt314-wallpaper and /tmp/qt314-wallpaper and ~/.config/qt314wall, where two identical images are stored.  You need to setup your desktop environment to look at one of these folders per your selection as a slideshow.  I suggest an interval of 2sec, or 1/5 of your duration in qt314wall.

Use [qfilelister] to easily create a usable file list.  The other widgets in the dialog have the usual meanings for wallpaper settings.

## Prequisities
You need the Qt5 sdk installed and an edition of imagemagick.  On ubuntu you can
install them with

>sudo apt-get install qtcreator imagemagick

## Compile

Designed with Qt5 in mind.  Compile with qmake or Qt Creator, and a C++11 compiler.

[qfilelister]:https://github.com/cmdrkotori/qfilelister
