# raudio2

**raudio2 is a simple and easy-to-use audio library based on raudio + miniaudio.**  

`raudio2` is intended for audio devices management and audio playing,
it also supports multiple audio file-formats loading.  

`raudio2` API tries to be very simple and intuitive and it represents a thin layer
over the powerful [miniaudio library](https://github.com/dr-soft/miniaudio),
including support for multiple audio formats: FLAC, MOD, MP3, OGG, OPUS, VGM, WAV, XM.  

`raudio2` improves `raudio` by adding plugin support and rewriting all audio formats
as input plugins, similar to Winamp input plugins.

## features

 - Simplifies `miniaudio` usage exposing only basic functionality
 - Audio formats supported: `.flac`, `.mod`, `.mp3`, `.ogg`, `.opus`, `.vgm`, `.wav`, `.xm`
 - Select desired input formats at compilation time
 - Plugin architecture
 - Load and play audio, static or streamed modes
 - Support for pluggable audio effects with callbacks

## usage

For usage example, it's recommended to check the provided [`examples`](examples).
They also contain detailed documentation at the start of the examples.

## documentation

[documentation](docs/readme.md)

## license

`raudio2` is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
BSD-like license that allows static linking with closed source software.
Check [LICENSE](LICENSE) for further details.
