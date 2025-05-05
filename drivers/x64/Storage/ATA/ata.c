#include "ata.h"
#include "io.h"
#include "string.h"

static void ata_delay() {
    for(int i = 0; i < 4; i++) inb(ATA_STATUS);
}

void ata_wait_bsy() {
    while(inb(ATA_STATUS) & ATA_STATUS_BSY);
}

void ata_wait_drq() {
    while(!(inb(ATA_STATUS) & ATA_STATUS_DRQ));
}

bool ata_identify(uint8_t drive, ata_identify_t* data) {
    ata_wait_bsy();
    outb(ATA_DRIVE, 0xA0 | (drive << 4));
    outb(ATA_SECTOR_CNT, 0);
    outb(ATA_LBA_LOW, 0);
    outb(ATA_LBA_MID, 0);
    outb(ATA_LBA_HIGH, 0);
    outb(ATA_STATUS, ATA_CMD_IDENTIFY);
    
    if(inb(ATA_STATUS) == 0) return false;
    ata_wait_bsy();
    if(inb(ATA_ERROR) & 0x01) return false;
    
    ata_wait_drq();
    insw(ATA_DATA, data, sizeof(ata_identify_t)/2);
    return true;
}

void ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint16_t* buffer) {
    ata_wait_bsy();
    outb(ATA_DRIVE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, count);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_STATUS, ATA_CMD_READ);
    
    ata_wait_bsy();
    ata_wait_drq();
    
    insw(ATA_DATA, buffer, 256 * count);
}

void ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint16_t* buffer) {
    ata_wait_bsy();
    outb(ATA_DRIVE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, count);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_STATUS, ATA_CMD_WRITE);
    
    ata_wait_bsy();
    ata_wait_drq();
    
    outsw(ATA_DATA, buffer, 256 * count);
    outb(ATA_STATUS, 0xE7);
    ata_wait_bsy();
}

void ata_init() {
    ata_delay();
}