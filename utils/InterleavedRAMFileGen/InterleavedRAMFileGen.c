#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int stat(const char *path, struct stat *buf);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);

typedef union {
	uint32_t w;
	uint8_t b[4];
} split_4Byte_t ;

char gf22dxMemInitFilesPathBuf[256] = {0};
FILE *gf22dxPrivRam0FilePtr[2] = {0};
FILE *gf22dxPrivRam1FilePtr[2] = {0};
FILE *gf22dxCol0FilePtr[7] = {0};
FILE *gf22dxCol1FilePtr[7] = {0};
FILE *gf22dxCol2FilePtr[7] = {0};
FILE *gf22dxCol3FilePtr[7] = {0};

void gf22dxInitPathAndOpenFilePtrs(void)
{
    int i = 0;
    for( i=0; i<2; i++ )
    {
        sprintf(gf22dxMemInitFilesPathBuf, "gf22dxMemoryInitFiles/privateBank0_%d.mem", i);
        gf22dxPrivRam0FilePtr[i] = fopen (gf22dxMemInitFilesPathBuf,"w");
    }
    for( i=0; i<2; i++ )
    {
        sprintf(gf22dxMemInitFilesPathBuf, "gf22dxMemoryInitFiles/privateBank1_%d.mem", i);
        gf22dxPrivRam1FilePtr[i] = fopen (gf22dxMemInitFilesPathBuf,"w");
    }
    for( i=0; i<7; i++ )
    {
        sprintf(gf22dxMemInitFilesPathBuf, "gf22dxMemoryInitFiles/col0_%d.mem", i);
        gf22dxCol0FilePtr[i] = fopen (gf22dxMemInitFilesPathBuf,"w");
    }

    for( i=0; i<7; i++ )
    {
        sprintf(gf22dxMemInitFilesPathBuf, "gf22dxMemoryInitFiles/col1_%d.mem", i);
        gf22dxCol1FilePtr[i] = fopen (gf22dxMemInitFilesPathBuf,"w");
    }

    for( i=0; i<7; i++ )
    {
        sprintf(gf22dxMemInitFilesPathBuf, "gf22dxMemoryInitFiles/col2_%d.mem", i);
        gf22dxCol2FilePtr[i] = fopen (gf22dxMemInitFilesPathBuf,"w");
    }

    for( i=0; i<7; i++ )
    {
        sprintf(gf22dxMemInitFilesPathBuf, "gf22dxMemoryInitFiles/col3_%d.mem", i);
        gf22dxCol3FilePtr[i] = fopen (gf22dxMemInitFilesPathBuf,"w");
    }
}

void gf22dxInitCloseFilePtrs(void)
{
    int i = 0;
    for( i=0; i<2; i++ )
    {
        fclose (gf22dxPrivRam0FilePtr[i]);
    }
    for( i=0; i<2; i++ )
    {
        fclose (gf22dxPrivRam1FilePtr[i]);
    }
    for( i=0; i<7; i++ )
    {
        fclose (gf22dxCol0FilePtr[i]);
    }

    for( i=0; i<7; i++ )
    {
        fclose (gf22dxCol1FilePtr[i]);
    }

    for( i=0; i<7; i++ )
    {
        fclose (gf22dxCol2FilePtr[i]);
    }

    for( i=0; i<7; i++ )
    {
        fclose (gf22dxCol3FilePtr[i]);
    }
}


void gf22dxSplitFilesInto4KLineWords(FILE *inFile, FILE **outFile, int aNumOfSplits, int aChunkSize )
{
    int i = 0, j = 0;
    char lReadBuf[10] = {0};
    size_t lReadSts = 0;
    fseek(inFile, 0, SEEK_SET); // seek back to beginning of file
    for( i=0; i<aNumOfSplits; i++ )
    {
        for( j=0; j<aChunkSize; j++ )
        {
            lReadSts = fread(lReadBuf, sizeof(uint8_t), 9, inFile);
            if( lReadSts != EOF )
            {
                fprintf(outFile[i], "%s",lReadBuf);
            }
            else
            {
                fprintf(outFile[i], "00000000\n");
            }
        }
    }
}

//stops parsing, if a NULL or space or is encountered or entire length of the buffer is read
uint32_t atoh(uint8_t *aBuf, uint16_t aSize)
{
    uint16_t i = 0;
    uint32_t lValue = 0;
    uint8_t lChar = 0;

    for(i=0; ( (i < aSize ) && (aBuf[i] != '\0') && (aBuf[i] != ' ') ); i++)
    {
        if( (aBuf[i] >= '0') && ( aBuf[i] <= '9') )
        {
            lChar = aBuf[i] - '0';
        }
        else if( (aBuf[i] >= 'A') && ( aBuf[i] <= 'F') )
        {
            lChar = (aBuf[i] - 'A') + 10;
        }
        else if( (aBuf[i] >= 'a') && ( aBuf[i] <= 'f') )
        {
            lChar = (aBuf[i] - 'a') + 10;
        }
        lValue *= 16;
        lValue += lChar;
    }
    return lValue;
}


