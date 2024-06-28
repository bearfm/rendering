{
  description = "idk";

  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
	      buildInputs = with pkgs; [
	          vulkan-loader
            SDL2
            SDL2_image
            SDL2_mixer
            SDL2_net
            SDL2_ttf
            SDL2_gfx
	          pkg-config
            clang_18
            glslang
            vulkan-headers
            vulkan-loader
            vulkan-validation-layers
            glm
          ];

        pkgs = import nixpkgs { inherit system; };
      in {
        devShell = pkgs.mkShell {
          buildInputs = buildInputs;
          shellHook = ''
            export LD_LIBRARY_PATH=${pkgs.SDL2}/lib:${pkgs.vulkan-loader}/lib:$LD_LIBRARY_PATH
            export SDL2PATH=${pkgs.SDL2}/
	          export VKPATH=${pkgs.vulkan-loader}/lib
          '';
        };
      });
}
