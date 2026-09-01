// Copyright (c) Kuba Szczodrzyński 2025-8-4.

#pragma once
#define ulong unsigned long

// MFM data
#define PIN_READ		  5
#define PIN_WRITE		  3
// Control Input
#define PIN_SELECT_0	  11
#define PIN_SELECT_1	  22
#define PIN_WRITE_GATE	  2
#define PIN_HEAD_1		  4
#define PIN_HEAD_2		  17
#define PIN_HEAD_4		  16
#define PIN_HEAD_8		  14
#define PIN_DIR_IN		  13
#define PIN_STEP		  12
// Control Output
#define PIN_SEEK_COMPLETE 10
#define PIN_SERVO_GATE	  7
#define PIN_INDEX		  6
#define PIN_TRACK_0		  9
#define PIN_READY		  8
// Buzzer
#define PIN_BUZZER		  15
// SD Card
#define PIN_SD_SCK		  18
#define PIN_SD_MOSI		  19
#define PIN_SD_MISO		  20
#define PIN_SD_CS		  21

#define DATA_RATE		  5000000
#define MARK_LBYTES		  32
#define HEADER_LBYTES	  32
#define DATA_LBYTES		  256
#define SECTORS_PER_PULSE 1
#define PULSES_PER_TRACK  32
#define HEADS			  11
#define CYLINDERS		  1024
#define	DRIVES			  2

#define MARK_BITS	(MARK_LBYTES * 8 * 2)
#define SECTOR_BITS ((HEADER_LBYTES + DATA_LBYTES) * 8 * 2)
#define PULSE_BITS	(MARK_BITS + SECTOR_BITS * SECTORS_PER_PULSE)
#define TRACK_BITS	(PULSE_BITS * PULSES_PER_TRACK)

#define MARK_WORDS		(MARK_BITS / 32)
#define SECTOR_WORDS	(SECTOR_BITS / 32)
#define PULSE_WORDS		(PULSE_BITS / 32)
#define TRACK_BYTES		(TRACK_BITS / 8)
#define TRACK_WORDS		(TRACK_BYTES / 4)
#define CYLINDER_BYTES	(TRACK_BYTES * HEADS)
#define CYLINDER_BLOCKS (((CYLINDER_BYTES - 1) / 512) + 1)
#define TRACK_BLOCKS	(CYLINDER_BLOCKS / HEADS)
#define DRIVE_BYTES		(CYLINDER_BYTES * CYLINDERS)

#define IDLE_TIMEOUT 5000

#define CYL_INVALID 2048

#define PIO_RDDT pio0
#define PIO_RDGT pio0
#define PIO_STEP pio0
#define PIO_WRDT pio1
#define PIO_WRGT pio1
#define PIO_WRRM pio1

#define SM_RDDT	 0
#define SM_RDGT	 1
#define SM_STEP	 2
#define SM_WRDT	 0
#define SM_WRGT	 1
#define SM_WRRMC 2
#define SM_WRRMA 3

#define PIO_SM_RDDT	 PIO_RDDT, SM_RDDT
#define PIO_SM_RDGT	 PIO_RDGT, SM_RDGT
#define PIO_SM_STEP	 PIO_STEP, SM_STEP
#define PIO_SM_WRDT	 PIO_WRDT, SM_WRDT
#define PIO_SM_WRGT	 PIO_WRGT, SM_WRGT
#define PIO_SM_WRRMC PIO_WRRM, SM_WRRMC
#define PIO_SM_WRRMA PIO_WRRM, SM_WRRMA
