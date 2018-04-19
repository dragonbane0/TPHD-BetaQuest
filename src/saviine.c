#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "common/kernel_defs.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/aoc_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "game/rpx_rpl_table.h"
#include "game/memory_area_table.h"
#include "utils/net.h"
#include "saviine.h"
#include "utils/logger.h"

#define BUFFER_SIZE                 0x400 * 101
#define BUFFER_SIZE_STEPS           0x200

struct pygecko_bss_t {
	int error, line;
	void* thread;
	unsigned char stack[0x8000];
};

#define CHECK_ERROR(cond) if (cond) { bss->line = __LINE__; goto error; }

static int socket_saviine = -1;
static int isTWWHD = 0;

int start_saviine(int argc, void *argv)
{
	void *pFSClient = NULL;
	void *pFSCmd = NULL;

	pFSClient = malloc(FS_CLIENT_SIZE);
	if (!pFSClient)
		return 0;

	pFSCmd = malloc(FS_CMD_BLOCK_SIZE);
	if (!pFSCmd)
		return 0;

	FSInit();
	FSInitCmdBlock(pFSCmd);

	FSAddClientEx(pFSClient, 0, -1);

	if (OSGetTitleID() != 0 && (OSGetTitleID() == 0x0005000010143500 || OSGetTitleID() == 0x0005000010143600 || OSGetTitleID() == 0x0005000010143400)) //TWW HD is running
		isTWWHD = 1;
	else
		isTWWHD = 0;

	while (1)
	{
		int sockfd = -1, ret = 0, len;
		struct sockaddr_in addr;
		struct pygecko_bss_t *bss = argv;

		addr.sin_family = AF_INET;
		addr.sin_port = 7333; //Saviine uses 7333
		addr.sin_addr.s_addr = 0;
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //open a file handle to socket
		CHECK_ERROR(sockfd == -1);
		ret = bind(sockfd, (void *)&addr, 16);
		CHECK_ERROR(ret < 0);
		ret = listen(sockfd, 20);
		CHECK_ERROR(ret < 0);

		while (1) 
		{
			len = 16;
			socket_saviine = accept(sockfd, (void *)&addr, &len);
			CHECK_ERROR(socket_saviine == -1);

			log_printf("Client connected\n");

			//Do handshake
			CHECK_ERROR(cafiine_handshake(socket_saviine) == -1);

			log_printf("Handshake passed\n");

			Handle_Connection(pFSClient, pFSCmd, -1);

			log_printf("Connection closed\n");

			socketclose(socket_saviine);
			socket_saviine = -1;
		}

		socketclose(sockfd);
		sockfd = -1;

		error:
			if (socket_saviine != -1)
				socketclose(socket_saviine);
			if (sockfd != -1)
				socketclose(sockfd);
			bss->error = ret;	
	}
	
	FSDelClient(pFSClient);

	if (pFSCmd)
		free(pFSCmd);
	if (pFSClient)
		free(pFSClient);

	if (socket_saviine >= 0)
	{
		cafiine_disconnect(socket_saviine);
		socket_saviine = -1;
	}

	return 0;
}

void Handle_Connection(void *pClient, void *pCmd, int error)
{
	log_printf("Handle a Connection");

	unsigned char slotNo;
	long id = getUserID(&slotNo);

	if ((unsigned int)id >= 0x80000000 && (unsigned int)id <= 0x90000000) //Checks for valid user ID
	{
		init_Save(slotNo);
		int mode = 0;

		while (1)
		{
			log_printf("Getting Mode..");

			mode = getMode(socket_saviine);

			if (mode == -1)
			{
				log_printf("Getting Mode failed!");
				break;
			}

			log_printf("Received Mode: %i", mode);

			if (mode == 1) //1 for dumping, 2 for injecting
			{
				log_printf("Dump Mode!");
				dumpSavaData(pClient, pCmd, id, error);
			}
			else if (mode == 2)
			{
				log_printf("Inject Mode!");
				injectSaveData(pClient, pCmd, id, error);
			}
		}
	}
}

