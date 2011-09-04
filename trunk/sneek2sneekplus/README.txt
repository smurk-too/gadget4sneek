
This is a simple application for switching between SNEEK with and without DI.

This is a beta release so there are RISKS you could make your Wii
unoperable (i.e. bricked). Do not use this application if you 
cannot take such a risk.

Installing wad file into real NAND could corrupt the Wii filesystem, you 
should understand the risks prior to doing that and take precautionary
measure.

Requirements:
1. SD card root directory or folder has the following files inside:
	a. boot2sneekplus.bin taken from compiling SNEEK+DI (i.e. boot2_di.bin)
      b. disneekplus.bin taken from compiling SNEEK+DI (i.e. di.bin)
	c. boot2.bin taken from compiling SNEEK (i.e. boot2_sd.bin)
      d. switchtosneekplus
2. SD card has apps directory or folder, and inside that folder:
	a. sneek2sneekplus folder with the following files:
		i. boot.dol
		ii. icon.png
		iii. meta.xml
3. Alternative to point 2 above is that you could install the wad file
   sneek2sneekplus-S2SP.wad into your SD NAND (e.g. using ShowMiiWads app) and
   it will show up as a channel

Instructions:

You could copy switchtosneekplus file in folder To_SD_root into your SD card 
root directory/folder.

Then you could copy folder sneek2sneekplus into the apps folder in your SD card
or just install the sneek2sneekplus channel (by installing the included wad file).

Insert back SD card to your Wii, and turn on Wii.

Before you run this application, you should already be running SNEEK (without DI).

Once you are running SNEEK (without DI).
Run homebrew channel, and choose sneek2sneekplus wiibrew application, or
run the installed sneek2sneekplus channel

Push button [A] to switch to SNEEK+DI, and you will see the next menu where
you could select the games in your USB drive (follow the SNEEK guidelines
on how to put the games in your USB drive). Push button [+] to move forward
in the games list, and push button [-] to move backward. Once you see the
games that you want to play, push button [A] to confirm.

Wii will reset and after that you will be then using SNEEK+DI, and the
chosen games will be ready on the disc channel.

Once you are done playing with SNEEK+DI and want to switch back to
SNEEK (without DI), either go to homebrew channel, and choose sneek2sneekplus 
wiibrew application again or run the sneek2sneekplus installed channel.

Push button [B] to switch back to SNEEK (without DI).

Wii will reset and after that you will be then using SNEEK (without DI)


Important Notes:

1. Be careful when running this application. Use this application only when you are 
   running SD NAND (through SNEEK+/-DI). You may corrupt your real NAND if you try to 
   push button A or B in sneek2sneekplus when running real NAND.






