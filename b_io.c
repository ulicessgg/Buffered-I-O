/**************************************************************
* Class::  CSC-415-01 Fall 2024
* Name:: Ulices Gonzalez
* Student ID:: 923328897
* GitHub-Name:: ulicessgg
* Project:: Assignment 5 – Buffered I/O read
*
* File:: <b_io.c>
*
* Description::
*
**************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLowSmall.h"

#define MAXFCBS 20	//The maximum number of files open at one time


// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb
	{
	fileInfo * fi;	//holds the low level systems file info

	// Add any other needed variables here to track the individual open file

		// to keep track of the files create a local buffer to read to, a count
		// for the space being used, and an integer to track the block position
		char* buffer;
		int bufferUsed;
		int bytePosition;
		int blockPosition;

	} b_fcb;
	
//static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;	

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init ()
	{
	if (startup)
		return;			//already initialized

	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].fi = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free File Control Block FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].fi == NULL)
			{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;		//Not thread safe but okay for this project
			}
		}

	return (-1);  //all in use
	}

// b_open is called by the "user application" to open a file.  This routine is 
// similar to the Linux open function.  	
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.  
// For this assignment the flags will be read only and can be ignored.

b_io_fd b_open (char * filename, int flags)
	{
	if (startup == 0) b_init();  //Initialize our system

	//*** TODO ***//  Write open function to return your file descriptor
	//				  You may want to allocate the buffer here as well
	//				  But make sure every file has its own buffer

	// This is where you are going to want to call GetFileInfo and b_getFCB

		// create file descriptor using return value of b_getFCB
		b_io_fd fd = b_getFCB();

		// if b_getFCB returns -1 print error and return
		if(fd == -1)
		{
			return -1;
		}
		
		// save the file info returned from GetFileInfo
		fcbArray[fd].fi = GetFileInfo(filename);

		// if GetFileInfo returns null print error and return
		if(fcbArray[fd].fi == NULL)
		{
			return -1;
		}

		// allocate buffer for respective file being opened
		fcbArray[fd].buffer = malloc(B_CHUNK_SIZE);
		if(fcbArray[fd].buffer == NULL)
		{
			return -1;
		}

		fcbArray[fd].bufferUsed = 0;
		fcbArray[fd].bytePosition = 0;
		fcbArray[fd].blockPosition = 0;

		return fd;
	}



// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to 
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File	
int b_read (b_io_fd fd, char * buffer, int count)
	{
	//*** TODO ***//  
	// Write buffered read function to return the data and # bytes read
	// You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.
		
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	// and check that the specified FCB is actually in use	
	if (fcbArray[fd].fi == NULL)		//File not open for this descriptor
		{
		return -1;
		}	

	// Your Read code here - the only function you call to get data is LBAread.
	// Track which byte in the buffer you are at, and which block in the file

		// if no bytes are requested or a negative amount is, return
		if(count <= 0)
		{
			return 0;
		}

		// create count of bytes copied to be returned upon termination
		int bytesCopied = 0;
		int bytesToCopy = 0;
		int blocksToCopy = 0;
		int blocksCopied = 0;

		// if the buffer still contains contents copy whats left to the user
		if(fcbArray[fd].bufferUsed > 0)
		{
			
		}
		// if the count exceeds a block then copy whole block to user
		if(count >= B_CHUNK_SIZE)
		{

		}
		// if the buffer is currently empty or new file is being copied
		if(count > 0)
		{
			// calculate the size of the next batch of incoming bytes
			bytesToCopy = count - bytesCopied;

			// since count is less than 512 only 1 block is needed
			blocksToCopy = 1;

			// read to current fcbarray buffer and return total blocks read
			blocksCopied = LBAread(fcbArray[fd].buffer, blocksToCopy, fcbArray[fd].fi->location);
			// copy from current fcbarray to users buffer the remaining bytes
			memcpy(buffer, fcbArray[fd].buffer, bytesToCopy);

			// update bytesCopied before returning
			bytesCopied += bytesToCopy;
			// update the file position in bytes
			fcbArray[fd].bytePosition = bytesCopied;
			// clear the buffer count before next use
			fcbArray[fd].bufferUsed = 0;
		}

		return bytesCopied;
	}

// b_close frees and allocated memory and places the file control block back 
// into the unused pool of file control blocks.
int b_close (b_io_fd fd)
	{
	//*** TODO ***//  Release any resources

		// deallocate buffer for respective file
		free(fcbArray[fd].buffer);

		// reset fcbArray values
		fcbArray[fd].fi = NULL;
		fcbArray[fd].buffer = NULL;
		fcbArray[fd].bufferUsed = 0;
		fcbArray[fd].bytePosition = 0;
		fcbArray[fd].blockPosition = 0;

		return 0;
	}