#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "main.h"
#include "dirent.h"

u8 * fstbuf = NULL;
u32 currentOffset = 0x10000000;// just arbitrarily set the first offset.  it just needs to be high enough that it will
          //be higher than ( (fst_offset <<2) + fst_size ) described in the boot.bin and low enough that the last offset
          // plus size will be less than u64
u32 nameOff = 0;
u32 totalCnt = 0;

int main( int argc, char *argv[] )
{
    char *path;
    int sizepath;
    FILE *cbubble;
    FILE *wf;
    u8 *boot_bin;
    u32 size;

    printf("giantpune's fst.bin creatorizer 0.02 (Windows adoption by conanac)\n");
    if( argc < 3 )
    {
        printf("usage:\n %s <directory> <output file> <boot.bin>\n", argv[ 0 ] );
        printf("boot.bin is optional.  if you give one, this will update the size of the fst.bin in that file");
        exit( 0 );
    }

	sizepath = strlen(argv[1]) + 2;
	path = (char*)malloc(sizepath*sizeof(char));
    memset( path, '\0', sizepath );

    //make sure to put / at the end of the path and get the initial count
    sprintf( path, "%s", argv[ 1 ] );
    if( path[ strlen( path ) - 1 ] != '/' )
        path[ strlen( path ) ] = '/';

    //get the initial count
    totalCnt = PathCount( path ) + 1;
    nameOff  = totalCnt * 0x0c;
    //printf("totalCnt: %d\nnameOff: %08x\n",totalCnt, nameOff);

    //create the buffer to hold this thing
    //give enough room for each naem to be 0x100 bytes
    fstbuf = malloc( nameOff + ( totalCnt * 0x100 ) );
    if( !fstbuf )
    {
        printf("can't malloc memory for the fstbuffer\n");
        exit( 2 );
    }

    //make sure it is empty
    memset( fstbuf, 0, nameOff + ( totalCnt * 0x100 ) );

    //add the root entry
    SetEntry( 0, 1, be32( 0 ), be32( 0 ), be32( totalCnt ) );

    //start adding entries.  this function recurses subfolders and all that 
    AddPath( path, 0 );
    //hexdump(fstbuf, nameOff+1);

    //round up to the nearest 4
    if( nameOff % 4)
        nameOff += 4 - ( nameOff % 4);

    //make sure the offset will show the correct number when divided but 4 and then multiplied by 4
    if( nameOff != ( nameOff >> 2) << 2 )
    {
        printf("ERROR! %x != %x\n", nameOff, ( nameOff >> 2 ) << 2 );
        exit( 7 );
    }

    //write the buffer to the output file.
    cbubble = fopen( argv[ 2 ], "wb" );
    if( !cbubble )
    {
        printf( "Can't open %s to write\n", argv[ 2 ] );
        free( fstbuf );
        exit( 69 );
    }
    fwrite( fstbuf, nameOff, 1, cbubble );
    fclose( cbubble );
    printf("wrote \"%s\" (%d bytes, %d entries) OK\n", argv[ 2 ], nameOff , totalCnt );

    //free that booger and return
    if( fstbuf )free( fstbuf );

    //update the boot.bin
    if( argc > 3 )
    {
        boot_bin = NULL;
        size = ReadFileToBuffer( argv[ 3 ], &boot_bin );
        if( size != 0x440 )
		{
            printf("unexpected boot.bin size, not gonna touch it\n");
            if( boot_bin )free( boot_bin );
            exit( 3 );
		}

		//conanac: for gamecube game, change nameOff >> 2 to just nameOff
            boot_bin[ 0x428 ] = ((nameOff >> 2) & 0xFF000000) >> 24;
		boot_bin[ 0x429 ] = ((nameOff >> 2) & 0x00FF0000) >> 16;
		boot_bin[ 0x42A ] = ((nameOff >> 2) & 0x0000FF00) >> 8;
		boot_bin[ 0x42B ] = ((nameOff >> 2) & 0x000000FF);

		boot_bin[ 0x42C ] = ((nameOff >> 2) & 0xFF000000) >> 24;
		boot_bin[ 0x42D ] = ((nameOff >> 2) & 0x00FF0000) >> 16;
		boot_bin[ 0x42E ] = ((nameOff >> 2) & 0x0000FF00) >> 8;
		boot_bin[ 0x42F ] = ((nameOff >> 2) & 0x000000FF);

        wf = fopen( argv[ 3 ], "wb" );
        if( !wf )
		{
            printf( "Can't open %s to write\n", argv[ 3 ] );
            free( boot_bin );
            exit( 69 );
		}

        fwrite( boot_bin, size, 1, wf );
        fclose( wf );
        printf("wrote \"%s\" (%d bytes) OK\n", argv[ 3 ], size );

        if( boot_bin )free( boot_bin );
	}

    return 0;
}

