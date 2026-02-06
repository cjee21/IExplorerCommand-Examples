# IExplorerCommand-Examples

Examples of implementing IExplorerCommand Windows File Explorer context menu shell extension

- IExplorerCommand_CppWinRT
  - IExplorerCommand implemented using [C++/WinRT](https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/)

- IExplorerCommand_WRL
  - IExplorerCommand implemented using [Windows Runtime C++ Template Library (WRL)](https://learn.microsoft.com/en-us/cpp/cppcx/wrl/windows-runtime-cpp-template-library-wrl?view=msvc-170)

A demo application to demonstrate the use of the C++/WinRT shell extension can be found in the [demo](Demo) directory.

## GitHub Actions status

[![Build](https://github.com/cjee21/IExplorerCommand-Examples/actions/workflows/build.yml/badge.svg)](https://github.com/cjee21/IExplorerCommand-Examples/actions/workflows/build.yml) [![Cppcheck](https://github.com/cjee21/IExplorerCommand-Examples/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/cjee21/IExplorerCommand-Examples/actions/workflows/cppcheck.yml) [![Build and release demo](https://github.com/cjee21/IExplorerCommand-Examples/actions/workflows/release.yml/badge.svg)](https://github.com/cjee21/IExplorerCommand-Examples/actions/workflows/release.yml)

## Other examples

- [MediaInfo](https://github.com/MediaArea/MediaInfo/blob/master/Source/WindowsShellExtension/dllmain.cpp) (C++/WinRT)
- [NanaZip](https://github.com/M2Team/NanaZip/blob/main/NanaZip.UI.Modern/NanaZip.ShellExtension.cpp) (C++/WinRT)
- [VSCode](https://github.com/microsoft/vscode-explorer-command/blob/main/src/explorer_command.cc) (WRL)

## IExplorerCommand documentation

- <https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-iexplorercommand>
