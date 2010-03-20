
This is a simple application for switching between real NAND
and SD NAND (through SNEEK).

This is a beta release so there are RISKS you could make your Wii
unoperable (i.e. bricked). Do not use this application if you 
cannot take such a risk.

Since I have bootmii as boot2, this application is only tested
and used by me with that condition. If you have bootmii not as boot2
I dont know whether this application will work for you or not.

Requirements:
1. Bootmii installed as boot2
2. SD card has bootmii directory or folder with the following files inside:
	a. armboot.bin from bootmii installation
	b. armbootsneek.bin from SNEEK v2 compilation
	c. ppcboot.elf from bootmii installation
	d. bootmii.ini (set AUTOBOOT=SYSMENU, and BOOTDELAY=0)
	e. switchtosneek
3. SD card has apps directory or folder, and inside that folder:
	a. switch2sneek folder with the following files:
		i. boot.dol
		ii. icon.png
		iii. meta.xml

Instructions:

You could copy bootmii.ini and switchtosneek files in folder To_bootmii
into your SD card bootmii folder (but first edit the setting of VIDEO
in the bootmii.ini file accordingly).

Then you could copy folder switch2sneek into the apps folder in your SD card.

Insert back SD card to your Wii, and turn on Wii.

Run homebrew channel, and choose switch2sneek wiibrew application

Push button A to switch to SD NAND (through SNEEK)

Wii will reset and after that you will be then using SD NAND (through SNEEK)

Once you are done playing with SD NAND and want to switch back to real NAND,
go to homebrew channel, and choose switch2sneek wiibrew application again.

Push button B to switch back to real NAND (through bootmii)

Wii will reset and after that you will be then using real NAND (through bootmii)


Important Notes:

1. Be careful when pushing button B in switch2sneek. Do this only when you are 
   running SD NAND (through SNEEK). You may corrupt your real NAND if you try to 
   push button B in switch2sneek when running real NAND.






