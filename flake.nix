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
        name = "todolist-web-c";
        image = {
          inherit name;
          registry = "ghcr.io";
          owner = "kmjayadeep";
        };
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

        packages = rec {
          default = webserver;

          webserver = pkgs.stdenv.mkDerivation {
            pname = name;
            version = "1.0";

            src = ./.;
            nativeBuildInputs = with pkgs; [
              clang
              cjson
            ];

            buildPhase = ''
              make
            '';

            installPhase = ''
              mkdir -p $out/bin
              cp webserver $out/bin/
              chmod +x $out/bin/webserver
              cp templates -r $out/
            '';
          };

          docker = pkgs.dockerTools.buildLayeredImage {
            name = "${image.registry}/${image.owner}/${image.name}";
            tag  = "latest";

            config = {
              Cmd = [ "${webserver}/bin/webserver" ];
              WorkingDir = "${webserver}";
              ExposedPorts."10000/tcp" = { };
            };
          };

        };

      });

}
