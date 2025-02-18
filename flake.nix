{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }: (flake-utils.lib.eachDefaultSystem (system: let
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells = with pkgs; {
      default = mkShell {
        nativeBuildInputs = [
          pkg-config
          gnumake
        ];

        packages = [
          clang-tools
          llvm_17
          lldb_17
          bear
        ];
      };
    };
  }));
}
