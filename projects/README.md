# MaixCam & MaixCam2 Auto Build & Pack Script
A bash script to **automatically compile MaixCDK projects for MaixCam and MaixCam2 platforms**, package the compiled binaries into a portable ZIP file, and generate a self-executing `main.py` for one-click deployment on both devices.

## Features
- **Two Compilation Modes**: Batch compile all valid projects in a directory, or compile a single project independently.
- **Platform Selection**: Choose to build for MaixCam only, MaixCam2 only, or both platforms (default).
- **Project Exclusion**: Configure an exclusion list to skip specific projects during batch compilation.
- **Cross-Platform Support**: Compiles binaries for both MaixCam and MaixCam2 in one run.
- **Smart Packaging**: 
  - Separates platform-specific binaries into dedicated folders (`maixcam/`, `maixcam2/`).
  - Copies shared resources (assets, app.yaml, README) to the root of the package.
  - Generates an auto-executing `main.py` that **auto-detects the device model** (when building both) and runs the corresponding binary.
- **Auto-Clean**: Clears compilation caches after each build to avoid version conflicts.
- **Error Handling**: Stops execution on critical errors (batch mode skips failed projects and continues).
- **Colorful Logs**: Clear visual feedback with colored info/warn/error messages for easy debugging.
- **Timestamped ZIP**: Generates uniquely named ZIP packages with timestamps to avoid overwriting.

