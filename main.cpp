#include <sys/wait.h>
#include<bits/stdc++.h>
#define MYSH_BUFF 1024
#define MYSH_TOK_BUFSIZE 64
#define MYSH_TOK_DELIM " \t\r\n\a"


auto initialize = []() {
    // std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    return 0;
}();

void mysh_loop();
char* mysh_readLine();
char** mysh_splitLine(char*);
int mysh_execute(char**);
int mysh_cd(char **args);
int mysh_help(char **args);
int mysh_exit(char **args);
/*This is going to be the main function that will be looping
The shell will be so simple that there won’t be any configuration files, and there won’t be any shutdown command. 
So, we’ll just call the looping function and then terminate.*/
int main(int argc, char ** argv)
{
    mysh_loop();
    return EXIT_SUCCESS;
}


void mysh_loop()
{
    char *line;
    char **args;
    int status;

    do{
        std::cout<<">";
        line = mysh_readLine();
        args = mysh_splitLine(line);
        status = mysh_execute(args);
        free(line);
        free(args);
    }while(status);
}

/* Reading a line from the stdin works differently as we can not limit the user to a certain block size, thus we start with allocating a block 
and if the user exceeds it we reallocate more space */
char* mysh_readLine(void)
{

  int bufsize = MYSH_BUFF;
  int position = 0;
  char *buffer = (char*)malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "mysh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += MYSH_BUFF;
      buffer = (char*)realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

/*Now that we have the line of input we will implement  parseing into the list of arguments. 
We will use whitespace to seperate arguments from each other.
Once this is done we need to tockenize the string using whitespace delimiters*/

char** mysh_splitLine(char* line)
{
  int bufsize = MYSH_TOK_BUFSIZE, position = 0;
  char **tokens = (char**)malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "mysh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MYSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += MYSH_TOK_BUFSIZE;
      tokens = (char**)realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, MYSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int mysh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("mysh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("mysh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int (*builtin_func[]) (char **) = {
  &mysh_cd,
  &mysh_help,
  &mysh_exit
};

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int mysh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int mysh_cd(char ** args)
{
    if(args[1]==NULL)
    {
        fprintf(stderr,"mysh: Expected a directory name to \"cd\"\n");
    }
    else{
        if(chdir(args[1])!=0)
        {
            perror("mysh error");
        }
    }
    return 1;
}

int mysh_help(char **args)
{
    int i=1;
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

  for (i = 0; i < mysh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int mysh_exit(char** args){
    return 0;
}

int mysh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < mysh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return mysh_launch(args);
}
