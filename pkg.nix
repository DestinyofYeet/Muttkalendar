{
  pkgs ? import <nixpkgs> {},
  ...
}:

pkgs.stdenv.mkDerivation {
  name = "Muttkalender";

  nativeBuildInputs = with pkgs; [
    automake
    autoconf
    autoreconfHook
  ];

  src = ./.;

  # configurePhase = ''
    # echo "Current dir is $(pwd)"
    # ${pkgs.autoconf}/bin/autoreconf -i  -f 
    # mkdir build && cd build
    # ../configure
  # '';

  # buildPhase = ''
    # make
  # '';
}