//adds a name to the end of the string table, adds a terminating 0, and increases
//the offset so next time this runs, it will put the next name at the new offset
//after the last name is added, it still increases the offset and then we use that
//to know the size of the entire file
void AddName( const char name[] )
{
    unsigned int i;
    unsigned int len = strlen( name );

    for ( i = 0; i < len ; i++ )
          fstbuf[ i + nameOff ] = name[ i ];

    nameOff += ( len + 1 );
    fstbuf[ nameOff ] = 0;
}

//set values for a given fst entry [0x0c bytes]
//  u8 type  u24 nameoffset      u32 offset      u32 size
//
// type 1 is folder, 0 is file
// nameoffset is the offset of this name from the start if the string table
// offset for files is the parent folder, files is the offset on the dvd /4.  since there is no dvd, we can just make one up
// size for folders is the index of the last file in this folder +1, for files this is the real filesize
void SetEntry( int num, int type, int nameoffset, int offset, int size )
{
    u32 * buf = (u32*)fstbuf;

    buf[ ( num * 3 ) ] = (u32)( nameoffset );
    fstbuf[ num * 0x0c ] = type;
    buf[ ( num * 3 ) + 1 ] = (u32)( offset );
    buf[ ( num * 3 ) + 2 ] = (u32)( size );
}

//count files and folders in a given path
int PathCount( const char dir_path[] )
{
    char *tmp;
	DIR *dir;
	struct dirent *dp;
	int ret;

    ret = 0;
    dir = opendir( dir_path );

    //skip . and .. to avoid endless loop that eventually crashes
    //skip all these files because they dont need to be in this fst.  it doesnt hurt to have them, but there's still no reason to add them
	while ( ( dp = readdir( dir ) ) != NULL ) {
      if( !strcmp( dp->d_name, "." ) || !strcmp( dp->d_name, ".." ) || !strcmp( dp->d_name, "boot.bin" )
    || !strcmp( dp->d_name, "bi2.bin" ) || !strcmp( dp->d_name, "main.dol" ) || !strcmp( dp->d_name, "apploader.img" )
    || !strcmp( dp->d_name, "ticket.bin" ) || !strcmp( dp->d_name, "tmd.bin" ) || !strcmp( dp->d_name, "cert.bin" )
    || !strcmp( dp->d_name, "fst.bin" ) )continue;

    tmp = path_cat( dir_path, dp->d_name );

	if( dp->data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )//this is a file, add 1
    {
        ret++;
    }
    else if( dp->data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )//this is a folder, add 1 for the folder and then add all those in the folder
    {
        tmp = path_cat( tmp, "/" );
        ret++;
        ret += PathCount( tmp );
    }
    else
    {
        printf("FATAL ERROR: %s has type %d\n", dp->d_name, dp->data.dwFileAttributes );
        exit( 1 );
    }
    //free the char*
    free( tmp );
    tmp = NULL;
  }
  //close the directory and return the amount of stuff in it
  closedir( dir );
  return ret;
}

