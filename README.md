#Yazaki project

## About EWD

EWD is a project that aims to provide a simple and easy-to-use tool for electrical wiring design.

## Requirements

* CMake 3.14.
* Modern C++ compiler like Visual Studio 2019.
* Python 3.7 or higher version.
* (optional) Gurobipy
* (optional) CPLEX

## Compilation

EWD uses CMake as build system, and requires at least version 3.14. 

1. Clone this Repo on Github Desktop and Open in VSCode

2. Build the file with CMake :
Go inside the project folder and create a build directory if not inside already
```
cd yazaki_floorplan2wire
mkdir build
cd build

Then build and compile on VS Code terminal:
```
cmake .. -T host=x64 -A x64
cmake --build . --config Release
```
This would compile the EWD library and its python interface using SWIG.

3. Virtual Environment and Python Packages

Create a virtual environment by selecting doing control+shift+p and creating a python environment, then selecting venv and then select the requirements.txt.
Now we need to download the required packages.

If doing it on the CLI, do this:
```
python -m venv venv
.\venv\Scripts\activate
pip install -r requirements.txt
```

4. Running the main function on Python:
Now go to the 'python' folder at the root of the project.
```
cd ../python
```
The python interface is at hand. 
Implement EWD on a real-world instance using the command
```
python realworld.py
```
Implement EWD on a synthetic instance using the command
```
python synthetic.py
```


For installing CadQuery, it is recommended to simply pip install the cadquery github. it will come with most of the packages.
Also it is necessary to pip install ocp, but you can just search ocp on pypi and the most recent one should be the one you should use.