int main(int argc, char *argv[])
{
    char *inFile = (char *)NULL;
    FILE *lBinFileReadPtr = (FILE *)NULL;
    FILE *lPrivateBank0FileWritePtr = (FILE *)NULL;
    FILE *lPrivateBank1FileWritePtr = (FILE *)NULL;
    FILE *lCol0FileWritePtr = (FILE *)NULL;
    FILE *lCol1FileWritePtr = (FILE *)NULL;
    FILE *lCol2FileWritePtr = (FILE *)NULL;
    FILE *lCol3FileWritePtr = (FILE *)NULL;
    int opt = 0;
    struct stat st = {0};

    long int lBinFileSize = 0;
    long int lReadByteCount = 0;
    uint8_t lData = 0;
    uint32_t i = 0, j=0;

    uint32_t lCol0FileStartOffset = 0;
    uint32_t lCol1FileStartOffset = 0;
    uint32_t lCol2FileStartOffset = 0;
    uint32_t lCol3FileStartOffset = 0;

    uint32_t lCol0FileEndOffset = 0;
    uint32_t lCol1FileEndOffset = 0;
    uint32_t lCol2FileEndOffset = 0;
    uint32_t lCol3FileEndOffset = 0;

    uint32_t lOffset = 0x0;
    split_4Byte_t l4ByteData;

    l4ByteData.w = 0;

    while ((opt = getopt(argc, argv, ":i:o:h")) != -1) {
        switch (opt) {
            case 'i':
                inFile = optarg;
                break;
            case 'o':
                lOffset = atoh((uint8_t *)optarg, (uint16_t)strlen(optarg));
                break;
            case ':':
                printf("option needs a value\n");
                break;
            case '?':
                printf("unknown option: %c\n",optopt);
                break;
            case 'h':
            default: /* '?' */
                fprintf(stderr, "Usage: %s -f infile [-o offset in hex]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("inFile=%s; lOffset=%d;\n", inFile, lOffset);
    for(; optind < argc; optind++){
        printf("extra arguments: %s\n", argv[optind]);
    }

    if( inFile != NULL )
    {
        printf("Converting %s.\n", inFile);

        if (stat("memoryInitFiles", &st) == -1)
#ifdef __linux__			
            mkdir("memoryInitFiles", 0700);
#else
            mkdir("memoryInitFiles");
#endif

        if (stat("gf22dxMemoryInitFiles", &st) == -1)
#ifdef __linux__			
            mkdir("gf22dxMemoryInitFiles", 0700);
#else
            mkdir("gf22dxMemoryInitFiles");
#endif

        lBinFileReadPtr = fopen(inFile,"rb");  // r for read, b for binary
        lPrivateBank0FileWritePtr = fopen ("memoryInitFiles/privateBank0.mem","w");
        lPrivateBank1FileWritePtr = fopen ("memoryInitFiles/privateBank1.mem","w");
        lCol0FileWritePtr = fopen ("memoryInitFiles/col0.mem","w");
        lCol1FileWritePtr = fopen ("memoryInitFiles/col1.mem","w");
        lCol2FileWritePtr = fopen ("memoryInitFiles/col2.mem","w");
        lCol3FileWritePtr = fopen ("memoryInitFiles/col3.mem","w");

        if(  ( lBinFileReadPtr ) && ( lPrivateBank0FileWritePtr ) && ( lPrivateBank1FileWritePtr ) && ( lCol0FileWritePtr )
            && (lCol1FileWritePtr ) && (lCol2FileWritePtr) && (lCol3FileWritePtr)
        )
        {
            fseek(lBinFileReadPtr, 0, SEEK_END); // seek to end of file
            lBinFileSize = ftell(lBinFileReadPtr); // get current file pointer
            fseek(lBinFileReadPtr, 0, SEEK_SET); // seek back to beginning of file
            printf("File [%s] is of %ld bytes\n",inFile, lBinFileSize);

            lCol0FileStartOffset = (0x00010000 - lOffset) + 0;
            lCol1FileStartOffset = (0x00010000 - lOffset) + 4;
            lCol2FileStartOffset = (0x00010000 - lOffset) + 8;
            lCol3FileStartOffset = (0x00010000 - lOffset) + 12;

            lCol0FileEndOffset = lBinFileSize - 16;
            lCol1FileEndOffset = lBinFileSize - 12;
            lCol2FileEndOffset = lBinFileSize - 8;
            lCol3FileEndOffset = lBinFileSize - 4;

            for(i=0; i<(lOffset/4); i++)
                fprintf(lPrivateBank0FileWritePtr,"00000000\n");

            for(i=0; i<(32768-lOffset);)
            {
                for(j=0; j< 4; j++ )
                {
                    fread(&lData, sizeof(lData), 1, lBinFileReadPtr);
                    lReadByteCount++;
                    l4ByteData.b[j] = lData;
                }
                i+=4;
                fprintf(lPrivateBank0FileWritePtr, "%08X\n",l4ByteData.w);
                fflush(lPrivateBank0FileWritePtr);
            }

            fseek(lBinFileReadPtr, (32768-lOffset), SEEK_SET); //
            for(i=0; i<32768; )
            {
                for(j=0; j< 4; j++ )
                {
                    fread(&lData, sizeof(lData), 1, lBinFileReadPtr);
                    lReadByteCount++;
                    l4ByteData.b[j] = lData;
                }
                i+= 4;
                fprintf(lPrivateBank1FileWritePtr, "%08X\n",l4ByteData.w);
                fflush(lPrivateBank1FileWritePtr);
            }

            while( lCol0FileStartOffset <= lCol0FileEndOffset )
            {
                fseek(lBinFileReadPtr, lCol0FileStartOffset, SEEK_SET);
                for( i=0; i<4; i++ )
                {
                    fread(&lData, sizeof(lData), 1, lBinFileReadPtr);
                    lReadByteCount++;
                    l4ByteData.b[i] = lData;
                }
                fprintf(lCol0FileWritePtr, "%08X\n",l4ByteData.w);
                fflush(lCol0FileWritePtr);

                lCol0FileStartOffset += 16;
            }

            while( lCol1FileStartOffset <= lCol1FileEndOffset )
            {
                fseek(lBinFileReadPtr, lCol1FileStartOffset, SEEK_SET);
                for( i=0; i<4; i++ )
                {
                    fread(&lData, sizeof(lData), 1, lBinFileReadPtr);
                    lReadByteCount++;
                    l4ByteData.b[i] = lData;
                }
                fprintf(lCol1FileWritePtr, "%08X\n",l4ByteData.w);
                fflush(lCol1FileWritePtr);

                lCol1FileStartOffset += 16;
            }

            while( lCol2FileStartOffset <= lCol2FileEndOffset )
            {
                fseek(lBinFileReadPtr, lCol2FileStartOffset, SEEK_SET);
                for( i=0; i<4; i++ )
                {
                    fread(&lData, sizeof(lData), 1, lBinFileReadPtr);
                    lReadByteCount++;
                    l4ByteData.b[i] = lData;
                }
                fprintf(lCol2FileWritePtr, "%08X\n",l4ByteData.w);
                fflush(lCol2FileWritePtr);

                lCol2FileStartOffset += 16;
            }


            while( lCol3FileStartOffset <= lCol3FileEndOffset )
            {
                fseek(lBinFileReadPtr, lCol3FileStartOffset, SEEK_SET);
                for( i=0; i<4; i++ )
                {
                    fread(&lData, sizeof(lData), 1, lBinFileReadPtr);
                    lReadByteCount++;
                    l4ByteData.b[i] = lData;
                }
                fprintf(lCol3FileWritePtr, "%08X\n",l4ByteData.w);
                fflush(lCol3FileWritePtr);

                lCol3FileStartOffset += 16;
            }

            fclose(lBinFileReadPtr);
            fclose(lPrivateBank0FileWritePtr);
            fclose(lPrivateBank1FileWritePtr);
            fclose(lCol0FileWritePtr);
            fclose(lCol1FileWritePtr);
            fclose(lCol2FileWritePtr);
            fclose(lCol3FileWritePtr);

            lPrivateBank0FileWritePtr = fopen ("memoryInitFiles/privateBank0.mem","r");
            lPrivateBank1FileWritePtr = fopen ("memoryInitFiles/privateBank1.mem","r");
            lCol0FileWritePtr = fopen ("memoryInitFiles/col0.mem","r");
            lCol1FileWritePtr = fopen ("memoryInitFiles/col1.mem","r");
            lCol2FileWritePtr = fopen ("memoryInitFiles/col2.mem","r");
            lCol3FileWritePtr = fopen ("memoryInitFiles/col3.mem","r");

            gf22dxInitPathAndOpenFilePtrs();

            gf22dxSplitFilesInto4KLineWords(lPrivateBank0FileWritePtr, gf22dxPrivRam0FilePtr, 2, 4096 );
            gf22dxSplitFilesInto4KLineWords(lPrivateBank1FileWritePtr, gf22dxPrivRam1FilePtr, 2, 4096 );
            gf22dxSplitFilesInto4KLineWords(lCol0FileWritePtr, gf22dxCol0FilePtr, 7, 4096 );
            gf22dxSplitFilesInto4KLineWords(lCol1FileWritePtr, gf22dxCol1FilePtr, 7, 4096 );
            gf22dxSplitFilesInto4KLineWords(lCol2FileWritePtr, gf22dxCol2FilePtr, 7, 4096 );
            gf22dxSplitFilesInto4KLineWords(lCol3FileWritePtr, gf22dxCol3FilePtr, 7, 4096 );

            gf22dxInitCloseFilePtrs();
            fclose(lPrivateBank0FileWritePtr);
            fclose(lPrivateBank1FileWritePtr);
            fclose(lCol0FileWritePtr);
            fclose(lCol1FileWritePtr);
            fclose(lCol2FileWritePtr);
            fclose(lCol3FileWritePtr);
            printf("[DONE] Read %ld / %ld bytes \n", lReadByteCount, lBinFileSize);
        }
        else
        {
            printf("FILE access error\n");
        }
    }

    return 0;
}

