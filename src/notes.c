#include "notes.h"

char **getJournalsFromVault(char *pathToVault, char *vault, char *journalRegex,  int *count, int shouldDebug) {
    debug("Searching %s for journals", vault);
    // originally from https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    struct dirent *vaultEntry;
    char tempPath[PATH_MAX];
    snprintf(tempPath, sizeof(tempPath), "%s/%s", pathToVault, vault); // sets the full absolute path to fullPathEntry
    DIR *vaultDirectory = opendir(tempPath);
    error(vaultDirectory==NULL, "program", "Could not open directory %s", tempPath);
    char **journalsArray = NULL; // will contain all the notes
    size_t journalsCount = 0; // we need to count how many notes there is to always readjust how many memory we alloc
    
    // https://stackoverflow.com/a/1085120 for regex code
    regex_t regex;
    int regexReturn;
    // compiles the regex
    regexReturn = regcomp(&regex, journalRegex, 0);
    error(regexReturn, "program", "Regex could not compile. Perhaps there is an error with the regex string");
    debug("Regex compiled succesfully");
    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    debug("┌------------------------------------------\nDetected files and dirs from the vault");
    while ((vaultEntry = readdir(vaultDirectory)) != NULL) { // we iterate over every entry from the dir. So files and dirs (. and .. included)
      altDebug("%s  ", vaultEntry->d_name);
      // (TODO LATER) check if it is a file or a dir
      if (vaultEntry->d_name[0] != '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
        regexReturn = regexec(&regex, vaultEntry->d_name, 0, NULL, 0);
        if (!regexReturn) { // if the regex matches
          altDebug("matched with the regex. It is a journal.\n");
          journalsArray = realloc(journalsArray, (journalsCount + 1)*sizeof(char*)); // resize notesArray so that
          journalsArray[journalsCount] = strdup(vaultEntry->d_name); // copy the dir name into notesArray
          journalsCount++;
        } else {
          altDebug("did not matched with the regex. It is a note.\n");
        }
      } else {
        altDebug(" was ignored\n");
      }
    }
    altDebug("└ ------------------------------\n");
    // free's some used memory
    closedir(vaultDirectory);
    *count = journalsCount; // passes the number of files
    return journalsArray;
}

char** getNotesFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int shouldDebug) {
    // this function is inputed a path to a vault (which was selected before) and outpus all the suitable notes (so not the hidden ones)
    // (TODO LATER) Check how it handles non .md files
    // originally from https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    debug("Searching %s for notes", vault);
    struct dirent *vaultEntry; // (TODO LATER) change name of these variables. notesDirectory is dumb as it is the directory of vaults
    char tempPath[PATH_MAX];
    snprintf(tempPath, sizeof(tempPath), "%s/%s", pathToVault, vault); // sets the full absolute path to fullPathEntry
    DIR *vaultDirectory = opendir(tempPath);
    error(vaultDirectory==NULL, "program", "Could not open directory %s", tempPath);
    char **notesArray = NULL; // will contain all the notes
    size_t notesCount = 0; // we need to count how many notes there is to always readjust how many memory we alloc
    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    debug("┌------------------------------\nDetected Files and dirs from the vault:");
    while ((vaultEntry = readdir(vaultDirectory)) != NULL) {

      altDebug("%s ", vaultEntry->d_name);
      // for the regex code https://stackoverflow.com/a/1085120 
      regex_t regex;
      int regexReturn;
      // compiles the regex
      regexReturn = regcomp(&regex, journalRegex, 0);
      if (vaultEntry->d_name[0] != '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
        char fullPathEntry[PATH_MAX]; // creates a string of size of the maximum path lenght
        snprintf(fullPathEntry, sizeof(fullPathEntry), "%s/%s/%s", pathToVault, vault, vaultEntry->d_name); // sets the full absolute path to fullPathEntry
        // check if it matches the regex. If it does not match regexReturn != 0.
        regexReturn = regexec(&regex, vaultEntry->d_name, 0, NULL, 0);
        struct stat metadataPathEntry;
        if (stat(fullPathEntry, &metadataPathEntry) == 0 && regexReturn && S_ISREG(metadataPathEntry.st_mode)) { // if this entry is a file
          notesArray = realloc(notesArray, (notesCount + 1)*sizeof(char*)); // resize notesArray so that
          notesArray[notesCount] = strdup(vaultEntry->d_name); // copy the dir name into notesArray
          notesCount++;
          altDebug("did not match with the regex. It is a note.\n");
        } else if (!regexReturn) {altDebug("matched with the regex. It is a journal.\n");}
      } else {altDebug("was ignored\n");}
    }
    altDebug("└ ------------------------------\n");
    // (TODO LATER) Alphabetically sort them
    // free's some used memory
    closedir(vaultDirectory);
    *count = notesCount; // passes the number of files
    return notesArray;
}

