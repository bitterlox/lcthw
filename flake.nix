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
        let
          valgrind = if system == "x86_64-linux" then [ pkgs.valgrind ] else [ ];
        in
        {
          devShells.default = pkgs.mkShell {
            packages = [
              pkgs.opencode
              pkgs.man-db
              pkgs.gcc
              pkgs.gcc.man
              pkgs.gnumake
              pkgs.gnumake.man
              pkgs.lldb
              pkgs.gdb
            ]
            ++ valgrind;
            shellHook = ''
              echo "welcome to a shell for C development"
            '';
          };
        };
    };
}
