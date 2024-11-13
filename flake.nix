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

      # packages = [ (pkgs.callPackage ./pkg.nix {}) ];
      
      buildInputs = with pkgs; [
        autoconf
        automake
        gcc
        gccgo
        gnumake
      ];

      hardeningDisable = [ "fortify" ];

      shellHook = ''
        ${pkgs.nushell}/bin/nu
        exit
      '';
    };


    packages.x86_64-linux.default = pkgs.callPackage ./pkg.nix {};
  };
}
