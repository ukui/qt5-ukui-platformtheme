# qt5-ukui
The UKUI platform theme for qt5 QPA.

## Description
In the early development of the UKUI 3.0, we used qt5-gtk2-platformtheme to ensure the unity of the UKUI desktop style. However, it has many limitations for our new desgin.

This project is intend to provide a common resolution of desktop application development with qt5 in UKUI3.0. We hope provide a platform theme to unify and beautify all qt applications according to the design of UKUI3.0, not only our own applications. We are also committed to building our applications that can adapt to different styles. This project is first step to archive those objectives.

qt5-ukui's route brings us closer to the upstream community. It is not a division, but a desire for fusion and individuality in a compatible way.

## Build and Test
### Build Depends
- pkg-config
- qt5-default
- libkf5windowsystem-dev
- libgsettings-qt-dev
- libglib2.0-dev

### Test
To test the project, you should first install it into system and make sure that the current qpa platform is ukui.
You can export the QT_QPA_PLATFORMTHEME in terminal.

> export QT_QPA_PLATFORMTHEME=ukui

One more important job,

> sudo glib-compile-schemas /usr/share/glib-2.0/schemas

That will let the gsettings used by qt5-ukui worked.

Then you can run the test in project, or run any qt5 program for testing with ukui platformtheme.

### ToDoList
- menu blur
- custom palette
- style switch/management
- Change style's details through configuration file