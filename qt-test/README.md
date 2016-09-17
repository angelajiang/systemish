# Setting up Qt on Ubuntu

 * Install Qt into `/opt/Qt5/`
 * Install OpenGL
```
	sudo apt-get install libgl1-mesa-dev
```
 * Edit `/usr/lib/x86_64-linux-gnu/qt-default/qtchooser/default.conf` to:
```
	/opt/Qt5/5.7/gcc_64/bin
	/opt/Qt5/5.7/gcc_64/lib
```
 * Export an environment variable for Qt platform plugins:
```
	export QT_QPA_PLATFORM_PLUGIN_PATH=/opt/Qt5/5.7/gcc_64/plugins/platforms
```

## Compiling a project
 * Generate the .pro file
```
	qmake -project
```
 * Modify the .pro file so that the generated Makefile includes required headers
```
	QT += gui widgets
```
 * Generate the makefile
```
	qmake
```
