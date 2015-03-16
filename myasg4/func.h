#define INQUIRY 0
#define FETCH 1
#define INSERT 2
#define DELETE 3
#define INVALID 99

#define BLOCKS 500
#define BLK_LEN 80
#define MD_LEN 16
#define MSG_LEN 139

#define INSERT_LEN strlen("INSERT")
#define FETCH_LEN strlen("FETCH")
#define INQUIRY_LEN strlen("INQRY")
#define FOUND_LEN strlen("FOUND")
#define MD5_LEN 16
#define SIZE_OF_INT sizeof(int)
#define COMMA 1

#define ENTRIES 100
// #define MAX_KEY_SIZE 100
// #define MAX_DATA_SIZE 1000
// #define FILLED 0xDEADD00D




// parse message
int get_operation(char *cmd)
{
    char str[][7] = { "INQRY" , "FETCH" , "INSERT", "DELETE"};
    // int n;

    // char fn_file[20];
    int op;
    if (!strncmp (str[0],cmd, 5)){
        op = INQUIRY;

    }
    else if(!strncmp (str[1],cmd, 5)) {
        op = FETCH;
           }
    else if(!strncmp (str[2],cmd, 6)) {
        op = INSERT;

    }
    else if(!strncmp (str[3],cmd, 6)) {
        op = DELETE;
    }
    else{
        op = INVALID;
        
    }
    return op;
}

char * get_str(const char * input, 
    const char * str_start, const char * str_end)
{
  const char * start, * end;

  if((start = strstr(input, str_start)) != NULL)
  {
    start += strlen(str_start);
    if((end = strstr(start, str_end)) != NULL)
    {
      char *out = malloc(end - start + 1);
      if(out != NULL)
      {
        memcpy(out, start, (end - start));
        out[end - start] = '\0';
        return out;
      }
    }
  }
  return NULL;
}

int get_pos(char * msg, char delimiter, int times) {
    if (times <= 0)
        return -1;
    int i, num = 0;
    for (i = 0; num < times && i < MSG_LEN; ++i)
        if (msg[i] == delimiter)
            ++num;
    return i;
}


// modify the length for transmission
int h2n_len(size_t size) {
    int len = htonl((uint32_t)size);
    // printf("from client: len = %d\n", len);
    return len;
}

size_t n2h_len(int * intp) {
    int ilen = ntohl(*intp);
    size_t slen = (size_t)ilen;
    return slen; 
}


