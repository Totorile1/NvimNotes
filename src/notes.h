#ifndef NOTES_H
#define NOTES_H
#include "utils.h"
#include "ui.h"
char **getJournalsFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int shouldDebug);
// gets the journals from the vault
// to distinguish journals from note we use regex.
// Note: we must handle them separatly as journals can either be a single file (which will treat specially) or a directory

char **getNotesFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int shouldDebug);
// this function is inputed a path to a vault (which was selected before) and outpus all the suitable notes (so not the hidden ones)
// journalRegex is the regex code for the journals. If a note matches this code, it is treated as a journal and it is not outputed from this function.
char **getVaultsFromDirectory(char *dirString, size_t *count, int shouldDebug);
// this function is inputed a path to a directory (which comes usually from the config file) and outpus all the suitable directories (so not the hidden ones) which will serve as separate vaults for notes
char *updateJournal(char *path, char *journal, char *timeFormat, int shouldDebug);
// path is the path to the file
// journal is the name of the file
// handles both type of journal (divided and unified)
// creates new entry with date
// for divided select if we want to acces to a new entry or a old one
// returns the path to the file that needs to be opened
#endif