//add files and folders from a given path to the fst
int AddPath( const char dir_path[], int thisItem )
{
	char *tmp;
	u64 size;
    int ret;
    int count;
    struct dirent *dp;
	DIR *dir;

	ret = 0;
	count = thisItem;
    dir = opendir( dir_path );

    //skip all this same thing again
    while ( ( dp = readdir( dir ) ) != NULL ) {
      if( !strcmp( dp->d_name, "." ) || !strcmp( dp->d_name, ".." ) || !strcmp( dp->d_name, "boot.bin" )
    || !strcmp( dp->d_name, "bi2.bin" ) || !strcmp( dp->d_name, "main.dol" ) || !strcmp( dp->d_name, "apploader.img" )
    || !strcmp( dp->d_name, "ticket.bin" ) || !strcmp( dp->d_name,"tmd.bin" ) || !strcmp( dp->d_name, "cert.bin" )
    || !strcmp( dp->d_name, "fst.bin" ) )continue;

    count ++;
    tmp = path_cat( dir_path, dp->d_name );

    if( dp->data.dwFileAttributes !=  FILE_ATTRIBUTE_DIRECTORY)//add a file to the fst, add its name to the string table, increase the offset for the next file
    {
        size = getFileSize( tmp );
        SetEntry( count, 0, be32( ( nameOff - (totalCnt * 0x0c) ) ), be32( currentOffset ), be32( size ) );
        //printf("setentry - [%d] [%d] nameoff: %08x offset: %08x size: %08x\n\n", count, 0, ( nameOff - (totalCnt* 0x0c) ), currentOffset, size);
        AddName( dp->d_name );

        //the offset in the fst are 1/4 of the real offset
        currentOffset += size >> 2;

        //bump it up to a nice round number.
        currentOffset = ( currentOffset + 31 ) & ( ~31 );
    }
    else if( dp->data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )//add subfolders
    {
        tmp = path_cat( tmp, "/" );
        size = PathCount( tmp );
        SetEntry( count, 1, be32( ( nameOff - (totalCnt* 0x0c) ) ), be32( thisItem ) , be32( count + size + 1 ) );
        //printf("setentry - [%d] [%d] nameoff: %08x offset: %08x size: %08x (count: %04x size: %04x)\n\n", count, 1, ( nameOff - (totalCnt* 0x0c) ), thisItem, count + size +1, count, size);
        AddName( dp->d_name );

        //add the subfolders and all those
        AddPath( tmp, count );

        //increase count by whatever was in that subfolder we just added
        count += size;
    }
    else
    {   //it wasn't a file or a folder.  what was it?
        printf("FATAL ERROR: %s has type %d\n", dp->d_name, dp->data.dwFileAttributes );
        exit( 1 );
    }

    //free the path
    free( tmp );
    tmp = NULL;
  }
  //close the directory and return.  theres no real reason to return anything, but why not?
  closedir( dir );
  return ret;

}

//make a path out of 2 parts
char *path_cat ( const char str1[], const char str2[] ) 
{
  size_t str1_len;
  size_t str2_len;
  unsigned int i,j;
  char *result;

  str1_len = strlen( str1 );
  str2_len = strlen( str2 );
  result = (char*)malloc( ( str1_len+str2_len + 1 )*sizeof(char) );

  strcpy ( result, str1 );

  for( i = str1_len, j = 0; ( ( i < ( str1_len + str2_len ) ) && ( j<str2_len ) ) ; i++, j++ ) {
    result[ i ] = str2[ j ];
  }

  result[ str1_len + str2_len ] = '\0';

  return result;
}


u32 be32( unsigned int i )
{
  return ( i << 24 ) | ( i >> 24 ) | ( ( i << 8 ) &0xFF0000 ) | ( ( i>> 8 ) &0xFF00 );
}

u64 getFileSize( const char filepath[] )
{
	FILE *fp;
	u64 size;

	fp = fopen(filepath, "rb");
	if (fp==NULL)
	{
		printf ("Couldn't open file %s\n", filepath);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fclose(fp);
	return size;
}

char ascii( char s ) {
    if ( s < 0x20 ) return '.';
    if ( s > 0x7E ) return '.';
    return s;
}

//hexhdump (12 bytes wide instead of 16 since this is based in 12 byte sections)
void hexdump( void *d, int len ) {
    u8 *data;
    int i, off;
    data = (u8*)d;
    printf("\n");
    for ( off = 0; off < len; off += 12 ) {
  printf( "%08x  ", off );
  for ( i=0; i<12; i++ )
      if ( ( i + off ) >= len ) printf("   ");
      else printf("%02x ",data[ off + i ]);

  printf(" ");
  for ( i = 0; i < 12; i++ )
      if ( ( i + off) >= len ) printf(" ");
      else printf("%c", ascii( data[ off + i ]));
  printf("\n");
    }
}

u32 ReadFileToBuffer(const char in[], u8 **outbuf)
{
	FILE *file;
    u8 *buffer = NULL;
    u32 filesize = 0;

    file = fopen(in, "rb");
    if (!file) return 0;

    fseek (file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);

    buffer = (u8*)malloc(filesize);
    //buffer = (u8*)memalign(32, filesize);

    if (fread (buffer, 1, filesize, file) != filesize) 
	{
       fclose (file);
       free(buffer);
       return 0;
    }
    fclose (file);
    //hexdump(buffer, filesize);
    *outbuf = buffer;
    return filesize;
}
