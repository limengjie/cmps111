#include "sigleInstanceStore.c"


int main () {
	// initialize the hash table and append log
    char * hashTable = "testHashTable";
            // printf("hash table has %u entries\n", (unsigned)HT_ENTRIES);
    initialize(hashTable, (unsigned)HT_ENTRIES);
}