#include "utils.h"

const char *supportedEditor[] = {"neovim", "vim"};
const int numEditors = 2;

//(TODO LATER) We should write a debug() function and an error(function)

int compareString(const void *a, const void *b) {
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;
    return strcmp(str1, str2); // strcmp returns <0, 0, >0
}

void _debug(const int d, const char *file, const int line, const char *function, const char *message, ...) { // use for formatted debug
  if (d) {
    va_list args; //variadic function stuff
    va_start(args, message);

    fprintf(stderr, "\e[0;32m[DEBUG] From file %s line %d function %s:\e[0m\n", file, line, function);
    vfprintf(stderr, message, args);
    printf("\e[0m\n");
    va_end(args);
  }
}
void _altDebug(const int d, const char *message, ...) { // use for less formal debuggin. (usefull if enumerating or making a list
  if (d) {
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
  }
}

void _error(const int shouldDebug, const int condition, const char *type, const char *file, const int line, const char *function, const char *message, ...) { // used for formatted errors
  if (condition) {
    fprintf(stderr, "\e[0;31m[%s ERROR] From file %s line %d function %s:\n", type, file, line, function);
    if (errno != 0) {
        fprintf(stderr, " (System-level error message: %s)\n", strerror(errno));
    } else {
        fprintf(stderr, " (No system-level error; issue is application-level)\n");
    }
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    if (!shouldDebug) {
      fprintf(stderr, "\nRunning notewrapper -V might give you more information.");
    }
    fprintf(stderr, "\e[0m\n");
    exit(1);
  }
}


int doesEditorExist (char *editorToCheck, int shouldDebug) {     // Some exectuables have not exaclty the same name as the editor.
  char *editor;   
    if (strcmp(editorToCheck, "neovim") == 0) {
      editor = strdup("nvim"); // we must use strdup and not just copy as we would have modified editorToOpen in main
    }
    else {
      editor = strdup(editorToCheck);
    }
    char *path_env = getenv("PATH");
    error(!path_env, "program", "getenv(\"PATH\") failed to get your path. NoteWrapper is unable to check if your desired editor is installed\n");
    debug("Your PATH is %s", path_env);
    char *paths = strdup(path_env); // duplicate because strtok modifies the string
    char *dir = strtok(paths, ":");
    while (dir) {
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, editor);
        if (access(fullpath, X_OK) == 0) { // program found
            free(paths);
            free(editor);
            return 1;
        }
        dir = strtok(NULL, ":");
    }

    free(paths);
    free(editor);
    return 0; // program not found
}

int isStringInArray(const char *string, const char **array, const int len) {
  for (int i = 0; i < len; i++) {
    if (strcmp(string, array[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

void sanitize(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if ((!isalnum((unsigned char)string[i]) && strchr("/\\:*?\"\'<>\n\r\t", string[i])) || ((i == 0 || i == 1) && string[i] == '.')) { // replace unwanted chars by '_'. '.' is replaced if it is only the first two chars
                                                                                                                    // (TODO LATER fixe case where it "*.*", "..." and so on
        string[i] = '_';
    }
  }
}

//  both functions are from https://stackoverflow.com/a/5467788
// from what i understood:
// remove() can't delete directories with files
// so it walks the file tree and deletes it's content before removing the directory
// (TODO LATER) this seems safe, but it's maybe a good idea to add some checks to not remove something it should not remove
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
  int shouldDebug = 1; //normally shouldDebug should be passed to the function. However, it's to difficult here and the best it to set it to 1.
  int rv = remove(fpath);
  error(rv, "program", "remove() failed to delete %s", fpath);
  return rv;
}

int rmrf(char *path) {
  // (TODO LATER) Check if it not a root directory or something like this 
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

char** getVaultsFromDirectory(char *dirString, size_t *count, int shouldDebug) { 
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
    if (shouldDebug) {printf("┌------------------------------\n\e[0;32m[DEBUG]\e[0m Files and dirs from the directory\n");}
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
        if (stat(fullPathEntry, &metadataPathEntry) == 0 && regexReturn && !S_ISDIR(metadataPathEntry.st_mode)) { // if this entry is a file
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


int openEditor(char *path, char *editor, int render, int endOfFile, int shouldDebug) {
  // (TODO LATER) Bug app breaks if browser was not already launched before vivify
  // (TODO LATER) find better name for endOfFile
  //(TODO LATER) for nvim and vim we should check if there is swap files or recovery files and handle that
  pid_t pid = fork(); // this forking allows the programs to return when nvim is closed
  error(pid<0, "program", "fork() failed.");
  if (pid == 0) {
      // Child process: replace with editor of choice
    if (strcmp(editor, "neovim") == 0) { // opens with Neovim 
    //(TODO LATER) we should (with a config option) append a new line every time it opens
      if (render) { // don't render using vivify
        if (endOfFile) { // goes to the end of the file on opening. (TODO LATER) find a better way to do this loops. Maybe an array of args and if () we add the arg to the array and we pass the whole array to execlp
          // :$ goes to the end of the file. :Vivify runs vivify
          debug("Running nvim +:$ +:Vivify %s", path);
          execlp("nvim", "nvim", "+:$", "+:Vivify", path, NULL);
          error(1, "program", "execlp() failed."); // if something after execlp is executed it means something failed. Normally this function is not called
        } else { // don't go to the end of the file
          debug("Running nvim +:Vivify %s", path);
          execlp("nvim", "nvim", "+:Vivify", path, NULL);
          error(1, "program", "execlp() failed.");
        }
      } else { // don't render using vivify
        if (endOfFile) { // go to end of the file on opening
          debug("Running nvim +:$ %s", path);
          execlp("nvim", "nvim", "+:$", path, NULL);
          error(1, "program", "execlp() failed.");
        } else { // don't go to the end of the file on opening
          debug("Running nvim %s", path);  
          execlp("nvim", "nvim", path, NULL);
          error(1, "program", "execlp() failed.");
        }
      }
    } else if (strcmp(editor, "vim") == 0) { // opens with Vim // see comments for neovim for explanations
    //(TODO LATER) we should (with a config option) append a new line every time it opens
      if (render) {
        if (endOfFile) {
          debug("Running vim +:$ +:Vivify %s", path);  
          execlp("vim", "vim", "+:$", "+:Vivify", path, NULL);
          error(1, "program", "execlp() failed.");
        } else {
          debug("Running vim +:Vivify %s", path);  
          execlp("vim", "vim", "+:Vivify", path, NULL);
          error(1, "program", "execlp() failed.");
        }
      } else {
        if (endOfFile) {
          debug("Running vim +:$ %s", path);  
          execlp("vim", "vim", "+:$", path, NULL);
          error(1, "program", "execlp() failed.");
        } else {
          debug("Running vim %s", path);
          execlp("vim", "vim", path, NULL);
          error(1, "program", "execlp() failed.");
        }
      }
    }
  } else {
    // Parent process: wait for child to finish
    int status;
    waitpid(pid, &status, 0);
  } // (TODO LATER) add a options to kill the browser when closing. This will solve the bug where -R does renders when the file was previously opened with -r.
  return 0;
}
