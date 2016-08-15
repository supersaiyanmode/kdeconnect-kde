#include <Python.h>
#include <cstdlib>

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QProcess>


#include "script.h"

Script::Script(const std::string& d, const std::string& n): _dir(d), _name(n) {
    
}

Script::~Script() {

}

std::string Script::name() const {
    return _name;
}

std::string Script::guid() const {
    return _name;
}

QString Script::invoke(const std::map<std::string, std::string>& params) const {
    Py_Initialize();
    PyObject* sysPath = PySys_GetObject((char*)"path");
    PyObject* programName = PyString_FromString((_dir).c_str());
    PyList_Append(sysPath, programName);
    Py_DECREF(programName);
    std::string cmd = "from " + _name + ".main import main; main()";
    PyRun_SimpleString(cmd.c_str());
    Py_Finalize();
    return "done";
}

