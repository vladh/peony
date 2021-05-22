@echo off

echo ################################################################################
echo ### Building
echo ################################################################################

if not defined DevEnvDir (
  call vcvarsall x64
)

msbuild bin/peony.sln -m -p:Configuration=Debug
