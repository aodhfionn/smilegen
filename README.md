# smilegen

Generates unique smiley faces based off of a given seed  
Each seed generates a new unique image, so if the same seed is inputted the same image will be outputted

Seeds 0 to 9,223,372,036,854,775,807 are all valid

Could likely be used to generate unique profile pictures for new users on a site or something

## Dependencies

- libpng
- that's it

## Installation

1. `git clone https://github.com/abdowns/smilegen.git`
1. `cd smilegen`
1. `make`

## Usage

`make` builds it  
`make clean` clears all binaries and generated images


Once built, use `./smilegen [seed (should be a number)]` to generate an image


You can configure the generation by editing the config.h file (instructions inside)  
Images are outputted in the "output" directory by default, but this can be changed in the config
