#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFSIZE 1024
#define maxOPTs 10
#define maxLEN 100
#define DEFAULT 0
#define OUTPUT 1
#define INPUT 2
#define APPEND 3
#define PIPE 4
#define CD 5
#define RDERR 6

extern char **getline();

int getOpt(char ** args, int * cend, int * fend, int * fstart) {
  int i;
  int opt = DEFAULT;

  for(i = 0; args[i] != NULL; ++i) {
    printf("Argument %d: %s\n", i, args[i]);

    if (!strcmp(args[i], "<")) {
      *cend = i;  
      *fstart = i+1;
      opt = INPUT;
    }
    else if (!strcmp(args[i], ">")) {
      *fstart = i+1;
      if (!strcmp(args[i-1], ">")) {
	opt = APPEND;
      }
      else {
        *cend = i;
        opt = OUTPUT;
      }
    }
    else if (!strcmp(args[i], "|")) {
      *cend = i;  
      *fstart = i+1;
      opt = PIPE;
    }
    else if (!strcmp(args[i], "cd")) {
      *cend = i+1;
      *fstart = i+1;
      opt = CD;
    }
    else if (!strcmp(args[i], "&")) {
      *fstart = i+1;
      opt = RDERR;
    }
  } /* endFor */
  if (opt == DEFAULT)
    *cend = i;
  else
    *fend = i;
  
  return opt;
}


void getCmds(char **args, int endPos1, int startPos2, \
             int endPos2, char **cmd1, char **cmd2) {
  int i, j =0;
  /* generate command 1 */
  for (i = 0; i < endPos1; ++i) {
    cmd1[i] = args[i];
    printf("cmd1:%s\n", cmd1[i]);
  }
  cmd1[i] = NULL;

  if (startPos2 != 0) {
    /* generate command 2 */
    for (i = startPos2; i < endPos2; ++i) {
      cmd2[j++] = args[i];
      printf("cmd2:%s\n", cmd2[j-1]);
    }
    cmd2[j] = NULL;
  }

}

void cd(char *path) {
  char *ptr, buf[BUFSIZE];

  ptr = getcwd(buf, sizeof(buf));

  /* printf("current working dir: %s\n", buf);*/

  if (chdir(path)) {
    puts("no such directory");
    return;
  }

  ptr = getcwd(buf, sizeof(buf));
  printf("cwd: %s\n", buf);

}

void red_output(char **cmd1, char *outFile) {
  int outFD;

  outFD = open(outFile, O_RDWR|O_CREAT|O_TRUNC, 777);
  printf("filename %s descriptor %d",outFile, outFD);
  close(1);
  dup2(outFD, 1);
  close(outFD);
  execvp(cmd1[0],cmd1);
}
  
void red_input(char **cmd1, char *inFile) {
  int inFD;

  inFD = open(inFile, O_RDWR);
  close(0);
  dup2(inFD, 0);
  close(inFD);
  execvp(cmd1[0],cmd1);
  
}

void append(char **cmd1, char *outFile) {
  int outFD;
 
  outFD = open(outFile, O_WRONLY|O_APPEND);
  if (outFD == -1) {
    puts("no such file");
    return;
  }
  
  printf("filename %s descriptor %d",outFile, outFD);
  close(1);
  dup2(outFD, 1);
  close(outFD);
  execvp(cmd1[0],cmd1);
}


void Pipe(char **cmd1, char **cmd2) {
  int cpid, status, fd[2];


  pipe(fd);
  
  switch (cpid = fork()) {
    case 0:
            dup2(fd[0], 0);
            close(fd[1]);
            execvp(cmd2[0], cmd2);
    default:
            dup2(fd[1], 1);
            wait(&status);
            close(fd[0]);
            execvp(cmd1[0], cmd1);
  }
}
  

int main() {
  int pid = -1;
  int status;
  char **args;

  int cmds;


  /* commands'start and end positions in args */
  int cEnd, fEnd = 0, fStart = 0;
  int opt;
  int exec;

  char *cmd1[maxLEN], *cmd2[maxLEN];


  while(1) {
    args = getline();


    /* test exit */
    if(!strcmp(args[0], "exit"))
      exit(0);


    /* get options */
    opt = getOpt(args, &cEnd, &fEnd, &fStart);

    printf("endPos1=%d, startPos2=%d, endPos2=%d\n",  
          cEnd, fStart, fEnd); 



    /* get commands*/
    getCmds(args, cEnd, fStart, fEnd, cmd1, cmd2);

    


    /* fork and execute */
    if (opt == CD) {
      puts("call cd");
      cd(cmd2[0]);
    }
    else {
      pid = fork();
      printf("pid: %d, status: %d\n", pid, status);
      
      if (pid == 0) {
        switch (opt) {
          case OUTPUT:
	             red_output(cmd1, cmd2[0]);
		     break;
          case APPEND:
	             append(cmd1, cmd2[0]);
		     break;
          case INPUT:
	             red_input(cmd1, cmd2[0]);
		     break;
          case PIPE:
	             Pipe(cmd1, cmd2);
		     break;
          case RDERR:
                     puts("redirect std error");
		     break;
          default:
                     if ((exec = execvp(cmd1[0],cmd1)) == -1) {
	               puts("No such command");
		     }
		     break;
        } /* endSwitch */
      } /* endIf */ 
      else
        wait(&status); /* ?????????????? */
    } /* endElse */

/*
    pid = fork();
    printf("pid: %d, status: %d\n", pid, status);

    if (pid == 0) {
        switch (opt) {
                case 1:
                        outFD = open(outF, O_RDWR|O_CREAT|O_TRUNC, 777);
                        printf("filename %s descriptor %d",outF, outFD);
                        close(1);
                        dup(outFD);
                        close(outFD);
                        execvp(cmd1[0],cmd1);
                        wait(&status);
                        break;
                case 2:
                        inFD = open(outF, O_RDWR);
                        close(0);
                        dup(inFD);
                        close(inFD);
                        execvp(cmd1[0],cmd1);
                        wait(&status);
                        break;
                case 3:
                        pipe(fd);
                        printf("This is pipe.\n");
                        {
                          int cpid;
                          switch (cpid = fork()) {
                            case 0:
                                    dup2(fd[0], 0);
                                    close(fd[1]);
                                    execvp(cmd2[0], cmd2);
                            default:
                                     dup2(fd[1], 1);
				     wait(&status);
                                     close(fd[0]);
                                     execvp(cmd1[0], cmd1);
                          }
                        }
                        break;
                default:
                        execvp(cmd1[0],cmd1);
        }
     }
     wait(&status);
     */

  } /* end of while */
  return 0;
}
