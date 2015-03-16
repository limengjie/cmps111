
#ifndef HASH_STORE_H
#define HASH_STORE_H



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//inlcude md5 functions header

#include "md5.h"

#define NO_FOUND '0'
#define FOUND '1'
#define INSERTED '2'
#define DEDUPLICATED '3'


#define DIGEST_LEN 16

#define HASH_TABLE_SIZE (50)

#define HASH_ENTRY_SIZE sizeof(struct hash_entry);

struct hash_table
{
    char name[100];
    int max_size;
    int cur_size;
    
    int hash_table_fp;
    int data_store_fp;
    
    unsigned base64_blk_size;
};

struct hash_entry
{
    char used;
    char md5[DIGEST_LEN];
    unsigned int data_pos;
};



void print_decode_base64_block(char *data_start, int data_len)
{
    unsigned char *de_data;
    size_t de_data_len;
    
    de_data = base64_decode(data_start, data_len, &de_data_len);
    
    int i;
    for (i=0; i<de_data_len; i++) {
        printf("%c", de_data[i]);
    }
    printf("\n");
    fflush(0);
    
    base64_cleanup();
    free(de_data);
    
}



struct hash_table* initialize(char *hash_table_file, unsigned int hash_table_size)
{
    
    int hash_table_fp = open(hash_table_file, O_RDWR | O_CREAT );
    int store_file_fp = open("store_log.txt", O_RDWR | O_CREAT  );
    
    fchmod(hash_table_fp,0666);
    fchmod(store_file_fp,0666);
    
    struct hash_table *ht = malloc(sizeof(struct hash_table));
    strcpy(ht->name, hash_table_file);
    ht->max_size = hash_table_size;
    
    ht->data_store_fp = store_file_fp;
    ht->hash_table_fp = hash_table_fp;
    
    
    //current number of hash entries
    int file_size;
    struct stat st;
    stat(hash_table_file, &st);
    file_size = st.st_size;
    
    
    if (file_size == 0) {
        unsigned int i;
        struct hash_entry t;
        memset(&t, 0, sizeof(t));
        for (i=0; i<hash_table_size; i++) {
            write(hash_table_fp, &t, sizeof(t));
        }
    }
    
    //set the start position of the file
    lseek(hash_table_fp, 0L,SEEK_SET);
    lseek(store_file_fp, 0L,SEEK_SET);
    
    return ht;
}



unsigned int fold_md5(char *md5)
{
    int i;
    unsigned int result = 0;
    unsigned int *digest = (int *)md5;
    
    for (i=0; i<DIGEST_LEN/4; i++) {
        result ^= *(digest + i);
    }
    return result % HASH_TABLE_SIZE;
}



struct hash_entry* lookup(struct hash_table *ht, char *md5)
{
    unsigned int hash_slot = fold_md5(md5);
    unsigned long offset = hash_slot * sizeof(struct hash_entry);
    
    struct hash_entry *temp = (struct hash_entry*)malloc(sizeof (struct hash_entry));
    while (1) {
        lseek(ht->hash_table_fp, offset, SEEK_SET);
        read(ht->hash_table_fp, temp, sizeof (struct hash_entry));
        
        if (temp->used) {
            if (memcmp(temp->md5, md5, DIGEST_LEN)==0) {
                return temp;
            }
            else
            {
                hash_slot ++;
                hash_slot %= ht->max_size;
                
                offset = hash_slot * sizeof(struct hash_entry);
            }
        }
        else
        {
            free(temp);
            return NULL;
        }
    }
}



int inquiry(struct hash_table *ht, char *md5)
{
    struct hash_entry* t = lookup(ht, md5);
    if (t) {
        return FOUND;
    }
    else
        return NO_FOUND;
}



char* generate_found_msg(char *block, unsigned int len)
{
    char *found_message = malloc(3+len+sizeof(len));
    int k=0;
    found_message[k++] = FOUND;
    found_message[k++] = ',';
    
    memcpy(&found_message[k++], block, len);
    k += len;
    found_message[k++] = ',';
    
    int tmp = htonl((uint32_t)len);
    memcpy(&found_message[k], &tmp, sizeof(unsigned int));
    
    
    printf("find and fetch data.\n");
    return found_message;
}
char* generate_no_found_msg()
{
    printf("not find data block in store.\n");
    return NULL;
}


