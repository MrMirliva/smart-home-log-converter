#include "binaryReader.h"
#include "compare_records.h" // <<< BURAYI EKLE
#include <stdlib.h>

void sort_records(Record *records, int count) {
    qsort(records, count, sizeof(Record), compare_records);
}
