#ifndef NOTES_H
#define NOTES_H
#include "utils.h"
#include "ui.h"
// find which directory contains targetVault
char *getDirectoryFromVault(char *targetVault, char **vaultsArray, int vaultTotalNumber, int *vaultNumberPerDirectory, char **directoryArray, int directoryNumber, int shouldDebug);
// gets the journals from the vault.
//to distinguish journals from note we use regex. 
//Note: we must handle them separatly as journals can either be a single file (which will treat specially) or a directory.
char **getJournalsFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int shouldDebug);
// this function is inputed a path to a vault (which was selected before) and outpus all the suitable notes (so not the hidden ones).
// journalRegex is the regex code for the journals. If a note matches this code, it is treated as a journal and it is not outputed from this function.
char **getNotesFromVault(char *pathToVault, char *vault, char *journalRegex, int *count, int shouldDebug);
// function gets as input an array of directoryNumber strings. Which are the directories the function will search for vaults.
// it returns an array of vaults.
// the vaults are organized in order per directory.
// vaultsPerDirectoryNumber and count should be initiallized before calling the function and the address be inputed.
// count is the total number of vaults.
// vaultsPerDirectoryNumber gives how many vaults each directory has.
char **getVaultsFromDirectories(char **directoryStringArray, int directoryNumber, int *vaultsPerDirectoryNumber,  int *count, int shouldDebug);
// path is the path to the file.
// journal is the name of the file.
// journalWasUpdated will be set to 1 if a new entry was created
// handles both type of journal (divided and unified).
// creates new entry with date.
// for divided select if we want to acces to a new entry or a old one.
// returns the path to the file that needs to be opened.
char *updateJournal(char *path, char *journal, char *timeFormat, int *journalWasUpdated, int shouldDebug);
#endif