void dumpSavaData(void *pClient, void *pCmd,long persistentID,int error)
{
	//Allocate buffer for dump
	int buf_size = BUFFER_SIZE;
 	char * pBuffer;
	int failed = 0;

	do
	{
		buf_size -= BUFFER_SIZE_STEPS;
		if(buf_size < 0)
		{
			log_printf("Error on buffer allocation");
			failed = 1;
			break;
		}
		pBuffer = (char *)memalign(0x40,buf_size);

		if(pBuffer) 
			memset(pBuffer, 0x00, buf_size);
	}
	while(!pBuffer);

	if(!failed)
	{
		//char buffer[60];
		//__os_snprintf(buffer, sizeof(buffer), "Allocated %d bytes", buf_size);
		//log_printf(buffer);

		char savepath[20];

		__os_snprintf(savepath, sizeof(savepath), "/vol/save/%08x", persistentID);
		log_printf("Dumping User Savedata");

		if (dump_dir(pClient, pCmd, savepath, pBuffer, buf_size, error, 50) == -1)
		{
			log_printf("Error Dumping User Dir");
		}
		else
		{
			//TWW HD: Dump additional files from memory
			if (isTWWHD == 1)
			{
				int ret = 0;
				int size_contents = 8966;
				int size_info = 180;
				int my_handle = 0;

				const unsigned char *MemoryCardPointerPtr = (const unsigned char *)0x1096346C; //In newest Gecko: 0x14FBE370
				unsigned int MemoryCardPointer;

				//Get Memory Card Pointer
				memcpy(&MemoryCardPointer, MemoryCardPointerPtr, 4);

				if (MemoryCardPointer > 0x10000000)
				{
					unsigned char *ContentsPtr = (unsigned char *)MemoryCardPointer + 0x290 + 0x40; //14FBE600 //+0x40 in ZE 5.0
					unsigned char *HeaderPtr = (unsigned char *)(MemoryCardPointer + 0xAF210) + 0x1340; //1506D880

					log_printf("Dump additional files from memory for TWW HD");

					my_handle = 0x00000011;
					int my_ret = cafiine_send_handle(socket_saviine, "SaveContents.bin", my_handle);

					if (my_ret != -1)
					{
						log_printf("Send additional file handle and created file on PC");

						memcpy(pBuffer, ContentsPtr, size_contents);

						cafiine_send_file(socket_saviine, pBuffer, size_contents, my_handle);

						log_printf("Entire file send");

						cafiine_fclose(socket_saviine, &ret, my_handle, 1);

						my_handle = 0x00000012;
						int my_ret = cafiine_send_handle(socket_saviine, "SaveInfo.bin", my_handle);

						if (my_ret != -1)
						{
							log_printf("Send additional file handle and created file on PC");

							memcpy(pBuffer, HeaderPtr, size_info);

							cafiine_send_file(socket_saviine, pBuffer, size_info, my_handle);

							log_printf("Entire file send");

							cafiine_fclose(socket_saviine, &ret, my_handle, 1);
						}
						else
						{
							log_printf("Error on creating additional file on PC");
						}
					}
					else
					{
						log_printf("Error on creating additional file on PC");
					}
				}
			}
		}

		if (!saviine_end_dump(socket_saviine))
			log_printf("saviine_end_dump() failed!");

		log_printf("End of Dump");
		
		free(pBuffer);
	}
}

