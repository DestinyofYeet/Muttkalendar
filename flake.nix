{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }@inputs : 
  let 
    pkgs = import nixpkgs { system = "x86_64-linux"; };
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        autoconf
        automake
        gcc
        gccgo
        gnumake
      ];
    };


    packages.x86_64-linux.default = pkgs.callPackage ./pkg.nix {};
  };
}
