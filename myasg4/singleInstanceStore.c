/*
 * Single instance store
 * For Fixed size data blocks
 * DATA_SIZE is the fixed block size
 * KEY_SIZE is always 16 now
 * 'append_log' is used as the log
 * 'hashData' file is the hashTable
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

//inlcude md5 functions header
// #include "md5.h"

 // #include "func.h"

#define MAX_KEY_SIZE 16
#define MAX_DATA_SIZE 1000
#define FILLED 0xDEADD00D
#define BLK_LEN 80


//define a structure for storing your hashEntry
typedef struct {
	int occupied;
	unsigned char key[16];
	off_t data;
	} HashEntry;
	

//global value to store hashTable size
int hashTableSize = 0;

//gloabl value current size of hashTable
int currentSize = 0;

//global value to store max size limit of single entry
int hashEntrySize = 0;

//global value to store hashTable descriptor in
int fd = 0;

//global value to store appendLog descriptor in
int app = 0;

//internal function used by initialize to reserve space for hash table
void reserveFileSpace(int fileDes){
	int entry;
	//store blank entries of type HashEntry
	//Blank HashEntry
	HashEntry h;
	hashEntrySize = sizeof(h);
	memset(&h, 0, hashEntrySize);
	// h.occupied = 0x00000000;
	// // h.key = malloc(MAX_KEY_SIZE);
	// h.data = 0x00000000;
	// //update hashEntry size
	//total number of entries = hashTableSize
	for(entry = 0; entry < hashTableSize; entry++){
		//write values
		write(fileDes,&h,hashEntrySize);
	}
	
	//return file descrtor to beginning
	lseek(fileDes, 0, SEEK_SET);
}
		


//initialize function
//initialize append log also
int initialize(char *file, int size) {
	//try opening the file with read write permission and create it if not found
	// the flags specify the permissions
	// truncate file if already exists to empty it
	//also set proper read, write modes
	fd = open(file, O_RDWR | O_CREAT ); 
	//set modes for file
	fchmod(fd,0666);
	
	//apend log initialize
	app = open("append_log", O_RDWR | O_CREAT  );
	fchmod(app,0666);
	
	//update HashEntry
	HashEntry h;
	memset(&h, 0, hashEntrySize);
	//update hashEntry sizec
	hashEntrySize = sizeof(h);
	
	//initialize hashTableSize
	hashTableSize = size;
	//update current size by reading file
	currentSize = (lseek(app,0,SEEK_END)/hashEntrySize);
	lseek(app,0,SEEK_SET); 
	
	//initialize file contents to all empty reserved space
	if(currentSize == 0)
		reserveFileSpace(fd);
	
	
	//return file descriptor
	//fd is already set to -1 if an error occurs
	return fd;
}

// //internal MD5 digest function
// unsigned char* getDigest(char* key){
// 	//get length of key
// 	int l = strlen(key);	
	
// 	//calculate md5 digest
// 	MD5_CTX mdContext;
// 	MD5Init (&mdContext);
// 	MD5Update (&mdContext, key, l);
// 	MD5Final (&mdContext);

// 	unsigned char* result = malloc(16*sizeof(unsigned char));
// 	//copy digest
// 	int i;
// 	for(i=0;i<16;i++)
// 		result[i] = mdContext.digest[i];
	
// 	return result;
	
// }

// //internal hash function
// //MODIFIED to use MD5
// int hash(char* key){
	
// 	//get message digest
// 	unsigned char* digest = getDigest(key);

// 	//calculate key by xoring all values
// 	int hashValue = (int)digest[0];

// 	int i;
// 	for(i=1;i<16;i++)
// 		hashValue = hashValue^((int)digest[i]);
	
// 	return hashValue;
// }



//intenal file index function
//takes a hashValue and returns its lseek index for SEEK_SET
unsigned long fileIndex(unsigned hashValue){
	return (unsigned long)hashValue*hashEntrySize;
}

//internal function to see if hash slot is filled
//returns 1 if filled, 0 if not
//resets fd back to position before function call
int isFilled(){
	//Blank HashEntry
	HashEntry h;
	//read in value
	read(fd,&h,hashEntrySize);
	//reset fd seek position
	lseek(fd, -hashEntrySize, SEEK_CUR );
	
	//return if slot filled
	return (h.occupied == FILLED);
}

//insert a value, use linear probe to find space
long insert(unsigned slot, char *key, void *value, int length){
	//if filled return -1
	if(currentSize == hashTableSize)
		return -1;
	
	puts("from insert");
	//increase size
	currentSize++;
	
	// //get hashValue slot to check
	// int slot = hash(key);
	
	//form new HashEntry
	HashEntry h;
	memset(&h, 0, hashEntrySize);
	h.occupied = FILLED; 
	h.data = lseek(app, 0, SEEK_END);

	// append block to app log
	strncpy(h.key, key, MAX_KEY_SIZE);
	char* data = malloc(MAX_DATA_SIZE);
	memset(data,0,MAX_DATA_SIZE);
	// printf("val: %s\n", (char*)value);
	strncpy(data,(char *)value, strlen((char*)value));
	write(app,data,MAX_DATA_SIZE);
	
	//get index
	int index = fileIndex(slot);
	

	//search for empty slot to write
	lseek(fd,index, SEEK_SET);
	//linear probe if filled
	while(isFilled()){
		slot = (slot+1)%hashTableSize;
		index = fileIndex(slot);
		lseek(fd,index, SEEK_SET);
	}
	
	//after getting empty location, put in value
	write(fd, &h, hashEntrySize);
	

	//return slot location
	return (long)slot;
}

// check if the block is in the hash table or not
int inquiry(unsigned slot, char * key) {
		//if empty return -1
		if(currentSize == 0)
			return -1;
		
		// puts("from inquiry");

		//form new HashEntry
		HashEntry h;
		memset(&h, 0, hashEntrySize);

		//get index
		unsigned long index = fileIndex(slot);
		//search for filled slot to read
		lseek(fd,index, SEEK_SET);
			
		//flag if found
		int found = -1;

		//linear probe if filled
		while(isFilled()){
			read(fd,&h,hashEntrySize);

			
			//compare keys if correct data
			if( memcmp(h.key, key, MAX_KEY_SIZE) == 0){
				found = slot;
				break;
			}
			slot = (slot+1)%hashTableSize;

			index = fileIndex(slot);
			lseek(fd,index, SEEK_SET);
		}

		if (found != -1)
			printf("block exists\n");
		else
			puts("not found");

		return found;
}

//fetch a value from the hash table
int fetch (unsigned slot, char *key, void *value, int *length){
	int pos = inquiry(slot, key);

	if(pos != -1) {
		//read data from append log
		//get index
		unsigned long index = fileIndex(pos);
		//search for filled slot to read
		lseek(fd,index, SEEK_SET);


		HashEntry h;
		memset(&h, 0, hashEntrySize);
		read(fd,&h,hashEntrySize);
		lseek(app, h.data, SEEK_SET);

		read(app, value, BLK_LEN);
		
		// printf("slot: %d  block: %s \n", pos, (char *)value);
		
		//printf("slot to fetch: %d\n",slot);
		//return slot location
		return pos;
	}

		//if not found
		return -1;
}

// //probe function, uses fetch internally
// int probe(char *key){
// 	//create dummy value pointer
// 	void* value = malloc(MAX_DATA_SIZE);
// 	int sz = sizeof(value);

// 	//use fetch to probe
// 	int ret = fetch(key,value,&sz);
// 	free(value);
// 	return ret;
// }































//delete function
/* not used here */
/*uses probe internally
int delete(char *key){
	int slot = probe(key);
	
	//if key not found
	if(slot<0)
		return slot;

	

	//delete the key found
	//write in a dummy hashEntry
	HashEntry h;
	h.occupied = 0x00000000;
	h.key = malloc(MAX_KEY_SIZE);
	h.data = malloc(MAX_DATA_SIZE);

	//get delete index
	int index = fileIndex(slot);

	//seek to delte index 
	lseek(fd,index, SEEK_SET);

	//write blank entry
	write(fd,&h,hashEntrySize);

	//pull up entries following the location
	//unless value in proper location
	//or value is empty
	int prevSlot = slot;
	int curSlot = (slot+1)%hashTableSize;
	int prevIndex = fileIndex(prevSlot);
	int curIndex = fileIndex(curSlot);
	//lseek to current slot
	lseek(fd,curIndex, SEEK_SET);

	while(isFilled()){
		//read curIndex value
		read(fd,&h,hashEntrySize);
		
		//check if correct hashValue slot
		if(hash(h.key) == curSlot){
			
			//break
			break;
		}else{
			//copy up value
			//seek to prevIndex
			lseek(fd,prevIndex, SEEK_SET);
			//write to previous index
			write(fd,&h,sizeof(h));
			//lseek to current slot
			lseek(fd,curIndex, SEEK_SET);
			//write occupied slot to free
			h.occupied = 0x00000000;
			write(fd,&h,sizeof(h));
			
			//upldate slot locations
			prevSlot = curSlot;
			curSlot = (curSlot+1)%hashTableSize;
			//update indexes
			prevIndex = fileIndex(prevSlot);
			curIndex = fileIndex(curSlot);
			//lseek to current slot
			lseek(fd,curIndex, SEEK_SET);
		}
	}

	//decrease size
	currentSize--;

	//return successfull delete
	return 0;
}
* */


