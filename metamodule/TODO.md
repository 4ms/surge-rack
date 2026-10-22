TODO

Check our submodules: any overlap with upstream?
  - upstream uses c++20?
  - Look for anything that would be useful as an issue or PR


Mod Buttons:
  - could be AltParamAction? So just click, no mapping. Would need to redraw them (blue light when on), which AltParams don't support

Reverb: 
  - Room Size "slider" should be a multi-pos switch

LFO: 
  - can't change waveshape (clickable buttons at top of LCD)

Fix lots of console errors with paths:
 - can't we just add those files to the plugin?
stat() on file SurgeXTUserDefaults.xml on vol 2 failed
stat(SurgeXTUserDefaults.xml) returned -1
stat() on file configuration.xml on vol 2 failed
stat(configuration.xml) returned -1
stat() on file SurgeMIDIDefaults.xml on vol 2 failed
stat(SurgeMIDIDefaults.xml) returned -1
stat() on file windows.wt on vol 2 failed
stat(windows.wt) returned -1
stat() on file FX Presets on vol 2 failed
stat(FX Presets) returned -1
stat() on file fx_presets on vol 2 failed
stat(fx_presets) returned -1


Fix compile warnings
