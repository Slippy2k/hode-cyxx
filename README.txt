
HODe README
Release version: 0.1.0
-------------------------------------------------------------------------------


About:
------

HODe is a reimplementation of the engine used by the game 'Heart of Darkness'
developed by Amazing Studios.


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


Status:
-------

The game is not completable. Levels load and Andy can be controlled and can move
through the level screens. Cinematics are also played.

.lvl file parsing
.sss partially (only with demo datafiles)
.paf

.mst
sound in .paf
menus

Andy cannon plasma
Andy special powers



Credits:
--------

All the team at Amazing Studio for possibly the best cinematic platformer ever developed.


URLs:
-----

[1] https://www.mobygames.com/game/heart-of-darkness


Notes:
------

The datafiles of Heart of Darkness are quite interesting. Each level is made of 3 different files, .lvl, .sss and .mst.
The files contains C structures, which includes pointers (eg. to data or to other structures). When loading these files,
the original engine needs to fixup the structure and the pointers.

As the game was written for 32 bits platform, the same method cannot be directly applied on more modern platforms.

Although focus has been on the PC version, PSX and Saturn use a similar structure.
