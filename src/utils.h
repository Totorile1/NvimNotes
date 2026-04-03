#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <pwd.h>
#include <ncurses.h>
#include <ctype.h>
#include <sys/wait.h>
#include <limits.h>
#include <cjson/cJSON.h>
#include <regex.h>
#include <errno.h> 
#define debug(message, ...) \
    _debug(shouldDebug, __FILE__, __LINE__, __func__, message, ##__VA_ARGS__)
#define altDebug(message, ...) \
    _altDebug(shouldDebug, message, ##__VA_ARGS__)
#define error(condition, type, message, ...) \
  _error(shouldDebug, condition, type, __FILE__, __LINE__, __func__, message, ##__VA_ARGS__)
#define HASH_MACRO "0ea1d20bcdd52c46c086d3dba125b9b83ad8cbea2e026d5646775f48bae8f867" // if the user inputs this hash. The program brokes. It was the best way i found to see if some values were unchanged
// you must edit this two values if you want to add suport for an editor
extern const char *supportedEditor[]; // array of supported editors
extern const int numEditors; // number of supported editors

int compareString(const void *a, const void *b);
//compares two strings alphabetically.
//this function is used for qsort
int doesEditorExist(char *editorToCheck, int debug);
// this basically checks all the dirs from your path for the editor. This is a safety check
// If the executable from an editor is not the editor name (for example neovim and nvim), you must handle at the start of the function.
void _debug(const int d, const char *file, const int line, const char *function, const char *message, ...);
//formated debugging
void _altDebug(const int d, const char *message, ...);
//use for less formal debuggin. (usefull if enumerating or making a list)
void _error(const int shouldDebug, const int condition, const char *type, const char *file, const int line, const char *function, const char *message, ...);
// formated error
// if the error is not a critical one (for ex: file not found) System error message will be Succesfull
int isStringInArray(const char *string, const char **array, const int len);
// Returns 1 if the string is in the array
// Returns 0 if the string is not in the array
// If you want to only check the first n elements of the array, pass n as len
void sanitize(char *string);
// replace unwanted chars by '_'. '.' is replaced if it is only the first two chars
int rmrf(char *path);
//from https://stackoverflow.com/a/5467788
//deletes an entire directory. Use with parsimony and carefullness
char **getVaultsFromDirectory(char *dirString, size_t *count, int debug);
// this function is inputed a path to a directory (which comes usually from the config file) and outpus all the suitable directories (so not the hidden ones) which will serve as separate vaults for notes
char **getNotesFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int debug);
// this function is inputed a path to a vault (which was selected before) and outpus all the suitable notes (so not the hidden ones)
// journalRegex is the regex code for the journals. If a note matches this code, it is treated as a journal and it is not outputed from this function.
char **getJournalsFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int debug);
// gets the journals from the vault
// to distinguish journals from note we use regex.
// Note: we must handle them separatly as journals can either be a single file (which will treat specially) or a directory
int openEditor(char *path, char *editor, int render, int endOfFile, int debug);
// Inputs are the path to the file, the editor to open and some rendering option
// render: if we render the .md file with Vivify
// endOfFile: if we put the cursor at the end of the file when opening
// The program resumes when the editor is closed
#endif
