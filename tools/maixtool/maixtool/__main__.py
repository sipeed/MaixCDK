from .maixcdk import main as _maixcdk_main
from .maixtool import main as _maixtool_main

def main():
    _maixtool_main()

def maixcdk_main():
    _maixcdk_main()

if __name__ == '__main__':
    main()
