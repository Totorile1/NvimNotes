{ pkgs ? import <nixpkgs> {} }:

pkgs.stdenv.mkDerivation {
  pname = "notewrapper";
  version = "1.0";

  src = ./.;

  nativeBuildInputs = [
    pkgs.pkg-config
  ];

  buildInputs = [
    pkgs.ncurses
    pkgs.cjson
    pkgs.fzf
    pkgs.gnused
    pkgs.ripgrep
  ];

  buildPhase = ''
    make
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp notewrapper $out/bin/
  '';

  meta = with pkgs.lib; {
    description = " Journaling and notetaking TUI wrapper on top of neovim, vim and nano.";
    license = licenses.gpl3Only;
    platforms = platforms.linux;
  };
}
