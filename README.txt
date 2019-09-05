
hode README
Release version: 0.1.9
-------------------------------------------------------------------------------


About:
------

hode is a reimplementation of the engine used by the game 'Heart of Darkness'
developed by Amazing Studio.


Datafiles:
----------

The original datafiles from the Windows releases (Demo or CD) are required.

- hod.paf (hod_demo.paf)
- setup.dat
- *_hod.lvl
- *_hod.sss
- *_hod.mst


Running:
--------

By default the engine will try to load the files from the current directory
and start the game from the first level.

These defaults can be changed using command line switches :

    Usage: hode [OPTIONS]...
    --datapath=PATH   Path to data files (default '.')
    --level=NUM       Start at level NUM
    --checkpoint=NUM  Start at checkpoint NUM

Display settings can be configured in the 'hode.ini' file.


Status:
-------

What is working :
* Cinematics (PAF)
* Andy
* Plasma Cannon
* Level screens
* Hint screens
* Monsters logic (MST)
* Special Powers
* Shooting collisions

What is partial :
* Sound playback (SSS)

What is missing :
* Menus


Credits:
--------

All the team at Amazing Studio for possibly the best cinematic platformer ever developed.


URLs:
-----

[1] https://www.mobygames.com/game/heart-of-darkness
[2] http://heartofdarkness.ca/
