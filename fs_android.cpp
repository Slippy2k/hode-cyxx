
#define LOG_TAG "HodJni"

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <SDL.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fs.h"

static AAssetManager *_assetManager;

static int asset_readfn(void *cookie, char *buf, int size) {
	AAsset *asset = (AAsset *)cookie;
	return AAsset_read(asset, buf, size);
}

static int asset_writefn(void *cookie, const char *buf, int size) {
	return -1;
}

static fpos_t asset_seekfn(void *cookie, fpos_t offset, int whence) {
	AAsset *asset = (AAsset *)cookie;
	return AAsset_seek(asset, offset, whence);
}

static int asset_closefn(void *cookie) {
	AAsset *asset = (AAsset *)cookie;
	AAsset_close(asset);
	return 0;
}

void android_setAssetManager(AAssetManager *assetManager) {
	_assetManager = assetManager;
}

FILE *android_fopen(const char *fname, const char *mode) {
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "android_fopen '%s' mode '%s'", fname, mode);
	assert(mode[0] == 'r');
	if (0) { // support for gamedata files installed on external storage
		struct stat st;
		if (stat(fname, &st) == 0 && S_ISREG(st.st_mode)) {
			return fopen(fname, mode);
		}
	}
	AAsset *asset = AAssetManager_open(_assetManager, fname, AASSET_MODE_STREAMING);
	if (asset) {
		FILE *fp = funopen(asset, asset_readfn, asset_writefn, asset_seekfn, asset_closefn);
		return fp;
	}
	return fopen(fname, mode);
}

char **android_listAssets(int *filesCount) {
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "android_listAssets");
	int count = 0;
	char **fnames = 0;
	static const char *assetsDirs[] = { "", "DATA", "DATA/SOUND", 0 };
	for (int i = 0; assetsDirs[i]; ++i) {
		AAssetDir *dir = AAssetManager_openDir(_assetManager, assetsDirs[i]);
		__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "android_listAssets dir '%s' %p", assetsDirs[i], dir);
		if (dir) {
			const bool prependDir = strlen(assetsDirs[i]) > 0;
			const char *fname;
			while ((fname = AAssetDir_getNextFileName(dir)) != 0) {
				fnames = (char **)realloc(fnames, (count + 1) * sizeof(char *));
				if (fnames) {
					if (prependDir) {
						char filepath[MAXPATHLEN];
						snprintf(filepath, sizeof(filepath), "%s/%s", assetsDirs[i], fname);
						fnames[count] = strdup(filepath);
					} else {
						fnames[count] = strdup(fname);
					}
					++count;
				}
				__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "android_listAssets '%s' count %d", fname, count);
			}
			AAssetDir_close(dir);
		}
	}
	*filesCount = count;
	return fnames;
}

//
// FileSystem
//

FileSystem::FileSystem(const char *dataPath) {
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();

	jmethodID methodGetAssets = env->GetMethodID(env->GetObjectClass(activity), "getAssets", "()Landroid/content/res/AssetManager;");
	assert(methodGetAssets);

	jobject localAssetManager = env->CallObjectMethod(activity, methodGetAssets);
	assert(localAssetManager);

	jobject globalAssetManager = env->NewGlobalRef(localAssetManager);
	assert(globalAssetManager);

	_assetManager = AAssetManager_fromJava(env, globalAssetManager);
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "dataPath %s _assetManager %p", dataPath, _assetManager);
}

FileSystem::~FileSystem() {
}

FILE *FileSystem::openFile(const char *filename) {
	char *name = (char *)alloca(strlen(filename) + 1);
	for (int i = 0; i < strlen(filename) + 1; ++i) {
		if (filename[i] >= 'A' && filename[i] <= 'Z') {
			name[i] = 'a' + (filename[i] - 'A');
		} else {
			name[i] = filename[i];
		}
	}
	FILE *fp = android_fopen(name, "rb");
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "android_fopen %s %p", name, fp);
	return fp;
}

void FileSystem::closeFile(FILE *fp) {
	if (fp) {
		fclose(fp);
	}
}
