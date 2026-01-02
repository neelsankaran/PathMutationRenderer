## Showcase

<table align="center">
  <tr>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/abf0fc27-36ea-4bca-b5b2-da00af051cff"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Individual Ray Traces</b>
    </td>

    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/4c080685-c172-49f2-af87-f6a793d2043d"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Path Mutation Visualization</b><br />
      <sub>Original path (green), mutation (red)</sub>
    </td>
  </tr>
</table>

<br />

<table align="center">
  <tr>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/f4170a84-9fd3-4fef-8683-0e6b7c378d76"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Individual Ray Traces</b>
    </td>

    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/c416e150-d078-48c4-866d-6f24bc313e5f"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Path Mutation Visualization</b><br />
      <sub>Original path (green), mutation (red)</sub>
    </td>
  </tr>
</table>

<br />

<table align="center">
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/ea8df63f-9e08-4533-bc12-353cbb347d49" width="240" />
      <br />
      <sub><b>Meshwalk mutation</b></sub>
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/28f647d1-b9d3-4fcd-90fe-f936dd1db5ba" width="240" />
      <br />
      <sub><b>Resampling mutation</b></sub>
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/d68f2817-9ef5-4ff9-80c2-792013398490" width="240" />
      <br />
      <sub><b>Project mutation</b></sub>
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/63fa2725-4447-4054-b06f-60c90ff1c1de" width="240" />
      <br />
      <sub><b>Retrace mutation</b></sub>
    </td>
  </tr>
</table>

## Run

This project uses **vcpkg** for dependency management.

### Prerequisites
- CMake ≥ 3.22  
- A C++23-compatible compiler
- vcpkg installed locally

### 1. Install vcpkg
```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat   # Windows
./bootstrap-vcpkg.sh  # Linux

### 2. Set the vcpkg root path
Set the **VCPKG_ROOT** environment variable to the directory where vcpkg was cloned.
set VCPKG_ROOT=C:\path\to\vcpkg   # Windows
export VCPKG_ROOT=/path/to/vcpkg  # Linux

### 3. Configure the project
From the project root (the directory containing CMakeLists.txt and vcpkg.json):
cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake  # Windows
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake   # Linux

### 4. Build
cmake --build build --config Release

### 5. Run
build\Release\renderer.exe

### 6. Change obj file paths
You may additionally want to adjust the paths in main.cpp to reflect the location of your desired boject files. The showcase scene was sourced from: https://github.com/vaffeine/vulkano-raytracing/blob/master/assets/cornell-box.obj.

