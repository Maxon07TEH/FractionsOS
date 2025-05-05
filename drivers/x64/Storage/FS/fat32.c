#include "fat32.h"
#include "ata.h"
#include "string.h"
#include "memory.h"

static fat32_boot_sector bs;
static uint32_t fat_begin_lba;
static uint32_t data_begin_lba;
static uint32_t sectors_per_cluster;
static uint8_t current_drive;

static uint32_t cluster_to_lba(uint32_t cluster) {
    return data_begin_lba + (cluster - 2) * sectors_per_cluster;
}

bool fat32_init(uint8_t drive) {
    current_drive = drive;
    ata_read_sectors(drive, 0, 1, (uint16_t*)&bs);
    
    if(memcmp(bs.fs_type, "FAT32   ", 8) != 0) return false;
    
    sectors_per_cluster = bs.sectors_per_cluster;
    fat_begin_lba = bs.reserved_sectors;
    data_begin_lba = fat_begin_lba + (bs.fat_count * bs.sectors_per_fat);
    
    return true;
}

bool fat32_open(const char* path, fat32_file* file) {
    uint32_t current_cluster = bs.root_cluster;
    char name[11];
    memset(name, ' ', 11);
    
    // Парсинг пути
    const char* p = path;
    if(*p == '/') p++;
    
    while(*p) {
        // Поиск следующего компонента пути
        const char* end = p;
        while(*end && *end != '/') end++;
        
        // Формирование 8.3 имени
        memset(name, ' ', 11);
        int i = 0;
        while(p < end &&