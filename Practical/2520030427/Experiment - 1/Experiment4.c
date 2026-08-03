#include<stdio.h>
#include<unistd.h>
int main() {
     pid_t pid;
     pid = fork();
   if(pid == 0)  {
    printf("Child PID = %d\n",getpid());
    printf("Parent PID = %d\n",getppid());
}
  else  {
     printf("Child PID = %d\n",pid);
     printf("Parent PID = %d\n",getpid());
}
   return 0;
}
