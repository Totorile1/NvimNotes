{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.vivify
    pkgs.cjson
    pkgs.pkg-config
  ];
}
