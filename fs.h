
#ifndef FS_H__
#define FS_H__

#include "fileio.h"

struct FileSystem {

	const char *_dataPath;
	int _filesCount;
	char **_filesList;

	FileSystem(const char *dataPath);
	~FileSystem();

	bool openFile(const char *filename, File *);
	void closeFile(File *);

	void addFilePath(const char *path);
	void listFiles(const char *dir);
};

#endif // FS_H__
