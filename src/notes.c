#include "notes.h"
#include "utils.h"

char *getDirectoryFromVault(char *targetVault, char **vaultsArray, int vaultTotalNumber,
                            int *vaultNumberPerDirectory, char **directoryArray,
                            int directoryNumber, int shouldDebug) {
    debug("Searching the vault %s inside all the directories...", targetVault);
    debug("Here are how many vaults there is per directory:");
    for (int i = 0; i < directoryNumber; i++) {
        altDebug("%d for %s\n", vaultNumberPerDirectory[i], directoryArray[i]);
    }
    debug("Here are all the vaults in the order they will be searched:");
    for (int i = 0; i < vaultTotalNumber; i++) {
        altDebug("%s\n", vaultsArray[i]);
    }
    int index = 0;

    for (int i = 0; i < directoryNumber; i++) {
        for (int j = 0; j < vaultNumberPerDirectory[i]; j++) {
            if (strcmp(vaultsArray[index], targetVault) == 0) {
                debug("%s found in %s. (index %d)", targetVault, directoryArray[i], index);
                return directoryArray[i];
            }
            index++;
        }
    }
    error(1, "program", "the vault %s was not found", targetVault);
    return "this makes the compiler and clangd happy. Who doesn't want GCC and clangd to be happy? "
           "Such person would be a terrible monster... One must imagine GCC and clangd happy.";
}

