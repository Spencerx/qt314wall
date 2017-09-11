## Cutie-pie Wallpaper Changer

Slideshow generator for KDE.

This program fetches images via several means and creates optionally multiplied images at regular intervals, suitable for feeding into KDE or any other desktop environment that can show a folder as a slideshow.  It creates a folder containing scaled and/or cropped images that is generated at run-time and takes zero disk space by default due to clever use of /dev/shm.

The inspriation of this program was KDE3's feature of applying overlays to wallpapers before displaying them.  This can lazily be reproduced by feeding an image set to imagemagick and a copious amount of hard disk space, but we do it at runtime.

Sources include:

* an actual image

* text file containing file locations

* a folder containing files

* drag and dropped images on the window

* one of several imageboards

* a file as a commandline argument

## Usage

The program creates folders in /dev/shm/qt314-wallpaper, /tmp/qt314-wallpaper, or ~/.config/qt314wall.  If you're not running KDE or your DE doesn't understand `xsetbg`, you need to setup your desktop environment to look at one of these folders per your selection as a slideshow.  I suggest an interval of 2sec, or 1/5 of your duration in qt314wall.

Use [qfilelister] to easily create a usable file list.  The other widgets in the dialog have the usual meanings for wallpaper settings.

## Prequisities
You need the Qt5 sdk installed and an edition of imagemagick.  On ubuntu you can
install them with

>sudo apt-get install qtcreator imagemagick

## Compile

Designed with Qt5 in mind.  Compile with qmake or Qt Creator, and a C++14 compiler.

[qfilelister]:https://github.com/cmdrkotori/qfilelister
