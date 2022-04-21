//-------------------------------------------------------------------------------------------------
// FLI demo calling a Python Constraint solver, the results are returned to VHDL as an integer array
//                                                                         
// https://github.com/htminuslab                                                   
//
//-------------------------------------------------------------------------------------------------
//
// Update: Created 14/06/2017
// Update: Updated for Python3.8/Modelsim 2022.1 20/04/2022 
//-------------------------------------------------------------------------------------------------
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <mti.h>


PyObject *pModule;

//-------------------------------------------------------------------------------------------------
// Convert VHDL string into a NULL terminated C string 
//-------------------------------------------------------------------------------------------------
static char *get_string(mtiVariableIdT id)
{
	static char buf[1000];
	mtiTypeIdT type;
	int len;
	mti_GetArrayVarValue(id, buf);
	type = mti_GetVarType(id);
	len = mti_TickLength(type);

	buf[len] = 0;
	return buf;
}

//-------------------------------------------------------------------------------------------------
// Free python memory when simulation stops
// This is called automatically
//-------------------------------------------------------------------------------------------------
void call_python_cleanup(void * param)
{
	mti_PrintMessage("Cleaning up Python\n");
	Py_DECREF(pModule);	
	Py_FinalizeEx();
}

//-------------------------------------------------------------------------------------------------
// Load Python module 
//-------------------------------------------------------------------------------------------------
int call_python_module(mtiVariableIdT module_name) 
{
    PyObject *pName;

	mti_PrintMessage("Python FLI Constraint Test\n");

    Py_Initialize();
	if (!Py_IsInitialized()) {
		mti_PrintFormatted("Unable to initialize Python3.8 interpreter\n");
		return 1;
	} else {		
		mti_PrintFormatted("Python3.8 interpreter initialized\n");
	}
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append(r'E:\\GitHub\\Modelsim-Python-Demo1')"); // CHANGE ME!!!

	mti_PrintFormatted("Loading Module %s\n",get_string(module_name));
	pName = PyUnicode_DecodeFSDefault(get_string(module_name));

    pModule = PyImport_Import(pName);		// Global variable
    Py_DECREF(pName);

    if (pModule == NULL) {
		PyErr_Print();
        mti_PrintFormatted("Failed to load \"%s\"\n" ,get_string(module_name));
        return 1;
    }
	
	mti_AddQuitCB(call_python_cleanup,NULL);
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
// Call function from Python module 
//-------------------------------------------------------------------------------------------------
int call_python_function(mtiVariableIdT function_name, mtiVariableIdT vhdl_array)
{
	PyObject *pFunc;
	PyObject *pArgs, *pValue;
	int *val;
	
	pFunc = PyObject_GetAttrString(pModule,get_string(function_name));
	
	if (pFunc && PyCallable_Check(pFunc)) {
		pArgs = PyTuple_New(0);
		pValue = PyObject_CallObject(pFunc,pArgs);
		Py_DECREF(pArgs);
		
        if (pValue != NULL) {
            for (int i=0; i<(int)PyList_Size(pValue); i++) {
				val = mti_GetArrayVarValue(vhdl_array, NULL);
				val[i]=(int)PyLong_AsLong(PyList_GetItem(pValue, (Py_ssize_t)i));
			}
            Py_DECREF(pValue);
        }
        else {
            Py_DECREF(pFunc);
            Py_DECREF(pModule);
            PyErr_Print();
            mti_PrintFormatted("Error, function %s returned an error\n",get_string(function_name));          
			return 1;
        }
	} else {
		if (PyErr_Occurred()) PyErr_Print();
		mti_PrintFormatted("Fatal error, cannot find function \"%s\"\n", get_string(function_name));
		mti_FatalError();
	}
	Py_XDECREF(pFunc);
	return 0;	
}
	
