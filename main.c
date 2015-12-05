/**************************************************************

Copyright Jan Marchant 2015
Licensed under the do whatever you want public license.

TERMS AND CONDITIONS OF THE DWYW PL:

1. Just DO WHATEVER YOU WANT.

Converts old SPC format to an ascii file.

Many features not supported because I have specific needs - if you
need them implemented let me know and maybe I'll extend.

Arguments:	<input file>
Outputs:	ascii table to standard output
To compile:	gcc main.c -o spc2ascii -lm

*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include "spc.h"


uint32_t reverse_words(uint32_t b) {
	b = (b & 0xFFFF0000) >> 16 | (b & 0x0000FFFF) << 16;
	return b;
}

float raw2float(uint32_t b,float e) {
	return powf(2,e)*reverse_words(b)/powf(2,32);
}

int main(int argc, char ** argv)
{
	if (argc<2)
	{
		fprintf(stderr,"Usage: %s <input file>\n<input file> should be an SPC format file.\n",argv[0]);
		return 1;
	}

	FILE *input;

        if (!(input = fopen(argv[1], "r"))) {
                fprintf(stderr,"Couldn't open %s\n",argv[1]);
                return 1;
        }

	OSPCHDR hdr;

	fread(&hdr,256,1,input);

        if (hdr.oversn==0x4B) {
                fprintf(stderr,"Only old LabCalc format supported - use R package hyperSpec for new format!\n");
                fclose(input);
                return 1;
        } else if (hdr.oversn==0x4C) {
                fprintf(stderr,"Only old LabCalc format supported - this format is very rare and not well documented...\n");
                fclose(input);
                return 1;
        } else if (hdr.oversn!=0x4D) {
                fprintf(stderr,"This doesn't look like a valid SPC file (second byte should be 0x4B or 0x4D)\n");
                fclose(input);
                return 1;
        }

	unsigned int mask = 128;

	while(mask){
		if (hdr.oftflgs & mask) {
			switch (mask) {
				case 128 :
					//fallthrough
				case 64:
					fprintf(stderr,"Sorry, XY files not supported\n");
					fclose(input);
					return 1;
				case 32:
					fprintf(stderr,"Sorry, custom axis labels not supported (obsolete)\n");
					fclose(input);
					return 1;
				case 16:
				case 8:
				case 4:
                                        fprintf(stderr,"Sorry, multifiles not supported\n");
					fclose(input);
                                        return 1;
				case 2:
                                        fprintf(stderr,"Sorry, can't use experiment extension\n");
					fclose(input);
                                        return 1;
				case 1:
                                        fprintf(stderr,"Sorry, only 32-bit precision data supported\n");
					fclose(input);
                                        return 1;
			}

		}

		mask >>= 1;
	}

	uint32_t * data=calloc(hdr.onpts,32);
	fread(data,32,hdr.onpts,input);
	fclose(input);

	for (int i=0;i<hdr.onpts;i++) {

		fprintf(stdout,"%f %f\n",hdr.ofirst+i*(hdr.olast-hdr.ofirst)/(hdr.onpts-1),raw2float(data[i],hdr.oexp));
	}

	free(data);
	return 0;
} 
