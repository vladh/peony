@echo off

echo ################################################################################
echo ### Building
echo ################################################################################

if not defined DevEnvDir (
  call vcvarsall x64
)

msbuild bin/peony.sln -m:16 -p:Configuration=Debug
