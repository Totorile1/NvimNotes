#include "notes.h"
#include "ui.h"
#include <stdio.h>

char **getJournalsFromVault(char *pathToVault, char *vault, char *journalRegex,  int *count, int shouldDebug) {
    debug("Searching %s for journals", vault);
    // originally from https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    struct dirent *vaultEntry;
    char tempPath[PATH_MAX];
    snprintf(tempPath, sizeof(tempPath), "%s/%s", pathToVault, vault); // sets the full absolute path to fullPathEntry
    DIR *vaultDirectory = opendir(tempPath);
    error(vaultDirectory==NULL, "program", "Could not open directory %s", tempPath);
    char **journalsArray = NULL; // will contain all the notes
    int journalsCount = 0; // we need to count how many notes there is to always readjust how many memory we alloc
    
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
    // originally from https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    debug("Searching %s for notes", vault);
    struct dirent *vaultEntry; // (TODO LATER) change name of these variables. notesDirectory is dumb as it is the directory of vaults
    char tempPath[PATH_MAX];
    snprintf(tempPath, sizeof(tempPath), "%s/%s", pathToVault, vault); // sets the full absolute path to fullPathEntry
    DIR *vaultDirectory = opendir(tempPath);
    error(vaultDirectory==NULL, "program", "Could not open directory %s", tempPath);
    char **notesArray = NULL; // will contain all the notes
    int notesCount = 0; // we need to count how many notes there is to always readjust how many memory we alloc
    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    debug("┌------------------------------\nDetected Files and dirs from the vault:");
    while ((vaultEntry = readdir(vaultDirectory)) != NULL) {
      char *entryName = vaultEntry->d_name;
      int entryLenght = strlen(entryName);
      altDebug("%s ", entryName);
      // for the regex code https://stackoverflow.com/a/1085120 
      regex_t regex;
      int regexReturn;
      // compiles the regex
      regexReturn = regcomp(&regex, journalRegex, 0);
      if (entryName[0] != '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
        char fullPathEntry[PATH_MAX]; // creates a string of size of the maximum path lenght
        snprintf(fullPathEntry, sizeof(fullPathEntry), "%s/%s/%s", pathToVault, vault, entryName); // sets the full absolute path to fullPathEntry
        // check if it matches the regex. If it does not match regexReturn != 0.
        regexReturn = regexec(&regex, entryName, 0, NULL, 0);
        struct stat metadataPathEntry;
        if (stat(fullPathEntry, &metadataPathEntry) == 0 && entryName[entryLenght - 3] == '.' && entryName[entryLenght - 2] == 'm' && entryName[entryLenght - 1] == 'd' && regexReturn && S_ISREG(metadataPathEntry.st_mode)) { // if this entry is a file ending in .md and that does no match the regex
          notesArray = realloc(notesArray, (notesCount + 1)*sizeof(char*)); // resize notesArray so that
          notesArray[notesCount] = strdup(entryName); // copy the dir name into notesArray
          notesCount++;
          altDebug("did not match with the regex. It is a note.\n");
        } else if (!regexReturn) {altDebug("matched with the regex. It is a journal.\n");}
      } else {altDebug("was ignored\n");}
    }
    altDebug("└ ------------------------------\n");

    // free's some used memory
    closedir(vaultDirectory);
    *count = notesCount; // passes the number of files
    return notesArray;
}

