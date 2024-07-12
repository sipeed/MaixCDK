MaixCDK i18n demo
=====

## Usage

1. coding like [main.cpp](./main/src/main.cpp), use `tr` function to call the string you want to translate.
2. Execute `maixtool i18n -d . -r` in project dir to generate translation files, they will be in `locales` dir, all are `yaml` file, the locale name can be found in [here](https://www.science.co.il/language/Locale-codes.php) or [wikipedia](https://en.wikipedia.org/wiki/Language_localisation), all letters use lower case.
3. Translate files.
4. Compile and you will get binary files in dist dir, copy them to board and run.



