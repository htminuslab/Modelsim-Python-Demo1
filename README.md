# Modelsim-Python-Demo1

Demo showing how to add a Python Constraint Solver to Modelsim
 
 
 <img src="modeltech.png" alt="Old Modeltech logo"/>  <img src="python.png" alt="Python logo"/> 
 
This repository contains a simple FLI **foreign subprogram** example which calls a constraint solver written in Python. The demo shows how easy it is to load a Python interpreter and to run it *perceptionally* concurrent with VHDL (or Verilog via a thin VHDL-Verilog wrapper). The Python generator construct is used to feed a VHDL testbench a new solution every time it asks for one. 

To simplify running the demo a pre-build dll (fli_python.dll) is supplied, however, from a security point of view it is highly recommended to build the dll from source.

Note that this is just an example on how to add Python to Modelsim, there are better ways to add a constraint solver to Modelsim (UVVM, OS-VVM), Questa Prime has a constraint solver build into the product.

## FLI
The **Foreign Language Interface** (FLI) is a VHDL C/C++ API used on Siemens' Modelsim and Questa simulator products. The FLI allows a C/C++ program to simulate together with VHDL (or Verilog via a wrapper), enabled a C/C++ program to traverse the hierarchy of a mixed VHDL/Verilog design, get information about and set values of VHDL/Verilog objects, get information about and control a simulation, add commands etc. The FLI is the oldest and most mature C/C++ API on Modelsim/Questa and was added during the early development years of Modelsim.
 
The FLI supports 2 modes of operations, the simpler **foreign subprogram** in which a VHDL procedure is implemented in C/C++ and a **foreign architecture** which has a VHDL architecture implemented in C/C++. A demo of the latter can be found [here](https://github.com/htminuslab/Modelsim-Unicorn). A second Python demo which calls numpy and matplotlib from Modelsim can be found on this [github page](https://github.com/htminuslab/Modelsim-Python-Demo2).

 
## Requirements for this demo
1) 64bits Modelsim DE (or Questa), Modelsim PE users need to install 64bits Modelsim DE (works on a PE license!)
2) Visual Studio 2019 (free community edition)
3) Python 3.8
4) Python [Constraint solver library](https://github.com/python-constraint)

## Checks before building/running the demo
1) Modelsim installation root directory is set via the **MTI_HOME** environmental variable. This is required because the build batch file uses %MTI_HOME%/include and %MTI_HOME%/win64pe.

```
E:\Modelsim-Python-Demo1\sim>echo %MTI_HOME%
D:\Products\modelsim_de_2022
```

2) Open the run.bat file and correct the path to the Python include and lib directory.

```
cl -c /Zi -I%MTI_HOME%\include -I**C:\utils\Python38\include** src\fli_python.c
link -dll /EXPORT:call_python_function /EXPORT:call_python_module /EXPORT:call_python_cleanup fli_python.obj %MTI_HOME%\win64pe\mtipli.lib **C:\utils\Python38\libs\python38.lib** /out:fli_python.dll
```
  
3) Open the src/fli_python.c file and correct the path to the demo directory in line 63

```C
	PyRun_SimpleString("sys.path.append(r'E:\\Modelsim-Python-Demo1')"); // CHANGE ME!!
```	

4) Install the constraint solver library via pip

```Python
pip install python-constraint
```

5) The 64bits Visual Studio compilers are added to the search PATH.

```
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

E:\Modelsim-Python-Demo1>cl /?
Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30137 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.
```
 
## How does the demo work?

The design contains 2 foreign procedure calls, **call_python_module()** and **call_python_function()**. 

```VHDL
	procedure call_python_module(module_name:in string);
	attribute FOREIGN of call_python_module : procedure is "call_python_module ./fli_python.dll";	
		
	procedure call_python_function(function_name:in string; vhdl_array:OUT intarray);
	attribute FOREIGN of call_python_function : procedure is "call_python_function ./fli_python.dll";	
```

The call_python_module() is used to load a module and call_python_function() calls a function inside the Python module. The FOREIGN attribute tells Modelsim which C function to call (call_python_module) and where it is located (/fli_python.dll). 

