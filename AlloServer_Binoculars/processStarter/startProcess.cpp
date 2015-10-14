#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main (void) {
//    pid_t processId;
//    if ((processId = fork()) == 0) {
//        char app[] = "../glxgears";
//        char * const argv[] = { app , NULL};
//        if (execv(app, argv) < 0) {
//            perror("execv error");
//        }
//    } else if (processId < 0) {
//        perror("fork error");
//    } else {
//        return EXIT_SUCCESS;
//    }
//    return EXIT_FAILURE;
    if(0 != system("../glxgears"))
        return 1;
}
