#include <stdio.h>
#include <stdlib.h>
#include "binaryReader.h"

// Binary dosyayı oku ve kayıtları yükle
// Binary dosyayı oku ve kayıtları yükle
// Bu fonksiyon, binary dosyayı okuyarak kayıtları belleğe yükler
// ve kayıt sayısını döndürür.
// filename: Okunacak binary dosyanın adı
// record_count: Kayıt sayısını döndüren işaretçi
// return: Okunan kayıtların işaretçisi
Record* read_binary_file(const char *filename, int *record_count) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Binary dosyası açılamadı");
        return NULL;
    }

    // Dosyanın boyutunu öğren
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    // Kaç kayıt olduğunu hesapla
    int count = filesize / sizeof(Record);
    *record_count = count;

    // Kayıtları okuyacak alanı ayır
    Record *records = (Record *)malloc(count * sizeof(Record));
    if (!records) {
        perror("Bellek ayıramadı");
        fclose(fp);
        return NULL;
    }

    fread(records, sizeof(Record), count, fp);

    fclose(fp);
    return records;
}
