## Showcase
<img width="1707" height="1215" alt="image" src="https://github.com/user-attachments/assets/02428287-054d-4be4-b61b-3c81d1ce5470" />
<img width="1641" height="1228" alt="image" src="https://github.com/user-attachments/assets/a14f38ab-8e34-4f3b-a3ea-b146cf7a0bf8" />
<img width="2197" height="1622" alt="image" src="https://github.com/user-attachments/assets/bd62e0ac-2aa4-477a-bd1b-7fe676439155" />
<img width="2141" height="1520" alt="image" src="https://github.com/user-attachments/assets/e51dbd67-ca33-4d07-b5f7-14740022389f" />


<table align="center">
  <tr>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/02428287-054d-4be4-b61b-3c81d1ce5470"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Individual Ray Traces</b>
    </td>

    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/a14f38ab-8e34-4f3b-a3ea-b146cf7a0bf8"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Individual Ray Traces Highlighted</b><br />
    </td>
  </tr>
</table>

<br />

<table align="center">
  <tr>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/bd62e0ac-2aa4-477a-bd1b-7fe676439155"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Path Mutation Visualization</b>
    </td>

    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/e51dbd67-ca33-4d07-b5f7-14740022389f"
        width="400"
        height="400"
        style="object-fit: cover;"
      />
      <br />
      <b>Path Mutation Visualization(Original Path in Green, Mutation in Red)</b><br />
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

