// arcst506mfm.c
// create a raw ST506 MFM bitstream file in Archimedes/HD63463 low level format for pico506 emulator
// usage: arcst506mfm [inputfilename]
// provide input filename of existing disc image to convert it, otherwise a bitstream of an empty disc will be created, to be initialised with !HForm

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Drive shape
#define DRIVES  1
#define CYLS    1024
#define HEADS   11
#define SECTORS 32

// Track SPECIFY parameters
#define RL      256
#define GPL1    16
#define GPL2    15
#define GPL3    16
#define AMEX    1
#define ECD     1
#define PADP    1
#define CRCI    1
#define CRCP    1

// Drive filename programmed into pico506
#define FNAME   "ARCST506.MFM"

// Track bitstream format programmed into pico506
#define BITRATE 2*5E6
#define MK_LB   32
#define HD_LB   32
#define SPP     1
#define PPT     32

// Constants
#define CRCPOLY (CRCP?((1<<16)|(1<<12)|(1<<5)|1):((1<<16)|1))
#define ECCPOLY ((1<<23)|(1<<21)|(1<<11)|(1<<2)|1)
#define SCTDATA 0x00
#define INVERT  0

int write_mfm(FILE *output, unsigned char byte, int count, bool *lastbit, bool validclock) {
    int towrite = count;
    uint16_t encoded;
    bool databit, clockbit;
    
    while(towrite-->0) {
        encoded = 0;
        for(int j=0;j<8;j++) {
            databit = (byte >> (7-j)) & 1;
            clockbit = (!validclock && (j==5)) ? *lastbit : !(databit | *lastbit);
            encoded = (encoded << 2) | (clockbit << 1) | databit;
            *lastbit = databit;
        }
        fputc((encoded >> 8) ^ (INVERT*0xff),output);
        fputc((encoded & 0xff) ^ (INVERT*0xff),output);
    }    
    return 2*count;
}

uint32_t calc_crc(unsigned char *data, int len, uint32_t crc, uint32_t poly, int bits) {
    for (int i=0;i<len;i++) {
        crc ^= data[i] << (bits-8);
        for (int j=0; j<8; j++)
            crc = ((crc<<1) ^ (crc & (1<<(bits-1)) ? poly : 0)) & ((((1<<(bits-1))-1)<<1)+1);
    }
    return crc;
}

int main(int argv, char *argc[]) {
    FILE *output, *input;
    uint32_t crc, ecc;
    int written, tracklen = 2*PPT*(MK_LB+SPP*(HD_LB+RL));
    unsigned char idfield[7], datafield[2+RL];
    bool lastbit = 0;
    
    if(argv==2) input=fopen(argc[1],"r");
    else input = NULL;
    
    if(input) printf("Converting input file '%s' to MFM and padding/truncating to size.\n", argc[1]);
    else puts("Input file not found. Creating an empty MFM file.");
    
    printf("Bitstream format gives %d MFM bytes per track (or %d RPM).\n", tracklen, (int)(60*BITRATE/tracklen/8));
    
    datafield[0] = idfield[0] = 0xa1;
    datafield[1] = 0xf8;
   
    if((output=fopen(FNAME,"w+"))) {
        printf("Creating raw MFM stream for %d drive%s, C/H/S = %d/%d/%d, cylinder bytes %d, total bytes %d, data per drive %.2f MB.\n",
            DRIVES, DRIVES==1?"":"s", CYLS, HEADS, SECTORS, HEADS*tracklen, DRIVES*CYLS*HEADS*tracklen, (float)(CYLS*HEADS*SECTORS*RL)/(1<<20));
        
        for(int d=0;d<DRIVES;d++) {
            for(int c=0;c<CYLS;c++) {
                for(int h=0;h<HEADS;h++) {
                    written = write_mfm(output,0x4e,GPL1,&lastbit,true);
                    for(int s=0;s<SECTORS;s++) {
                        written += write_mfm(output,0x00,GPL2,&lastbit,true);
                        
                        idfield[1] = c >> 8;
                        idfield[2] = c & 0xff;
                        idfield[3] = h;
                        idfield[4] = s;
                        crc = calc_crc(idfield+AMEX,5-AMEX,CRCI?0xffff:0,CRCPOLY,16);
                        idfield[5] = crc >> 8;
                        idfield[6] = crc & 0xff;
                        
                        for(int i=0;i<7;i++)
                            written += write_mfm(output,idfield[i],1,&lastbit,i!=0);
                            
                        written += write_mfm(output,PADP?0x4e:0,3-AMEX,&lastbit,true);
                        written += write_mfm(output,0x00,GPL2,&lastbit,true);
                        
                        written += write_mfm(output,datafield[0],1,&lastbit,false);
                        written += write_mfm(output,datafield[1],1,&lastbit,true);
                        
                        memset(2+datafield,SCTDATA,RL);
                        for(int i=0;i<RL;i+=2) {
                            if(input && d==0) if(!feof(input)) datafield[2+i+1] = fgetc(input);
                            if(input && d==0) if(!feof(input)) datafield[2+i] = fgetc(input);
                            written += write_mfm(output,datafield[2+i],1,&lastbit,true);
                            written += write_mfm(output,datafield[2+i+1],1,&lastbit,true);
                        }
                        ecc = calc_crc(datafield+AMEX,2+RL-AMEX,ECD?0:CRCI?0xffff:0,ECD?ECCPOLY:CRCPOLY,ECD?32:16);
    
                        for(int i=0;i<2+2*ECD;i++)
                            written += write_mfm(output,(ecc>>(8*(2*ECD+1-i)))&0xff,1,&lastbit,true);

                        written += write_mfm(output,PADP?0x4e:0,3-AMEX,&lastbit,true);
                        written += write_mfm(output,0x4e,GPL3,&lastbit,true);                      
                    }
                    if(written>tracklen) {
                        printf("Error: Track MFM bytes %d exceed defined bitstream MFM bytes %d, aborting now.\n", written, tracklen);
                        if(input) fclose(input);
                        fclose(output);
                        exit(1);
                    }
                    if(d+c+h==0) printf("Track MFM bytes %d, GPL4 = %d\n",written,(tracklen-written)/2);
                    write_mfm(output,0x4e,(tracklen-written)/2,&lastbit,true);
                }            
            }        
        }
        if(input) fclose(input);
        fclose(output);
        printf("File saved as '%s'.\n", FNAME); 
    }
    else puts("Couldn't open output file.");

    puts("Done.");
    return 0;
}
