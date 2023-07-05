# DarkAlliance
Tools to extract and convert games from Dark Alliance (2001) game.


I started down this trying to just make the game more fun and less a grind. Go figure it wouldn't be too easy.

But like any hacker, i just opened up files in a text editor and converted text to hex. Some things quickly became apparent with a little match and checking, and so now i've started a number of tools.

First with no documentation this is all experimental and guess work.

Second i'm not using other libraries, so included image support like bmp is bare minimum with expectations within reason. That means it only supports the 54 byte header with 256 color palettes, no compression.

Lastly, being a work in progress, if someone else figures something out i'll update accordingly.

----

At present, i can extract lmp files (basically a minimum archive), and convert (hopefully) tex files to bmp files. They look okay to me, but a few byte codes don't tell me what they mean.

Support to come, is to pack lmp files, along with converting bmp back to tex files. However you can use them as is for the moment if you want to reference or try your hand at re-texuring some images.

If anyone has code for the vif to nif or some other modeling i'll add it.


License GPL v2
