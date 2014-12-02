#ifndef __Shard2__shardshare__
#define __Shard2__shardshare__

struct shardshare_headers {
    char* version;
    char* description;
    char* original_file_name;
    char* original_file_sha256;
    char* total_shards;
    char* threshold;
    char* x;
    
    long data_offset;
};

int do_gfsplit(unsigned int, unsigned int, char *, char * );
int do_gfcombine( char *, char **, int );
struct shardshare_headers* get_headers(char *);
void free_headers(struct shardshare_headers*);

#endif /* defined(__Shard2__shardshare__) */
