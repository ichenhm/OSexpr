#include<unistd.h>
#define COLLISIONFACTOR 0.5
typedef struct{
	int sig;
	int reclen;
	int total_rec_num;
	int current_rec_num;
}HashFileHeader;

typedef struct {
	char collision;
	char free;
}CFTag;

int hashfile_creat(const char *filename,mode_t mode,int reclen,int recnum);
//int hashfile_open(const char *filename,int flags);
int hashfile_open(const char *filename,int flags,mode_t mode);
int hashfile_close(int fd);
int hashfile_read(int fd,int keyoffset ,int keylen,void *buf);
int hashfile_write(int fd,int keyoffset,int keylen,void *buf); 
int hashfile_delrec(int fd,int keyoffset,int keylen,void *buf); 
int hashfile_findrec(int fd,int keyoffset,int keylen,void *buf); 
int hashfile_saverec(int fd,int keyoffset,int keylen,void *buf); 
int hash(int keyoffset,int keylen,void * buf int recnum);
int readHashFileHeader(int fd,HashFileHeader *hfh);