char **getJournalsFromVault(char *pathToVault, char *vault, char *journalRegex, int *count,
                            int shouldDebug) {
    debug("Searching %s for journals", vault);
    // originally from
    // https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    struct dirent *vaultEntry;
    char tempPath[PATH_MAX];
    snprintf(tempPath, sizeof(tempPath), "%s/%s", pathToVault,
             vault); // sets the full absolute path to fullPathEntry
    DIR *vaultDirectory = opendir(tempPath);
    error(vaultDirectory == NULL, "program", "Could not open directory %s", tempPath);
    char **journalsArray = NULL; // will contain all the notes
    int journalsCount =
        0; // we need to count how many notes there is to always readjust how many memory we alloc

    // https://stackoverflow.com/a/1085120 for regex code
    regex_t regex;
    int regexReturn;
    // compiles the regex
    regexReturn = regcomp(&regex, journalRegex, 0);
    error(regexReturn, "program",
          "Regex could not compile. Perhaps there is an error with the regex string");
    debug("Regex compiled succesfully");
    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    debug("┌------------------------------------------\nDetected files and dirs from the vault");
    while (
        (vaultEntry = readdir(vaultDirectory)) !=
        NULL) { // we iterate over every entry from the dir. So files and dirs (. and .. included)
        altDebug("%s  ", vaultEntry->d_name);
        if (vaultEntry->d_name[0] !=
            '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
            regexReturn = regexec(&regex, vaultEntry->d_name, 0, NULL, 0);
            if (!regexReturn) { // if the regex matches
                altDebug("matched with the regex. It is a journal.\n");
                journalsArray =
                    realloc(journalsArray,
                            (journalsCount + 1) * sizeof(char *)); // resize notesArray so that
                journalsArray[journalsCount] =
                    strdup(vaultEntry->d_name); // copy the dir name into notesArray
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

char **getNotesFromVault(char *pathToVault, char *vault, char *journalRegex, int *count,
                         int shouldDebug) {
    // this function is inputed a path to a vault (which was selected before) and outpus all the
    // suitable notes (so not the hidden ones) originally from
    // https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
    debug("Searching %s for notes", vault);
    struct dirent *vaultEntry;
    char tempPath[PATH_MAX];
    snprintf(tempPath, sizeof(tempPath), "%s/%s", pathToVault,
             vault); // sets the full absolute path to fullPathEntry
    DIR *vaultDirectory = opendir(tempPath);
    error(vaultDirectory == NULL, "program", "Could not open directory %s", tempPath);
    char **notesArray = NULL; // will contain all the notes
    int notesCount =
        0; // we need to count how many notes there is to always readjust how many memory we alloc
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
        if (entryName[0] !=
            '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
            char fullPathEntry[PATH_MAX]; // creates a string of size of the maximum path lenght
            snprintf(fullPathEntry, sizeof(fullPathEntry), "%s/%s/%s", pathToVault, vault,
                     entryName); // sets the full absolute path to fullPathEntry
            // check if it matches the regex. If it does not match regexReturn != 0.
            regexReturn = regexec(&regex, entryName, 0, NULL, 0);
            struct stat metadataPathEntry;
            if (stat(fullPathEntry, &metadataPathEntry) == 0 && entryName[entryLenght - 3] == '.' &&
                entryName[entryLenght - 2] == 'm' && entryName[entryLenght - 1] == 'd' &&
                regexReturn &&
                S_ISREG(metadataPathEntry.st_mode)) { // if this entry is a file ending in .md and
                                                      // that does no match the regex
                notesArray = realloc(notesArray, (notesCount + 1) *
                                                     sizeof(char *)); // resize notesArray so that
                notesArray[notesCount] = strdup(entryName); // copy the dir name into notesArray
                notesCount++;
                altDebug("did not match with the regex. It is a note.\n");
            } else if (!regexReturn) {
                altDebug("matched with the regex. It is a journal.\n");
            }
        } else {
            altDebug("was ignored\n");
        }
    }
    altDebug("└ ------------------------------\n");

    // free's some used memory
    closedir(vaultDirectory);
    *count = notesCount; // passes the number of files
    return notesArray;
}

char **getVaultsFromDirectories(char **directoryStringArray, int directoryNumber,
                                int *vaultsPerDirectoryNumber, int *count, int shouldDebug) {
    // to avoid having to work with a tree-dimensional array. We will use a 2d and
    // vaultsPerDirectoryNumber will indicate the width of the directories.
    char **vaultsArray = NULL;
    int nthVault = 0; // this is only used internally to set the string into the right place in
                      // directoryStringArray.
    int previousStartIndex = 0;
    for (int i = 0; i < directoryNumber; i++) {
        debug("Opening %s", directoryStringArray[i]);
        // originally from
        // https://www.geeksforgeeks.org/c/c-program-list-files-sub-directories-directory/
        struct dirent *vaultsDirectoryEntry;
        DIR *vaultsDirectory = opendir(directoryStringArray[i]);
        error(!vaultsDirectory, "program", "Could not open directory %s", directoryStringArray[i]);
        vaultsPerDirectoryNumber[i] = 0;
        debug("┌------------------------------\n Detected files and dirs %s:",
              directoryStringArray[i]);
        // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
        // for readdir()
        while ((vaultsDirectoryEntry = readdir(vaultsDirectory)) != NULL) {
            char *entryName = vaultsDirectoryEntry->d_name; // gets the entry as a string value
            altDebug("%s", entryName);
            if (entryName[0] != '.') {            // if not hidden file/dir
                char tempFullEntryPath[PATH_MAX]; // we recreate the full path to check it's
                                                  // proprieties
                snprintf(tempFullEntryPath, PATH_MAX, "%s%s", directoryStringArray[i], entryName);
                altDebug(" (%s)", tempFullEntryPath);
                // checking the metadata to see if it is a dir
                struct stat metadataEntry;
                if (stat(tempFullEntryPath, &metadataEntry) == 0 &&
                    S_ISDIR(metadataEntry.st_mode)) { // get's the metadata (stat should return 0 if
                                                      // fails) and sees if it is a dir.
                    altDebug(" is a vault");
                    vaultsArray =
                        realloc(vaultsArray, sizeof(char *) * (nthVault + 1)); // resize vaultsArray
                    error(vaultsArray == NULL, "program", "realloc failed");
                    vaultsPerDirectoryNumber[i]++; // it will be used later to know which vaults
                                                   // goes into which directory
                    vaultsArray[nthVault] =
                        strdup(entryName); // we use strdup and not strcpy, because memory used with
                                           // opendir and readdir will be closed.
                    error(vaultsArray[nthVault] == NULL, "program", "strdup failed");
                    nthVault++; // it is used immediatly to set the vault into directoryStringArray
                } else {
                    altDebug(" is not a vault (not a dir.)");
                }
            } else {
                altDebug(" is not a vault (hidden file/dir)");
            }
        }
        // we sort entries for the same vault. We can't sort them all. This would break the order.
        qsort(vaultsArray + previousStartIndex, nthVault - previousStartIndex, sizeof(const char *),
              compareString); // sorts the vaults alphabetically
        closedir(vaultsDirectory);
        previousStartIndex = nthVault;
    }
    // we calculate once the total number of vaults to avoid recalculation every time we use it
    *count = 0;
    for (int i = 0; i < directoryNumber; i++) {
        *count += vaultsPerDirectoryNumber[i];
    }
    return vaultsArray;
}

char *updateJournal(char *path, char *journal, char *timeFormat, int *journalWasUpdated,
                    int shouldDebug) {
    path[PATH_MAX] =
        '\0'; // it assures it is a string (Most cases this does nothing). But rewriting at least
              // one bytes make the compile happy. He doesn't want to return an unchanged input.
    debug("Handling the journal %s", path);

    char *date = getFormatedTime(timeFormat, shouldDebug);
    debug("Formated time for the journal's entry is %s", date);
    char dateWithExtension[PATH_MAX];
    snprintf(dateWithExtension, PATH_MAX, "%s.md", date);
    sanitize(dateWithExtension);
    debug("Sanitized date: %s\n(it might be used later for a file name if the journal is divided)",
          dateWithExtension);
    struct stat metadata;
    stat(path, &metadata);
    if (S_ISREG(metadata.st_mode)) {
        debug("%s is a unified journal.", path);
        if (!isStringInFile(path, date, shouldDebug)) { // if there is no entry for current date
            appendToFile(path, date, shouldDebug);
            *journalWasUpdated = 1;
        } // if there is an entry do nothing
    } else if (S_ISDIR(metadata.st_mode)) {
        debug("%s is a divided journal.", path);
        struct dirent *dividedJournalEntry;
        // snprintf does not like to have the same variable as input and output so we use a buffer
        char temp[PATH_MAX];
        snprintf(temp, PATH_MAX, "%s/", path); // appends a /. For safety
        strncpy(path, temp, PATH_MAX);
        DIR *dividedJournalDirectory = opendir(path);
        error(dividedJournalDirectory == NULL, "program", "Could not open directory %s", path);
        /*char **entryArray = NULL; // will contain all the entries of the divided journal
        // This time it will be different. We must add "invert" the extraOptions to the options so
        that the Create new entry in journal is on top entryArray = realloc(entryArray,
        sizeof(char*));*/
        // simpler to just malloc 2 and later realloc
        const int extraOptions = 3;
        char **entryArray = malloc(extraOptions * sizeof(char *));
        char *createEntryMessage = malloc(PATH_MAX);
        snprintf(createEntryMessage, PATH_MAX, "Create new entry for the journal %s", journal);
        entryArray[0] = createEntryMessage;
        entryArray[1] = "Open random entry";
        entryArray[2] = "Search inside entries";
        int entryCount = extraOptions; // we need to count how many dirs there is to always readjust
                                       // how many memory we alloc
        // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
        // for readdir()
        debug("┌------------------------------\n Detected files and dirs from %s:", path);
        // iterates over all the entries from the dir
        while ((dividedJournalEntry = readdir(dividedJournalDirectory)) != NULL) {
            altDebug("%s\n", dividedJournalEntry->d_name);
            if (dividedJournalEntry->d_name[0] !=
                '.') { // if the entry don't start with a dot (so hidden dirs and hidden files)
                char fullPathEntry[PATH_MAX]; // creates a string of size of the maximum path lenght
                snprintf(
                    fullPathEntry, sizeof(fullPathEntry), "%s/%s", path,
                    dividedJournalEntry->d_name); // sets the full absolute path to fullPathEntry

                struct stat metadataPathEntry;
                if (stat(fullPathEntry, &metadataPathEntry) == 0 &&
                    S_ISREG(metadataPathEntry.st_mode)) { // if this entry is a directory
                    entryArray = realloc(entryArray, (entryCount + 1) * sizeof(char *));
                    entryArray[entryCount] =
                        strdup(dividedJournalEntry->d_name); // copy the dir name into entryArray
                    entryCount++;
                }
            }
        }
        altDebug("└------------------------------\n");
        // free's some used memory
        closedir(dividedJournalDirectory);
        qsort(entryArray + extraOptions, entryCount - extraOptions, sizeof(const char *),
              reverseCompareString); // sorts the journals entries alphabetically. // the +
                                     // extraOptions - extraOptions is to not sort "Create new entry
                                     // for the journal" or Open random entry which is the first
                                     // element

        // we must now select to create new entry or to enter in old one
        char *selectedOption = ncursesSelect(
            entryArray,
            "Create new entry or acces old entry (Use arrows or WASD, Enter to select):",
            extraOptions, entryCount - extraOptions, " ", " ", "",
            shouldDebug); // the "Create new entry for the journal %s" will be the only options. All
                          // other will be extraOptions. This is made so that "Create [...] %s" will
                          // always be on top
        debug("Selected option from journal entry selection: %s", selectedOption);
        if (strcmp(selectedOption, createEntryMessage) == 0) { // create new entry
            char temp[PATH_MAX];
            error(strlen(path) + 1 + strlen(dateWithExtension) + 1 > PATH_MAX,
                  "Error file path too long. %s/%s must not exceed PATH_MAX", path,
                  dateWithExtension);
            snprintf(temp, PATH_MAX, "%s/%s", path, dateWithExtension);
            strncpy(path, temp, PATH_MAX);
            if (!isStringInArray(dateWithExtension, (const char **)entryArray,
                                 entryCount)) { // it only creates it if it doesn't already exist
                                                // if it does exist it will just pass the full path
                debug("Creating entry %s inside %s", date, path);
                FILE *file;
                file = fopen(path, "w");
                error(file == NULL, "program", "%s couldn't be created", path);
                debug("Writing %s\\n to %s", date, path);
                fprintf(file, "%s\n", date);
                fclose(file);
                free(createEntryMessage);
                *journalWasUpdated = 1;
            } else {
                debug("Today's entry (%s) already exist. We won't create a new one.",
                      dateWithExtension);
            }
        } else if (strcmp(selectedOption, entryArray[1]) ==
                   0) { // if we choosed to open a random entry
            // setting up the seed for rand()
            srand(time(NULL)); // not totaly random, just pseudorandom
            int randomEntry =
                extraOptions +
                (rand() %
                 (entryCount -
                  extraOptions)); // we get a random number between extraOptions and entryCount (so
                                  // all journal entries and not extraOptions)
            char temp[PATH_MAX];
            snprintf(temp, PATH_MAX, "%s/%s", path,
                     entryArray[randomEntry]); // reconstruct the path
            strncpy(path, temp, PATH_MAX);
            debug("Selected random entry %s. It's path is %s.", entryArray[randomEntry], path);
        } else if (strcmp(selectedOption, entryArray[2]) == 0) { // if we choosed to search
            char *temp = fzfSelect(path, "Input text to be searched",
                                   shouldDebug); // uses ripgrep and fzf to search inside the files
            char *selectedOption =
                malloc(PATH_MAX); // we can't just use path as both input and output of snprintf
            snprintf(selectedOption, PATH_MAX, "%s/%s", path, temp);
            path = strdup(selectedOption);
            free(selectedOption);
        } else { // we just recreate the path to the selected entry
            // snprintf does not like to have the same variable as input and output so we use a
            // buffer
            char temp[PATH_MAX];
            snprintf(temp, PATH_MAX, "%s/%s", path, selectedOption);
            strncpy(path, temp, PATH_MAX);
            debug("Returning path to the selected entry: %s", path);
        }
    } else {
        error(1, "program", "%s is not a file and not a directory.", path);
    }

    return path;
}