During elaboration Modelsim *binds* the 2 function calls to C functions compiled in the fli_python.dll file. When the simulator call these VHDL functions during simulation the equivalent C functions are called. In turn the two C functions are *linked* to the equivalent Python functions. In order to call the Python functions we first need to initialize the Python interpreter (*by calling Py_Initialize()*) and then load the Python module. Given that Python is an interpreted language after initializing the interpreter we can simply execute Python code by calling the **PyRun_SimpleString()** function. Thus the equivalent of:

```Python
import constraint
print("constraint test")
```

becomes under C as simple as:

```C
PyRun_SimpleString("import constraint");
PyRun_SimpleString("print(\"constraint test\")");
```

Thus you can run a Python program line for line using PyRun_SimpleString() but this becomes a bit cumbersome. Instead you can use **PyImport_Import()** to import a module which is what the demo does. Next we want to call a Python function (create_constraints, get_next) inside the Python module, for this we need to ask the Python interpreter to give us a so called handle which is like (but not the same as) a pointer to a function in C. We can obtain the handle using the **PyObject_GetAttrString()** function.  Now that the module has been loaded and we have the "pointers" to the functions we can call them from VHDL (or Verilog via wrapper).

The first thing the testbench does is to call the Python module. 

```VHDL
  begin
    call_python_module("constraint_test");	
	wait for 100 ns;
``` 
The call_python_module("constraint_test") will initialize the Python interpreter and executes the **constraint_test.py** module. The constraint_test.py module solves a very simple constraint for a modbus serial frame which requires that the maximum number of bits (startbit, databits, parity, stopbits) must not exceed 10bits. The constraint solver then works out all the valid combinations and passes them back to the VHDL testbench for running the tests. In order to get one valid solution each time we run a VHDL test we use the Python generator concept (see **yield** and **get_next** in constraint_test.py).
  
The allocated memory used by the Python interpreter needs to be released (by calling **Py_FinalizeEx()**) when the simulations ends. The FLI has the **mti_AddQuitCB()** function which is called when the user ends the simulation (*quit -sim*) or closes Modelsim. This function is used to call Py_FinalizeEx() and reported in the transcript window as "Cleaning up Python". 
Note that the FLI also has the **mti_AddRestartCB()** function which is called when the simulation restart (*restart -f*), this function is not used in the demo.
 
  
## Build DLL and run the demo
To build the DLL and run the demo execute the **run.bat** file in a CMD prompt. This should produce the **fli_python.dll** file and invoke Modelsim. The output should be something like:

```
E:\Modelsim-Python-Demo1>run
Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30137 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.

fli_python.c
Microsoft (R) Incremental Linker Version 14.29.30137.0
Copyright (C) Microsoft Corporation.  All rights reserved.

   Creating library fli_python.lib and object fli_python.exp
Reading pref.tcl

# 2022.1

# vsim -quiet -c tb -do "run 1 us; quit -f"
# //  ModelSim DE-64 2022.1 Jan 29 2022
# //
# //  Copyright 1991-2022 Mentor Graphics Corporation
# //  All Rights Reserved.
# //
# //  ModelSim DE-64 and its associated documentation contain trade
# //  secrets and commercial or financial information that are the property of
# //  Mentor Graphics Corporation and are privileged, confidential,
# //  and exempt from disclosure under the Freedom of Information Act,
# //  5 U.S.C. Section 552. Furthermore, this information
# //  is prohibited from disclosure under the Trade Secrets Act,
# //  18 U.S.C. Section 1905.
# //
# run 1 us
# Python FLI Constraint Test
# Python3.8 interpreter initialized
# Loading Module constraint_test
# ** Note: Constraint databits=8 startbits=1 stopbits=1 parity=0
#    Time: 100 ns  Iteration: 0  Instance: /tb
# ** Note: Constraint databits=7 startbits=1 stopbits=2 parity=0
#    Time: 200 ns  Iteration: 0  Instance: /tb
# ** Note: Constraint databits=7 startbits=1 stopbits=1 parity=1
#    Time: 300 ns  Iteration: 0  Instance: /tb
# ** Note: Constraint databits=7 startbits=1 stopbits=1 parity=0
#    Time: 400 ns  Iteration: 0  Instance: /tb
#  quit -f
# Cleaning up Python
# solution created
#
# Errors: 0, Warnings: 0
```

The Modelsim output shows 4 solutions each not exceeding 10 bits. 