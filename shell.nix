{ pkgs ? import <nixpkgs> {}}:
let
llvmpkgs = pkgs.llvmPackages_23;
in
pkgs.mkShell.override{
	stdenv = llvmpkgs.stdenv;
} {
	nativeBuildInputs = with pkgs.buildPackages; [
		pkg-config
		llvmpkgs.clang-tools
		gnumake
		bear
		ninja
		meson
	];
	buildInputs = with pkgs; [];
}
