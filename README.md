# CameraCinematic
[![CodeFactor](https://www.codefactor.io/repository/github/intemporel/cameracinematic/badge/main)](https://www.codefactor.io/repository/github/intemporel/cameracinematic/overview/main)

## Introduction
 This tool allow you to create / load / edit models used for create a cinematic in game for World of Warcraft 3.3.5 version.
 
## Warning
Be prudent with `CinematicCamera.dbc`, the tool always work with `OriginFacing = 0`, you can load model with `OriginFacing != 0`, but can't edit it for now. Otherwise the tool will corrupt the model.  
You can't start a Cinematic inside a transport, like boat for exemple, because your client take transport's map as the map origin used by model, and it will corrupt all the Position/Target path used by camera.  
When you push `Generate`, it will overwritte the current file.

## Preview
[![CameraCinematic - Editor](https://i.imgur.com/EAJ4Hes.png)](https://youtu.be/9iAwpgmzEXQ)

## Thanks to
 - Schlumpf, my work is based on his script, which you can find here. [CameraGenerator](https://github.com/stoneharry/Misc-WoW-Stuff/blob/master/Camera%20Generator/camera.cpp)
