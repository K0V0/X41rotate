X41Rotate
=========

X41Rotate is another (and in 2014 due to changes in linux distros probably only one working) screen [auto]rotation daemon/script for old IBM Thinkpad tablets/convertible laptops. 
This one is specially designed for X41 tablet series.

Features/functions
------------------

- automatic rotation using built-in accelerometer designed for Harddisk active protection system (HDAPS).
- manual rotation - clockwise or counter-clockwise
- automatic remaping of lid up and down buttons according to orientation
- notifications 

Installation/requirements
-------------------------

Program requires that you have XrandR installed on your system. On debian squeeze and derivates it should be a part of x11-xserver-utils.

```apt-get install x11-xserver-utils```

Repository contains precompiled binary for 32 bit systems, but should also work on 64 bits. If you are still interested in compilation, nothing special happens for first-lessons-homework type program like this. 

```gcc -o thinkrotate thinkrotate.c```

Make it executable (if not)

```chmod +x thinkrotate```

To make it automatically start with system :

A.) 
1. Go to system > preferences > startup applications
2. Simply add path to binary into command field
3. Name it whatever you want

B.)
Starting version 0.1.2 There is added functionality for mapping lid up and down key to up and down arrow depending on screen orientation, however to use this functionality program needs sudo privilegies at some part, which could be potential safety risk. If you want this functionality and know the risk you should add this line into /etc/sudoers file:

```gksu gedit /etc/sudoers``` 

add:

```yourusername ALL=(ALL:ALL) NOPASSWD:/usr/bin/setkeycodes```

It is good thing to use one of the X41's hardware buttons located on lid to take action, ideal first one from the left after power and lock button - one with rotate icon. Standartly these buttons are not recognized in most distros, so before making "keyboard shortcut" you must map them. We are talking here about X41, but process is similar for every model.

1. Add keycodes to /etc/rc.local

```gksu gedit /etc/rc.local``` 

add this (or simillar depends on model) BEFORE exit 0;

```setkeycodes 6c 130```

2. Now you sholud be able to detect this key and add command for it in system > preferences > keyboard shortcuts > add.
In command window add /path/to/binary/thinkrotate (r or l, see usage).  

Usage
-----

Currently are supported only these options:

to rotate counter-clockwise:

```/path/to/binary/thinkrotate r```

to rotate clockwise:

```/path/to/binary/thinkrotate l```

to kill daemon

```/path/to/binary/thinkrotate k```







