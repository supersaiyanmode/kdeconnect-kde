#include <Python.h>
#include <string>

#include <QLoggingCategory>

#include "interpreter.h"
#include "modules/module.h"

namespace {
    static PyObject* PyInit_PyExt() {
        PyModuleDef PyExtModule = {
            PyModuleDef_HEAD_INIT, "emb", NULL, -1, module_exports,
            NULL, NULL, NULL, NULL
        };
        return PyModule_Create(&PyExtModule);
    }
}

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PYEXT_INTERPRETER, "kdeconnect.plugin.pyext.interpreter")

Py3xSimpleInterpreter::Py3xSimpleInterpreter() {
    
}

Py3xSimpleInterpreter::~Py3xSimpleInterpreter() {
    Py_Finalize();
}

void Py3xSimpleInterpreter::initialize() {
    PyImport_AppendInittab("pyext", &PyInit_PyExt);
    Py_Initialize();
}

void Py3xSimpleInterpreter::addSysPath(const std::string& path){
    PyObject* sysPath = PySys_GetObject("path");
    PyObject* programName = PyUnicode_FromString(path.c_str());
    PyList_Append(sysPath, programName);
    Py_DECREF(programName);
}

std::map<std::string, std::string> Py3xSimpleInterpreter::callFunction(
        const std::string& plugin_dir, 
        const std::string& func, 
        const std::map<std::string, std::string>& params) {
    
    std::string cmd = "from " + plugin_dir + " import " 
                        + func + "; " + func + "()";
    qCDebug(KDECONNECT_PLUGIN_PYEXT_INTERPRETER) << "Command: "<<cmd.c_str();
    int res = PyRun_SimpleString(cmd.c_str());
    return {};
}
