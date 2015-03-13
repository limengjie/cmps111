#include <stdio.h>
#include <stdlib.h>

#include "md5.h"
#include "base64.h"

#define BLOCKS 100
#define LEN 80

extern unsigned char * MDString (char *);

int main () {
   /*open a file*/
   FILE * fp;
   char * filename = "cheatsheet.txt";
   fp = fopen(filename, "r");
   if (fp == NULL) {
	   fprintf(stderr, "can't open the file!\n");
	   exit(1);
   }

   /*read and divide a file into serveral blocks*/
   int c, i, j = 0;
   int blks;
   i = 0;
   char blocks[BLOCKS][LEN];
   while ((c = fgetc(fp)) != EOF && j < BLOCKS) {
      blocks[j][i] = c;
      i++;
      if (i == LEN-1) {
	      blocks[j][i] = '\0';
	      j++;
	      i = 0;
      }
   }
   blks = j;
   // for (i = 0; i < blks; ++i) {
   //    printf("%s\n", blocks[i]);
   //    printf("i = %d\n", i);	   
   // }

   /*encode base64 */
   char * input = "hello world", * output, * origin;
   size_t size;
   output = base64_encode(input, strlen(input), &size);
   printf("base64 encode: %s\n", output);
   printf("size = %d\n", size);

   /*size_t de_size;*/
   /*origin = base64_decode(output, size, &de_size);*/
   /*printf("base64 decode: %s \n", origin);*/


   /*xor fold message digest */
   char * str = "Stat rosa pristina nomine, nomina nuda tenemus abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz";
   unsigned char * msg_digest = MDString(str);
   printf("msg_digest's len = %d\n", strlen(msg_digest));
   printf("message digest: %x\n", msg_digest);
   /*for (i = 0; i < 16; ++i) {*/
      /*printf("%x ", msg_digest[i]);*/
   /*}*/
   /*printf("\n");*/
   /*unsigned char xor_res[5] = {};  */
   /*for (i = 0; i < 16/4; ++i)*/
	   /*for (j = 0; j < 4; ++j)*/
		   /*xor_res[j] = xor_res[j] ^ msg_digest[4*i + j];*/
   /*xor_res[4] = 0;*/
   /*int xor_r = 0x0;*/
   /*char msgdigest[8];*/
   /*msgdigest[0] = 0x0;*/
   /*msgdigest[1] = 0x0;*/
   /*msgdigest[2] = 0x0;*/
   /*msgdigest[3] = 0x0;*/
   /*msgdigest[4] = 0x1;*/
   /*msgdigest[5] = 0x1;*/
   /*msgdigest[6] = 0x1;*/
   /*msgdigest[7] = 0x1;*/
   /*int *p = (int *)msgdigest;*/
   /*for (i = 0; i < 2; ++i) {*/
	   /*xor_r = xor_r ^ p[i];*/
   /*}*/
   /*printf("xor = %x \n", xor_r);*/
   /*for (i = 0; i < 8/4; ++i)*/
	   /*for (j = 0; j < 4; ++j)*/
		   /*xor_res[j] = xor_res[j] ^ msgdigest[4*i + j];*/
   /*xor_res[4] = 0;*/
   /*printf("xor result = %x\n", (int *)xor_res);*/

   /*send and recv message*/
   /*char message[1000];*/

   /*for (i = 0; i < 8; ++i)*/
	   /*message[i] = msgdigest[i];*/
   /*message[i++] = ',';*/
   /*for (j = 0; j < strlen(output); ++j, ++i)*/
	   /*message[i] = output[j];*/
   /*[>printf("message = %s\n", &message[9]);<]*/
   /*message[i++] = ',';*/
   /*message[i++] = (char)strlen(output);*/
   /*printf("output's len: %d\n", strlen(output));*/
   /*printf("current i = %d\n", i);*/
   /*message[i] = '\0';*/

   /*int a = (int)(message[strlen(output)+1+9]);*/
   /*printf("a=%d\n", a);*/
   
   /*char recv_blk[100];*/
   /*for (i = 0; i < a; ++i) */
      /*recv_blk[i] = message[i+9];*/
   /*recv_blk[i] = '\0';*/
   /*printf("recv block is %s\n", recv_blk);*/

   /*size_t de_size;*/
   /*origin = base64_decode(recv_blk, a, &de_size);*/
   /*printf("base64 decode: %s \n", origin);*/
   /*[>free(msg_digest);<]*/
   
   
   /*free(output);*/
   /*free(origin);*/
   return 0;
}
