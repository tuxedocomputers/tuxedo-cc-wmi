# TUXEDO WMI module

## Project structure
```
tuxedo-wmi
|  README.md
|  Makefile             Build/packaging scripts
|  dkms.conf
|--src                  Module source
```

## Description
This module provides an interface to controlling various functionality (mainly connected to the EC) through WMI.

## Build commands
- Package source (requires dkms, debhelper)
  ```
  make package
  ```
- Clean packaging artifacts
  ```
  make clean-package
  ```
- Build locally for current kernel \
  _Note: Make sure to clean local build artifacts before packaging._
  ```
  make
  ```
- Clean local build
  ```
  make clean
  ```
