#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int global_var = 100;          // Global variable
int global_uninitialized;      // BSS segment

static int static_var = 200;   // Static variable

void code_function() {
    printf("Inside code function\n");
}

int main() {

    int stack_var = 10;        // Stack variable

    int *heap_var = malloc(sizeof(int));
    *heap_var = 20;             // Heap variable

    printf("===== PROCESS MEMORY ADDRESSES =====\n\n");

    printf("Code (function) address       : %p\n",
           (void *)code_function);

    printf("Global variable address       : %p\n",
           (void *)&global_var);

    printf("Global BSS variable address   : %p\n",
           (void *)&global_uninitialized);

    printf("Static variable address       : %p\n",
           (void *)&static_var);

    printf("Heap variable address         : %p\n",
           (void *)heap_var);

    printf("Stack variable address        : %p\n",
           (void *)&stack_var);

    printf("\nProcess ID: %d\n", getpid());

    printf("\nProcess will remain running...\n");
    printf("Use another terminal to inspect /proc/%d/maps\n",
           getpid());

    while (1) {
        sleep(5);
    }

    free(heap_var);

    return 0;
}
