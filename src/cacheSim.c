#include <stdint.h>
#include "cacheSim.h"
#include <string.h>
#include <stdbool.h>

// In this question, we will assume DRAM will take a 4-byte values starting from 0 to
void init_DRAM()
{
	unsigned int i = 0;
	DRAM = malloc( sizeof( char ) * DRAM_SIZE );

	for( i = 0; i < DRAM_SIZE / 4; i++ )
	{
		*( ( unsigned int * )DRAM + i ) = i;
	}
}

void printCache()
{
	int i, j, k;
	printf( "===== L1 Cache Content =====\n" );

	for( i = 0; i < 2; i++ )
	{
		printf( "Set %d :", i );

		for( j = 0; j < 2; j++ )
		{
			printf( " {(TAG: 0x%x)", ( unsigned int )( L1_cache[i][j].tag ) );

			for( k = 0; k < 16; k++ )
			{
				printf( " 0x%x,", ( unsigned int )( L1_cache[i][j].data[k] ) );
			}

			printf( " |" );
		}

		printf( "\n" );
	}

	printf( "===== L2 Cache Content =====\n" );

	for( i = 0; i < 4; i++ )
	{
		printf( "Set %d :", i );

		for( j = 0; j < 4; j++ )
		{
			printf( " {(TAG: 0x%x)", ( unsigned int )( L2_cache[i][j].tag ) );

			for( k = 0; k < 16; k++ )
			{
				printf( " 0x%x,", ( unsigned int )( L2_cache[i][j].data[k] ) );
			}

			printf( " |" );
		}

		printf( "\n" );
	}
}

uint32_t read_fifo( uint32_t address )
{
	///// IMPLEMENT THIS /////
	return 0;
}

int L1lookup( uint32_t address )
{
	///// IMPLEMENT THIS /////
	int i = 0;

	do
	{
		if( ( getL1Tag( address ) == L1_cache[getL1SetID( address )][i].tag ) )
		{
			return 1;
		}

		i++;
	}
	while( i < 2 );

	return 0;
}

int L2lookup( uint32_t address )
{
	///// IMPLEMENT THIS /////
	int i = 0;

	do
	{
		if( ( getL2Tag( address ) == L2_cache[getL2SetID( address )][i].tag ) )
		{
			return 1;
		}

		i++;
	}
	while( i < 4 );

	return 0;
}

unsigned int getL1SetID( uint32_t address )
{
	///// IMPLEMENT THIS /////
	unsigned int c1 = ( address << 4 ) & 1;
	return c1;
}

unsigned int getL2SetID( uint32_t address )
{
	///// IMPLEMENT THIS /////
	unsigned int c2 = ( address << 4 ) & 3;
	return c2;
}

unsigned int getL1Tag( uint32_t address )
{
	///// IMPLEMENT THIS /////
	unsigned int b1 = ( address >> 5 );
	return b1;
}

unsigned int getL2Tag( uint32_t address )
{
	///// IMPLEMENT THIS /////
	unsigned int b2 = ( address >> 6 );
	return b2;
}

unsigned int helper_L1( uint32_t address, int a, char * fff )
{
	memcpy( L1_cache[getL1SetID( address )][a].data, fff, 16 );
	L1_cache[getL1SetID( address )][a].tag = getL1Tag( address );
	L1_cache[getL1SetID( address )][a].timeStamp = cycles;
}

unsigned int helper_L2( uint32_t address, int b, char * fff )
{
	memcpy( L2_cache[getL2SetID( address )][b].data, fff, 16 );
	L2_cache[getL2SetID( address )][b].tag = getL2Tag( address );
	L2_cache[getL2SetID( address )][b].timeStamp = cycles;
}

unsigned int helper_L1look( uint32_t address, int a, int j )
{
	memcpy( L1_cache[getL1SetID( address )][a].data,
			L2_cache[getL2SetID( address )][j].data, 16 );
	L1_cache[getL1SetID( address )][a].tag = getL1Tag( address );
	L1_cache[getL1SetID( address )][a].timeStamp = cycles;
}

void write( uint32_t address, uint32_t data )
{
	///// REPLACE THIS /////
	int byt = 15;
	int a = 0;
	int i = 0;

	do
	{
		if( L1_cache[getL1SetID( address )][a].timeStamp > L1_cache[getL1SetID(
					address )][i].timeStamp )
		{
			a = i;
		}

		if( ( getL1Tag( address ) ^ L1_cache[getL1SetID( address )][i].tag ) != 1 )
		{
			L1_cache[getL1SetID( address )][i].data[address & byt] = data;
		}

		i++;
	}
	while( i < 2 );

	int b = 0;
	int j = 0;

	do
	{
		if( L2_cache[getL2SetID( address )][j].timeStamp < L2_cache[getL2SetID(
					address )][b].timeStamp )
		{
			b = j;
		}

		if( ( getL2Tag( address ) ^ L2_cache[getL2SetID( address )][j].tag ) != 1 )
		{
			L2_cache[getL2SetID( address )][j].data[address & byt] = data;

			if( ( L1lookup( address ) ) == false )
			{
				helper_L1look( address, a, j );
			}
		}

		i++;
	}
	while( j < 4 );

	int byte = 16;
	unsigned char * fff[byte];
	int gf = address % byte;
	int r = 0;

	while( r < byte )
	{
		*fff[r] = DRAM[gf++];
		r++;
	}

	*fff[address & 15] = data;

	if( L1lookup( address ) != true )
	{
		helper_L1( address, a, *fff );
	}

	if( L2lookup( address ) != true )
	{
		helper_L2( address, b, *fff );
	}

	return;
}

int main()
{
	init_DRAM();
	cacheAccess buffer;
	int timeTaken = 0;
	FILE * trace = fopen( "input.trace", "r" );
	int L1hit = 0;
	int L2hit = 0;
	cycles = 0;

	while( !feof( trace ) )
	{
		fscanf( trace, "%d %x %x", &buffer.readWrite, &buffer.address, &buffer.data );
		printf( "Processing the request for [R/W] = %d, Address = %x, data = %x\n",
				buffer.readWrite, buffer.address, buffer.data );

		// Checking whether the current access is a hit or miss so that we can advance time correctly
		if( L1lookup( buffer.address ) ) // Cache hit
		{
			timeTaken = 1;
			L1hit++;
		}
		else if( L2lookup( buffer.address ) ) // L2 Cache Hit
		{
			L2hit++;
			timeTaken = 5;
		}
		else
		{
			timeTaken = 50;
		}

		if( buffer.readWrite )
		{
			write( buffer.address, buffer.data );
		}
		else
		{
			read_fifo( buffer.address );
		}

		cycles += timeTaken;
	}

	printCache();
	printf( "Total cycles used = %ld\nL1 hits = %d, L2 hits = %d", cycles, L1hit,
			L2hit );
	fclose( trace );
	free( DRAM );
	return 0;
}
