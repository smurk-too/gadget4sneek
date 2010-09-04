/*

switch2uneek application

Author: conanac
Created: 09/04/2010
version 0.2beta 

*/

#include <fat.h>
#include <math.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <sdcard/wiisd_io.h>
#include <dirent.h>

//these are the only stable and speed is good -- from wiixplorer
#define CACHE 8
#define SECTORS 64

u8 loaderhbc = 0;
static u32 *xfb[2] = { NULL, NULL };
static GXRModeObj *vmode;
static char nandlocation[256] = "NOFILE";
s32 slotnumber = 0x00;
s32 priorslotnumber = 0x00;
s32 chosenslotnumber = 0x00;
s32 maxnands = 0x01;
s32 checknand = 0x00;
bool newsetup = false;
bool switchnow = false;
bool switchback = false;
bool slotfile = false;
bool locationfile = false;
bool bootmiiexist = false;
bool altbootmiiexist = false;
static char nandsubs[9][256] = 
{
	"IMPORT",
	"META",
	"SHARED1",
	"SHARED2",
	"SYS",
	"TICKET",
	"TITLE",
	"TMP",
	"WFS"
};

int USBDevice_Init()
{
        //closing all open Files write back the cache and then shutdown em!
        fatUnmount("usb:/");
		if (fatMount("usb", &__io_usbstorage, 0, CACHE, SECTORS))
		{
			return 1;
        }
        return -1;
}

void USBDevice_deInit()
{
        //closing all open Files write back the cache and then shutdown em!
        fatUnmount("usb:/");
        __io_usbstorage.shutdown();
}

bool USBDevice_Inserted()
{
        return __io_usbstorage.isInserted();
}

bool SDCard_Inserted()
{
		return __io_wiisd.isInserted();
}

int SDCard_Init()
{
        //closing all open Files write back the cache and then shutdown em!
        fatUnmount("sd:/");
        //right now mounts first FAT-partition
        if (fatMount("sd", &__io_wiisd, 0, CACHE, SECTORS))
                return 1;
        return -1;
}

void SDCard_deInit()
{
        //closing all open Files write back the cache and then shutdown em!
        fatUnmount("sd:/");
        __io_wiisd.shutdown();
}