// //test harness
// int main(int argc,char *argv[]){
// 	//initialize a file called hashData
// 	// the function of the argument length is not defined in the question for function initialize
// 	// let size be 1000 values
// 	fd = initialize("hashData", 0 , 1000);
	
// 	//print file descriptor returned to check if proper value returned
// 	printf("Value of File Descriptor returned is %d\n",fd);

// 	//testing
// 	char *testKey;
// 	char *testData;

// 	//number of test cases
// 	int testCases = 700;

// 	//check return value
// 	int ret;

// 	//table empty, test delete for empty table
// 	printf("\n\nTable empty, test delete for empty table: ");
// 	if(delete("NON_EXISTENT KEY") != -1){
// 		printf("Deletion test failed for EMPTY hash table\n");
// 		return -1;
// 	}
// 	printf("SUCCESSFUL\n");

// 	//testing inserts for empty table
// 	printf("\n\nTesting inserts for empty table: \n\n");
// 	int i;
// 	for(i=0;i<testCases;i++){
// 		testData = malloc(MAX_DATA_SIZE);
// 		testKey = malloc(MAX_KEY_SIZE);

// 		//generate data to store
// 		sprintf(testKey,"KEY-%d",i);
// 		sprintf(testData,"DATA WITH KEY-%d",i);

