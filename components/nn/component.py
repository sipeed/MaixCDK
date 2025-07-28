def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    return []



def add_requirements(platform : str, find_dirs : list):
    reqs = ["basic", "ini", "vision", "clipper2"]
    if platform == "maixcam":
        reqs.extend([
            "maixcam_lib",
            "alsa_lib"
        ])
    elif platform == "maixcam2":
        reqs.extend([
            "eigen",
            "librosa_simple",
            "OpenCC",
            "onnxruntime"
        ])
    elif platform == "linux":
        pass
    else:
        raise Exception("nn component.py not add this platform support yet")
    return reqs


