# SurgeXT for MetaModule

This repository is a fork of the SurgeXT for Rack repository, in order to build 
a MetaModule plugin that's compatible with the SurgeXT Rack modules.


## How to develop with these modules

Checkout sources:
```
git clone https://github.com/4ms/surge-rack
cd surge-rack
git submodule update --init --recursive
```

```
cd metamodule
cmake --fresh -B build -DMETAMODULE_SDK_DIR=path/to/metamodule-plugin-sdk
cmake --build build
```

This builds the SurgeXTRack.mmplugin file in `metamodule/metamodule-plugins/`

## License and Copyright

This software is licensed under the Gnu General Public License v3 or later.
The license is included in the file "LICENSE" in this repository.

It is a derived work of both Surge
(which is GPL3 or later) and VCVRack (which is GPL3 or later) and
their dependencies, and the MetaModule SDK (which is GPL3 or later).

It contains the quicksand font which is licensed under OpenFontLicense 1.1.

The SVGs used (in res/xt) are distributed under the Creative Commons CC-BY-NC-SA-4.0 and are
copyright Steve and Pierre. These SVGs are not used directly in the MetaModule
plugin but they were the sources for the .png files that the MetaModule uses.

Copyright to this software is held by the authors with authorship indicated by the github transaction log.