// 		//check insert
// 		if((ret=insert(testKey,(void *)testData,strlen(testData))) < 0){
// 			printf("Insertion test failed for Data no %d\n",i);
// 			return -1;
// 		}

// 		printf("Insertion test SUCCESSFUL for Data no %d, slot location is %d\n",i,ret);


// 		//cleanup
// 		free(testData);
// 		free(testKey);
// 	}

// 	/*table filled, test insert for full table
// 	printf("\n\nTable filled, test insert for full table: ");
// 	if(insert("EMPTY",(void *)("SHOULD NOT INSERT"),strlen("SHOULD NOT INSERT")) != -1)
// 	{
// 		printf("Insertion test failed FULL hash table\n");
// 		return -1;
// 	}
// 	printf("SUCCESSFUL\n");*/
	

// 	//fetch half the keys
// 	printf("\n\nTesting fetch for half the keys: \n\n");
// 	void *data = malloc(MAX_DATA_SIZE);
// 	int sz = sizeof(data);
// 	for(i=0;i<(testCases/2);i++){
// 		testData = malloc(MAX_DATA_SIZE);
// 		testKey = malloc(MAX_KEY_SIZE);

// 		//generate data to test fetch
// 		sprintf(testKey,"KEY-%d",i);
// 		sprintf(testData,"DATA WITH KEY-%d",i);
// 		sz = strlen(testData);

// 		if(fetch(testKey,data,&sz) < 0){
// 			printf("Fetch test failed for Data no %d\n",i);
// 			return -1;
// 		}
		

// 		if( strcmp((char *)data, testData)!=0 ){
// 			printf("Data fetched for no %d is corrupted\n",i);
// 			return -1;
// 		}
// 		printf("Fetch test SUCCESSFUL for Data no %d\n",i);

// 		//cleanup
// 		free(testData);
// 		free(testKey);
// 		free(data);
// 		data = malloc(MAX_DATA_SIZE);
// 	}

// 	//delete half the keys
// 	printf("\n\nTesting delete for half the keys: \n\n");
// 	for(i=(testCases/2);i<testCases;i++){
		
// 		testKey = malloc(MAX_KEY_SIZE);

// 		//generate data to test fetch
// 		sprintf(testKey,"KEY-%d",i);
		

// 		if(delete(testKey) != 0){
// 			printf("Delete test failed for Data no %d\n",i);
// 			return -1;
// 		}
// 		printf("Delete test SUCCESSFUL for Data no %d\n",i);

// 		//cleanup
		
// 		free(testKey);
		
// 	}

// 	//try to fetch/delete non-existent keys
// 	printf("\n\nTesting fetch/delete for non-existent keys: \n\n");
// 	for(i=(testCases/2);i<testCases;i++){
		
// 		testKey = malloc(MAX_KEY_SIZE);

// 		//generate data to test fetch
// 		sprintf(testKey,"KEY-%d",i);

// 		if(fetch(testKey,data,&sz) != -1){
// 			printf("Fetch test failed for Non-existent Data no %d\n",i);
// 			return -1;
// 		}
		

// 		if(delete(testKey) != -1){
// 			printf("Delete test failed for Non-existent Data no %d\n",i);
// 			return -1;
// 		}
// 		printf("Fetch/Delete test SUCCESSFUL for Non-existent Data no %d\n",i);

// 		//cleanup
		
// 		free(testKey);
// 		free(data);
// 		data = malloc(MAX_DATA_SIZE);
// 	}

// 	printf("\n\nCONGRATS: ALL TESTS SUCCESSFUL\n");	
	
	
// 	//close fd
// 	close(fd);
	
// 	//end test harness
// 	return 0;
// }