### Why?
I started journaling and notetaking with **[Obsidian](https://obsidian.md/)**, but wanted to only use free software. I also tried **[Logseq](https://logseq.com/)** and **[Joplin](https://github.com/laurent22/joplin)**, but preferred a terminal-based app. 

Using my Neovim setup, I explored plugins like **[neorg](https://github.com/nvim-neorg/neorg)**, **[orgmode](https://github.com/nvim-orgmode)**, and **[today.nvim](https://github.com/VVoruganti/today.nvim)**. While they offered useful features, none fully met my needs: some lacked external Markdown rendering, some used custom file formats, and some didn’t provide a true journal mode. For real-time Markdown rendering from Neovim, I found **[Vivify](https://github.com/jannis-baum/Vivify/)**, which NoteWrapper relies on.

The goal was a terminal-based interface for accessing vaults and notes using standard Markdown, with minimal extra features. Although it currently works only with Neovim, it is designed as a standalone wrapper that could be adapted for other editors with minimal changes.

### How to install
1. Have installed `ncurses`, `cjson`, `make`, `Vivify`, `rsync`, `pkg-config`, `sed`, `ripgrep` and `fzf`. <TODO LATER we should put hyprlinks to each program.>
2. Have a supported editor: `vim`, `neovim` and `nano` (for the moment)
3. Clone the repository and enter it.
```shell
git clone https://github.com/Totorile1/NoteWrapper.git
cd NoteWrapper
```
4. Compile the project. On NixOS run `nix-build`. Other OS, run `make`.
5. [configure](#configuration) `./config.json` (the file is created on launching, if it already doesn't exist).
6. run the binary `notewrapper`

### Usage
```
Usage: notewrapper [options]
Options:
  -c, --config <path/to/config>               Specify the config file.
  -d, --directory <path/to/directory>         Specify the vaults' directory.
  -h, --help                                  Display this message.
  -e, --editor                                Specify the editor to open.
  -j, --jump                                  Jumps to the end of the file on opening.
  -J, --no-jump                               Do not jump to the end of the file
  -n, --note  <note's name>                   Specify the note (or journal).
  -r, --render                                Renders the note with Vivify.
  -R, --no-render                             Do not render.
  -v, --vault <vault's name>                  Specify the vault.
  --version                                   Display the program version and the GPL3 notice.
  -V, --verbose                               Show debug information.
```
### Editor support

`NoteWrapper` relies on certain editor features, so not all of its functionality is supported by every editor.
Here are the features that depend on editor support:

- **Bufferless rendering**: As soon as you write something in the note (even without saving), it is rendered.
- **Cursor following**: The rendered view follows your cursor. For example, if you move to the bottom of a note, the rendered view will also scroll to the bottom.
- **Jump to end on open**: Automatically jumps to the end of the file when opening it.

The first two features depend on [Vivify's plugin for editors](https://github.com/jannis-baum/Vivify?tab=readme-ov-file#existing-integration) and are only usefull if you want to render the markdown externally on your browser. If your editor misses these features, you can just create a plugin or an extension which uses [Vivify's simple API](https://github.com/jannis-baum/Vivify?tab=readme-ov-file#editor-support).


| Editor  | Bufferless | Cursor | Jump to end | Necessary plugin |
|---------|------------|--------|-------------|------------------|
| Neovim  | ✅         | ✅     | ✅          | [vivify-vim](https://github.com/jannis-baum/vivify.vim) |
| Vim     | ✅         | ✅     | ✅          | [vivify-vim](https://github.com/jannis-baum/vivify.vim) |
| Nano    | ❌         | ❌     | ✅          | |

### Configuration
Change `~/.config/notewrapper/config.json`. If it does not exist, the program should create on launch.

```json
{
  "directory": "~/Documents/Notes/",
  "render": true,
  "jumpToEndOfFileOnLaunch": true,
  "editor": "neovim",
  "journalRegex": ".*journal.*",
  "dateEntry": "# %a %d %m %Y",
  "newLineOnOpening": true,
  "backup": {
    "enable": false,
    "directory": "/path/to/backup",
    "interval": "weekly",
    "rsyncArgs": ["-Lqah", "--update"]
  }
}
```
`directory` is the directory where the program will search for vaults.

`render` is a boolean that tells the program if it needs to render the markdown file using `Vivify`

`jumpToEndOfFileOnLaunch` is a boolean that tells the programs if it needs to set the cursor at the end of the file on launch.

`editor` is the editor the program will call. (It must be a supported editor.

`journalRegex` is a regex code. If the note name matches with this code, the program will treat it as a journal.

`dateEntry` is the style of the file title or paragraph title (depending on the type of journal). See [strftime](https://pubs.opengroup.org/onlinepubs/7908799/xsh/strftime.html?) for more info.

`newLineOnOpening` is a boolean that tells the program to append a newline when opening a note.

`backup.enable` enable the backups. The program relies on `rsync` for backuping.

`backup.directory` is the directory where the backup will go.

`backup.interval` can either be `daily`, `weekly`, `monthly` or an integer. It specifies the interval between two backups.
`rsyncArgs` are the array of arguments that will be passed into rsync. See `man rsync` or `rsync --help`.

#### Vivify's Configuration

Some settings must be set in Vivify's configuration file. See [documentation](https://github.com/jannis-baum/Vivify/blob/main/docs/customization.md).

I recommend changing the browser settings to a browser different to your main browser.

### What needs to be done (a lot)
- [x] Add a way to create vaults
- [x] Add a way to delete vaults
- [ ] Add a way to delete notes
- [x] Stylisize a bit the TUI
- [x] Add more options to the config file
- [x] Search for config.json in other directories such as ~/.config/notewrapper/ and not only this directory
- [x] Port vivify.vim to nixpkgs
- [ ] Add a way to have vaults in different directories
- [x] some kind of FZF search for notes
- [x] A button to randomly select a note or an entry in a journal
- [x] Fix crash when the window is resized
- [x] Actually open vivify when opening nvim
- [x] Write the journaling code (separate files or one big journal files)
- [x] Adapt createNewNote with journals
- [x] Automatic backups
- [x] Add flags for customization
- [ ] Write a good README.md
- [x] Option to not render and to only open nvim
- [x] Figure out this vivify issue https://github.com/jannis-baum/Vivify/issues/291
- [ ] Port this to other editors
- [x] Port to vim
- [ ] Fixe all the small stuff marked //(TODO LATER) in the code
- [x] Comply with GPL-3 notice (add info about no waranty, etc.)
- [ ] A converter for both type of journals

