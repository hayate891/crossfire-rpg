Crossfire Client User Guide
===========================

Crossfire is a free, open-source, cooperative multi-player RPG and adventure
game. Since its initial release, Crossfire has grown to encompass over 150
monsters, 3000 areas to explore, an elaborate magic system, 13 races, 15
character classes, and many powerful artifacts scattered far and wide. Set
in a fantastical medieval world, it blends the style of Gauntlet, NetHack,
Moria, and Angband.

For more information about Crossfire or using the client, see the `Crossfire Wiki <http://wiki.cross-fire.org/>`_.


Installation
------------
Briefly, change to the source directory and execute the following commands::

    $ mkdir build; cd build
    $ cmake ..
    $ make
    $ sudo make install

To change the compile-time default options, run **ccmake** instead of
**cmake**.

To compile with debugging symbols, set *CMAKE_BUILD_TYPE* to ``Debug``.

For more details, see `Compiling the Crossfire Client <http://wiki.cross-fire.org/dokuwiki/doku.php/client:client_compiling>`_ on the Crossfire Wiki.


Sound Support
-------------
.. note:: This legacy documentation does not necessarily reflect the behavior of the current version of the client.

If you want sound support (only useful if you have a system that has good
sound hardware), you will also need to get some sounds for the program.
You will need the raw sound files - the .au files no longer work with the
client.

The sounds file determines what file is played for what sound.  The default
location for the sound files is '<prefix>/share/cfclient/sounds'. <prefix>
is the -prefix= option given when running configure, default of '/usr/local'.

You can set this to another directory at the configure stage by using
--with-sound-dir=/some/directory.  This file is only used for compiled in
defaults.  If you have a '~/.crossfire/sounds' file (some format), that file
will be used instead of the built in defaults.

Sound are now played by separate process invoked from crossfire client.
The process - called sound server - adjusts sounds' volume according to
relative position of player and sound source on map and mixes the sounds
together, so many sounds can be played simultaneously.

Configuration
~~~~~~~~~~~~~
The config file '~/.crossfire/sndconfig' contains some configurable settings. If
there is no such file, one with default settings will be created.

The file contains following fields:

stereo
    1 means stereo sound, 0 - mono

bits
    bitrate of generated sound - 8 or 16 16 means better quality, especially
    when more sounds are to be played simultaneously, but takes more memory

signed
    if we should sent signed data to the sound card. 1 means yes.

frequency
    speed of playing data. This should be 11025, or sound pitch will change

buffers
    how many buffers to allocate

buflen
    how big the buffers should be. buffers*buflen shouldn't be smaller than
    the longest sound to be played.

simultaneously
    The number of sounds that can be played at the same time. When this
    setting is larger, each sound volume will decrease.

How Does it Work?
~~~~~~~~~~~~~~~~~
The sound server gets information about sounds to be played on standard input.
The information is a line:
<sound number> <sound type> <relative x> <relative y>
All those numbers are hex.
The file ~/.crossfire/sounds contains description of sound numbers and types.
For example:
3 0 5 0
Means that normal sound SOUND_FUMBLE spell should be played as it's source was
5 units to the right of player.

Sounds are mixed in special buffers, which are in fact one buffer, which should
be big enough for the biggest sound to be played.

The buffers, if contain anything, are sent one by one to the sound device.
Each buffer is cleaned after playing.

Sounds data is multiplied by some ratio (<1) evaluated from it's position and
volume and added to the buffers, starting from the next after the one being
played.

So bigger buffer means bigger delay, before the sound is actually played, but
the smaller buffer is, the bigger is possibility, we won't succeed filling the
next buffer, before last is played.

Preloaded Bitmaps
-----------------
.. note:: This legacy documentation does not necessarily reflect the behavior of the current version of the client.

The client will get any images that it is missing from the server.  This
can include the entire image set.

To decrease bandwidth used when actually playing the sounds, it is suggested
you download the image archive and install it.  The default location
for the archive is *<prefix>/share/cfclient/*, where *<prefix>* is determined
by the -prefix= option given when running configure.

The mechanism the client uses to find a matching image file when the
server tells it an image name is thus:

- Look in *~/.crossfire/gfx*.  If an image is found here, irrespective of
  the set and checksum of the image on the server, it will be used.  The only
  way images are placed into the gfx directory is by the user actually
  copying them to that directory.  The gfx directory allows a user to
  override images with versions he prefers.
- Look in *~/.crossfire/image-cache* then
  '<prefix>/share/cfclient/crossfire-images'. If the checksum matches the
  image from the respective directory is used. Note that if the checksums
  match, it really doesn't matter what image location we use, as it is the
  same image.
- Get the image from the server. If -cache is set, a copy of it is put into
  *~/.crossfire/image-cache*.


Credits
-------
.. include:: AUTHORS
