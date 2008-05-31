/*********************************************************************************/
/* cleanjpg - cleanjpg.cpp                                                       */
/*                                                                               */
/* Programmed by Kevin Pickell, 2006                                             */
/*                                                                               */
/* http://code.google.com/p/cleanjpg/                                            */
/*                                                                               */
/*    cleanjpg is free software; you can redistribute it and/or modify           */
/*    it under the terms of the GNU Lesser General Public License as published by*/
/*    the Free Software Foundation; version 2.                                   */
/*                                                                               */
/*    cleanjpg is distributed in the hope that it will be useful,                */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/*    GNU General Public License for more details.                               */
/*                                                                               */
/*    http://www.gnu.org/licenses/lgpl.txt                                       */
/*                                                                               */
/*    You should have received a copy of the GNU General Public License          */
/*    along with cleanjpg; if not, write to the Free Software                    */
/*    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */
/*                                                                               */
/*********************************************************************************/

/****************************************/
/* what it does:                        */
/*                                      */
/* iterates through a given directory	*/ 
/* removed all unnecessary data blocks	*/
/* from jpg pictures					*/
/*                                      */
/****************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#else

#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>

//#include <linux/types.h>
//#include <linux/unistd.h>

#endif

/* settings */
bool recursive=true;

char *loadfilez(const char *filename,int *filesize)
{
	FILE *fp;
	char *filemem;
	int count,fs;

	if(filesize)
		*(filesize)=0;	/* if there is any errors, then filesize=0 */
	
	fp=fopen(filename,"rb");
	
	if(!fp)			/* error! */
	{
		printf("Error: (OPEN) Unable to open input file '%s'\n",filename);
		return(0);
	}
	if (fseek (fp,0l,SEEK_END))
	{
		printf("Error: (SEEK) Unable to open input file '%s'",filename);
		return (0);
	}
	fs=(int)ftell(fp);
	if(!fs)
	{
		printf("Error: (FTELL) Unable to get file size, or empty file '%s'",filename);
	}
	filemem=(char *)malloc(fs);	/* let's allocate the ram to load the file in! */
	if(!filemem)
	{
		printf("Error: (ALLOC) Unable allocate memory '%ld'",fs);
		fclose(fp);
		return(0);		/* not enough ram to load the file in! */
	}
	if (fseek (fp,0l,SEEK_SET))
	{
		printf("Error: (SEEK) Unable to open input file '%s'",filename);
		free(filemem);
		return (0);
	}
	count = (int)fread (filemem,1,fs,fp);	/* read in the file */
	fclose(fp);
	if(fs!=count)
	{
		free(filemem);
		printf("Error: (COUNT MISMATCH) Unable to open input file '%s',%ld!=%ld",filename,count,fs);
		free(filemem);
		return(0);
	}		

	/* everything was ok! */
	if(filesize)
		*(filesize)=fs;
	return(filemem);
}

void dofunc(const char *filename)
{
	unsigned char *pstart;
	unsigned char *pend;
	unsigned char *cp;
	int psize;
	FILE *out;
	int endblocks;
	int skip;

	pstart=(unsigned char *)loadfilez(filename,&psize);
	if(!pstart)
		return;
	pend=pstart+psize;

	/* traverse blocks */
	cp=pstart;
	if(cp[0]==0xff && cp[1]==0xd8)	/* valid header for a JPG file */
	{
		printf("file='%s'\n",filename);
		out=fopen(filename,"wb");
		fwrite(cp,1,2,out);
		cp+=2;
		endblocks=0;
		while(cp<pend && endblocks==0)
		{
			unsigned int id;
			int blocklen;

			/* get block ID and block length */
			id=(cp[0]<<8)|cp[1];			
			blocklen=2+(((unsigned int)cp[2]<<8)+cp[3]);

			if(id>=0xffe0 && id<=0xfff0)
				skip=1;
			else
				skip=0;

			if(!skip)
				fwrite(cp,1,blocklen,out);

			printf("  block id=%04x - len=%d",id,blocklen);
			switch(id)
			{
			case 0xffe0:
				printf(" - APP0 Application Marker");
			break;
			case 0xffdb:
				printf(" - DQT Quantization Table");
			break;
			case 0xffc0:
				printf(" - SOF0 Start of frame");
			break;
			case 0xffc4:
				printf(" - DHT Define Huffman Table");
			break;
			case 0xffda:
				printf(" - SOS Start of scan");
				endblocks=1;
				//no more packets
			break;
			case 0xffed:
				printf(" - APP14 This is the marker where Photoshop stores its information");
			break;
			}
			printf("\n");
			cp+=blocklen;
		}

		/* write remainder of file (image data) */
		fwrite(cp,1,(pend-cp),out);
		printf("	imagedatasize=%d\n",pend-cp);

		fclose(out);
	}
	free(pstart);
}

