

def add_requirements(platform : str, find_dirs : list):
    if platform == "maixcam":
        return [
            "cvi_tpu",
            "media_server",
            "sophgo-middleware",
        ]
    elif platform == "maixcam2":
        return [
            "maixcam2_msp"
        ]
    else:
        raise Exception("maixcam_lib component.py not add this platform support yet")

