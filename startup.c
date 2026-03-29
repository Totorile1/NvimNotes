#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <dirent.h>

int main() {
    // Read config file
    FILE *f = fopen("./config.json", "r");
    if (!f) {
        fprintf(stderr, "Could not open config.json\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *data = malloc(size + 1);
    size_t read_bytes = fread(data, 1, size, f);
    if (read_bytes != size) {
        fprintf(stderr, "Failed to read file\n");
        free(data);
        fclose(f);
        return 1;
    }
    data[size] = 0;
    fclose(f);

    // Parse JSON
    cJSON *json = cJSON_Parse(data);
    if (!json) {
        fprintf(stderr, "JSON parse error\n");
        free(data);
        return 1;
    }

    // Get "directory"
    cJSON *dirJsonFormat = cJSON_GetObjectItem(json, "directory");
    
    char *notesDirectoryString = NULL;
    if (dirJsonFormat && cJSON_IsString(dirJsonFormat) && dirJsonFormat->valuestring != NULL) {
        notesDirectoryString = strdup(dirJsonFormat->valuestring);  // independent string
    } else {
        notesDirectoryString = "~/Documents/Notes/";  // default
    }

    // (TODO LATER) it might be a good idea to check if these directories exist
    // (TODO LATER) expand ~ as it does not work with opendir()
    printf("Opening %s\n", notesDirectoryString);

    //---------------------------------------------------------------------------------------
    // https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    struct dirent *notesDirectoryEntry;
    DIR *notesDirectory = opendir(notesDirectoryString);
        if (notesDirectory == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        return 1;
    }
    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while ((notesDirectoryEntry = readdir(notesDirectory)) != NULL)
            printf("%s\n", notesDirectoryEntry->d_name);


    //--------------------------------------------------------------------------------------

    // list all the directories (except hidden ones). They are like obsidian vaults

    // Cleanup
    if (dirJsonFormat && notesDirectoryString != dirJsonFormat->valuestring) free(notesDirectoryString); // only free strdup
    closedir(notesDirectory);
    cJSON_Delete(json);
    free(data);

    return 0;
}
