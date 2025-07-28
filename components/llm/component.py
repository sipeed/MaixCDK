

def add_requirements(platform : str, find_dirs : list):
    if platform == "maixcam":
        return [
            "nn",
        ]
    elif platform == "maixcam2":
        return [
            "nn",
            "json"
        ]
    elif platform == "linux":
        return []
    else:
        raise Exception("llm component.py not add this platform support yet")

