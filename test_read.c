#include <stdio.h>
#include <stdlib.h>

#pragma pack(1)

typedef struct {
    char device_id[8];
    char timestamp[20];
    float temperature;
    int humidity;
    char status;
    char location[31];
    char alert_level[10];
    int battery;
    char firmware_ver[10];
    int event_code;
} Record;

int main() {
    FILE *bin = fopen("logdata.dat", "rb");
    if (!bin) {
        perror("Binary dosyası açılamadı");
        return 1;
    }

    Record rec;
    int counter = 1;

    while (fread(&rec, sizeof(Record), 1, bin)) {
        printf("\n--- Kayıt %d ---\n", counter++);
        printf("Device ID: %s\n", rec.device_id);
        printf("Timestamp: %s\n", rec.timestamp);
        printf("Temperature: %.2f\n", rec.temperature);
        printf("Humidity: %d\n", rec.humidity);
        printf("Status: %c\n", rec.status);
        printf("Location: %s\n", rec.location);
        printf("Alert Level: %s\n", rec.alert_level);
        printf("Battery: %d\n", rec.battery);
        printf("Firmware Version: %s\n", rec.firmware_ver);
        printf("Event Code: %d\n", rec.event_code);
    }

    fclose(bin);
    return 0;
}