int dump_dir(void *pClient, void *pCmd, char *path, void * pBuffer, int size,int error, int handle)
{
	int dir_handle = handle;
	int my_handle = handle +1;
	int ret = 0;
	int final_result = 0;

	if ((ret = FSOpenDir(pClient, pCmd, path, &dir_handle, FS_RET_ALL_ERROR)) == FS_STATUS_OK)
	{
		char buffer[strlen(path) + 30];

		__os_snprintf(buffer, sizeof(buffer), "Open dir %s",path);
		log_printf(buffer);

		FSDirEntry dir_entry;

		while (FSReadDir(pClient,  pCmd, dir_handle, &dir_entry, FS_RET_ALL_ERROR) == FS_STATUS_OK && final_result == 0)
		{
			char full_path[strlen(path) + 1 + strlen(dir_entry.name) +1];
			__os_snprintf(full_path, sizeof(full_path), "%s/%s",path,dir_entry.name);

			if((dir_entry.stat.flag & FS_STAT_FLAG_IS_DIRECTORY) == FS_STAT_FLAG_IS_DIRECTORY)
			{
				log_printf("-> dir");

				if(dump_dir(pClient, pCmd,full_path,pBuffer,size,error,my_handle) == -1)
				{
					log_printf("Error");
					final_result = -1;
				}
			}
			else
			{
				//DUMP

				//TWW HD: Only dump cking.sav and cking_playlog.sav
				if (isTWWHD == 1)
				{
					if (strcmp(dir_entry.name, "cking.sav") && strcmp(dir_entry.name, "cking_playlog.sav"))
						continue;
				}

				ret = FSOpenFile(pClient,  pCmd, full_path, "r", &my_handle, error);
				if (ret >= 0) 
				{
					__os_snprintf(buffer, sizeof(buffer), "Dumping %s",dir_entry.name);
					log_printf(buffer);

					int ret2;

					int  my_ret = cafiine_send_handle(socket_saviine, full_path, my_handle);
					
					if(my_ret != -1)
					{
						log_printf("Send file handle and created file on PC");

						while ((ret2 = FSReadFile(pClient, pCmd, pBuffer, 1, size, my_handle, 0, 0)) > 0)
						{
							cafiine_send_file(socket_saviine, pBuffer, ret2, my_handle);
						}

						log_printf("Entire file send");

						cafiine_fclose(socket_saviine, &ret2, my_handle,1);
					}
					else
					{
						log_printf("Error on opening file on PC");
						final_result = -1;
					}
					if((ret2 = FSCloseFile(pClient,  pCmd, my_handle, error)) < FS_STATUS_OK)
					{
						__os_snprintf(buffer, sizeof(buffer), "Error on FSOpenFile: %d",ret2);
						log_printf(buffer);
					}
				}
				else
				{
					__os_snprintf(buffer, sizeof(buffer), "Error on FSOpenFile: %d",ret);
					log_printf(buffer);
					final_result = -1;
				}
			}
		}
		if(FSCloseDir(pClient,  pCmd, dir_handle, error) <  FS_STATUS_OK)
		{
			log_printf("Error on FSCloseDir()");
		}
	}
	else
	{
		log_printf("Error on FSOpenDir()");
		final_result = -1;
	}
	return final_result;
}

/**************************
	Injection functions
**************************/

