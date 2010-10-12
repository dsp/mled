mled
====

mled is a program that translates stdin to morsecode and uses your CAPS
LOCK LED to display it.

    $ echo "sos" | sudo ./mled

At the moment it mled only supports ascii-characters A-Za-z0-9.

You can also abuse it as a binary clock with the -t switch.

Tested devices
--------------
 * ThinkPad X-Series
 * TypeMatrix 2030 (tested by maximeh)
 * hp nx7010 (winks)
