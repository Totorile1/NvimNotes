# Contributing to NoteWrapper

Thank you for your interest in contributing to NoteWrapper.

This project aims to remain minimal, terminal-focused, and easy to extend. Contributions should respect this design philosophy.

---

## Debugging and error handling

Please use the standardized debugging and error-handling functions defined in [/src/utils.c](./src/utils.c):

* `debug(...)`
  Used for formatted debugging output (printf-style formatting).

* `altDebug(...)`
  A lighter version of `debug()` when full formatting is not required.

* `error(condition, "user" | "program", "error message", ...)`
  Standard error reporting function (printf-style formatting):

  * `"user"`: errors caused by user input or configuration
  * `"program"`: internal or unexpected program errors

---

## Commit naming convention

This is mostly for me, as my commit messages can sometimes be cryptic and hard to follow later.

Format:

```
<module> (#issue): description
```

Examples:

* `Editor (#1): add support for Microsoft Word`
* `docs: fix typos in README.md`
* `Journal: improve entry parsing logic`

Keep commit messages short and descriptive.

---

## Adding editor support

To add support for a new editor, you must update both documentation and source code:

* Update `README.md`:

  * Add the editor to the **Editor support** table
  * Update the **Dependencies** section if necessary

* In `src/utils.c`:

  * Add the editor name to `supportedEditor[]`
  * Increment `numEditor`

* If the editor executable name differs from its logical name (for example `neovim` → `nvim`), update `doesEditorExist()` in `src/utils.c`

* Modify `openEditor()` in `src/utils.c`:

  * If the editor supports a Vivify plugin integration, follow the pattern used for **(neo)vim**
  * If it does not support integration, NoteWrapper must launch Vivify separately (see implementation used for **nano**)

---

## Changing default configuration

If you want to change the default configuration or add a new option:
1. Update [README's documentation](./README.md#configuration)
2. In `./src/utils.c`, update the function `initAppFilesAndDirs()` which creates a default configuration if `~/.config/notewrapper/config.json` or if neither `-c` nor `--config` is set.
3. In `./src/main.c` where the JSON is parsed, add the parsing logic. Do not forget the debugging information, errors when wrong type or when an important field is missing and add a default value if a less important field is missing.

---

## Before opening a pull request

Before submitting a pull request, ensure that:

* The project compiles successfully using:

  * `make`, or
  * `nix-build` (on NixOS)
* There are **no warnings or errors**