void injectSaveData(void *pClient, void *pCmd,long persistentID,int error)
{
	//Allocate buffer for injection
	char logbuffer[255];

	int buf_size = BUFFER_SIZE;
 	char * pBuffer;
	int failed = 0;
	do
	{
		buf_size -= BUFFER_SIZE_STEPS;
		if(buf_size < 0)
		{
			log_printf("Error on buffer Allocation");
			failed = 1;
			break;
		}
		pBuffer = (char *)memalign(0x40,buf_size);

		if(pBuffer) 
			memset(pBuffer, 0x00, buf_size);
	}
	while(!pBuffer);

	if (!failed)
	{
		//char buffer[60];
		//__os_snprintf(buffer, sizeof(buffer), "allocated %d bytes", buf_size);
		//log_printf(buffer);

		int result = 0;
		char savepath[20];

		__os_snprintf(savepath, sizeof(savepath), "/vol/save/%08x", persistentID);
		__os_snprintf(logbuffer, sizeof(logbuffer), "Injecting new Userdata in %08x", persistentID);
		log_printf(logbuffer);

		if (remove_files_in_dir(pClient, pCmd, savepath, 0) == 0)
		{
			//Inject Save
			result = injectFiles(pClient, pCmd, savepath, "/", savepath, pBuffer, buf_size, error);
			doFlushOrRollback(pClient, pCmd, result, savepath);
		}

		//TWW HD: Inject additional files into memory
		if (isTWWHD == 1)
		{
			int myhandle;
			int ret = 0;

			const unsigned char *MemoryCardPointerPtr = (const unsigned char *)0x1096346C; //In newest Gecko: 0x14FBE370
			unsigned int MemoryCardPointer;

			//Get Memory Card Pointer
			memcpy(&MemoryCardPointer, MemoryCardPointerPtr, 4);

			if (MemoryCardPointer > 0x10000000)
			{
				unsigned char *ContentsPtr = (unsigned char *)MemoryCardPointer + 0x290 + 0x40; //+0x40 in ZE 5.0
				unsigned char *HeaderPtr = (unsigned char *)(MemoryCardPointer + 0xAF210) + 0x1340;

				log_printf("Inject additional files into memory for TWW HD");

				if ((cafiine_fopen(socket_saviine, &ret, "SaveContents.bin", "r", &myhandle)) == 0 && ret == FS_STATUS_OK)
				{
					int retsize = 0;

					log_printf("Opened SaveContents.bin");

					if (cafiine_fread(socket_saviine, &retsize, pBuffer, buf_size, myhandle) == FS_STATUS_OK)
					{
						memcpy(ContentsPtr, pBuffer, retsize);
						DCFlushRange(ContentsPtr, retsize);

						log_printf("Flushed SaveContents.bin");

						if ((cafiine_fopen(socket_saviine, &ret, "SaveInfo.bin", "r", &myhandle)) == 0 && ret == FS_STATUS_OK)
						{
							log_printf("Opened SaveInfo.bin");

							if (cafiine_fread(socket_saviine, &retsize, pBuffer, buf_size, myhandle) == FS_STATUS_OK)
							{
								//Copy only relevant sections to preserve internal pointer consistency
								memcpy(HeaderPtr + 0x4, pBuffer + 0x4, 0x1C);
								DCFlushRange(HeaderPtr + 0x4, 0x1C);

								memcpy(HeaderPtr + 0x24, pBuffer + 0x24, 0x1C);
								DCFlushRange(HeaderPtr + 0x24, 0x1C);

								memcpy(HeaderPtr + 0x44, pBuffer + 0x44, 0x1C);
								DCFlushRange(HeaderPtr + 0x44, 0x1C);

								memcpy(HeaderPtr + 0x64, pBuffer + 0x64, 0x1C);
								DCFlushRange(HeaderPtr + 0x64, 0x1C);

								memcpy(HeaderPtr + 0x84, pBuffer + 0x84, 0x1C);
								DCFlushRange(HeaderPtr + 0x84, 0x1C);

								memcpy(HeaderPtr + 0xA4, pBuffer + 0xA4, 0x10);
								DCFlushRange(HeaderPtr + 0xA4, 0x10);


								log_printf("Flushed SaveInfo.bin");
							}
							else
							{
								log_printf("FRead Error on PC");
							}
						}
						else
						{
							log_printf("FOpen Error on PC");
						}
					}
					else
					{
						log_printf("FRead Error on PC");
					}
				}
				else
				{
					log_printf("FOpen Error on PC");
				}
			}
		}

		if (!saviine_end_injection(socket_saviine))
			log_printf("saviine_end_injection() failed");

		log_printf("End of Injection");
	}

	free(pBuffer);
}

int injectFiles(void *pClient, void *pCmd, char * path,char * relativepath, char * basepath, char *  pBuffer, int buffer_size, int error)
{
	int failed = 0;
	int filesinjected = 0;
	int type = 0;

	char namebuffer[255];
	char logbuffer[255];
	int filesize = 0;

	if(!failed)
	{
		log_printf("injecting Files");

		while(saviine_readdir(socket_saviine,path,namebuffer, &type,&filesize) && !failed)
		{
			char newpath[strlen(path) + 1 + strlen(namebuffer) + 1];
			__os_snprintf(newpath, sizeof(newpath), "%s/%s",path,namebuffer);

			if(type == BYTE_FILE)
			{
				__os_snprintf(logbuffer, sizeof(logbuffer), "File: %s%s size: %d",relativepath,namebuffer,filesize);
				log_printf(logbuffer);

				__os_snprintf(logbuffer, sizeof(logbuffer), "Newpath: %s ",newpath);
				log_printf(logbuffer);

				int handle = 10;

				if(FSOpenFile(pClient, pCmd, newpath,"w+",&handle,error) >= FS_STATUS_OK)
				{
					if(filesize > 0)
					{
						failed = doInjectForFile(pClient,pCmd,handle,newpath,filesize,basepath,pBuffer,buffer_size);

						if(failed == 2) // trying it again if the journal was full
							failed = doInjectForFile(pClient,pCmd,handle,newpath,filesize,basepath,pBuffer,buffer_size);
					}
					else
					{
						log_printf("Filesize is 0");
					}

					if((FSCloseFile (pClient, pCmd, handle, error)) < FS_STATUS_OK)
					{
						log_printf("FSCloseFile failed");
						failed = 1;
					}
				}
				else
				{
					log_printf("Opening the file failed");
					failed = 1;
				}

				if(!failed) 
					filesinjected++;
			}
			else if( type == BYTE_FOLDER)
			{
				__os_snprintf(logbuffer, sizeof(logbuffer), "Dir: %s",namebuffer);
				log_printf(logbuffer);

				log_printf(newpath);

				int ret = 0;
				if((ret = FSMakeDir(pClient, pCmd, newpath, -1)) == FS_STATUS_OK || ret == FS_STATUS_EXISTS )
				{
					char op_offset[strlen(relativepath) + strlen(namebuffer)+ 1 + 1];
					__os_snprintf(op_offset, sizeof(op_offset), "%s%s/",relativepath,namebuffer);
					int injectedsub = injectFiles(pClient, pCmd, newpath,op_offset,basepath,pBuffer,buffer_size,error);

					if(injectedsub == -1)
					{
						failed = 1;
					}
					else
					{
						filesinjected += injectedsub;
					}
				}
				else
				{
					log_printf("Folder creation failed");
					failed = 1;
				}
			}
		}

		if(failed) 
			return -1;
		else 
			return filesinjected;
	}
	else
	{
		return -1;
	}
}

