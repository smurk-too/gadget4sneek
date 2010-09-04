
This is a simple application for switching between real NAND
and UBS NAND (through UNEEK).

This is a beta release so there are RISKS you could make your UNEEK NAND
being corrupted/deleted. Do not use this application if you cannot take such a risk.

Since I have bootmii as boot2, this application is only tested
and used by me with that condition. If you have bootmii not as boot2
I dont know whether this application will work for you or not.

Use this application after you manage to get UNEEK to work with your SD card,
and USB drive. And make sure that you follow the process of setting-up described
below.

Requirements:
1. Bootmii installed as boot2
2. SD card has "bootmiiuneek" directory or folder with the following file inside:
	a. "armboot.bin" from UNEEK compilation
3. SD card has "sneek" directory or folder with the following file inside:
	a. "kernel.bin" from UNEEK compilation
4. SD card has apps directory or folder, and inside that folder:
	a. switch2uneek folder with the following files:
		i. boot.dol
		ii. icon.png
		iii. meta.xml
5. USB drive has "sneek" directory or folder with the following file inside:
	a. "kernel.bin" from UNEEK compilation
	b. "di.bin" from UNEEK compilation
	c. "font.bin"
	d. "diconfig.bin"
6. USB drive has two files in  the main directory or folder:
	a. "nandslot.bin" from this package
	b. "nandpath.txt" from this package
7. USB drive has "nands" directory or folder that contains NAND contents (see below
	for setting up this structure)

Initial set-ups:
1. If you have "bootmii" directory or folder (from bootmii installation) in your SD card, 
   make a back-up then delete it, or just rename it.
2. Create "bootmiiuneek" directory or folder in your SD card and copy the "armboot.bin" file
   generated from compiling UNEEK. In the end the structure of the SD card looks like:


    	SD  --- apps (a folder) --- switch2uneek (a folder) --- boot.dol (a file)
	    |						    |
	    |						    |-- meta.xml (a file)
	    |						    |
	    |						    |-- icon.png (a file)
	    |
	    |-- sneek (a folder) --- kernel.bin (a file)
	    |
	    |-- bootmiiuneek (a folder) --- armboot.bin (a file)
					        	         
   
3. In USB drive, create "nands" directory or folder which contains at least one sub-folder
   with NAND contents. For running the first time, do NOT have any NAND contents in the main 
   folder of USB drive. Below is an example of the structure of the USB drive directories/folders.
   As you can see there is no {"title", "ticket", "meta", "shared1", "shared2", "sys", "import", 
   "tmp", "wfs"} folders in the main directory or folder of the USB drive. All those NAND contents 
   are stored in each corresponding sub-folders under "nands" directory/folder (in the example below, 
   there are two choices of NAND, one is from SM4.3U and the other is from SM3.2U).


	USB --- sneek (a folder) --- kernel.bin (a file)
	    |			 |
	    |			 |-- di.bin (a file)
	    |			 |
	    |			 |-- font.bin (a file)
	    |			 |
	    |			 |-- diconfig.bin (a file)
	    |
            |
	    |-- games (a folder to store the choices of games)
	    |
	    |-- nandslot.bin (a file)
	    |
	    |-- nandpath.txt (a file)
	    |
	    |-- nands (a folder) --- SM4.3U (a folder) --- title (a folder)
				 |		       |
				 |		       |-- ticket (a folder)
				 |		       |
				 |		       |-- sys (a folder)
				 |		       |
				 |		       |-- meta (a folder)
				 |		       |
				 |		       |-- import (a folder)
				 |		       |
				 |		       |-- shared1 (a folder)
				 |		       |
				 |		       |-- shared2 (a folder)
				 |		       |
				 |		       |-- tmp (a folder)
				 |		       |
				 |		       |-- wfs (a folder)
				 |		       
				 |-- SM3.uU (a folder) --- title (a folder)
				 		       |
				 		       |-- ticket (a folder)
				 		       |
				 		       |-- sys (a folder)
				 		       |
				 		       |-- meta (a folder)
				 		       |
				 		       |-- import (a folder)
				 		       |
				 		       |-- shared1 (a folder)
				 		       |
						       |-- shared2 (a folder)
				 		       |
				 		       |-- tmp (a folder)
				 		       |
				 		       |-- wfs (a folder)					               
	         

4. Copy "nandslot.bin" and "nandpath.txt files" from this package to the root directory or
   main folder of the USB drive.
 
Instructions:
1. After setting-up the SD card and USB drive as described above, put them back to the Wii
2. Run homebrew channel, and choose switch2uneek wiibrew application
3. Follow the instructions on the screen to switch from real NAND to UNEEK NAND,
   and you will be given the name(s) of sub-folder(s) in the "nands" folder as the label for
   selecting different NAND. Choose one of those. And the application will move those
   sub-folders to the root directory or main folder of the USB drive.
4. Wii will reset and after that you will be using your choice of UNEEK NAND
5. Once you are done playing with UNEEK NAND and want to switch back to real NAND,
   go to homebrew channel, and choose switch2uneek wiibrew application again.
6. Follow the instructions on the screen to switch from UNEEK NAND to real NAND
7. Wii will reset and after that you will be using real NAND




