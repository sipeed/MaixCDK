dl
====

Downloaded files should be put in pkgs directory.

Extracted files should be put in extracted directory.

so:
* If you are developing a package, and need data files, cause only code should be add to git repo, you should put the data files on internet, and download them is `dl/pkgs` directory, to use them, extract them to `dl/extracted` directory. You just need edit the `CMakeLists.txt` file, edit `ADD_FILE_DOWNLOADS` variable, and use the extracted files with `${DL_EXTRACTED_PATH}/your_path`.
> e.g. For python packages, will be downloaded to `dl/pkgs/python_srcs`, and extracted to `dl/extracted/python_srcs`, which download commands in CMakelists.txt.

* If you users to compile projects, you can download files manually to dl/pkgs directory first, when compile `pkgs_info.json` will be write to this directory, all files need to be downloaded will be listed in this file.