char* fetch(struct hash_table *ht, char *md5)
{
    struct hash_entry* t;
    
    t = lookup(ht, md5);
    if (t) {
        char *base64_blk_data = malloc(ht->base64_blk_size+1);
        lseek(ht->data_store_fp, t->data_pos, SEEK_SET);
        read(ht->data_store_fp, base64_blk_data, ht->base64_blk_size);
        base64_blk_data[ht->base64_blk_size]='\0';
        
        
        char *found_msg;
        found_msg = generate_found_msg(base64_blk_data, ht->base64_blk_size);
        
        //printf("block size: %d\n--------------------\n", ht->base64_blk_size);
        //print_decode_base64_block(found_msg+2, ht->base64_blk_size);
        
        
        free(base64_blk_data);
        free(t);
        return found_msg;
    }
    else
    {
        generate_no_found_msg();
        return NULL;
    }
}


unsigned char insert(struct hash_table *ht, char *md5, char *base64_blk, int data_block_size)
{
    
    unsigned int hash_slot = fold_md5(md5);
    unsigned long offset = hash_slot * sizeof(struct hash_entry);
    
    struct hash_entry *temp = (struct hash_entry*)malloc(sizeof (struct hash_entry));
    while (1) {
        lseek(ht->hash_table_fp, offset, SEEK_SET);
        read(ht->hash_table_fp, temp, sizeof (struct hash_entry));
        
        if (temp->used) {
            if (memcmp(temp->md5, md5, DIGEST_LEN)==0) {
                free(temp);
                
                printf("find block.\n");
                
                return DEDUPLICATED;
            }
            else
            {
                hash_slot ++;
                hash_slot %= ht->max_size;
                
                offset = hash_slot * sizeof(struct hash_entry);
                
                printf("go to next entry...\n");
            }
        }
        else
        {
            struct hash_entry new_entry;
            new_entry.used = 1;
            memcpy(new_entry.md5, md5, DIGEST_LEN);
            
            int pos = lseek(ht->data_store_fp, 0, SEEK_END);
            new_entry.data_pos = pos;
            
            lseek(ht->hash_table_fp, offset, SEEK_SET);
            write(ht->hash_table_fp, &new_entry, sizeof(new_entry));
            
            write(ht->data_store_fp, base64_blk, ht->base64_blk_size);
            
            printf("not found, insert it.\n");
            free(temp);
            return INSERTED ;
        }
    }
}



void cleanup(struct hash_table* ht)
{
    close(ht->hash_table_fp);
    close(ht->data_store_fp);
    
    free(ht);
}



char *md5_string(char *data, int data_len)
{
    MD5_CTX mdContext;
    
    MD5Init (&mdContext);
    MD5Update (&mdContext, data, data_len);
    MD5Final (&mdContext);

    int i;
    for (i = 0; i < DIGEST_LEN; i++)
        printf ("%02x", mdContext.digest[i]);
    printf ("\n");

    char *md5 = malloc(DIGEST_LEN);
    memcpy(md5, mdContext.digest, DIGEST_LEN);
    return md5;
}

#endif


//
//int main()
//{
//    struct hash_table* table = initialize("hash_table.txt", HASH_TABLE_SIZE);
//    
//    char b64_blk1[BASE64_BLK_SIZE+1] = "abcd";
//    char b64_blk2[BASE64_BLK_SIZE+1] = "1234";
//    
//    char *md5_1 = md5_string(b64_blk1, strlen(b64_blk1));
//    char *md5_2 = md5_string(b64_blk2, strlen(b64_blk2));
//    
//    insert(table, md5_1, b64_blk1, 10);
//    insert(table, md5_2, b64_blk2, 10);
//    insert(table, md5_1, b64_blk1, 10);
//    
//    if (inquiry(table, md5_1)==FOUND)
//        printf("found md5.\n");
//    else
//        printf("not found md5.\n");
//    
//    char b64_blk3[BASE64_BLK_SIZE+1] = "5689";
//    char *md5_3 = md5_string(b64_blk3, strlen(b64_blk3));
//    
//    if (inquiry(table, md5_3)==FOUND)
//        printf("found md5.\n");
//    else
//        printf("not found md5.\n");
//    
//    
//    
//    fetch(table, md5_1);
//    fetch(table, md5_2);
//    fetch(table, md5_3);
//    
//    
//    cleanup(table);
//}



