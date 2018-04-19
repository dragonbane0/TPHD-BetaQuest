#ifndef _DISCDUMPER_H_
#define _DISCDUMPER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char tag;
    unsigned int length;
    unsigned char data[0];
} __attribute__((packed)) SendData;

int start_saviine(int argc, void *argv);
long getUserID(unsigned char * slotno);
void Handle_Connection(void *pClient, void *pCmd,int error);
void dumpSavaData(void *pClient, void *pCmd,long persistentID,int error);
int dump_dir(void *pClient, void *pCmd, char *path, void * pBuffer, int size,int error, int handle);
void injectSaveData(void *pClient, void *pCmd,long persistentID,int error);
int injectFiles(void *pClient, void *pCmd, char * path,char * relativepath, char * basepath, char *  pBuffer, int buffer_size, int error);
int doInjectForFile(void * pClient, void * pCmd,int handle,char * filepath,int filesize, char * basepath,void * pBuffer,int buf_size);
void doFlushOrRollback(void *pClient, void *pCmd,int result,char *savepath);
void init_Save(unsigned char slotNo);
int remove_files_in_dir(void * pClient,void * pCmd, char * path, int handle);


#ifdef __cplusplus
}
#endif

#endif
