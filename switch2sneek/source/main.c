/*

switch2sneek application

Author: conanac
Created: 02/27/2010
version 0.3beta (04/04/2010) -- adding exitme and checkhbc to avoid core dump at exit 
	when run as a channel

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

#include "../build/certs_dat.h"
#include "../build/fake_su_tmd_dat.h"
#include "../build/fake_su_ticket_dat.h"

u8 loaderhbc = 0;
static u32 *xfb[2] = { NULL, NULL };
static GXRModeObj *vmode;

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

	printf("switch2sneek: simple application to boot to sneek v2\n\n");
}

void initfat(void)
{
	if (!fatInitDefault()) die("Unable to initialise FAT subsystem, exiting.\nYour SD card may not be placed in the Wii correctly.");	
}


bool switchnow = false;
bool switchback = false;
bool armbootsneek = false;
bool armbootmii = false;
bool switchmii = false; // true when running armboot.bin of sneek
bool switchsneek = false; // true when running armboot.bin of bootmii

void checkfiles(void)
{
  FILE *fil;
  
  if ((fil = fopen("sd:/bootmii/armbootsneek.bin","rb")) == NULL) 
  { armbootsneek = false;
  }
  else 
  { armbootsneek = true;
    fclose(fil);
  }
  
  if ((fil = fopen("sd:/bootmii/armbootmii.bin","rb")) == NULL) 
  { armbootmii = false;
  }
  else 
  { armbootmii = true;
    fclose(fil);
  }
  
  if ((fil = fopen("sd:/bootmii/switchtomii","rb")) == NULL) 
  { switchmii = false;
  }
  else 
  { switchmii = true;
    fclose(fil);
  }

  if ((fil = fopen("sd:/bootmii/switchtosneek","rb")) == NULL) 
  { switchsneek = false;
  }
  else 
  { switchsneek = true;
    fclose(fil);
  }
}

void renamefiles(void)
{
  if( armbootmii == true && armbootsneek == true )
  {
    remove("sd:/bootmii/armbootmii.bin");
    rename("sd:/bootmii/armboot.bin","sd:/bootmii/armbootmii.bin");
    rename("sd:/bootmii/armbootsneek.bin","sd:/bootmii/armboot.bin");
    remove("sd:/bootmii/armbootsneek.bin");
  }
  if( armbootmii == false && armbootsneek == true ) 
  { 
    rename("sd:/bootmii/armboot.bin","sd:/bootmii/armbootmii.bin");
    rename("sd:/bootmii/armbootsneek.bin","sd:/bootmii/armboot.bin"); 
    remove("sd:/bootmii/armbootsneek.bin");
  }
}

void renamefilesback(void)
{
  s32 ret;
  char filepath[256];
  char filepathold[256];
  
  if( armbootmii == true && armbootsneek == true )
  { 
    sprintf(filepath, "/bootmii/armbootsneek.bin");
    ret = ISFS_Delete(filepath);
	if (ret < 0)
		printf("\nError! ISFS_Delete(%s) returned %d)\n", filepath, ret);
	
	sprintf(filepathold, "/bootmii/armboot.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
	
	sprintf(filepathold, "/bootmii/armbootmii.bin");
	sprintf(filepath, "/bootmii/armboot.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);	
  }
  if( armbootmii == true && armbootsneek == false ) 
  { 
    sprintf(filepath, "/bootmii/armbootsneek.bin");
	sprintf(filepathold, "/bootmii/armboot.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);
	
	sprintf(filepathold, "/bootmii/armbootmii.bin");
	sprintf(filepath, "/bootmii/armboot.bin");
	ret = ISFS_Rename(filepathold, filepath);
	if (ret < 0)
		printf("\nError! ISFS_Rename(%s, %s) returned %d)\n", filepathold, 
			filepath, ret);				
  }
}

void removeswitchmii(void)
{
  s32 ret;
  char filepath[256];

  if(switchmii == true)
  {
	sprintf(filepath, "/bootmii/switchtomii");
	
	ret = ISFS_Delete(filepath);
	
	if (ret < 0)
		printf("\nError! ISFS_Delete(%s) returned %d)\n", filepath, ret);

    switchmii = false;
  }
}

void removeswitchsneek(void)
{
  if(switchsneek == true)
  {
    remove("sd:/bootmii/switchtosneek");
    switchsneek = false;
  }
}

void createswitchsneek(void)
{
  s32 ret;
  char filepath[256];

  if(switchsneek == false)
  {
    sprintf(filepath, "/bootmii/switchtosneek");
    ret = ISFS_CreateFile (filepath, 0, 3, 1, 1);

    if(ret != ISFS_OK) 
      printf("\nError! ISFS_CreateFile(%s) returned %d\n", filepath, ret);
  }
}

void createswitchmii(void)
{
  FILE *fil;
  
  if(switchmii == false)
  {
    if((fil = fopen("sd:/bootmii/switchtomii","wb")) == NULL) 
    { 
      printf("\nError! Cannot create switchtomii file");
    }
    else 
    { 
      switchmii = true;
      fclose(fil);
    }
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

int main(void) 
{
	initialise();
	checkhbc();
	
	printf("Push button [A] to start switching to SD NAND\n");
	printf("Push button [B] to start switching back to real NAND\n");
	printf("Push button [Home] to exit now\n");
	while(1){
		WPAD_ScanPads();
            u32 wpaddown = WPAD_ButtonsDown(0);
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

	initfat();
	checkfiles();
	
	if(switchback == true)
	{
	   printf("\n");

       if((switchmii == false) &&  (switchsneek == true))
       {
		  printf("It seems sneek is not running, check switchtosneek and switchtomii files\n\n");
	      sleep(5);
          exitme();
	   }

	   if(armbootmii == false)
       {
          printf("No sneek v2 file in bootmii directory: armbootmii.bin\n\n");
	      sleep(5);
          exitme();
       }

       printf("You have ");
       if( armbootsneek == true && armbootmii == true ) printf("sneek and bootmii setups");
       if( armbootsneek == false && armbootmii == true ) printf("bootmii setups");

       if( armbootsneek == false && armbootmii == true ) printf(" turn bootmii on");
       if( armbootsneek == true && armbootmii == true ) printf(" switch to bootmii");
       printf("\n");

	   miscinit();
	   
	   renamefilesback();
	   removeswitchmii();
	   createswitchsneek();

	   miscdeinit();
		
	   sleep(5);

	   WPAD_Shutdown();
	   STM_RebootSystem();				
	}
	
	if(switchnow == true)
	{	
	    printf("\n");
	
        if((switchmii == true) &&  (switchsneek == false))
        {
		   printf("It seems bootmii is not running, check switchtosneek and switchtomii files\n\n");
	       sleep(5);
           exitme();
	    }
	   
	    if(armbootsneek == false)
        {
           printf("No sneek v2 file in bootmii directory: armbootsneek.bin\n\n");
	       sleep(5);
           exitme();
        }

        printf("You have ");
        if( armbootsneek == true && armbootmii == true ) printf("sneek and bootmii setups");
        if( armbootsneek == true && armbootmii == false ) printf("sneek setups");

        if( armbootsneek == true && armbootmii == false ) printf(" turn sneek on");
        if( armbootsneek == true && armbootmii == true ) printf(" switch to sneek");
        printf("\n");

        renamefiles();
	    removeswitchsneek();
	    createswitchmii();

        sleep(5);

	    WPAD_Shutdown();
	    STM_RebootSystem();				
	}
		
	return 0;
}
