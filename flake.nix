{
  description = "Todo list webserver in C";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            cjson
            clang
            gnumake
            curl
            httpie
            zsh
            valgrind
          ];
          shellHook = ''
            exec zsh
            exit
          '';
        };
      });

}
