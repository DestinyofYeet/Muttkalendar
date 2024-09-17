{
  pkgs ? import <nixpkgs> {},
  ...
}:

pkgs.stdenv.mkDerivation {
  name = "Muttkalender";

  nativeBuildInputs = with pkgs; [
    autoreconfHook
  ];

  src = ./.;
}
