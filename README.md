# AlloUnity

Developed by [Tibor Goldschwendt, Four Eyes Lab, UCSB](https://ilab.cs.ucsb.edu/index.php/people/6-visitors/86-tibor-goldschwendt-).

AlloUnity will enable the full-surround display of Unity3D-rendered scenes in the AlloSphere. Therefore Unity3D renders the scene to a cubemap. The cubemap is then extracted from Unity3D, H.264 encoded and sent over RTSP/RTP. In the AlloSphere the stream is decoded, drawn as the background of a omni-stereo rendering and displayed.

Work in progress.

## Modules

AlloUnity currently consists of these five modules:

1. **CubemapRenderingPlugin**: C# script for rendering a cubemap in Unity3D.
2. **CubemapExtractionPlugin**: Extracts the cubemap from Unity3D and copies the texture data from the GPU to the CPU.
3. **AlloServer**: Encodes the cubemap as six streams and provides the stream via RTSP/RTP.
4. **AlloPlayer**: Receives the cubemap in the AlloSphere, decodes it and displays it.
5. **AlloShared**: Resources shared between the modules explained above.

## Building

Install dependencies:

- [boost](http://www.boost.org/)
- [FFmpeg](https://www.ffmpeg.org/)
- [x264](http://www.videolan.org/developers/x264.html)
- [live555](http://www.live555.com/liveMedia/)
- [AlloSystem](https://github.com/AlloSphere-Research-Group/AlloSystem)
- [Unity3D 5](https://unity3d.com/)
- [CMake](http://www.cmake.org/)

Then build **Unit3D scene** and **source**.

### Unity3D Scene

1. Create or open a Unity3D scene (placed in `<UnityProjectFolder>`)
2. Import asset `CubemapRenderingPlugin/RenderCubemap.cs`
3. Add `RenderCubemap.cs` to the camera that should be the viewpoint of the rendered cubemap.
4. Build scene for your operating system (named `UnityProject`)

### Source

Build source with CMake and your favourite build environment, e.g.:

```bash
cmake -G "Unix Makefiles" -DUNITY_PROJECT_PLUGIN_DIR:PATH=<UnityProjectFolder>/Assets/Plugin
make
```
On success, this places

- *CubemapExtractionPlugin* in `<UnityProjectFolder>/Assets/Plugins/`
- *AlloServer* and *AlloPlayer* in `Bin/` 
- *AlloShared* in `Lib/`

Compilation tested with Visual Studio 2013 Ultimate on Windows, Xcode 6 on OS X and make on Ubuntu.

## Launching

1. Start `<UnityProject>` on rendering machine
2. Start *AlloServer* on rendering machine
3. Start *AlloPlayer* on display machine

Order matters!

## License

BSDv3 License.