@echo off

shaderc.exe -i C:\Users\adhtr\src\imgui_bgfx\submodules\bgfx.cmake\bgfx\src\ -f vert.sc --type v -o vert.h --bin2c vertSh -p s_5_0 --varyingdef varying.def.sc --verbose --platform windows
shaderc.exe -i C:\Users\adhtr\src\imgui_bgfx\submodules\bgfx.cmake\bgfx\src\ -f frag.sc --type f -o frag.h --bin2c fragSh -p s_5_0 --varyingdef varying.def.sc --verbose --platform windows