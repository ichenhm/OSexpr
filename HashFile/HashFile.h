#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#define COLLISIONFACTOR 0.5
//linux

struct HashFileHeader
{
	int sig;
	int reclen;
	int total_rec_num;
	int current_rec_num;

};
struct CFTag
{
	char collision;
	char free;
};
int hashfile_creat(const char *filename,mode_t mode,int reclen,int recnum);
//int hashfile_open(const char *filename,int flags);
int hashfile_open(const char *filename,int flags,mode_t mode);
int hashfile_close(int fd);
int hashfile_read(int fd,int keyoffset,int keylen,void *buf);
int hashfile_write(int fd,int keyoffset,int keylen,void *buf);
int hashfile_delrec(int fd,int keyoffset,int keylen,void *buf);
int hashfile_findrec(int fd,int keyoffset,int keylen,void *buf);
int hashfile_saverec(int fd,int keyoffset,int keylen,void *buf);
int hash(int keyoffset,int keylen,void *buf,int recnum);
int checkHashFildFull(int fd);
int readHashFileHeader(int fd,struct HashFileHeader *hfh);
