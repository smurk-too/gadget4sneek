/*

fixelfmod - A tool to inject code into DML

FixELF: Copyright (C) 2011  crediar
Modified by conanac (2012)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <stdio.h>
#include <string.h>
#include <vector>
#include <time.h>
#include "ELF.h"

#define DMLLOADER 0x4E0

unsigned int s32( unsigned int i )
{
	return (i>>24) | (i<<24) | ((i>>8)&0xFF00) | ((i<<8)&0xFF0000);
}
unsigned short s16( unsigned short s )
{
	return (s>>8) | (s<<8);
}

char ELFMagic[] = {
 0x7F, 'E', 'L', 'F',
};

int main( int argc, char *argv[] )
{
	int i;

	printf("fixelfmod v 0.1b\n");
	printf("Built: %s %s\n", __TIME__, __DATE__ );

	printf("It is not allowed to resell, rehost, redistribute\nor include this file in any packages!\n\n");

	if( argc != 4 )
	{
		printf("%s dml.app diosmioslite.elf out.app", argv[0] );
		return -1;
	}
	
	FILE *in = fopen( argv[2], "rb" );
	if( in == NULL )
	{
		printf("Could not open \"%s\"\n", argv[2] );
		perror("");
		return -1;
	}

	FILE *out = fopen( argv[3], "wb" );
	if( out == NULL )
	{
		printf("Could not open \"%s\"\n", argv[3] );
		perror("");
		return -1;
	}
	
	FILE *dml = fopen( argv[1], "rb" );
	if( dml == NULL )
	{
		printf("Could not open \"%s\"\n", argv[1] );
		perror("");
		return -1;
	}

	//read whole input file

	fseek( in, 0, SEEK_END );
	unsigned int size = ftell(in);
	fseek( in, 0, 0 );

	char *buf = new char[size];
	fread( buf, 1, size, in );
	fclose( in );


	if( memcmp( buf, ELFMagic, sizeof(ELFMagic) ) != 0 )
	{
		printf("Invalid magic bytes!\n");
		exit(1);
	}

	//Prepare new ELF header, output will have SEVEN program headers

#define PHCOUNT	7

	Elf32_Ehdr OEHdr;

	memset( &OEHdr, 0, sizeof(Elf32_Ehdr) );
	
	OEHdr.e_ident[EI_MAG0]	= 0x7F;
	OEHdr.e_ident[EI_MAG1]	= 'E';
	OEHdr.e_ident[EI_MAG2]	= 'L';
	OEHdr.e_ident[EI_MAG3]	= 'F';
	
	OEHdr.e_ident[EI_CLASS]	 = 0x01;
	OEHdr.e_ident[EI_DATA]	 = 0x02;
	OEHdr.e_ident[EI_VERSION]= 0x01;
	OEHdr.e_ident[EI_PAD]	 = 0x61;
	OEHdr.e_ident[8]		 = 0x01;

	OEHdr.e_type		= s16(0x0002);
	OEHdr.e_machine		= s16(0x0028);
	OEHdr.e_version		= s32(0x0001);
	OEHdr.e_entry		= s32(0xFFFF0000);
	
	OEHdr.e_flags		= s32(0x00000606);
	OEHdr.e_ehsize		= s16(sizeof(Elf32_Ehdr));
	
	OEHdr.e_phentsize	= s16(sizeof(Elf32_Phdr));
	OEHdr.e_phoff		= s32(sizeof(Elf32_Ehdr));
	OEHdr.e_phnum		= s16(PHCOUNT);

	Elf32_Phdr OPHdr[PHCOUNT];
	memset( &OPHdr, 0, sizeof(Elf32_Phdr) * PHCOUNT );

	Elf32_Ehdr *IEHdr = (Elf32_Ehdr *)buf;
	
	printf("Program Headers:%d\n", s16( IEHdr->e_phnum ) );
	printf("Program Offset :%04X\n", s32( IEHdr->e_phoff ) );
	printf("Section Headers:%d\n", s16( IEHdr->e_shnum ) );
	printf("Section Offset :%04X\n", s32( IEHdr->e_shoff ) );
	
	Elf32_Phdr *IPHdr = (Elf32_Phdr *)( buf + s32( IEHdr->e_phoff ) );
	Elf32_Shdr *ISHdr = (Elf32_Shdr *)( buf + s32( IEHdr->e_shoff ) );
	
	fwrite( &OEHdr, sizeof(Elf32_Ehdr), 1, out );
	fwrite( &OPHdr, sizeof(Elf32_Phdr), PHCOUNT, out );

	for( i=0; i < s16( IEHdr->e_phnum ); ++i )
	{
		memcpy( &OPHdr[ 2 + i ], &IPHdr[i], sizeof(Elf32_Phdr) );
		printf("[%02d]PA:%08X VA:%08X MS:%06X FO:%04X FS:%04X FL:%06X\n", i, s32( IPHdr[i].p_paddr ),  s32( IPHdr[i].p_vaddr ), s32( IPHdr[i].p_memsz ), s32( IPHdr[i].p_offset ),  s32( IPHdr[i].p_filesz ),  s32( IPHdr[i].p_flags ) );

		OPHdr[ 2 + i ].p_offset = s32(ftell(out));
		fwrite( buf + s32( IPHdr[i].p_offset ), 1, s32( IPHdr[i].p_filesz ), out );
	}
	
	fseek( dml, DMLLOADER, SEEK_SET );

	Elf32_Ehdr DMLHeader;
	fread( &DMLHeader, sizeof( Elf32_Ehdr ), 1, dml );

	Elf32_Phdr *DMLPh = new Elf32_Phdr[ s16( DMLHeader.e_phnum ) ];
	std::vector< char*> DMLPData;
	DMLPData.resize( s16( DMLHeader.e_phnum ) );

	fseek( dml, s32( DMLHeader.e_phoff ) + DMLLOADER, SEEK_SET );
	fread( DMLPh, sizeof( Elf32_Phdr ), s16( DMLHeader.e_phnum ), dml );
	
	int EntryCount = 4;

	for( i=0; i < s16( DMLHeader.e_phnum ); ++i )
	{
		DMLPData[i] = new char[  s32( DMLPh[i].p_filesz ) ];
		fseek( dml, s32( DMLPh[i].p_offset) + DMLLOADER, SEEK_SET );
		fread( DMLPData[i], sizeof( char ), s32( DMLPh[i].p_filesz ), dml );
		if( i >= 4 )
		{
			printf("[%02d]PA:%08X VA:%08X MS:%06X FO:%04X FS:%04X FL:%06X\n", i, s32( DMLPh[i].p_paddr ), s32( DMLPh[i].p_vaddr ), s32( DMLPh[i].p_memsz ), s32( DMLPh[i].p_offset ), s32( DMLPh[i].p_filesz ), s32( DMLPh[i].p_flags ) );
			memcpy( (void*)(&OPHdr[ EntryCount ]), (void*)(&DMLPh[i]), sizeof(Elf32_Phdr) );
			OPHdr[ EntryCount++ ].p_offset = s32(ftell(out));
			fwrite( DMLPData[i], sizeof( char ), s32( DMLPh[i].p_filesz ), out );
		}
	}
	
	OPHdr[0].p_type		= s32(0x00000006);
	OPHdr[0].p_offset	= OEHdr.e_phoff;
	OPHdr[0].p_vaddr	= s32(0x138F0000);
	OPHdr[0].p_paddr	= s32(0x138F0000);
	OPHdr[0].p_filesz	= s32(sizeof(Elf32_Phdr) * PHCOUNT);
	OPHdr[0].p_memsz	= s32(sizeof(Elf32_Phdr) * PHCOUNT);
	OPHdr[0].p_flags	= s32(0x00F00000);
	OPHdr[0].p_align	= s32(0x00000004);
	
	OPHdr[1].p_type		= s32(0x00000001);
	OPHdr[1].p_offset	= OEHdr.e_phoff;
	OPHdr[1].p_vaddr	= s32(0x138F0000);
	OPHdr[1].p_paddr	= s32(0x138F0000);
	OPHdr[1].p_filesz	= s32(sizeof(Elf32_Phdr) * PHCOUNT);
	OPHdr[1].p_memsz	= s32(0x0C0000);
	OPHdr[1].p_flags	= s32(0x00F00000);
	OPHdr[1].p_align	= s32(0x00000004);

	fseek( out, s32(OEHdr.e_phoff), 0 );
	fwrite( &OPHdr, sizeof(Elf32_Phdr), PHCOUNT, out );

	fclose(out);

	delete buf;

// add elfloader

//read our output

	out = fopen( argv[3], "rb+" );
	if( out == NULL )
	{
		printf("Could not open \"%s\"\n", argv[3] );
		perror("");
		return -1;
	}

	fseek( out, 0, SEEK_END );
	size = ftell(out);
	fseek( out, 0, 0 );

	buf = new char[size];
	fread( buf, 1, size, out );

//read the elfloader from DML

	fseek( dml, 0, 0 );

	char *eloader = new char[DMLLOADER];
	fread( eloader, 1, DMLLOADER, dml );

	fseek( out, 0, 0 );
	fwrite( eloader, 1, DMLLOADER, out );
	fwrite( buf, 1, size, out );

	//write new DML size
	fseek( out, 8, 0 );
	size = s32(size);
	fwrite( &size, sizeof(int), 1, out );

	fclose( out );

	free( buf );
	return 0;

}