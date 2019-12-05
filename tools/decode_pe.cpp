
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winver.h>

static void dumpVersionInfo(const char *filepath) {
	DWORD handle;
	DWORD versionInfoSize = GetFileVersionInfoSize(filepath, &handle);
	if (versionInfoSize != 0) {
		VOID *versionInfoData = malloc(versionInfoSize);
		if (GetFileVersionInfo(filepath, handle, versionInfoSize, versionInfoData)) {

			CHAR languageEntry[512];
			// get language ID
			UINT bufLen;
			VOID *buf = 0;
			if (VerQueryValue(versionInfoData, "\\VarFileInfo\\Translation", &buf, &bufLen) && bufLen == 4) {
				DWORD languageID = *(DWORD *)buf;
				snprintf(languageEntry, sizeof(languageEntry), "%02X%02X%02X%02X", (languageID >> 8) & 0xFF, languageID & 0xFF, (languageID >> 24) & 0xFF, (languageID >> 16) & 0xFF);
			} else {
				snprintf(languageEntry, sizeof(languageEntry), "%04X04B0", GetUserDefaultLangID());
			}

			// get fields
			static const char *names[] = {
				"Comments",
				"CompanyName",
				"FileDescription",
				"FileVersion",
				"InternalName",
				"LegalCopyright",
				"LegalTrademarks",
				"OriginalFilename",
				"PrivateBuild",
				"ProductName",
				"ProductVersion",
				"SpecialBuild",
				0
			};
			for (int i = 0; names[i]; ++i) {
				CHAR entryName[512];
				snprintf(entryName, sizeof(entryName), "\\StringFileInfo\\%s\\%s", languageEntry, names[i]);
				if (VerQueryValue(versionInfoData, entryName, &buf, &bufLen)) {
					fprintf(stdout, "%s: %s\n", names[i], (CHAR *)buf);
				}
			}
		}
		free(versionInfoData);
	}
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		dumpVersionInfo(argv[1]);
	}
	return 0;
}
