#ifndef BINARY_READER_H
#define BINARY_READER_H

#include "setupParams.h" // JSON parametrelerini okumak için gerekebilir

typedef struct {
    char device_id[8];
    char timestamp[20];
    float temperature;
    int humidity;
    char status[8];
    char location[31];
    char alert_level[10];
    int battery;
    char firmware_ver[10];
    int event_code;
} Record;

// Binary dosyayı oku ve kayıtları yükle
Record* read_binary_file(const char *filename, int *record_count);

#endif
