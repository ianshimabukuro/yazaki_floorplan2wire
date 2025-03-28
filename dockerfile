# escape=`

# Stage 1: Build C++ EWD Library with MSVC & CMake
FROM mcr.microsoft.com/windows/servercore:ltsc2019 AS builder

# Set CMD as the default shell (batch processing recommended for VS installation)
SHELL ["cmd", "/S", "/C"]

# Set working directory
WORKDIR C:\BuildTools

# Download Visual Studio Build Tools installer (official Microsoft link)
# Download Visual Studio Build Tools installer (official Microsoft link)
ADD https://aka.ms/vs/16/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe

# Install Visual Studio Build Tools with only the required components
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --installPath "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
    || IF "%ERRORLEVEL%"=="3010" EXIT 0


# Cleanup after installation
RUN del /q vs_buildtools.exe

# Install CMake 3.31.2 manually
RUN curl -SL -o cmake.msi https://github.com/Kitware/CMake/releases/download/v3.31.2/cmake-3.31.2-windows-x86_64.msi `
    && msiexec /i cmake.msi /quiet /norestart /log cmake_install.log `
    && del cmake.msi
# Add CMake to the system PATH
RUN setx PATH "%PATH%;C:\Program Files\CMake\bin"
# Add the MSVC compiler to PATH
RUN setx PATH "%PATH%;C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64"

# Add VS 2022 compiler (cl.exe) to PATH
#RUN dir "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64"
# Install Python 3.10 manually inside the builder stage
RUN curl -SL -o python-installer.exe https://www.python.org/ftp/python/3.13.1/python-3.13.1-amd64.exe `
    && start /wait python-installer.exe /quiet InstallAllUsers=1 PrependPath=1 `
    && del python-installer.exe

   
# Ensure Python is added to PATH
RUN setx PATH "%PATH%;C:\Program Files\Python310\"

# Install Git (needed for certain Python packages)
RUN curl -SL -o git-installer.exe https://github.com/git-for-windows/git/releases/download/v2.35.3.windows.1/Git-2.35.3-64-bit.exe `
    && start /wait git-installer.exe /VERYSILENT `
    && del git-installer.exe

# Ensure Git is added to PATH
#RUN REG ADD "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /V PATH /T REG_EXPAND_SZ /F /D "%PATH%;C:\Program Files\Git\bin"


# Set the working directory for building the C++ project
WORKDIR C:\app

COPY src/ C:/app/src/
COPY data/ C:/app/data/
COPY CMakeLists.txt C:/app/
COPY dependency/ C:/app/dependency/
COPY cmake/ C:/app/cmake/


# Create build directory
RUN mkdir build

# Run CMake to configure and build
RUN cd build && cmake .. -T host=x64 -A x64 && cmake --build . --config Release
# Enable Windows Long Path Support inside the container
RUN reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem /v LongPathsEnabled /t REG_DWORD /d 1 /f

# Copy Python scripts & dependencies
COPY python/ python/s

COPY requirements.txt .
RUN pip install --no-cache-dir --only-binary=:all: numpy
RUN pip install --no-cache-dir --no-deps casadi==3.6.7 contourpy==1.3.1 ezdxf==1.3.5
RUN pip install --no-cache-dir --no-deps matplotlib==3.10.0 nlopt==2.9.0 nptyping==2.5.0
# Install all dependencies from requirements.txt (including CadQuery)
RUN pip install --no-cache-dir -r requirements.txt

# Set entry point to Python scripts (default: realworld.py)
CMD ["cmd"]