int doInjectForFile(void * pClient, void * pCmd,int handle,char * filepath,int filesize, char * basepath,void * pBuffer,int buf_size)
{
	int failed = 0;
	int myhandle;
	int ret = 0;
	char logbuffer[255];

	//__os_snprintf(logbuffer, sizeof(logbuffer), "cafiine_fopen %s",filepath);
	//log_printf(logbuffer);

	if((cafiine_fopen(socket_saviine, &ret, filepath, "r", &myhandle)) == 0 && ret == FS_STATUS_OK)
	{
		//__os_snprintf(logbuffer, sizeof(logbuffer), "cafiine_fopen with handle %d",myhandle);
		//log_printf(logbuffer);
		
		int retsize = 0;
		int pos = 0;
		while (pos < filesize)
		{
			if (cafiine_fread(socket_saviine, &retsize, pBuffer, buf_size, myhandle) == FS_STATUS_OK)
			{
				int fwrite = 0;
				if ((fwrite = FSWriteFile(pClient, pCmd, pBuffer, sizeof(char), retsize, handle, 0, 0x0200)) >= FS_STATUS_OK)
				{

				}
				else
				{
					if (fwrite == FS_STATUS_JOURNAL_FULL || fwrite == FS_STATUS_STORAGE_FULL)
					{
						//log_printf("journal or storage is full, flushing it now.");

						if (FSFlushQuota(pClient, pCmd, basepath, FS_RET_ALL_ERROR) == FS_STATUS_OK)
						{
							//log_printf("success");
							failed = 2;
							break;
						}
						else
						{
							//log_printf("failed");
							failed = 1;
							break;
						}

					}
					__os_snprintf(logbuffer, sizeof(logbuffer), "my_FSWriteFile failed with error: %d", fwrite);
					log_printf(logbuffer);

					failed = 1;
					break;
				}
				
				pos += retsize;
			}
			else
			{
				log_printf("Error while receiving file");
				failed = 1;
				break;
			}
		}

		int result = 0;

		if ((cafiine_fclose(socket_saviine, &result, myhandle, 0)) == 0 && result == FS_STATUS_OK)
		{

		}
		else
		{

		}
	}
	else
	{
		log_printf("Cafiine_fopen failed");
		failed = 1;
	}
	return failed;
}

/*************************
	Util functions
**************************/

/*flush if result != -1*/

void doFlushOrRollback(void *pClient, void *pCmd,int result,char *savepath)
{
	char logbuffer[50 + strlen(savepath)];

	if(result != -1)
	{
		__os_snprintf(logbuffer, sizeof(logbuffer), "Injected %d files",result);
		log_printf(logbuffer);
		log_printf("Flushing data now");

		if(FSFlushQuota(pClient,pCmd,savepath,FS_RET_ALL_ERROR) == FS_STATUS_OK)
		{
			//log_printf("success");
		}
		else
		{
			//log_printf("failed");
		}
	}
	else
	{
		log_printf("Injection failed, trying to restore the data");

		if(FSRollbackQuota(pClient,pCmd,savepath,FS_RET_ALL_ERROR) == FS_STATUS_OK)
		{
			log_printf("Rollback done");
		}
		else
		{
			log_printf("Rollback failed");
		}
	}
}