void checkhbc(void)
{
	if (*((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x48415858)
		loaderhbc = 1;

	if (*((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x4A4F4449)
		loaderhbc = 1;

	if (*((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0xAF1BF516)
		loaderhbc = 1;
}

void exitme(void)
{
	if (loaderhbc)
	{
		WII_Initialize();
		if (WII_LaunchTitle(0x0001000148415858ULL) < 0)  
		    if (WII_LaunchTitle(0x000100014A4F4449ULL) < 0)
                    if (WII_LaunchTitle(0x00010001AF1BF516ULL) < 0)
                        WII_ReturnToMenu();
	}
	else
		WII_ReturnToMenu();
}

void die(char *msg) {
	printf(msg);
	sleep(5);
	SDCard_deInit();
	USBDevice_deInit();
	exitme();
}

void initialise(void) 
{
	VIDEO_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	vmode = VIDEO_GetPreferredMode(NULL);
	xfb[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	console_init(xfb[0],20,20,vmode->fbWidth,vmode->xfbHeight,vmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(vmode);
	VIDEO_SetNextFramebuffer(xfb[0]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	
	printf("\x1b[2;0H"); // start at 2nd row, 0th column
}

void pageheader(void)
{
	printf("\n\nswitch2uneek: simple application to boot to uneek\n\n");
}

void initfatSD(void)
{
	if(SDCard_Init() < 0)
		die("Unable to initialize SD card, exiting.\nYour SD card may not be placed in the Wii correctly.");
}

void initfatUSB(void)
{
	if(USBDevice_Init() < 0)
		die("Unable to initialize USB drive, exiting.\nYour USB drive may not be placed in the Wii correctly.");
}

void checkbootmiifolder(void)
{
	if(chdir("sd:/bootmii") == 0)
		bootmiiexist = true;
    else
		bootmiiexist = false;
	
	if(chdir("sd:/bootmiiuneek") == 0)
		altbootmiiexist = true;
	else
		altbootmiiexist = false;
}

void checkfilesUSB(void)
{
	FILE *fil;
  
	if ((fil = fopen("usb:/nandslot.bin","rb")) == NULL)  
		slotfile = false;
	else 
	{ 
		slotfile = true;
		fclose(fil);
	}

	if ((fil = fopen("usb:/nandpath.txt","r")) == NULL) 
		locationfile = false;
	else 
	{ 
		locationfile = true;
		fclose(fil);
	}
}

void renamefolder(void)
{
	if(!altbootmiiexist)
		die("\nCannot find sd:/bootmiiuneek, exiting.\n");
	else
	{
		if(bootmiiexist)
			if(rename("sd:/bootmii", "sd:/bootmiibackup") != 0)
				die("\nCannot back-up bootmii folder, exiting.\n");

		if(rename("sd:/bootmiiuneek", "sd:/bootmii") != 0)
			die("\nCannot move bootmiiuneek folder, exiting.\n");		
	}
}

void renamefolderback(void)
{
	if(!bootmiiexist)
		die("\nCannot find sd:/bootmii, exiting.\n");
	else
	{
		if(altbootmiiexist)
			if(rename("sd:/bootmiiuneek", "sd:/bootmiiuneekbackup") != 0)
				die("\nCannot back-up bootmiiuneek folder, exiting.\n");

		if(rename("sd:/bootmii", "sd:/bootmiiuneek") != 0)
			die("\nCannot move bootmii folder, exiting.\n");		
	}
}

void initialpage(void)
{
	if(bootmiiexist && altbootmiiexist)
	{
		printf("Push button [A] to start switching to UNEEK NAND\n");
		printf("Push button [B] to start switching back to real NAND\n");
	}
	else
	{
		if(bootmiiexist)
			printf("Push button [A] to start switching back to real NAND\n");
		else
		{
			if(altbootmiiexist)
				printf("Push button [A] to start switching to UNEEK NAND\n");
			else
				die("\nCannot find both bootmii and bootmiiuneek folders, exiting.\n");
		}
	}
	printf("Push button [Home] to exit now\n");
}

void changenandsheader(void)
{
	printf("Push button [+] to move forward in UNEEK NAND list\n");
	printf("Push button [-] to move backward in UNEEK NAND list\n");
	printf("Push button [A] to confirm the selection\n");
}

void menunands(void)
{
	printf("\nChoose NAND:\n");
	printf("    slot number = %d\n", slotnumber);
	printf("->  location    = %s\n", nandlocation);
}

void menunandsselected(void)
{
	printf("\nChoose NAND:\n");
	printf("    slot number = %d\n", slotnumber);
	printf("    location    = %s\n", nandlocation);
}

s32 getslotbin(void)
{
	s32 localslotnumber = 0x00;
  	FILE *f;

  	if ((f = fopen("usb:/nandslot.bin","rb")) != NULL) 
  	{ 
		fread(&localslotnumber, 1, sizeof(s32), f);
		fclose(f);	
  	}
	else
	{
		die("\nCannot read usb:/nandslot.bin file, exiting.\n");
	}

	return localslotnumber;
}

void getslotlocation(void)
{
	FILE *f;

  	if ((f = fopen("usb:/nandpath.txt","r")) != NULL) 
  	{ 
		fscanf(f, "%s", nandlocation);
		fclose(f);	
  	}
	else
	{
		die("\nCannot read usb:/nandpath.txt file, exiting.\n");
	}
}

void changeslotbin(void)
{
  	FILE *f;
  
  	if ((f = fopen("usb:/nandslot.bin","wb")) != NULL)
	{
		if ((slotnumber >= 0x00) & (slotnumber <= maxnands - 0x01))
		{ 
			fwrite(&slotnumber, 1, sizeof(s32), f);
		}
		fclose(f);	
	}
	else
	{
		die("\nCannot write usb:/nandslot.bin file, exiting.\n");
	}
}

void changeslotlocation(void)
{
  	FILE *f;
  
  	if ((f = fopen("usb:/nandpath.txt","w")) != NULL)
	{
		if ((slotnumber >= 0x00) & (slotnumber <= maxnands - 0x01))
		{ 
			fprintf(f, "%s", nandlocation);
		}
		fclose(f);	
	}
	else
	{
		die("\nCannot write usb:/nandpath.txt file, exiting.\n");
	}
}

s32 getcountnands(void)
{
	s32 countnands = 0x00;

	DIR* d = opendir("usb:/nands");

	if(d == NULL)
		return 0x00;
	
	while(true)
	{
		struct dirent* dent = readdir(d);
		if(dent == NULL) break;
		if(strcmp(".", dent->d_name) != 0 && strcmp("..", dent->d_name) != 0)
			countnands += 0x01;
	}

	closedir(d);	
	return countnands;
}

void getnandsinfo(void)
{
	s32 slotid = 0x00;

	DIR* d = opendir("usb:/nands/");

	if(d != NULL)
	{	
		while(true)
		{	
			struct dirent* dent = readdir(d);
			if(dent == NULL) break;
			if(strcmp(".", dent->d_name) != 0 && strcmp("..", dent->d_name) != 0)
				slotid += 0x01;
			if(slotnumber == slotid - 0x01)
			{
				sprintf(nandlocation, "usb:/nands/%s", dent->d_name);
				break;
			}
		}
		closedir(d);
	}	
}

s32 preparenand(void)
{
	char *nandsourcepath, *nandtargetpath;
	s32 countcopy = 0x00;
	int i;

	DIR* d = opendir(nandlocation);
	nandsourcepath = (char*)malloc(0x100);
	nandtargetpath = (char*)malloc(0x100);
	if(d != NULL)
	{	
		while(true)
		{
			struct dirent* dent = readdir(d);
			if(dent == NULL) break;
			for(i=0; i<9; i++)
			{
				if(strcasecmp(nandsubs[i], dent->d_name) == 0)
				{
					sprintf(nandsourcepath, "%s/%s", nandlocation, nandsubs[i]);
					sprintf(nandtargetpath, "usb:/%s", nandsubs[i]);
					if(rename(nandsourcepath, nandtargetpath) == 0)
					{
						countcopy++;
					}
				}
			}
		}
		closedir(d);
	}	
	free(nandsourcepath);
	free(nandtargetpath);

	return countcopy;
}

void checknandsfolder(void)
{
	DIR* n = opendir("usb:/nands");
	if(n == NULL)
	{  	
		die("\nCannot find usb:/nands folder, exiting.\n");		
	}
	else
	{  
		closedir(n);
    }
}

s32 storenand(void)
{
	char *nandsourcepath, *nandtargetpath;
	s32 countcopy = 0x00;
	int i;

	DIR* d = opendir("usb:/");
	nandsourcepath = (char*)malloc(0x100);
	nandtargetpath = (char*)malloc(0x100);
	if(d != NULL)
	{	
		while(true)
		{
			struct dirent* dent = readdir(d);
			if(dent == NULL) break;
			for(i=0; i<9; i++)
			{
				if(strcasecmp(nandsubs[i], dent->d_name) == 0)
				{  
					sprintf(nandtargetpath, "%s/%s", nandlocation, nandsubs[i]);
					sprintf(nandsourcepath, "usb:/%s", nandsubs[i]);
					if(rename(nandsourcepath, nandtargetpath) == 0)
					{
						countcopy++;
					}
				}
			}
		}
		closedir(d);
	}	
	free(nandsourcepath);
	free(nandtargetpath);

	return countcopy;
}

void checksetup(void)
{ 
	FILE *f;

  	if ((f = fopen("usb:/nandpath.txt","r")) != NULL)
	{
		fscanf(f, "%s", nandlocation);
		if(strcasecmp(nandlocation, "NOFILE") == 0)
		{
			newsetup = true;
		}
		else
		{
			newsetup = false;
		}
		fclose(f);	
	}
	else
	{
		die("\nCannot read usb:/nandpath.txt file, exiting.\n");
	}
}

int main(void) 
{
	u32 wpaddown;

	initialise();	
	
	checkhbc();

	initfatSD();
	checkbootmiifolder();

	pageheader();
	initialpage();
		
	while(1)
	{
		WPAD_ScanPads();
        u32 wpaddown = WPAD_ButtonsDown(0);
		if (wpaddown & WPAD_BUTTON_A)
		{
			if(bootmiiexist && altbootmiiexist)
			{
				switchnow = true;
				switchback = false;
				break;
			}
			else
			{
				if(bootmiiexist)
				{
					switchnow = false;
					switchback = true;
					break;
				}	
				else
				{
					if(altbootmiiexist)
					{
						switchnow = true;
						switchback = false;
						break;
					}
				}
			}
		}
		if (wpaddown & WPAD_BUTTON_HOME)
		{
			printf("exiting now...OK\n");
			sleep(5);
			SDCard_deInit();
			USBDevice_deInit();
			exitme();
		}
		if (wpaddown & WPAD_BUTTON_B)
		{
			if(bootmiiexist && altbootmiiexist)
			{
				switchnow = false;
				switchback = true;
				break;
			}
		}
	}
	
	if(switchback == true)
	{
	   printf("\n");

       printf("You have ");
       if(bootmiiexist && altbootmiiexist) 
			printf("UNEEK and real NAND setups, turn real NAND on\n");
	   else
	   {	if(bootmiiexist) 
				printf("real NAND setups, switch to real NAND\n");
	   }

       printf("\n");
	   
	   renamefolderback();
		
	   sleep(5);
	   SDCard_deInit();
	   USBDevice_deInit();
	   WPAD_Shutdown();
	   STM_RebootSystem();				
	}
	
	if(switchnow == true)
	{	
	    printf("\n");
		
		initfatUSB();
		checkfilesUSB();
		checksetup();

//choosing nand menu
//begin

		slotnumber = getslotbin();
	    priorslotnumber = slotnumber;
		checknandsfolder();
		maxnands = getcountnands();
		if(maxnands == 0x00)
			die("\nNo choice of NAND are available in usb:/nands folder, exiting.\n");
		getnandsinfo();

		if(slotnumber > maxnands - 1)
		{
           		printf("slot number = %d in nandslot.bin file is more than number of NAND = %d\n\n",
				slotnumber, maxnands - 1);
	       		die("\nCannot continue, exiting\n");
		}

		do{			
			/* Wait for vertical sync */
			VIDEO_WaitVSync();
	
			/* Clear screen */
			printf("\x1b[2J\n\n");
			printf("\x1b[2;0H"); // start at 2nd row, 0th column
			
			pageheader();
			changenandsheader();
			menunands();
					
			WPAD_ScanPads();
			wpaddown = WPAD_ButtonsDown(0);
			if (wpaddown & WPAD_BUTTON_PLUS) {
				slotnumber += 0x01;
				if(slotnumber > maxnands - 0x01)
					slotnumber = 0x00;
				getnandsinfo();
			}
			if (wpaddown & WPAD_BUTTON_MINUS) {
				slotnumber -= 0x01;
				if(slotnumber < 0x00)
					slotnumber = maxnands - 0x01;
				getnandsinfo();
			}
			if (wpaddown & WPAD_BUTTON_HOME)
			{
				printf("exiting now...OK\n");
				sleep(5);
				SDCard_deInit();
			    USBDevice_deInit();
				exitme();
			}
		} while (!(wpaddown & WPAD_BUTTON_A));

		/* Wait for vertical sync */
		VIDEO_WaitVSync();
	
		/* Clear screen */
		printf("\x1b[2J\n\n");
		printf("\x1b[2;0H"); // start at 2nd row, 0th column
			
		pageheader();
		changenandsheader();
		menunandsselected();
		chosenslotnumber = slotnumber;

		printf("\n");

//end

		printf("You have ");
		if(bootmiiexist && altbootmiiexist) 
			printf("UNEEK and real NAND setups, turn UNEEK NAND on\n");
		else
		{	
			if(altbootmiiexist) 
				printf("UNEEK NAND setups, switch to UNEEK NAND\n");
		}

//store prior nand from usb:/ to usb:/nands/nandlocation	
		if(!newsetup)
		{
			slotnumber = priorslotnumber;
			getnandsinfo();
			checknand = storenand();
			if(checknand <= 0 || strcasecmp(nandlocation, "NOFILE") == 0)
				die("\nCannot move back sub-folders of NAND, exiting.\n");
			else
				printf("\nMoved %d sub-folder(s) of NAND from usb:/ to %s\n", checknand, nandlocation);
		}

//prepare nand from usb:/nands/nandlocation to usb:/
		slotnumber = chosenslotnumber;
		getnandsinfo();
		changeslotbin();
		changeslotlocation();
		checknand = preparenand();
		if(checknand <= 0 || strcasecmp(nandlocation, "NOFILE") == 0)
		{
		    if(newsetup)
			{
				strcpy(nandlocation,"NOFILE");
				changeslotlocation();
			}
			die("\nCannot prepare sub-folders of NAND, exiting.\n");
		}
		else
			printf("\nMoved %d sub-folder(s) of NAND from %s to usb:/\n", checknand, nandlocation);

        renamefolder();

        sleep(5);

		SDCard_deInit();
		USBDevice_deInit();
	    WPAD_Shutdown();
	    STM_RebootSystem();				
	}
	
	SDCard_deInit();
	USBDevice_deInit();
    exitme();	
	return 0;
}
