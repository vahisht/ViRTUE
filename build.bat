@echo off
setlocal ENABLEDELAYEDEXPANSION

call "vcvarsall.bat" amd64

cl src\main.cpp /DNDEBUG /O2 /openmp /EHsc /nologo /I"include" /I"virtue" /Fe"bin\virtue.exe" /link /LIBPATH:"lib" SDL2.lib glew32.lib || goto error
::cl src\ebsynth.cpp src\ebsynth_cpu.cpp src\ebsynth_nocuda.cpp /DNDEBUG /O2 /openmp /EHsc /nologo /I"include" /Fe"bin\ebsynth.exe" || goto error
del main.obj 2> NUL
goto :EOF

:error
echo FAILED
@%COMSPEC% /C exit 1 >nul
