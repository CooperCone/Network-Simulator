#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "physicalLayer.h"

int main(int argc, char **argv) {

    char *buffer = "tmp text";

    // Configuration
    WireTerminal side1 = {0};
    WireTerminal side2 = {0};

    wireTerminal_connect(&side1, &side2);

    // Network Simulation

    side1.send(&side1, buffer, strlen(buffer));

    printf("Hello, World!\n");
}
