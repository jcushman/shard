/*
 * This file is Copyright Harvard Library Innovation Lab 2014.
 *
 * This file is derived largely from the gfsplit/gfcombine tools
 * which are Copyright Daniel Silverstone and released under the same license.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "shardshare.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libgfshare.h"

#define BUFFER_SIZE 4096
#define VERSION "0.0.1"

#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif



static void
gfsplit_fill_rand( unsigned char *buffer,
                  unsigned int count )
{
    size_t n;
    FILE *devrandom;
    
    devrandom = fopen("/dev/urandom", "rb");
    if (!devrandom) {
        perror("Unable to read /dev/urandom");
        abort();
    }
    n = fread(buffer, 1, count, devrandom);
    if (n < count) {
        perror("Short read from /dev/urandom");
        abort();
    }
    fclose(devrandom);
}

static unsigned int
getlen( FILE* f )
{
    unsigned int len;
    unsigned int curpos = ftell(f);
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, curpos, SEEK_SET);
    return len;
}


struct shardshare_headers *
get_headers(char *file_path){
    FILE *file = fopen(file_path, "rb");
    char buffer[1024];
    char* header_name;
    char* header_value;
    char delimiter[]=": ";
    char* delimiter_position;
    char** target_header;
    struct shardshare_headers *headers;

    if(file == NULL){
        perror(file_path);
        return NULL;
    }
    
    headers = malloc( sizeof(struct shardshare_headers) );
    if( headers == NULL )
        return NULL; /* errno should still be set from malloc() */
    
    while( fgets (buffer, 1024, file)!=NULL ) {
        // TODO: deal with lines longer than 1024 chars
        if(!strcmp(buffer, "\n")){
            headers->data_offset = ftell(file);
            break;
        }
        
        // strip \n from buffer
        for(char* p=buffer+strlen(buffer)-1;p[0]=='\n'&&p>=buffer;p--)
            p[0] = '\0';
        
        delimiter_position = strstr(buffer, delimiter);
        delimiter_position[0] = '\0';  // put string terminator after header name
        header_name = buffer;
        header_value = delimiter_position+strlen(delimiter);
        
        if(!strcmp(header_name, "Version"))
            target_header = &headers->version;
        else if(!strcmp(header_name, "Description"))
            target_header = &headers->description;
        else if(!strcmp(header_name, "Original-File-Name"))
            target_header = &headers->original_file_name;
        else if(!strcmp(header_name, "Original-File-SHA256"))
            target_header = &headers->original_file_sha256;
        else if(!strcmp(header_name, "Total-Shards"))
            target_header = &headers->total_shards;
        else if(!strcmp(header_name, "Recovery-Threshold"))
            target_header = &headers->threshold;
        else if(!strcmp(header_name, "X-Coordinate"))
            target_header = &headers->x;
        else{
            printf("Unknown header: %s", header_name);
            continue;
        }
        
        *target_header = malloc(strlen(header_value)+1);
        strcpy(*target_header, header_value);
    }
    
    fclose(file);
    
    return headers;
}

void
free_headers(struct shardshare_headers* headers){
    free(headers->version);
    free(headers->description);
    free(headers->original_file_name);
    free(headers->original_file_sha256);
    free(headers->total_shards);
    free(headers->threshold);
    free(headers->x);
    free(headers);
}

