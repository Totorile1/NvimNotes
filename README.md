### Why?
I started journaling and notetaking with **[Obsidian](https://obsidian.md/)**, but wanted to only use free software. I also tried **[Logseq](https://logseq.com/)** and **[Joplin](https://github.com/laurent22/joplin)**, but preferred a terminal-based app. 

Using my Neovim setup, I explored plugins like **[neorg](https://github.com/nvim-neorg/neorg)**, **[orgmode](https://github.com/nvim-orgmode)**, and **[today.nvim](https://github.com/VVoruganti/today.nvim)**. While they offered useful features, none fully met my needs: some lacked external Markdown rendering, some used custom file formats, and some didn’t provide a true journal mode. For real-time Markdown rendering from Neovim, I found **[Vivify](https://github.com/jannis-baum/Vivify/)**, which NoteWrapper relies on.

The goal was a terminal-based interface for accessing vaults and notes using standard Markdown, with minimal extra features. Although it currently works only with Neovim, it is designed as a standalone wrapper that could be adapted for other editors with minimal changes.

### How to install
1. Have installed `ncurses`, `cjson` and `pkg-config`.
2. Have a supported editor: `vim`, `neovim` and `nano` (for the moment)
3. Clone the repository.
```shell
git clone https://github.com/Totorile1/NoteWrapper.git
```
4. Compile the project. If you are on NixOS and have direnv installed, run `direnv allow`. It will create the nix-shell which will get all the necessary libraries.
```
cd NoteWrapper
make
```
5. configure `./config.json`
6. run the binary `notewrapper`

### Usage
If you want to render with Vivify with the flags `-r`, `--render` or with the option in the config file, you must have installed the plugins for your specific editor ([for Vim and Neovim](https://github.com/jannis-baum/vivify.vim))


### Editor support

`NoteWrapper` relies on certain editor features, so not all of its functionality is supported by every editor.

Here are the features that depend on editor support:

- **Bufferless rendering**: As soon as you write something in the note (even without saving), it is rendered.
- **Cursor following**: The rendered view follows your cursor. For example, if you move to the bottom of a note, the rendered view will also scroll to the bottom.
- **Jump to end on open**: Automatically jumps to the end of the file when opening it.

The first two features depend on [Vivify's plugin for editors](https://github.com/jannis-baum/Vivify?tab=readme-ov-file#existing-integration).

| Editor  | Bufferless | Cursor | Jump to end |
|---------|------------|--------|-------------|
| Neovim  | ✅         | ✅     | ✅          |
| Vim     | ✅         | ✅     | ✅          |
| Nano    | ❌         | ❌     | ✅          |

### Configuration
Change `$/.config/notewrapper/config.json`. If it does not exist. On building, it should copy a default config.

### What needs to be done (a lot)
- [x] Add a way to create vaults
- [x] Add a way to delete vaults
- [ ] Add a way to delete notes
- [x] Stylisize a bit the TUI
- [x] Add more options to the config file
- [x] Search for config.json in other directories such as ~/.config/notewrapper/ and not only this directory
- [x] Port vivify.vim to nixpkgs
- [ ] Add a way to have vaults in different directories
- [x] Actually open vivify when opening nvim
- [x] Write the journaling code (separate files or one big journal files)
- [x] Adapt createNewNote with journals
- [ ] Automatic backups
- [x] Add flags for customization
- [ ] Write a good README.md
- [x] Option to not render and to only open nvim
- [ ] Figure out this vivify issue https://github.com/jannis-baum/Vivify/issues/291
- [ ] Maybe port this to other editors
- [x] Port to vim
- [ ] Fixe all the small stuff marked //(TODO LATER) in the code
- [ ] Add a way to delete notes
- [x] Comply with GPL-3 notice (add info about no waranty, etc.)
- [ ] A converter for both type of journals

