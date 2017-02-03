# AMOS v14 November 2004
## By Lewis Sellers (aka Min)
## Intrafoundation Software
### http://www.intrafoundation.com/product/AMOS

A-MOS was a simple experiment over a weekend in May 2003 on Artificial Life/World algorithms (A-Life and A-World).

It is a small C++ application that runs under Windows NT/98/2000/XP.
It creates a small 170x170 map with natural resources and food then introduces 1 to 6 tribes and then proceeds to watch them as they give birth, go to war, steal and build cities. This all happens in a very abstracted fashion mind you. Never the less, the basic concept could be expanded out into an actual A-Life game if you so desire.

![AMOS](https://github.com/lasellers/AMOS/blob/master/screenshot1.png)

# A-LIFE/GAME FRAMEWORK
If you're interested in such things but not a very good programmer, then you should know this can serve as the frame work to build other A-Life or Gaming experiments. The complete C++ source code is included. Among other features, it uses a simple but fast flicker-free double-buffered DIB to render all the graphics.
Additionally, it plays a few sounds for announcement purposes (such as entire tribes dying off).

It has little in the way of internal documentation, but I do tend to write relatively clean, easy to understand code.

Thanks,
--min

# Version History

v15. Pulled from old Intrafoundation.com archive and repackaged for Visual Studio 2015 and Github hosting.
v14. November 2004. Recompile and new installation software.
v13. August 2004
v11-v12.
v6-v10. May 2003
v5.
Fixed the migration bug. Now the patterns are very different. in default mode,
It seems that tribes naturally start to pick up a swirling pattern of birth/migration.
This causes wave after wave of expansion to be thrown out.
v4. Sunday. Fixed several bugs and added a few more menu options. I really should change the herd-migration routines to use a bresham line alg to get rid of the artifacting, but I really just don’t have any more time to fool with it now. Maybe later.
v3. Saturday. March 2003. First public release.
The code is very... NEW is a good way to describe it.
It's literally only something fun to do Friday night/Saturday after all.
v2. Friday. I miss version 2 (Friday night's) because a serious bug in it caused a directional bias producing some very lovely "cloud formations" to be generated.
