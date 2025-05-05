#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>

#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_STATUS      0x1F7

#define ATA_CMD_READ    0x20
#define ATA_CMD_WRITE   0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

typedef struct {
    uint16_t flags;
    uint16_t unused1[9];
    char     serial[20];
    uint16_t unused2[3];
    char     firmware[8];
    char     model[40];
    uint16_t sectors_per_int;
    uint16_t unused3;
    uint16_t capabilities[2];
    uint16_t unused4[2];
    uint16_t valid_ext_data;
    uint16_t unused5[5];
    uint16_t size_of_sector;
    uint16_t total_sectors[4];
} __attribute__((packed)) ata_identify_t;

void ata_wait_bsy();
void ata_wait_drq();
bool ata_identify(uint8_t drive, ata_identify_t* data);
void ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint16_t* buffer);
void ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint16_t* buffer);
void ata_init();

#endif