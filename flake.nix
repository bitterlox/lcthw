{
  description = "A flake for urbit development";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {

      systems = [
        "x86_64-linux"
        "aarch64-darwin"
      ];

      perSystem =
        {
          pkgs,
          lib,
          system,
          ...
        }:
        {
          devShells.default = pkgs.mkShell {
            packages = [
              pkgs.stdenv
              pkgs.lldb
            ];
            shellHook = ''
              echo "welcome to a shell for C development"
            '';
          };
        };
    };
}
