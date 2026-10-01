Remove-Item -Path "../bin/resources" -Recurse -Force -ErrorAction SilentlyContinue

Copy-Item -Path "..\resources" -Destination ../bin/ -Recurse -Force

cd ../bin/

.\Forradia.exe

cd ../scripts/