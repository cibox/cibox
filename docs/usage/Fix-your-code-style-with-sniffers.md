Fix your code style with sniffers
=====

To run auto fix of code style of your code with sniffers use `runcodestyleautofix.sh` command within `/var/www/docroot` directory.
If you do not want to update software every time and save your time, plese use `-n` argument. *Note*, the first run of the command should be without any arguments to download the dependencies.
Example:
```
./runcodestyleautofix.sh -n
```