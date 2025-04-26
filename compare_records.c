#include <string.h>
#include "binaryReader.h"

// Global değişkenler
extern int keyStart;
extern int keyEnd;
extern int orderAsc;

int compare_records(const void *a, const void *b) {
    const Record *recA = (const Record *)a;
    const Record *recB = (const Record *)b;

    char keyA[10] = {0};
    char keyB[10] = {0};

    strncpy(keyA, recA->device_id + keyStart, keyEnd - keyStart);
    strncpy(keyB, recB->device_id + keyStart, keyEnd - keyStart);

    int cmp = strncmp(keyA, keyB, keyEnd - keyStart);

    if (orderAsc)
        return cmp;
    else
        return -cmp;
}
