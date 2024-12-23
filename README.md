# EWD

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
First, clone this repo:
```
git clone https://github.com/ianshimabukuro/yazaki_floorplan2wire
```
Then setup a build folder inside the local folder and call CMake as follows:
```
cd yazaki_floorplan2wire
mkdir build
cd build
cmake .. -T host=x64 -A x64
```
Then compile the code using 
```
cmake --build . --config Release
```
This would compile the EWD library and its python interface using SWIG.

Now we need to download the required packages.
For that we first create a virtual environment, activate it, then run pip to install the packages into the venv:
```
python -m venv venv
.\venv\Scripts\activate
pip install -r requirements.txt
```
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
