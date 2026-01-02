## Showcase
<table align="center">
  <!-- Row 1: Ray traced paths -->
  <tr>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/6b118673-ab03-40f8-af3a-16da814107e2"
        width="400"
      />
    </td>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/02699560-f05d-40c5-9122-7ed8a493de77"
        width="400"
      />
    </td>
  </tr>
  <tr>
    <td align="center" colspan="2">
      <b>Ray Traced Paths Visualized</b>
    </td>
  </tr>

  <!-- Spacer row -->
  <tr><td colspan="2"><br /></td></tr>

  <!-- Row 2: Path mutations -->
  <tr>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/63e08d4a-5cfc-427e-ae8f-6a8d0ca32b0f"
        width="400"
      />
    </td>
    <td align="center">
      <img
        src="https://github.com/user-attachments/assets/c47df25a-6208-4ef2-8263-7b8439b93490"
        width="400"
      />
    </td>
  </tr>
  <tr>
    <td align="center" colspan="2">
      <b>Path Mutations Visualized (Original Path in Green, Mutation in Red)</b>
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
      <sub><b>No Mutations, Just Resampling</b></sub>
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
```

### 2. Set the vcpkg root path
Set the **VCPKG_ROOT** environment variable to the directory where vcpkg was cloned.
```bash
set VCPKG_ROOT=C:\path\to\vcpkg   # Windows
export VCPKG_ROOT=/path/to/vcpkg  # Linux
```

### 3. Configure the project
From the project root (the directory containing CMakeLists.txt and vcpkg.json):
```bash
cmake -S . -B build ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake  # Windows
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake   # Linux
```

### 4. Build
```bash
cmake --build build --config Release
```

### 5. Run
```bash
build\Release\renderer.exe
```

### 6. Change obj file paths
You may additionally want to adjust the paths in main.cpp to reflect the location of your desired boject files. The showcase scene was sourced from: https://github.com/vaffeine/vulkano-raytracing/blob/master/assets/cornell-box.obj.