char **getVaultsFromDirectory(char *dirString, int *count, int shouldDebug) { 
    // (TODO LATER) it might be a good idea to check if these directories exist
    // (TODO LATER) expand ~ as it does not work with opendir()
    // this function is inputed a path to a directory (which comes usually from the config file) and outpus all the suitable directories (so not the hidden ones) which will serve as separate vaults for notes
    debug("Opening %s", dirString);
    // originally from https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    struct dirent *vaultsDirectoryEntry;
    DIR *vaultsDirectory = opendir(dirString);
    error(vaultsDirectory==NULL, "program", "Could not open directory %s", dirString);
    char **dirsArray = NULL; // will contain all the dirs/vaults
    int dirsCount = 0; // we need to count how many dirs there is to always readjust how many memory we alloc
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
  char dateWithExtension[PATH_MAX];
  snprintf(dateWithExtension, PATH_MAX, "%s.md", date);
  sanitize(dateWithExtension);
  debug("Sanitized date: %s\n(it might be used later for a file name if the journal is divided)", dateWithExtension);
  struct stat metadata;
  error(stat(path, &metadata), "program", "stat() failed to get information about %s", path);
  if (S_ISREG(metadata.st_mode)) {
    debug("%s is a unified journal.", path);
    if (!isStringInFile(path, date, shouldDebug)) { // if there is no entry for current date
      appendToFile(path, date, shouldDebug);
    } // if there is an entry do nothing 
  } else if (S_ISDIR(metadata.st_mode)) {
      debug("%s is a divided journal.", path);
      struct dirent *dividedJournalEntry;
      // snprintf does not like to have the same variable as input and output so we use a buffer
      char temp[PATH_MAX];
      snprintf(temp, PATH_MAX, "%s/", path); // appends a /. For safety
      strncpy(path, temp, PATH_MAX);
      DIR *dividedJournalDirectory = opendir(path);
      error(dividedJournalDirectory==NULL, "program", "Could not open directory %s", path);
      /*char **entryArray = NULL; // will contain all the entries of the divided journal
      // This time it will be different. We must add "invert" the extraOptions to the options so that the Create new entry in journal is on top
      entryArray = realloc(entryArray, sizeof(char*));*/
      // simpler to just malloc 2 and later realloc
      const int extraOptions = 3; 
      char **entryArray = malloc(extraOptions*sizeof(char*));
      char *createEntryMessage = malloc(PATH_MAX);
      snprintf(createEntryMessage, PATH_MAX, "Create new entry for the journal %s", journal);
      entryArray[0] = createEntryMessage;
      entryArray[1] = "Open random entry";
      entryArray[2] = "Search inside entries";
      int entryCount = extraOptions; // we need to count how many dirs there is to always readjust how many memory we alloc
      // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
      // for readdir()
      debug("┌------------------------------\n Detected files and dirs from %s:", path);
      // iterates over all the entries from the dir
      while ((dividedJournalEntry = readdir(dividedJournalDirectory)) != NULL) {
        altDebug("%s\n", dividedJournalEntry->d_name);
        if (dividedJournalEntry->d_name[0] != '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
          char fullPathEntry[PATH_MAX]; // creates a string of size of the maximum path lenght
          snprintf(fullPathEntry, sizeof(fullPathEntry), "%s/%s", path, dividedJournalEntry->d_name); // sets the full absolute path to fullPathEntry
          
          struct stat metadataPathEntry;
          if (stat(fullPathEntry, &metadataPathEntry) == 0 && S_ISREG(metadataPathEntry.st_mode)) { // if this entry is a directory
            entryArray = realloc(entryArray, (entryCount + 1)*sizeof(char*));
            entryArray[entryCount] = strdup(dividedJournalEntry->d_name); // copy the dir name into entryArray
            entryCount++;
          }
        }
      }
      altDebug("└------------------------------\n");
      // free's some used memory
      closedir(dividedJournalDirectory);
      qsort(entryArray + extraOptions, entryCount - extraOptions, sizeof(const char *), reverseCompareString); // sorts the journals entries alphabetically. // the + extraOptions - extraOptions is to not sort "Create new entry for the journal" or Open random entry which is the first element

      // we must now select to create new entry or to enter in old one
      char *selectedOption = ncursesSelect(entryArray, "Create new entry or acces old entry (Use arrows or WASD, Enter to select):", extraOptions, entryCount - extraOptions, " ", " ", "", shouldDebug); // the "Create new entry for the journal %s" will be the only options. All other will be extraOptions. This is made so that "Create [...] %s" will always be on top
      debug("Selected option from journal entry selection: %s", selectedOption);
      if (strcmp(selectedOption, createEntryMessage) == 0) { // create new entry
        char temp[PATH_MAX];
        error(strlen(path)+1+strlen(dateWithExtension)+1>PATH_MAX, "Error file path too long. %s/%s must not exceed PATH_MAX", path, dateWithExtension); 
        snprintf(temp, PATH_MAX, "%s/%s", path, dateWithExtension);
        strncpy(path, temp, PATH_MAX);
        if (!isStringInArray(dateWithExtension, (const char **)entryArray, entryCount)) { // it only creates it if it doesn't already exist
                                                              // if it does exist it will just pass the full path
          debug("Creating entry %s inside %s", date, path);
          FILE *file;
          file = fopen(path, "w");
          error(file == NULL, "program", "%s couldn't be created", path);
          debug("Writing %s\\n to %s", date, path);
          fprintf(file, "%s\n", date);
          fclose(file);
          free(createEntryMessage);
        } else {
          debug("Today's entry (%s) already exist. We won't create a new one.", dateWithExtension);
        }
      } else if (strcmp(selectedOption, entryArray[1]) == 0) { // if we choosed to open a random entry
        // setting up the seed for rand()
        srand(time(NULL)); // not totaly random, just pseudorandom
        int randomEntry = extraOptions + (rand() % (entryCount - extraOptions)); // we get a random number between extraOptions and entryCount (so all journal entries and not extraOptions)
        char temp[PATH_MAX];
        snprintf(temp, PATH_MAX, "%s/%s", path, entryArray[randomEntry]); // reconstruct the path
        strncpy(path, temp, PATH_MAX);
        debug("Selected random entry %s. It's path is %s.", entryArray[randomEntry], path);
      } else if (strcmp(selectedOption, entryArray[2]) == 0) { // if we choosed to search
        char *temp = fzfSelect(path, "Input text to be searched", shouldDebug); // uses ripgrep and fzf to search inside the files
        char *selectedOption = malloc(PATH_MAX); // we can't just use path as both input and output of snprintf
        snprintf(selectedOption, PATH_MAX, "%s/%s", path, temp);
        path = strdup(selectedOption);
        free(selectedOption);
      } else { // we just recreate the path to the selected entry
        // snprintf does not like to have the same variable as input and output so we use a buffer
        char temp[PATH_MAX];
        snprintf(temp, PATH_MAX, "%s/%s", path, selectedOption);
        strncpy(path, temp, PATH_MAX);
        debug("Returning path to the selected entry: %s", path);
      }
      // (TODO LATER) free all unused entryArray[i]
  } else {
    error(1, "program", "%s is not a file and not a directory.", path);
  }
  
  return path;
}
