#include "notes.h"
#include "ui.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    int shouldDebug = 0;
    int overwriteConfigPath = 0;
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        // Long options
        if (strncmp(arg, "--", 2) == 0) {
            if (strcmp(arg, "--verbose") == 0) {
                shouldDebug = 1;

            } else if (strcmp(arg, "--config") == 0) {
                error(i + 1 == argc, "user", "Missing argument. Use --config <path>");
                overwriteConfigPath = ++i; // consume argument
            }

            // Short options (allow grouping like -V)
        } else if (arg[0] == '-' && arg[1] != '\0') {
            for (int j = 1; arg[j] != '\0'; j++) {
                char opt = arg[j];

                switch (opt) {
                case 'V':
                    shouldDebug = 1;
                    break;

                case 'c':
                    // must NOT be combined (needs argument)
                    error(arg[j + 1] != '\0', "user", "-c cannot be combined (use -c <path>)");
                    error(i + 1 == argc, "user", "Missing argument for -c");

                    overwriteConfigPath = ++i; // consume argument
                    goto arg_next;

                default:
                    // ignore unknown here OR handle error
                    break;
                }
            }
        }

    arg_next:;
    }
    // gets the home directory
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    initAppFilesAndDirs(homedir, shouldDebug);
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------
    // this part handles the config.json file
    // gets the home directory
    char *configPath = malloc(PATH_MAX);
    if (overwriteConfigPath) {
        configPath = argv[overwriteConfigPath];
        debug("-c or --config specified the config file %s", configPath);
    } else {
        snprintf(configPath, PATH_MAX, "%s/.config/notewrapper/config.json", homedir);
        debug("Path to the config file is %s", configPath);
    }
    // check if the config file exists
    struct stat st = {0};
    error(stat(configPath, &st) == -1, "user",
          "The config file %s does not exist.\nMaybe try the default path to the config "
          "~/.config/notewrapper/config.json\nOr if you used the flag -c or --config, verifiy that "
          "you point to the correct file.",
          configPath); // if the config directory does not exist

    // opens config.json
    FILE *f = fopen(configPath, "r");
    error(!f, "program", "The config file does exist, but can not be open.");

    // loads and read the config file
    // gets the size
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);
    // gets the data
    char *data = malloc(size + 1);
    error(!data, "program", "malloc failed allocating memory for the variable data.");
    size_t readBytes = fread(data, 1, size, f); // 1 --> size of each item

    if (readBytes != size) {
        free(data);
        fclose(f);
    }
    error(readBytes != size, "program",
          "Failed to read config file (%s) (%zu bytes read, expected %ld)", configPath, readBytes,
          size);
    data[size] = '\0';
    fclose(f);

    // parse the JSON
    debug("Parsing the JSON config");

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        free(data);
    }
    error(!json, "program", "JSON parse error");

    // Parse all of the directories which will( or do) contain the vaults
    cJSON *dirJson = cJSON_GetObjectItem(json, "directory");
    error(dirJson && !cJSON_IsArray(dirJson), "user",
          "In %s, \"directory\" is missing or isn't an array", configPath);
    int numDirectories = cJSON_GetArraySize(dirJson);
    debug("In %s, detected %d paths in \"directory\"", configPath, numDirectories);
    error(numDirectories == 0, "user", "In %s, \"directory\" is an empty array.", configPath);

    char **directoriesArray = malloc(numDirectories * sizeof(char *));
    debug("Directories:");
    cJSON *tempEntry = NULL;
    int i = 0;
    cJSON_ArrayForEach(
        tempEntry,
        dirJson) { // iterate over all the elements of the array _i. e._ over all the dirs
        if (tempEntry && cJSON_IsString(tempEntry)) {
            if (cJSON_GetStringValue(tempEntry)[0] == '~') { // we must expand ~
                char *tempUnFixedName = cJSON_GetStringValue(tempEntry);
                tempUnFixedName++; // shifts and removes the ~
                directoriesArray[i] = malloc(PATH_MAX);
                debug("~ in %s was expanded to %s", tempUnFixedName, homedir);
                snprintf(directoriesArray[i], PATH_MAX, "%s%s", homedir, tempUnFixedName);
            } else {
                directoriesArray[i] = strdup(cJSON_GetStringValue(tempEntry));
                altDebug("%s\n", directoriesArray[i]);
            }
        } else {
            error(1, "user", "In %s, in \"directory\", invalid type of one of the entries.",
                  configPath);
        }
        i++;
    }

    // fetch the render and jumpToEnfOfFileOnLaunch bools
    int shouldRender = 1;
    cJSON *shouldRenderJSON = cJSON_GetObjectItem(json, "render");
    if (shouldRenderJSON && cJSON_IsBool(shouldRenderJSON)) {
        shouldRender = cJSON_IsTrue(shouldRenderJSON) ? 1 : 0;
        debug("In %s, \"render\" was set to %d.", configPath, shouldRender);
    } else {
        debug("In %s, \"render\" wasn't set. Defaulting to true.", configPath);
    }

    int shouldJumpToEnd = 1;
    cJSON *shouldJumpToEndJSON = cJSON_GetObjectItem(json, "jumpToEndOfFileOnLaunch");
    if (shouldJumpToEndJSON && cJSON_IsBool(shouldJumpToEndJSON)) {
        shouldJumpToEnd = cJSON_IsTrue(shouldJumpToEndJSON) ? 1 : 0;
        debug("In %s, \"jumpToEndOfFileOnLaunch\" was set to %d.", configPath, shouldJumpToEnd);

    } else {
        debug("In %s, \"jumpToEndOfFileOnLaunch\" wasn't set or we encountered a abnormal type. "
              "Defaulting to true.",
              configPath);
    }

    char *editorToOpen = getenv("EDITOR"); // default to $EDITOR
    int defaultEditor =
        1; // this will be used in the warning if the editor does not exists or is unsupported.
    cJSON *editorToOpenJSON = cJSON_GetObjectItem(json, "editor");
    if (editorToOpenJSON && cJSON_IsString(editorToOpenJSON)) {
        editorToOpen = strdup(cJSON_GetStringValue(
            editorToOpenJSON)); // we must strdup and not just = as we will free all the json after
                                // (before parsing args)
        defaultEditor = 0;
        debug("In %s, \"editor\" was set to %s.", configPath, editorToOpen);
        // error(!isStringInArray(editorToOpen, supportedEditor, numEditors), "user", "%s (fetched
        // from config.json) is not a supported editor.", editorToOpen); // we check if editor is
        // supported at the end
    } else {
        debug("In %s, \"editor\" wasn't set or we encountered a abnormal type. Defaulting to "
              "$EDITOR (%s).\n P.S. this might still be overwritten by -e or --editor.",
              configPath, editorToOpen);
    }

    cJSON *journalRegexJSON = cJSON_GetObjectItem(json, "journalRegex");
    char *journalRegex = ".*journal.*"; // default regex pattern for the journal
    if (journalRegexJSON && cJSON_IsString(journalRegexJSON)) {
        journalRegex = strdup(cJSON_GetStringValue(journalRegexJSON));
        debug("In %s, \"journalRegex\" was set to %s.", configPath, journalRegex);
    } else {
        debug("In %s, \"journalRegex\" wasn't set or we encountered a abnormal type. Defaulting to "
              "%s.",
              configPath, journalRegex);
    }

    char *timeFormat = "# \%Y \%m \%d \%a"; // default
    cJSON *timeFormatJSON = cJSON_GetObjectItem(json, "dateEntry");
    if (timeFormatJSON && cJSON_IsString(timeFormatJSON)) {
        timeFormat = strdup(cJSON_GetStringValue(timeFormatJSON));
        debug("In %s, \"dateEntry\" was set to %s.", configPath, timeFormat);
    } else {
        debug(
            "In %s, \"dateEntry\" wasn't set or we encountered a abnormal type. Defaulting to %s.",
            configPath, timeFormat);
    }
    int newLineOnOpening = 1;
    cJSON *newLineOnOpeningJSON = cJSON_GetObjectItem(json, "newLineOnOpening");
    if (newLineOnOpeningJSON && cJSON_IsBool(newLineOnOpeningJSON)) {
        debug("The value for newLineOnOpening in config.json is %d",
              cJSON_IsTrue(newLineOnOpeningJSON));

        newLineOnOpening = cJSON_IsTrue(newLineOnOpeningJSON) ? 1 : 0;
        debug("In %s, \"newLineOnOpening\" was set to %d.", configPath, newLineOnOpening);
    } else {
        debug("In %s, \"newLineOnOpening\" wasn't set or we encountered a abnormal type. "
              "Defaulting to true.",
              configPath);
    }

    int doesBackup = 0;
    int interval =
        0; // this is an int. But some times it will be inputed a string. We must translate it.
    // to handle linking each directory to its backup path. We create an array of char*. If we want
    // to backup the ith directory, the ith pointer will point to the path. Else, we set the ith
    // pointer to NULL.
    char **backupDirectoriesArray = malloc(numDirectories * sizeof(char *));
    char **rsyncArgs = NULL;
    int rsyncArgsNumber = 0;
    cJSON *backupJSON = cJSON_GetObjectItem(json, "backup");
    if (backupJSON && cJSON_IsObject(backupJSON)) {
        cJSON *doesBackupJSON = cJSON_GetObjectItem(backupJSON, "enable");
        if (doesBackupJSON && cJSON_IsBool(doesBackupJSON)) {
            doesBackup = cJSON_IsTrue(doesBackupJSON) ? 1 : 0;
            debug("doesBackup is set to %d", doesBackup);
            if (!doesBackup) {
                debug("a backup is not needed. Skipping parsing the rest of the json");
                goto backup_config_end; // easier to just go to rather than do a big if statement
            }

            // handles the path to the backup for each directory
            cJSON *pathToBackupJSON = cJSON_GetObjectItem(backupJSON, "directory");
            debug("%s", cJSON_Print(pathToBackupJSON));
            error(pathToBackupJSON == NULL &&
                      !(cJSON_IsObject(pathToBackupJSON) || cJSON_IsArray(pathToBackupJSON) ||
                        cJSON_IsString(pathToBackupJSON) || cJSON_IsNumber(pathToBackupJSON) ||
                        cJSON_IsBool(pathToBackupJSON)),
                  "user", "In %s, incorrect type for \"directory\". It must be a JSON object.",
                  configPath); // for some reason cJSON_IsObject does not work. So we must do it
                               // like this.

            // we can't just iterate over directoriesArray as we replaced ~ to $HOME
            // we must iterate over the json entries
            cJSON *directoryEntry = NULL;
            int i = 0;
            cJSON_ArrayForEach(
                directoryEntry,
                dirJson) { // iterate over all the elements of the array _i. e._ over all the dirs
                // we don't need to type error handle as we did it before
                char *directoryEntryPath = cJSON_GetStringValue(directoryEntry);
                cJSON *pathToBackupForIthDirectoryJSON =
                    cJSON_GetObjectItem(pathToBackupJSON, directoryEntryPath);
                if (pathToBackupForIthDirectoryJSON) { // if the entry exist
                    char *textPath = cJSON_GetStringValue(pathToBackupForIthDirectoryJSON);
                    if (textPath[0] == '~') {
                        textPath++; // cuts the ~
                        backupDirectoriesArray[i] = malloc(PATH_MAX);
                        error(backupDirectoriesArray[i] == NULL, "program", "malloc failed");
                        snprintf(backupDirectoriesArray[i], PATH_MAX, "%s%s", homedir, textPath);
                    } else {
                        backupDirectoriesArray[i] = strdup(textPath);
                    }
                    debug("%s will be backed up to %s", directoriesArray[i],
                          backupDirectoriesArray[i]);
                } else { // we set it to NULL to be sure we won't backup it
                    backupDirectoriesArray[i] = NULL;
                    debug("%s won't be backed up", directoriesArray[i]);
                }
                i++;
            }

            // handles the interval of backup
            cJSON *intervalJSON = cJSON_GetObjectItem(backupJSON, "interval");
            if (intervalJSON && cJSON_IsString(intervalJSON)) {
                char *temp = cJSON_GetStringValue(
                    intervalJSON); // se comment higher. temp will be freed when calling free(json)
                if (strcmp(temp, "daily") == 0) {
                    interval = DAILY;
                    debug("interval in %s is set to \"daily\" which is %d", configPath, DAILY);
                } else if (strcmp(temp, "weekly") == 0) {
                    interval = WEEKLY;
                    debug("interval in %s is set to \"weekly\" which is %d", configPath, WEEKLY);
                } else if (strcmp(temp, "monthly") == 0) {
                    interval = MONTHLY;
                    debug("interval in %s is set to \"monthly\" which is %d", configPath, MONTHLY);
                } else {
                    error(1, "user",
                          "Unexpected string %s for entry \"interval\" in %s. You must put an int "
                          "(number of seconds) or \"daily\" or \"weekly\" or \"monthly\".",
                          intervalJSON->valuestring, configPath);
                }
            } else if (intervalJSON && cJSON_IsNumber(intervalJSON)) {
                interval = (int)cJSON_GetNumberValue(intervalJSON);
                debug("interval in %s is set to %d", configPath, interval);
            } else {
                error(1, "user",
                      "%s did not contained an interval value inside the backup section or the "
                      "value is from an unexpected type",
                      configPath);
            }
        } else {
            error(1, "user",
                  "%s did not contained a enable value inside the backup section or the value is "
                  "from an unexpected type",
                  configPath);
        }
        // handle rsyncs array of arguments.
        cJSON *rsyncArgsJSON = cJSON_GetObjectItem(backupJSON, "rsyncArgs");
        if (rsyncArgsJSON && cJSON_IsArray(rsyncArgsJSON)) { // if it is what we expected
            rsyncArgsNumber = cJSON_GetArraySize(rsyncArgsJSON);
            debug("In %s, rsyncArgsNumber is calculated to %d.", configPath, rsyncArgsNumber);
            rsyncArgs = realloc(rsyncArgs, (size_t)rsyncArgsNumber);
            debug("In %c, rsyncArgs are:", configPath);
            for (int i = 0; i < rsyncArgsNumber; i++) {
                cJSON *argJSON = cJSON_GetArrayItem(rsyncArgsJSON, i);
                if (argJSON && cJSON_IsString(argJSON)) {
                    rsyncArgs[i] = strdup(cJSON_GetStringValue(argJSON));
                    altDebug("%s\n", rsyncArgs[i]);
                } else {
                    error(1, "user", "One element in rsyncArgs array in %s is not a string",
                          configPath);
                }
            }
        } else {
            error(1, "user",
                  "%s did not contained a rsyncArgs array inside the backup section or the value "
                  "is from an unexpected type",
                  configPath);
        }
    } else {
        debug("In %s, \"backup\" wasn't set or we encountered a abnormal type. Defaulting to "
              "{\"enable\": false}.",
              configPath);
    }
