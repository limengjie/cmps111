#define INQUIRY 0
#define FETCH 1
#define INSERT 2
#define DELETE 3
#define INVALID 99

#define BLOCKS 500
#define LEN 80
#define MD_LEN 16
#define MSG_LEN 139

#define INSERT_LEN strlen("INSERT")
#define MD5_DIGEST 16
#define SIZE_OF_INT sizeof(int)
#define COMMA 3

#define MAX_KEY_SIZE 100
#define MAX_DATA_SIZE 1000
#define FILLED 0xDEADD00D

int get_operation(char *cmd)
{
    char str[][7] = { "INQUIRY" , "FETCH" , "INSERT", "DELETE"};
    // int n;

    // char fn_file[20];
    int op;
    if (!strncmp (str[0],cmd, 7)){
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