void init_Save(unsigned char slotNo)
{
	int (*SAVEInit)();
	int (*SAVEInitSaveDir)(unsigned char accountSlotNo);
	unsigned int save_handle;

	OSDynLoad_Acquire("nn_save.rpl", &save_handle);
	OSDynLoad_FindExport(save_handle, 0, "SAVEInit", (void **)&SAVEInit);
	OSDynLoad_FindExport(save_handle, 0, "SAVEInitSaveDir", (void **)&SAVEInitSaveDir);

	SAVEInit();

	SAVEInitSaveDir(slotNo);

	SAVEInitSaveDir(255U);
}

long getUserID(unsigned char * slotno)
{
	unsigned int nn_act_handle;
	unsigned long (*GetPersistentIdEx)(unsigned char);
	int (*GetSlotNo)(void);
	void (*nn_Initialize)(void);
	void (*nn_Finalize)(void);

	OSDynLoad_Acquire("nn_act.rpl", &nn_act_handle);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetPersistentIdEx__Q2_2nn3actFUc", (void **)&GetPersistentIdEx);
	OSDynLoad_FindExport(nn_act_handle, 0, "GetSlotNo__Q2_2nn3actFv", (void **)&GetSlotNo);
	OSDynLoad_FindExport(nn_act_handle, 0, "Initialize__Q2_2nn3actFv", (void **)&nn_Initialize);
	OSDynLoad_FindExport(nn_act_handle, 0, "Finalize__Q2_2nn3actFv", (void **)&nn_Finalize);

	nn_Initialize(); // To be sure that it is really Initialized

	*slotno = GetSlotNo();

	long idlong = GetPersistentIdEx(*slotno);

	nn_Finalize(); //must be called an equal number of times to nn_Initialize
	return idlong;
}

int remove_files_in_dir(void * pClient,void * pCmd, char * path, int handle)
{
	int ret = 0;
	int my_handle = handle +1;
	char buffer[strlen(path) + 50];

	if ((ret = FSOpenDir(pClient, pCmd, path, &handle, FS_RET_ALL_ERROR)) == FS_STATUS_OK)
	{
		__os_snprintf(buffer, sizeof(buffer), "Remove files in dir %s",path);
		log_printf(buffer);

		FSDirEntry dir_entry;
		while (FSReadDir(pClient,  pCmd, handle, &dir_entry, FS_RET_ALL_ERROR) == FS_STATUS_OK)
		{
			char full_path[strlen(path) + 1 + strlen(dir_entry.name) +1];
			__os_snprintf(full_path, sizeof(full_path), "%s/%s",path,dir_entry.name);

			if((dir_entry.stat.flag & FS_STAT_FLAG_IS_DIRECTORY) == FS_STAT_FLAG_IS_DIRECTORY)
			{
				if(remove_files_in_dir(pClient,pCmd,full_path,my_handle) == -1) 
					return -1;
			}

			//TWW HD: Only delete cking.sav and cking_playlog.sav
			if (isTWWHD == 1)
			{
				if (strcmp(dir_entry.name, "cking.sav") && strcmp(dir_entry.name, "cking_playlog.sav"))
					continue;
			}

			char buffer[strlen(full_path) + 50];

			__os_snprintf(buffer, sizeof(buffer), "Deleting %s",full_path);
			log_printf(buffer);

			if((ret = FSRemove(pClient,pCmd,full_path,FS_RET_ALL_ERROR)) < FS_STATUS_OK)
			{
				__os_snprintf(buffer, sizeof(buffer), "Error: %d on removing %s",ret,full_path);
				log_printf(buffer);
				return -1;
			}

		}
		if((FSCloseDir(pClient,  pCmd, handle, FS_RET_NO_ERROR)) < FS_STATUS_OK )
		{
			log_printf("Error while closing dir");
			return -1;
		}
	}
	else
	{
		__os_snprintf(buffer, sizeof(buffer), "Error: %d on opening %s",ret,path);
		log_printf(buffer);
		return -1;
	}
	return 0;
}