/*

sneek2sneekplus application

Author: conanac
Created: 03/21/2010
version 0.2beta (04/04/2010) -- adding exitme and checkhbc to avoid core dump at exit 
	when run as a channel, and fix bugs

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
#include <dirent.h>

#include "../build/certs_dat.h"
#include "../build/fake_su_tmd_dat.h"
#include "../build/fake_su_ticket_dat.h"

u8 loaderhbc = 0;
static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void checkhbc(void)
{
	if (*((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x48415858)
		loaderhbc = 1;

	if (*((u32 *) 0x80001804) == 0x53545542 && *((u32 *) 0x80001808) == 0x4A4F4449)
		loaderhbc = 1;

}

void exitme(void)
{
	if (loaderhbc)
	{
		WII_Initialize();
		if (WII_LaunchTitle(0x0001000148415858ULL) < 0) 
		{ 
		    if (WII_LaunchTitle(0x000100014A4F4449ULL) < 0)
			    WII_ReturnToMenu();
		}
	}
	else
	{
		WII_ReturnToMenu();
	}
}

void die(char *msg) {
	printf(msg);
	sleep(5);
	fatUnmount("sd:");
	fatUnmount("usb:");
	exitme();
}

void basicinit(void) 
{
	VIDEO_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	
	printf("\x1b[2;0H"); // start at 2nd row, 0th column
}

void pageheader(void)
{
	printf("\n\nsneek2sneekplus: simple application to switch between sneek SD NAND +/- DI\n\n");
}

void initfat(void)
{
	if (!fatInitDefault()) die("Unable to initialise FAT subsystem, exiting.\nYour SD card or USB drive may not be placed in the Wii correctly.");	
}

bool switchnow = false;
bool switchback = false;
bool bootsneek = false;
bool bootsneekplus = false;
bool disneekplus = false;
bool di = false;
bool switchsneekplus = false; // true when running boot2.bin of sneek
bool switchsneek = false; // true when running boot2.bin of sneekplus

static char gamesname[255] = "";
s32 slotnumber = 0x00;
s32 maxgames = 0x00;

void checkfiles(void)
{
  FILE *fil;
  
  if ((fil = fopen("sd:/boot2sneek.bin","rb")) == NULL) 
  { bootsneek = false;
  }
  else 
  { bootsneek = true;
    fclose(fil);
  }
  
  if ((fil = fopen("sd:/boot2sneekplus.bin","rb")) == NULL) 
  { bootsneekplus = false;
  }
  else 
  { bootsneekplus = true;
    fclose(fil);
  }
  
  if ((fil = fopen("sd:/switchtosneekplus","rb")) == NULL) 
  { switchsneekplus = false;
  }
  else 
  { switchsneekplus = true;
    fclose(fil);
  }

  if ((fil = fopen("sd:/switchtosneek","rb")) == NULL) 
  { switchsneek = false;
  }
  else 
  { switchsneek = true;
    fclose(fil);
  }
  
  if ((fil = fopen("sd:/disneekplus.bin","rb")) == NULL) 
  { disneekplus = false;
  }
  else 
  { disneekplus = true;
    fclose(fil);
  }
  
  if ((fil = fopen("sd:/di.bin","rb")) == NULL) 
  { di = false;
  }
  else 
  { di = true;
    fclose(fil);
  }
}

// boot2.bin --> boot2sneekplus; boot2sneek --> boot2.bin
// di.bin --> disneekplus.bin
void renamefilesback(void)
{
  s32 ret;
  char filepath[256];
  char filepathold[256];
  
  if( bootsneek == true && bootsneekplus == true )
  { 
    sprintf(filepath, "/boot2sneekplus.bin");
    ret = ISFS_Delete(filepath);
	if (ret < 0)
		printf("\nError! ISFS_Delete(%s) returned %d)\n", filepath, ret);
	
	sprintf(filepathold, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
	
	sprintf(filepathold, "/boot2sneek.bin");
	sprintf(filepath, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);	
  }
  if( bootsneek == true && bootsneekplus == false ) 
  { 
    sprintf(filepath, "/boot2sneekplus.bin");
	sprintf(filepathold, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
	
	sprintf(filepathold, "/boot2sneek.bin");
	sprintf(filepath, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);				
  }
  if (di == true)
  {
    sprintf(filepath, "/disneekplus.bin");
	sprintf(filepathold, "/di.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
  }
  
}

// boot2.bin --> boot2sneek; boot2sneekplus --> boot2.bin
// disneekplus.bin --> di.bin
void renamefiles(void)
{
  s32 ret;
  char filepath[256];
  char filepathold[256];
  
  if( bootsneekplus == true && bootsneek == true )
  { 
    sprintf(filepath, "/boot2sneek.bin");
    ret = ISFS_Delete(filepath);
	if (ret < 0)
		printf("\nError! ISFS_Delete(%s) returned %d)\n", filepath, ret);
	
	sprintf(filepathold, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
	
	sprintf(filepathold, "/boot2sneekplus.bin");
	sprintf(filepath, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);	
  }
  if( bootsneekplus == true && bootsneek == false ) 
  { 
    sprintf(filepath, "/boot2sneek.bin");
	sprintf(filepathold, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
	
	sprintf(filepathold, "/boot2sneekplus.bin");
	sprintf(filepath, "/boot2.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);				
  }
  if (disneekplus == true)
  {
    sprintf(filepath, "/di.bin");
	sprintf(filepathold, "/disneekplus.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
  }
}

void removeswitchsneekplus(void)
{
  s32 ret;
  char filepath[256];

  if(switchsneekplus == true)
  {
	sprintf(filepath, "/switchtosneekplus");
	
	ret = ISFS_Delete(filepath);
	
	if (ret < 0)
		printf("\nError! ISFS_Delete(%s) returned %d)\n", filepath, ret);

    switchsneekplus = false;
  }
}

void removeswitchsneek(void)
{
  s32 ret;
  char filepath[256];

  if(switchsneek == true)
  {
	sprintf(filepath, "/switchtosneek");
	
	ret = ISFS_Delete(filepath);
	
	if (ret < 0)
		printf("\nError! ISFS_Delete(%s) returned %d)\n", filepath, ret);

    switchsneek = false;
  }
}

void createswitchsneek(void)
{
  s32 ret;
  char filepath[256];

  if(switchsneek == false)
  {
    sprintf(filepath, "/switchtosneek");
    ret = ISFS_CreateFile (filepath, 0, 3, 1, 1);

    if(ret != ISFS_OK) 
      printf("\nError! ISFS_CreateFile(%s) returned %d\n", filepath, ret);
  }
}

void createswitchsneekplus(void)
{
  s32 ret;
  char filepath[256];

  if(switchsneekplus == false)
  {
    sprintf(filepath, "/switchtosneekplus");
    ret = ISFS_CreateFile (filepath, 0, 3, 1, 1);

    if(ret != ISFS_OK) 
      printf("\nError! ISFS_CreateFile(%s) returned %d\n", filepath, ret);
  }
}

s32 Identify(const u8 *certs, u32 certs_size, const u8 *idtmd, u32 idtmd_size, const u8 *idticket, u32 idticket_size) {
	s32 ret;
	u32 keyid = 0;
	ret = ES_Identify((signed_blob*)certs, certs_size, (signed_blob*)idtmd, idtmd_size, (signed_blob*)idticket, idticket_size, &keyid);
	if (ret < 0){
		switch(ret){
			case ES_EINVAL:
				printf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				printf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				printf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				printf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				printf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
	}
	else
		printf("OK!\n");
	return ret;
}

s32 Identify_SU(void) {
	printf("\nUse super-user identity...");
	fflush(stdout);
	return Identify(certs_dat, certs_dat_size, fake_su_tmd_dat, fake_su_tmd_dat_size, fake_su_ticket_dat, fake_su_ticket_dat_size);
}

void miscinit(void)
{
	s32 ret;

	ret = Identify_SU();
		
	printf("\nInitializing file-system driver...");
	fflush(stdout);
	
	ret = ISFS_Initialize();
	if (ret < 0) {
		printf("\nError! ISFS_Initialize (ret = %d)\n", ret);
	} else {
		printf("OK!\n");
	}
}

void miscdeinit(void)
{
	fflush(stdout);
	ISFS_Deinitialize();
}

void initialpage(void)
{
	printf("Push button [A] to start switching to SD NAND + DI\n");
	printf("Push button [B] to start switching back to SD NAND\n");
	printf("Push button [Home] to exit now\n");
}

void changegamesheader(void)
{
	printf("Push button [+] to move forward in games list\n");
	printf("Push button [-] to move backward in games list\n");
	printf("Push button [A] to confirm the selection\n");
}

s32 getslotbin(void)
{
	s32 localslotnumber = 0x00;
  	FILE *f;

  
  	if ((f = fopen("usb:/sneek/slot.bin","rb")) != NULL) 
  	{ 
		fread(&localslotnumber, 1, sizeof(s32), f);
		fclose(f);	
  	}
	return localslotnumber;
}

void changeslotbin(void)
{
  	FILE *f;
  
  	if ((f = fopen("usb:/sneek/slot.bin","wb")) != NULL)
	{
		if ((slotnumber >= 0x00) & (slotnumber <= maxgames - 0x01))
		{ 
			fwrite(&slotnumber, 1, sizeof(s32), f);
		}
		fclose(f);	
	}
}

s32 getcountgames(void)
{
	s32 countgames = 0x00;

	DIR* d = opendir("usb:/games");

	if(d == NULL)
		return 0x00;
	
	while(true)
	{
		struct dirent* dent = readdir(d);
		if(dent == NULL) break;
		if(strcmp(".", dent->d_name) != 0 && strcmp("..", dent->d_name) != 0)
			countgames += 0x01;
	}

	closedir(d);	
	return countgames;
}

void getgamesinfo(void)
{
	s32 slotid = 0x00;
	static char gamepath[255] = "";
	FILE *f;
	u32 nameoffset = 0x20;
	u32 j;
	unsigned char *gameinfobuf;
	static char addcharacter[2];

	DIR* d = opendir("usb:/games");

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
				sprintf(gamepath, "usb:/games/%s/sys/boot.bin", dent->d_name);
				if ((f = fopen(gamepath,"rb")) != NULL) 
  				{ 	
					gameinfobuf = (unsigned char*)malloc(0x100);
					if(gameinfobuf == NULL)
						break;
					fread(gameinfobuf, 1, 0x100, f);
					fclose(f);
					strcpy(gamesname, "");
					j = nameoffset;
					while(gameinfobuf[j] != 0x00)
					{
						if(gameinfobuf[j] > 0x80)
						{
							addcharacter[0] = '*';
						}
						else
						{
							sprintf(addcharacter, "%c", gameinfobuf[j]);
						}
						addcharacter[1] = '\0';
						strcat(gamesname, addcharacter);
						j += 0x01;
						if (j >= 0x100) break;
					} 
					free(gameinfobuf);				
				}
				break;
			}
		}
		closedir(d);
	}	
}

void menugames(void)
{
	printf("\nChoose title:\n");
	printf("    slot number = %d\n", slotnumber);
	printf("->  title       = %s\n", gamesname);
}

void menugamesselected(void)
{
	printf("\nChoose title:\n");
	printf("    slot number = %d\n", slotnumber);
	printf("    title       = %s\n", gamesname);
}

int main(void) 
{
	u32 wpaddown;
	
	basicinit();
	pageheader();
	initialpage();

	initfat();
	checkfiles();
	
	while(1){
		WPAD_ScanPads();
            wpaddown = WPAD_ButtonsDown(0);
		if (wpaddown & WPAD_BUTTON_A){
			switchnow = true;
			break;
		}
		if (wpaddown & WPAD_BUTTON_HOME){
			printf("exiting now...OK\n");
			sleep(5);
			exitme();
		}
		if (wpaddown & WPAD_BUTTON_B){
			switchback = true;
			break;
		}
	}
	
	// from SD NAND to SD NAND + DI
	if(switchnow == true)
	{	
		printf("\n");

        	if((switchsneek == true) &&  (switchsneekplus == false))
        	{
			printf("It seems sneek v2 SD NAND is not running, check switchtosneek and switchtosneekplus files\n\n");
	     		sleep(5);
           		exitme();
	    	}
	   
		if(bootsneekplus == false)
        	{
           		printf("No sneek v2 file in SD root directory: boot2sneekplus.bin\n\n");
	       	sleep(5);
           		exitme();
        	}

		slotnumber = getslotbin();
		maxgames = getcountgames();
		getgamesinfo();

		if(slotnumber > maxgames - 1)
		{
           		printf("slot number = %d in slot.bin file is more than number of games = %d\n\n",
				slotnumber, maxgames - 1);
	       	sleep(5);
           		exitme();
		}

		do{			
			/* Wait for vertical sync */
			VIDEO_WaitVSync();
	
			/* Clear screen */
			printf("\x1b[2J\n\n");
			printf("\x1b[2;0H"); // start at 2nd row, 0th column
			
			pageheader();
			changegamesheader();
			menugames();
					
			WPAD_ScanPads();
			wpaddown = WPAD_ButtonsDown(0);
			if (wpaddown & WPAD_BUTTON_PLUS) {
				slotnumber += 0x01;
				if(slotnumber > maxgames - 0x01)
					slotnumber = 0x00;
				getgamesinfo();
			}
			if (wpaddown & WPAD_BUTTON_MINUS) {
				slotnumber -= 0x01;
				if(slotnumber < 0x00)
					slotnumber = maxgames - 0x01;
				getgamesinfo();
			}
			if (wpaddown & WPAD_BUTTON_HOME)
			{
				printf("exiting now...OK\n");
				sleep(5);
				exitme();
			}
		} while (!(wpaddown & WPAD_BUTTON_A));

		/* Wait for vertical sync */
		VIDEO_WaitVSync();
	
		/* Clear screen */
		printf("\x1b[2J\n\n");
		printf("\x1b[2;0H"); // start at 2nd row, 0th column
			
		pageheader();
		changegamesheader();
		menugamesselected();

		printf("\n");

        	printf("You have ");
        	if( bootsneekplus == true && bootsneek == true ) printf("sneek SD NAND + DI and sneek SD NAND setups");
        	if( bootsneekplus == true && bootsneek == false ) printf("sneek SD NAND + DI setups");

        	if( bootsneekplus == true && bootsneek == false ) printf(" turn sneek SD NAND + DI on");
        	if( bootsneekplus == true && bootsneek == true ) printf(" switch to sneek SD NAND + DI");
        	printf("\n");
		
		miscinit();

        	renamefiles();
	    	removeswitchsneekplus();
	    	createswitchsneek();
		changeslotbin();			
		
		miscdeinit();

        	sleep(5);

	    	WPAD_Shutdown();
	    	STM_RebootSystem();				
	}

	// from SD NAND + DI to SD NAND
	if(switchback == true)
	{
	   	printf("\n");

       	if((switchsneek == false) &&  (switchsneekplus == true))
       	{
		  	printf("It seems sneek SD NAND + DI is not running, check switchtosneek and switchtosneekplus files\n\n");
	      	sleep(5);
          		exitme();
	   	}

	   	if(bootsneek == false)
       	{
          		printf("No sneek v2 file in SD root directory: boot2sneek.bin\n\n");
	      	sleep(5);
          		exitme();
       	}

       	printf("You have ");
       	if( bootsneekplus == true && bootsneek == true ) printf("sneek SD NAND + DI and sneek SD NAND setups");
       	if( bootsneekplus == false && bootsneek == true ) printf("sneek SD NAND setups");

       	if( bootsneekplus == false && bootsneek == true ) printf(" turn sneek SD NAND on");
       	if( bootsneekplus == true && bootsneek == true ) printf(" switch to sneek SD NAND");
       	printf("\n");

	   	miscinit();
	   
	   	renamefilesback();
	   	removeswitchsneek();
	   	createswitchsneekplus();

	   	miscdeinit();
		
	   	sleep(5);

	   	WPAD_Shutdown();
	   	STM_RebootSystem();				
	}
		
	return 0;
}
