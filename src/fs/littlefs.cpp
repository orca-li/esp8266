#include "../../inc/esphdrs.h"

#define PUBLIC
#define MAX_PATH 256

static char currentDir[MAX_PATH] = "/";

static bool dirExists(const char *path)
{
    Dir dir = LittleFS.openDir(path);
    return dir.next() || LittleFS.exists(path);
}

PUBLIC bool fschdir(const char *newDir)
{
    char resolvedPath[MAX_PATH];

    if (newDir[0] == '/')
    {
        strncpy(resolvedPath, newDir, MAX_PATH - 1);
    }
    else
    {
        snprintf(resolvedPath, MAX_PATH, "%s%s%s",
                 currentDir,
                 (currentDir[strlen(currentDir) - 1] == '/' ? "" : "/"),
                 newDir);
    }

    if (dirExists(resolvedPath))
    {
        strncpy(currentDir, resolvedPath, MAX_PATH - 1);
        return true;
    }
    return false;
}

PUBLIC const char *fscurrdir(void)
{
    return currentDir;
}

PUBLIC void fslist(const char *dirname)
{
    if ((strcmp(dirname, ".") == 0) && (strcmp(currentDir, "/") != 0))
    {
        dirname = currentDir;
        printf(". .. ");
    }

    File root = LittleFS.open(dirname, "r");

    while (File file = root.openNextFile())
    {
        printf("%s ", file.name());
        fflush(stdout);
        file.close();
    }
    printf("\n");
    root.close();
}

#define FREAD_BUFFER_SIZE 4096
PUBLIC size_t fsread(char *filename)
{
    char fullPath[MAX_PATH];
    size_t fsize = 0;
    static char buffer[FREAD_BUFFER_SIZE];

    if (filename[0] == '/')
    {
        strncpy(fullPath, filename, MAX_PATH - 1);
        fullPath[MAX_PATH - 1] = '\0';
    }
    else
    {
        snprintf(fullPath, MAX_PATH, "%s%s%s",
                 currentDir,
                 (currentDir[strlen(currentDir) - 1] == '/' ? "" : "/"),
                 filename);
        fullPath[MAX_PATH - 1] = '\0';
    }

    File file = LittleFS.open(fullPath, "r");

    if (!file)
    {
        printf("(cat) %s: No such file or directory\n", filename);
        return 0;
    }

    fsize = file.read((uint8_t *)buffer, FREAD_BUFFER_SIZE - 1);
    buffer[fsize] = '\0';
    printf("%s", buffer);
    fflush(stdout);
    file.close();

    return fsize;
}

void FsInit(void)
{
    if (!LittleFS.begin())
    {
        printf("LittleFS mount failed\n");
        return;
    }

    FSInfo fsinfo;
    LittleFS.info(fsinfo);
    printf("Total bytes: %d\n", fsinfo.totalBytes);
    printf("Used bytes: %d\n", fsinfo.usedBytes);
    printf("File System Driver init...\n");
}