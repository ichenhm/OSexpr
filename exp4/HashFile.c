#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "HashFile.h"//

int hashfile_creat(const char * fileName,mode_t mode,int reclen,int total_rec_num)
{
	HashFileHeader hfh;
	int fd;
	int rtn;
	char * buf;
	int i=0;
	hfh.sig=31415926;
	hfh.reclen=reclen;
	hfh.total_rec_num=total_rec_num;
	hfh.current_rec_num=0;
	//fd = open(fileName,mode);
	fd=creat(fileName,mode);
	if(fd!=-1)
	{
		rtn = write(fd,&hfh,sizeof(HashFileHeader));
		//lseek(fd,sizeof(HashFileHeader),SEEK_SET);
		if(rtn!=-1)
		{
			buf=(char * ) malloc((reclen + sizeof(CFTag))*total_rec_num);
			memset(buf,0,(reclen+sizeof(CFTag))*total_rec_num);
			rtn=write(fd,buf,(reclen+sizeof(CFTag))*total_rec_num);
			free(buf);
		}
		close(fd);
		return rtn;

	}
	else
	{
		close(fd);
		return -1;
	}
}

int hashfile_open(const char * fileName,int flags,mode_t mode)
{
	int fd=open(fileName,flags,mode);
	HashFileHeader hfh;
	if(read(fd,&hfh,sizeof(HashFileHeader))!=-1)
	{
		lseek(fd,0,SEEK_SET);
		if(hfh.sig==31415926)
		{
			return fd;
		}
		else
			return -1;
	}
	else
		return -1;
}

int hashfile_close(int fd)
{
	return close(fd);
}

int hashfile_read(int fd,int keyoffset ,int keylen,void * buf)
{
	HashFileHeader hfh;
	readHashFileHeader(fd,&hfh);
	int offset = hashfile_findrec(fd,keyoffset,keylen,buf);
	if(offset!=-1)
	{
		lseek(fd,offset+sizeof(CFTag),SEEK_SET);
		return read(fd,buf,hfh.reclen);
	}
	else
	{
		return -1;
	}
}

int hashfile_write(int fd,int keyoffset,int keylen ,void *buf)
{
	return hashfile_saverec(fd,keyoffset,keylen,buf);
	// return -1;
}

int hashfile_delrec(int fd,int keyoffset,int keylen,void * buf)
{
	int offset=0;
	offset=hashfile_findrec(fd,keyoffset,keylen,buf);
	if(offset!=-1)
	{
		CFTag tag;
		read(fd,&tag,sizeof(CFTag));
		tag.free=0;//tag set free
		lseek(fd,offset,SEEK_SET);
		HashFileHeader hfh;
		readHashFileHeader(fd,&hfh);
		int addr = hash(keyoffset,keylen,buf,hfh.total_rec_num);
		offset=sizeof(HashFileHeader)+addr*(hfh.reclen+sizeof(CFTag));
		if(lseek(fd,offset,SEEK_SET==-1))
			return -1;
		read(fd,&tag,sizeof(CFTag));
		tag.collision--;// conflect cnt -1
		lseek(fd,offset,SEEK_SET);
		write(fd,&tag,sizeof(CFTag));
		hfh.current_rec_num--;
		lseek(fd,0,SEEK_SET);
		write(fd,&hfh,sizeof(HashFileHeader));

	}
	else
	{
		return -1;
	}

}

int hashfile_findrec(int fd,int keyoffset ,int keylen,void *buf)
{
	HashFileHeader hfh;
	readHashFileHeader(fd,&hfh);
	int addr = hash(keyoffset,keylen,buf,hfh.total_rec_num);
	int offset = sizeof(HashFileHeader)+addr*(hfh.reclen+sizeof(CFTag));
	if(lseek(fd,offset,SEEK_SET)==-1)
		return -1;
	CFTag tag;
	read(fd,&tag,sizeof(CFTag));
	char count =tag.collision;
	if(count==0)
		return -1;//not exist
recfree:
	if(tag.free==0)
	{
		offset+=hfh.reclen+sizeof(CFTag);
		if(lseek(fd,offset,SEEK_SET)==-1)
			return-1;
		read(fd,&tag,sizeof(CFTag));
		goto recfree;
	}
	else
	{
		char *p=(char*)malloc(hfh.reclen*sizeof(char));
		read(fd,p,hfh.reclen);
		char *p1,*p2;
		p1=(char*)buf+keyoffset;
		p2=p+keyoffset;
		int j=0;
		while((*p1==*p2)&&(j<keylen))
		{
			p1++;
			p2++;
			j++;
		}
		if(j==keylen)
		{
			free(p);
			p=NULL;
			return (offset);
		}
		else
		{
			if(addr==hash(keyoffset,keylen,p,hfh.total_rec_num))
			{
				count--;
				if(count==0)
				{
					free(p);
					p=NULL;
					return -1;
				}
			}
			free(p);
			p=NULL;
			offset+=hfh.reclen+sizeof(CFTag);
			if(lseek(fd,offset,SEEK_SET)==-1)
				return -1;
			read(fd,&tag,sizeof(CFTag));
			goto recfree;
		}
	}

}


int hashfile_saverec(int fd,int keyoffset,int keylen ,void *buf)
{
	if(checkHashFileFull(fd))
		return -1;
	HashFileHeader hfh;
	readHashFileHeader(fd,&hfh);
	int addr=hash(keyoffset,keylen,buf,hfh.total_rec_num);
	int offset=sizeof(HashFileHeader) +addr*(hfh.reclen+sizeof(CFTag));
	if(lseek(fd,offset,SEEK_SET)==-1)
		return -1;
	CFTag tag;
	read(fd,&tag,sizeof(CFTag));
	tag.collision++;
	lseek(fd,sizeof(CFTag)*(-1),SEEK_CUR);
	write(fd,&tag,sizeof(CFTag));
	while(tag.free!=0)
	{
		offset+=hfh.reclen+sizeof(CFTag);
		if(offset>=lseek(fd,0,SEEK_END))
			offset=sizeof(HashFileHeader);
		if(lseek(fd,offset,SEEK_SET)==-1)
			return -1;
		read(fd,&tag,sizeof(CFTag));
	}
	tag.free=1;
	lseek(fd,sizeof(CFTag)*(-1),SEEK_CUR);
	write(fd,&tag,sizeof(CFTag));
	write(fd,buf,hfh.reclen);
	hfh.current_rec_num++;
	lseek(fd,0,SEEK_SET);
	return write(fd,&hfh,sizeof(HashFileHeader));// save record;

}

int hash(int keyoffset,int keylen,void *buf,int total_rec_num)
{
	int i=0;
	char * p=(char *)buf +keyoffset;
	int addr=0;
	for(i=0;i<keylen;i++)
	{
		addr+=(int)(*p);
		p++;
	}
	return addr%(int)(total_rec_num*COLLISIONFACTOR);
}

int readHashFileHeader(int fd,HashFileHeader *hfh)
{
	lseek(fd,0,SEEK_SET);
	return read(fd,hfh,sizeof(HashFileHeader));
}

int checkHashFileFull(int fd)
{
	HashFileHeader hfh;
	readHashFileHeader(fd,&hfh);
	if(hfh.current_rec_num <hfh.total_rec_num)
		return 0;
	else
		return 1;
}
