int
do_gfsplit( unsigned int sharecount,
           unsigned int threshold,
           char *_inputfile,
           char *_outputstem )
{
    FILE *inputfile;
    unsigned char* sharenrs = malloc( sharecount );
    unsigned int i, j;
    FILE **outputfiles = malloc( sizeof(FILE*) * sharecount );
    char **outputfilenames = malloc( sizeof(char*) * sharecount );
    char* outputfilebuffer = malloc( strlen(_outputstem) + 10 );
    unsigned char* buffer = malloc( BUFFER_SIZE );
    gfshare_ctx *G;
    
    if( sharenrs == NULL || outputfiles == NULL || outputfilenames == NULL || outputfilebuffer == NULL || buffer == NULL ) {
        perror( "malloc" );
        return 1;
    }
    
    inputfile = fopen( _inputfile, "rb" );
    if( inputfile == NULL ) {
        perror( _inputfile );
        return 1;
    }
    for( i = 0; i < sharecount; ++i ) {
        unsigned char proposed = (random() & 0xff00) >> 8;
        if( proposed == 0 ) {
            proposed = 1;
        }
    SHARENR_TRY_AGAIN:
        for( j = 0; j < i; ++j ) {
            if( sharenrs[j] == proposed ) {
                proposed++;
                if( proposed == 0 ) proposed = 1;
                goto SHARENR_TRY_AGAIN;
            }
        }
        sharenrs[i] = proposed;
        sprintf( outputfilebuffer, "%s.%i.shard", _outputstem, i+1 );
        outputfiles[i] = fopen( outputfilebuffer, "wb" );
        if( outputfiles[i] == NULL ) {
            perror(outputfilebuffer);
            return 1;
        }
        outputfilenames[i] = strdup(outputfilebuffer);
    }
    
    // get pointer to file name from file path
    char *file_name;
    for(file_name=_inputfile+strlen(_inputfile)-1;file_name[0]!='/'&&file_name>=_inputfile;file_name--);
    file_name++;
    
    // write headers
    for( i = 0; i < sharecount; ++i ) {
        fprintf(outputfiles[i], "Version: %s\n", VERSION);
        fprintf(outputfiles[i], "Description: Shard of a file encoded with Shamir secret sharing in an 8-bit Galois field.\n");
        fprintf(outputfiles[i], "Original-File-Name: %s\n", file_name);
        fprintf(outputfiles[i], "Original-File-SHA256: TO COME\n");
        fprintf(outputfiles[i], "Total-Shards: %i\n", sharecount);
        fprintf(outputfiles[i], "Recovery-Threshold: %i\n", threshold);
        fprintf(outputfiles[i], "X-Coordinate: %i\n", sharenrs[i]);
        fprintf(outputfiles[i], "\n");
    }
    
    /* All open, all ready and raring to go... */
    G = gfshare_ctx_init_enc( sharenrs, sharecount,
                             threshold, MIN(BUFFER_SIZE, getlen( inputfile )) );
    if( !G ) {
        perror("gfshare_ctx_init_enc");
        return 1;
    }
    while( !feof(inputfile) ) {
        unsigned int bytes_read = fread( buffer, 1, BUFFER_SIZE, inputfile );
        if( bytes_read == 0 ) break;
        gfshare_ctx_enc_setsecret( G, buffer );
        for( i = 0; i < sharecount; ++i ) {
            unsigned int bytes_written;
            gfshare_ctx_enc_getshare( G, i, buffer );
            bytes_written = fwrite( buffer, 1, bytes_read, outputfiles[i] );
            if( bytes_read != bytes_written ) {
                perror(outputfilenames[i]);
                gfshare_ctx_free( G );
                return 1;
            }
        }
    }
    gfshare_ctx_free( G );
    fclose(inputfile);
    for( i = 0; i < sharecount; ++i ) {
        fclose(outputfiles[i]);
    }
    return 0;
}


int
do_gfcombine( char *outputfilename, char **inputfilenames, int filecount )
{
    FILE *outfile;
    FILE **inputfiles = malloc( sizeof(FILE*) * filecount );
    struct shardshare_headers **headers = malloc( sizeof(struct shardshare_headers*) * filecount );
    unsigned char* sharenrs = malloc( filecount );
    int i;
    unsigned char *buffer = malloc( BUFFER_SIZE );
    gfshare_ctx *G;
    unsigned int len1;
    
    if( inputfiles == NULL || sharenrs == NULL || buffer == NULL ) {
        perror( "malloc" );
        return 1;
    }
    
    if (strcmp(outputfilename, "-") == 0)
        outfile = fdopen(STDOUT_FILENO, "w");
    else
        outfile = fopen( outputfilename, "wb" );
    
    if( outfile == NULL ) {
        perror((strcmp(outputfilename, "-") == 0) ? "standard out" : outputfilename);
        return 1;
    }
    for( i = 0; i < filecount; ++i ) {
        headers[i] = get_headers(inputfilenames[i]);
        // TODO: error handling for header read
        inputfiles[i] = fopen( inputfilenames[i], "rb" );
        if( inputfiles[i] == NULL ) {
            perror(inputfilenames[i]);
            return 1;
        }
        fseek(inputfiles[i], headers[i]->data_offset, SEEK_SET);
        sharenrs[i] = atoi(headers[i]->x);
        unsigned int data_len = getlen(inputfiles[i]) - headers[i]->data_offset;
        if( i == 0 ) len1 = data_len;
        else {
            if( len1 != data_len ) {
                fprintf(stderr, "File length mismatch between input files.\n");
                return 1;
            }
        }
    }
    
    G = gfshare_ctx_init_dec( sharenrs, filecount, BUFFER_SIZE );
    
    while( !feof(inputfiles[0]) ) {
        unsigned int bytes_read = fread( buffer, 1, BUFFER_SIZE, inputfiles[0] );
        unsigned int bytes_written;
        gfshare_ctx_dec_giveshare( G, 0, buffer );
        for( i = 1; i < filecount; ++i ) {
            unsigned int bytes_read_2 = fread( buffer, 1, BUFFER_SIZE,
                                              inputfiles[i] );
            if( bytes_read != bytes_read_2 ) {
                fprintf( stderr, "Mismatch during file read.\n");
                gfshare_ctx_free( G );
                return 1;
            }
            gfshare_ctx_dec_giveshare( G, i, buffer );
        }
        gfshare_ctx_dec_extract( G, buffer );
        bytes_written = fwrite( buffer, 1, bytes_read, outfile );
        if( bytes_written != bytes_read ) {
            fprintf( stderr, "Mismatch during file write.\n");
            gfshare_ctx_free( G );
            return 1;
        }
    }
    fclose(outfile);
    for( i = 0; i < filecount; ++i ){
        fclose(inputfiles[i]);
        free_headers(headers[i]);
    }
    free(inputfiles);
    free(headers);
    free(sharenrs);
    free(buffer);
    return 0;
}