## Prerequisites
1. **MaixCDK Installed**: The script relies on the `maixcdk` command-line tool for compilation and cleaning.  
   Install MaixCDK following the official guide: [MaixCDK Documentation](https://wiki.sipeed.com/maixcdk)
2. **Bash Environment**: Works on Linux/macOS (native bash) or Windows (WSL2, Git Bash, or MSYS2).
3. **Basic Dependencies**: Ensure `zip`, `find`, `sed` are installed (pre-installed on most Linux/macOS systems).
4. **Valid MaixCDK Projects**: Projects must contain either `app.yaml` (MaixCDK app) or `CMakeLists.txt` (CMake-based project) to be recognized.

## Quick Start
### 1. Get the Script
Save the script as `build_and_pack.sh` and grant execution permission:
```bash
chmod +x build_and_pack.sh
```

### 2. Configure Exclusion List (Optional)
Edit the script to exclude specific projects from batch compilation:
```bash
# Open the script and modify line 14-15
EXCLUDE_PROJECTS=("project1" "project2" "project3")
```
Leave the array empty `EXCLUDE_PROJECTS=()` to compile all projects.

### 3. Show Help Information
```bash
./build_and_pack.sh -h
# or
./build_and_pack.sh --help
```

## Usage
### Mode 1: Batch Compile (Default)
Compile **all valid MaixCDK projects** in a target directory (skips non-project folders and a `build/` subdirectory if present).  
The compiled ZIP packages are output to a `build/` folder in the target directory.

#### Syntax
```bash
./build_and_pack.sh [PROJECTS_DIRECTORY] [--platform maixcam|maixcam2|both]
```
- `[PROJECTS_DIRECTORY]`: Optional, path to the folder containing all MaixCDK projects (defaults to the **current working directory** if not specified).
- `--platform` or `-p`: Optional, specify target platform(s) to build (defaults to `both`).

#### Examples
```bash
# Batch compile all projects in the current directory (both platforms)
./build_and_pack.sh

# Batch compile only for MaixCam
./build_and_pack.sh --platform maixcam

# Batch compile only for MaixCam2
./build_and_pack.sh --platform maixcam2

# Batch compile all projects in a specified directory for both platforms
./build_and_pack.sh /root/MaixCDK/projects --platform both
```

### Mode 2: Single Project Compile
Compile a **single MaixCDK project** (outputs the ZIP package directly to the project directory by default).

#### Syntax
```bash
./build_and_pack.sh --single [PROJECT_PATH] [--platform maixcam|maixcam2|both]
# or short version
./build_and_pack.sh -s [PROJECT_PATH] [-p maixcam|maixcam2|both]
```
- `[PROJECT_PATH]`: Optional, path to the single MaixCDK project (defaults to the **current working directory** if not specified).
- `--platform` or `-p`: Optional, specify target platform(s) to build (defaults to `both`).

#### Examples
```bash
# Compile the project in the current directory (both platforms)
./build_and_pack.sh --single

# Compile only for MaixCam
./build_and_pack.sh --single --platform maixcam

# Compile a specified single project only for MaixCam2
./build_and_pack.sh --single /root/MaixCDK/projects/app_camera --platform maixcam2
```

## Output File Structure
### Compiled ZIP Package Content
The generated ZIP file (e.g., `app_camera_release_20260129_153000.zip`) structure depends on the platform selection:

**When building both platforms (`--platform both` or default):**
```
app_camera_release_20260129_153000/
├── main.py          # Auto-execution script (auto-detects device & runs binary)
├── app.yaml         # Project configuration (shared)
├── assets/          # Resource files (images/fonts, shared)
├── README.md        # Project documentation (shared)
├── README_EN.md     # English documentation (shared, if exists)
├── maixcam/         # MaixCam-specific files
│   ├── [binary]     # Compiled binary for MaixCam
│   └── dl_lib/      # Deep learning libraries (if exists)
└── maixcam2/        # MaixCam2-specific files
    ├── [binary]     # Compiled binary for MaixCam2
    └── dl_lib/      # Deep learning libraries (if exists)
```

**When building single platform (`--platform maixcam` or `--platform maixcam2`):**
```
app_camera_release_20260129_153000/
├── main.py          # Direct execution script (runs platform-specific binary)
├── app.yaml         # Project configuration
├── assets/          # Resource files (images/fonts)
├── README.md        # Project documentation
├── README_EN.md     # English documentation (if exists)
└── maixcam/         # Only the selected platform folder
    ├── [binary]     # Compiled binary
    └── dl_lib/      # Deep learning libraries (if exists)
```

### Key Output Files
- **ZIP Package**: Named as `<PROJECT_NAME>_release_<TIMESTAMP>.zip` (timestamp format: `YYYYMMDD_HHMMSS`).
  - Batch mode: Stored in `[PROJECTS_DIRECTORY]/build/`.
  - Single mode: Stored in the project root directory by default.
- **`main.py`**: The core execution script—**no manual modification needed** (auto-replaces the binary name during compilation).

## How to Deploy on MaixCam/MaixCam2
1. Copy the generated ZIP file to the root directory of the MaixCam/MaixCam2 device (via SSH/SCP or SD card).
2. Unzip the package on the device:
   ```bash
   unzip <PROJECT_NAME>_release_<TIMESTAMP>.zip -d <PROJECT_FOLDER>
   ```
3. Enter the project folder and run the script:
   ```bash
   cd <PROJECT_FOLDER>
   python main.py
   ```
   The script will **automatically detect the device model** (MaixCam/MaixCam2), grant execute permission to the binary, and run it.

## Script Workflow
### For Single Project Compilation
1. Clean up previous build outputs (if any).
2. Based on `--platform` parameter:
   - `both` (default): Compile for **MaixCam** first, then **MaixCam2**
   - `maixcam`: Compile only for **MaixCam**
   - `maixcam2`: Compile only for **MaixCam2**
3. Copy shared resources (assets, app.yaml, README) to the output directory.
4. Generate platform-appropriate `main.py`:
   - For `both`: Auto-detects device and runs corresponding binary
   - For single platform: Directly runs the platform-specific binary
5. Package all files into a timestamped ZIP and clean up temporary build files.

### For Batch Compilation
1. Create a `build/` directory for output ZIP packages.
2. Traverse all subdirectories in the target folder, skip non-project folders, the `build/` directory, and projects in the exclusion list.
3. Compile each valid project with the **single project workflow** using the specified platform, outputting ZIPs to the `build/` directory.
4. Skip failed projects and continue compiling the rest.
5. Print a summary (total/succeeded/failed projects) and list failed projects (if any).
