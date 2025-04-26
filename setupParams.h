// setupParams.h

#ifndef SETUP_PARAMS_H
#define SETUP_PARAMS_H

typedef struct {
    char dataFileName[100];
    int keyStart;
    int keyEnd;
    char order[10];
} SetupParams;

int readSetupParams(const char *filename, SetupParams *params);

#endif