void LoadDir(const char *path,bool recursive,const char *ext);

int main(int argc, char **argv)
{
	bool recursive=true;

	printf("cleanjpg 1.0 - remove all non essential data from jpg files!\n");
	if(argc!=2)
		printf("usage: cleanjpg path\n");
	else
		LoadDir(argv[1],recursive,".jpg");
	return 0;
}

#ifdef WIN32
#define DIRCHAR "\\"
#else
#define DIRCHAR "/"
#endif

void LoadDir(const char *path,bool recursive,const char *ext)
{
	//arbitrary max filename length
	char tempname[2048];
	char *nameplace;
	char *fn;
	bool isdir;
	char dirc[]={DIRCHAR};
#ifdef WIN32
	intptr_t dir_handle;
	struct _finddata_t  dir_info;
#else
	int dir_handle;
	int bufsize;
	char buffer[8192];
	struct direct *direntry;
	struct stat statbuf;
#if _FILE_OFFSET_BITS==64
	long long zzzz;
#else
	long zzzz;
#endif
#endif

#ifdef WIN32
	strcpy(tempname,path);
	if(strlen(tempname)==0)
		strcat(tempname,DIRCHAR);
	else if(tempname[strlen(tempname)-1]!=dirc[0])
		strcat(tempname,DIRCHAR);
	nameplace=tempname+strlen(tempname);
	strcat(tempname,"*");
	dir_handle = _findfirst(tempname, &dir_info);
	if(dir_handle<0)
		return;			/* path must be unmounted or something is wrong! */
	fn=dir_info.name;
#else
	strcpy(tempname,path);
	if(strlen(tempname)>1)
	{
		//remove trailing '/'
		if(tempname[strlen(tempname)-1]==dirc[0])
			tempname[strlen(tempname)-1]=0;
	}
	dir_handle=open(tempname,O_RDONLY);				/* open directory for reading */
	if(dir_handle==-1)
		return;	/* cannot open dir */
	direntry=(struct direct *)buffer;
	fn=direntry->d_name;
	bufsize=0;

	// if path doesn't end in '/' then append '/'
	if(tempname[strlen(tempname)-1]!=dirc[0])
		strcat(tempname,DIRCHAR);
	nameplace=tempname+strlen(tempname);
#endif
	do
	{
#ifndef WIN32
		if(bufsize<=20)
			bufsize+=getdirentries(dir_handle,buffer+bufsize,sizeof(buffer)-bufsize,&zzzz);
		if(bufsize<=0)
			break;
#endif
		if(fn[0]!='.')
		{
			strcpy(nameplace,fn);	/* put name after path/ */

			isdir=false;
#ifdef WIN32
			if(dir_info.attrib&16)
				isdir=true;
#else
			if(stat(tempname,&statbuf)!=-1)
			{
				if(S_ISDIR(statbuf.st_mode))
					isdir=true;
			}
#endif
			if(isdir)
			{
				if(recursive==true)
					LoadDir(tempname,true,ext);
			}
			else
			{
				bool addme;

				/* if no extension then match everything */
				addme=true;
				if(ext)
				{
					addme=false;
					/* make sure extension matches */
					if(strlen(fn)>strlen(ext))
					{
						const char *eplace;
					
						/* compare the extension */
						eplace=fn+strlen(fn)-strlen(ext);
						if(!stricmp(eplace,ext))
							addme=true;
					}
				}
				/* call the processor on this file */
				if(addme==true)
					dofunc(tempname);
			}
		}
#ifdef WIN32
		if(_findnext(dir_handle, &dir_info)==-1)
			break;
#else
		bufsize-=direntry->d_reclen;
		memmove(buffer,buffer+direntry->d_reclen,bufsize);
#endif
	}while(1);
#ifdef WIN32
	_findclose(dir_handle);
#else
	close(dir_handle);
#endif
}