backup_config_end:

    // cleans up
    cJSON_Delete(json);
    free(data);
    debug("Finished parsing the JSON config");
    //---------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------

    // flags and arguments overwrite the config
    debug("Parsing the attribute flags.\n This flags might overwrite the options in the config "
          "file.");
    // thoses bypasses are used if some specific flags are passed as args. This allows to bypass the
    // TUI selectors
    int bypassSelectionVault = 0;
    char *bypassSelectionVaultValue = NULL;
    int bypassSelectionNote = 0;
    char *bypassSelectionNoteValue = NULL;
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (strncmp(arg, "--", 2) == 0) {

            if (strcmp(arg, "--verbose") == 0) {
                shouldDebug = 1;
                debug("--verbose enabled");

            } else if (strcmp(arg, "--config") == 0) {
                error(i + 1 == argc, "user", "Missing argument for --config");
                overwriteConfigPath = ++i;
                debug("--config set to %s", argv[i]);

            } else if (strcmp(arg, "--editor") == 0) {
                error(i + 1 == argc, "user", "Missing argument for --editor");
                editorToOpen = argv[++i];
                defaultEditor = 0;
                debug("--editor set to %s", editorToOpen);

            } else if (strcmp(arg, "--note") == 0) {
                error(i + 1 == argc, "user", "Missing argument for --note");
                bypassSelectionNote = 1;
                bypassSelectionNoteValue = argv[++i];
                debug("--note set to %s", bypassSelectionNoteValue);

            } else if (strcmp(arg, "--vault") == 0) {
                error(i + 1 == argc, "user", "Missing argument for --vault");
                bypassSelectionVault = 1;
                bypassSelectionVaultValue = argv[++i];
                debug("--vault set to %s", bypassSelectionVaultValue);

            } else if (strcmp(arg, "--render") == 0) {
                shouldRender = 1;
                debug("--render enabled");

            } else if (strcmp(arg, "--no-render") == 0) {
                shouldRender = 0;
                debug("--render disabled");

            } else if (strcmp(arg, "--jump") == 0) {
                shouldJumpToEnd = 1;
                debug("--jump enabled");

            } else if (strcmp(arg, "--no-jump") == 0) {
                shouldJumpToEnd = 0;
                debug("--jump disabled");

            } else if (strcmp(arg, "--help") == 0) {
                printf("Usage: notewrapper [options]\n");
                printf("Options:\n");
                printf("  -c, --config <path/to/config>               Specify the config file.\n");
                printf("  -h, --help                                  Display this message.\n");
                printf(
                    "  -e, --editor                                Specify the editor to open.\n");
                printf("  -j, --jump                                  Jumps to the end of the file "
                       "on opening.\n");
                printf("  -J, --no-jump                               Do not jump to the end of "
                       "the file\n");
                printf("  -n, --note  <note's name>                   Specify the note (or "
                       "journal).\n");
                printf("  -r, --render                                Renders the note with "
                       "Vivify.\n");
                printf("  -R, --no-render                             Do not render.\n");
                printf("  -v, --vault <vault's name>                  Specify the vault.\n");
                printf("  --version                                   Display the program version "
                       "and the GPL3 notice.\n");
                printf("  -V, --verbose                               Show debug information.\n");
                return 0;

            } else if (strcmp(arg, "--version") == 0) {
                printf("NoteWrapper %s\n", VERSION);
                return 0;

            } else {
                error(1, "user", "Unknown option \"%s\"", arg);
            }

        } else if (arg[0] == '-' && arg[1] != '\0') {

            for (int j = 1; arg[j] != '\0'; j++) {
                char opt = arg[j];

                switch (opt) {

                // -------- flags without arguments (can be combined) --------
                case 'r':
                    shouldRender = 1;
                    debug("-r enabled");
                    break;

                case 'R':
                    shouldRender = 0;
                    debug("-R disabled");
                    break;

                case 'j':
                    shouldJumpToEnd = 1;
                    debug("-j enabled");
                    break;

                case 'J':
                    shouldJumpToEnd = 0;
                    debug("-J disabled");
                    break;

                case 'V':
                    shouldDebug = 1;
                    debug("-V enabled");
                    break;

                case 'h':
                    printf("Usage: notewrapper [options]\n");
                    printf("Options:\n");
                    printf(
                        "  -c, --config <path/to/config>               Specify the config file.\n");
                    printf("  -h, --help                                  Display this message.\n");
                    printf("  -e, --editor                                Specify the editor to "
                           "open.\n");
                    printf("  -j, --jump                                  Jumps to the end of the "
                           "file on opening.\n");
                    printf("  -J, --no-jump                               Do not jump to the end "
                           "of the file\n");
                    printf("  -n, --note  <note's name>                   Specify the note (or "
                           "journal).\n");
                    printf("  -r, --render                                Renders the note with "
                           "Vivify.\n");
                    printf("  -R, --no-render                             Do not render.\n");
                    printf("  -v, --vault <vault's name>                  Specify the vault.\n");
                    printf("  --version                                   Display the program "
                           "version and the GPL3 notice.\n");
                    printf(
                        "  -V, --verbose                               Show debug information.\n");
                    return 0;

                // -------- flags with arguments (MUST be last in group) --------
                case 'e':
                case 'n':
                case 'v':
                case 'c': {

                    error(arg[j + 1] != '\0', "user", "-%c must not be combined with other flags",
                          opt);

                    error(i + 1 == argc, "user", "Missing argument for -%c", opt);

                    char *value = argv[++i];

                    switch (opt) {
                    case 'e':
                        editorToOpen = value;
                        defaultEditor = 0;
                        debug("-e set editor to %s", value);
                        break;

                    case 'n':
                        bypassSelectionNote = 1;
                        bypassSelectionNoteValue = value;
                        debug("-n set note to %s", value);
                        break;

                    case 'v':
                        bypassSelectionVault = 1;
                        bypassSelectionVaultValue = value;
                        debug("-v set vault to %s", value);
                        break;

                    case 'c':
                        overwriteConfigPath = i;
                        debug("-c set config to %s", value);
                        break;
                    }

                    goto next_arg;
                }

                default:
                    error(1, "user", "Unknown option -%c", opt);
                }
            }

        } else {
            error(1, "user", "Unexpected argument \"%s\"", arg);
        }

    next_arg:;
    }
    // if -n or --note is set but not -v or --vaults it gives an error
    error(!bypassSelectionVault && bypassSelectionNote, "user",
          "If you want to specify the note, you must also specify the vault with -v <vault's name> "
          "or --vault <vault's name>.");
    debug("Finished parsing the attribute flags");

    isEditorValid(editorToOpen, defaultEditor,
                  shouldDebug); // check if editor is supported and if it is installed. If not, it
                                // will throw an error.

    if (doesBackup) {
        debug("Handling backup");
        handleBackups(directoriesArray, numDirectories, backupDirectoriesArray, homedir, interval,
                      (const char **)rsyncArgs, rsyncArgsNumber, shouldDebug);
    }

    initscr(); // initialize ncurses

    int shouldExit = 0;
    while (!shouldExit) {
        // this loop is the vault selector
        // select a vault
        char *vaultSelected = NULL;

        int vaultsCount = 0;
        int *vaultsCountForEachDirectory = malloc(numDirectories * sizeof(int));
        char **vaultsArray = getVaultsFromDirectories(
            directoriesArray, numDirectories, vaultsCountForEachDirectory, &vaultsCount,
            shouldDebug); // when selecting each vault won't show the directory from which they
                          // come. We will find this later.

        // bypass if -v or --vault is set
        if (bypassSelectionVault) {
            // bypasses the vault selection if the flag is -v. If the vault doesn't exist, just
            // create a new one
            vaultSelected = bypassSelectionVaultValue;
            debug("bypassing vault selection");
            if (!isStringInArray(
                    bypassSelectionVaultValue, (const char **)vaultsArray,
                    vaultsCount)) { // we pass just vaultsCount and not vaultsCount + extraOptions
                                    // to avoid matching with the extraOptions.
                debug("[BYPASS] %s did not exist. Creating a new vault");
                goto vault_creation;
            } else {
                goto note_selection;
                debug("[BYPASS] %s does exist. Going to note selection");
            }
        }

        debug("┌--------------------------------\nAvailable vaults:");
        if (shouldDebug) {
            for (int i = 0; i < vaultsCount; i++) {
                altDebug("%s\n", vaultsArray[i]);
            }
            altDebug("└ ------------------------------\n");
        }

        // adds "create a new vault" into the vaultsArray
        const int extraOptions = 3;
        vaultsArray =
            realloc(vaultsArray, (vaultsCount + extraOptions) *
                                     sizeof(char *)); // resize vaultsArray to fit the extra options
        vaultsArray[vaultsCount] = "Create a new vault"; // some more options that are not vaults
        vaultsArray[vaultsCount + 1] = "Settings";
        vaultsArray[vaultsCount + 2] = "Quit (Ctrl+C)";

        vaultSelected = ncursesSelect(
            vaultsArray, "Select vault to open (Use arrows or WASD, Enter to select):", vaultsCount,
            extraOptions, " ", "Or select an option below", "", shouldDebug);

        // now that we won't use vaultsArray in this iteration of the loop, we should free it and
        // all its elements. (As this is memory in the heap and not the stack and thus is our
        // responsability to manage)
        for (int i = 0; i < vaultsCount; i++) {
            if (vaultSelected !=
                vaultsArray[i]) { // i forgot this condition before. and freed the pointer equal to
                                  // vaultSelected... So don't remove this condition
                free(vaultsArray[i]); // we must only free the vaults options and not the
                                      // extraOptions to avoid segfault
            }
        }

        debug("Selected vault: %s", vaultSelected);
        if (strcmp(vaultSelected, "Create a new vault") != 0 &&
            strcmp(vaultSelected, "Settings") != 0 && strcmp(vaultSelected, "Quit (Ctrl+C)") != 0) {
        note_selection:
            bypassSelectionVault = 0; // we must reset bypassSelectionVault to not get stuck in a
                                      // infinite loop of bypassing
            int shouldChangeVault = 0;
            // we must find the directory from which the vault comes again.
            char *notesDirectoryString = getDirectoryFromVault(
                vaultSelected, vaultsArray, vaultsCount, vaultsCountForEachDirectory,
                directoriesArray, numDirectories, shouldDebug);
            while (!shouldExit && !shouldChangeVault) {
                // this loop is the note selector
                int filesCount = 0;
                char **filesArray = getNotesFromVault(notesDirectoryString, vaultSelected,
                                                      journalRegex, &filesCount, shouldDebug);

                int journalCount = 0;
                char **journalArray = getJournalsFromVault(
                    notesDirectoryString, vaultSelected, journalRegex, &journalCount, shouldDebug);

                // appends the journal at the end of filesArray
                filesArray = realloc(filesArray, (filesCount + journalCount) * sizeof(char *));
                for (int i = 0; i < journalCount; i++) {
                    filesArray[i + filesCount] = journalArray[i];
                }
                filesCount = filesCount + journalCount;

                qsort(filesArray, filesCount, sizeof(const char *), compareString);
                debug("Available notes and journals:");
                if (shouldDebug) {
                    for (int i = 0; i < filesCount; i++) {
                        altDebug("%s\n", filesArray[i]);
                    }
                    altDebug("└------------------------------\n");
                }
                // adds options
                int extraNotesOptions = 4;
                filesArray = realloc(
                    filesArray, (filesCount + extraNotesOptions) *
                                    sizeof(char *)); // resize filesArray to fit the extra options
                filesArray[filesCount] = "Create new note";
                filesArray[filesCount + 1] = "Back to vault selection";
                filesArray[filesCount + 2] = "Delete vault";
                filesArray[filesCount + 3] = "Quit (Ctrl+C)";
                char *noteSelected;
                // if we set to bypass the note selector
                if (bypassSelectionNote) {
                    debug("We are bypassing note selection.");
                    noteSelected = bypassSelectionNoteValue;
                    if (isStringInArray(
                            noteSelected, (const char **)filesArray,
                            filesCount)) { // we just give filesCount and not filesCount +
                                           // extraOptions to avoid matching with an extra options.
                        debug("The note specified with -n or --note does exist. Opening it.");
                        goto open_note;
                    } else { // if the specified note doesn't exist. We creat it
                        debug("The note specified with -n or --note doesn't exist. Creating it.");
                        goto note_creation;
                    }
                }
                noteSelected = ncursesSelect(
                    filesArray,
                    "Select note or journal to open (Use arrows or WASD, Enter to select):",
                    filesCount, extraNotesOptions, " ", "Or select an option below", "",
                    shouldDebug);
                // now that we won't use filesArray in this iteration of the loop, we should free it
                // and all its elements. (As this is memory in the heap and not the stack and thus
                // is our responsability to manage)
                for (int i = 0; i < filesCount; i++) {
                    if (noteSelected != filesArray[i]) { // we must prevent noteSelected to be
                                                         // freed. It will cause a lot of problems
                        free(filesArray[i]); // we must only free the files options and not the
                                             // extraOptions to avoid segfault
                    }
                }
                free(filesArray);
                debug("Selected note: %s", noteSelected);
                if (strcmp(noteSelected, "Create new note") != 0 &&
                    strcmp(noteSelected, "Back to vault selection") != 0 &&
                    strcmp(noteSelected, "Delete vault") != 0 &&
                    strcmp(noteSelected, "Quit (Ctrl+C)") != 0) {
                open_note:
                    bypassSelectionNote =
                        0; // we must reset bypassSelectionNote to avoid getting into an infinite
                           // loop of bypassing the note selection
                    char *fullPath = malloc(PATH_MAX);
                    snprintf(fullPath, PATH_MAX, "%s/%s/%s", notesDirectoryString, vaultSelected,
                             noteSelected);
                    // if it is a journal we must update it before
                    regex_t regex;
                    int regexReturn = regcomp(&regex, journalRegex, 0);
                    error(regexReturn, "program", "Regex compilation failed.");
                    regexReturn = regexec(&regex, noteSelected, 0, NULL, 0);

                    int *journalWasUpdated = malloc(sizeof(int));
                    *journalWasUpdated = 0;
                    if (!regexReturn) { // if the regex matches -> it's a journal
                        debug("%s is a journal. Updating it...", noteSelected);
                        fullPath = updateJournal(
                            fullPath, noteSelected, timeFormat, journalWasUpdated,
                            shouldDebug); // we return the path. As if it is a divided journal we
                                          // must point to the correct entry
                    }
                    if (newLineOnOpening) {
                        if (*journalWasUpdated) {
                            appendToFile(
                                fullPath, " \n",
                                shouldDebug); // when updating the journal it adds a \n char at the
                                              // end. So appendToFile(\n) does not work. We append a
                                              // (special and rare) whitespace character + \n to
                                              // bypass this issue.
                        }
                        appendToFile(fullPath, "\n", shouldDebug);
                    }
                    openEditor(fullPath, editorToOpen, shouldRender, shouldJumpToEnd, shouldDebug);
                    free(fullPath);
                } else if (strcmp(noteSelected, "Create new note") == 0) {
                note_creation:
                    noteSelected =
                        createNewNote(notesDirectoryString, vaultSelected, bypassSelectionNote,
                                      bypassSelectionNoteValue, journalRegex, shouldDebug);
                    // we can just go back to open_note
                    goto open_note;
                } else if (strcmp(noteSelected, "Back to vault selection") == 0) {
                    shouldChangeVault = 1;
                } else if (strcmp(noteSelected, "Delete vault") == 0) {
                    // we must find where does the vault comes from.
                    char *notesDirectoryString = getDirectoryFromVault(
                        vaultSelected, vaultsArray, vaultsCount, vaultsCountForEachDirectory,
                        directoriesArray, numDirectories, shouldDebug);
                    const char *yesNo[] = {"No, go back to note selection.", "Yes."};
                    char *answer =
                        ncursesSelect((char **)yesNo,
                                      "Are you sure you want to delete the entire vault? This can "
                                      "not be undone (Use arrows or WASD, Enter to select):",
                                      1, 1, " ", "", "", shouldDebug);
                    debug("You answered: %s for deleting the vault %s", answer, vaultSelected);
                    if (strcmp(answer, "Yes.") == 0) {
                        // delete the vault after confirmation by the user
                        char pathToRMRF[PATH_MAX];
                        snprintf(pathToRMRF, PATH_MAX, "%s/%s", notesDirectoryString,
                                 vaultSelected);
                        debug("Removed the directory: %s", pathToRMRF);
                        rmrf(pathToRMRF, shouldDebug);
                        shouldChangeVault = 1;
                    }
                } else if (strcmp(noteSelected, "Quit (Ctrl+C)") == 0) {
                    debug("The program was exited.");
                    shouldExit = 1;
                }
            }
            free(vaultsArray);
        } else if (strcmp(vaultSelected, "Create a new vault") == 0) {
        vault_creation:
            createNewVault(directoriesArray, numDirectories, vaultsArray, vaultsCount,
                           bypassSelectionVault, bypassSelectionVaultValue, shouldDebug);
            bypassSelectionVault = 0; // we need to reset bypassSelectionVault to avoid getting into
                                      // an infinite loop of bypassing
        } else if (strcmp(vaultSelected, "Settings") == 0) {
            openEditor(
                configPath, editorToOpen, 0, 0,
                shouldDebug); // as this is not a md file we set render and jumptoEnfOfFile to 0
        } else if (strcmp(vaultSelected, "Quit (Ctrl+C)") == 0) {
            debug("The program was exited");
            shouldExit = 1;
        }
    }
    free(configPath);
    free(directoriesArray);
    return 0;
}