char **getVaultsFromDirectory(char *dirString, size_t *count, int shouldDebug) { 
    // (TODO LATER) it might be a good idea to check if these directories exist
    // (TODO LATER) expand ~ as it does not work with opendir()
    // this function is inputed a path to a directory (which comes usually from the config file) and outpus all the suitable directories (so not the hidden ones) which will serve as separate vaults for notes
    debug("Opening %s", dirString);
    // originally from https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    struct dirent *vaultsDirectoryEntry;
    DIR *vaultsDirectory = opendir(dirString);
    error(vaultsDirectory==NULL, "program", "Could not open directory %s", dirString);
    char **dirsArray = NULL; // will contain all the dirs/vaults
    size_t dirsCount = 0; // we need to count how many dirs there is to always readjust how many memory we alloc
    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    debug("┌------------------------------\n Detected files and dirs from the directory:");
    while ((vaultsDirectoryEntry = readdir(vaultsDirectory)) != NULL) {
      altDebug("%s\n", vaultsDirectoryEntry->d_name);
      if (vaultsDirectoryEntry->d_name[0] != '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
        char fullPathEntry[PATH_MAX]; // creates a string of size of the maximum path lenght
        snprintf(fullPathEntry, sizeof(fullPathEntry), "%s/%s", dirString, vaultsDirectoryEntry->d_name); // sets the full absolute path to fullPathEntry
        
        struct stat metadataPathEntry;
        if (stat(fullPathEntry, &metadataPathEntry) == 0 && S_ISDIR(metadataPathEntry.st_mode)) { // if this entry is a directory
          dirsArray = realloc(dirsArray, (dirsCount + 1)*sizeof(char*)); // resize dirsArray so that
          dirsArray[dirsCount] = strdup(vaultsDirectoryEntry->d_name); // copy the dir name into dirsArray
          dirsCount++;
        }
      }
    }
    altDebug("└------------------------------\n");
    // (TODO LATER) Alphabetically sort them

    // free's some used memory
    closedir(vaultsDirectory);
    *count = dirsCount;
    return dirsArray;
}

char *updateJournal(char *path, char *journal, char *timeFormat, int shouldDebug) {
  //(TODO LATER) Per journal format configuration
  // (TODO LATER) with all the usage of PATH_MAX we should really calculate max inputed path or something like this
  path[PATH_MAX] = '\0'; // it assures it is a string (Most cases this does nothing). But rewriting at least one bytes make the compile happy. He doesn't want to return an unchanged input.
  debug("Handling the journal %s", path);

  char *date = getFormatedTime(timeFormat, shouldDebug);
  debug("Formated time for the journal's entry is %s", date);

  struct stat metadata;
  error(stat(path, &metadata), "program", "stat() failed to get information about %s", path);
  if (S_ISREG(metadata.st_mode)) {
    debug("%s is a unified journal.", path);
    if (!isStringInFile(path, date, shouldDebug)) { // if there is no entry for current date
      appendToFile(path, date, shouldDebug);
    } // if there is an entry do nothing 
  } else if (S_ISDIR(metadata.st_mode)) {
    debug("%s is a divided journal.", path);
  
  } else {
    error(1, "program", "%s is not a file and not a directory.", path);
  }
  return path;
}
