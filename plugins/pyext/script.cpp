#include <Python.h>
#include <cstdlib>

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QProcess>
#include <QLoggingCategory>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "script.h"

Q_LOGGING_CATEGORY(KDECONNECT_PLUGIN_PYEXT_SCRIPT, "kdeconnect.plugin.pyext.script")


Script::Script(const std::string& d, const std::string& p): _basedir(d), _plugindir(p) {
    _valid = parseMetadata(_basedir, _plugindir);
}

Script::~Script() {

}

bool Script::valid() const {
    return _valid;
}

std::string Script::name() const {
    return _name;
}

std::string Script::description() const {
    return _description;
}

std::string Script::guid() const {
    return _guid;
}

bool Script::invoke(const std::map<std::string, std::string>& params) const {
    Py_Initialize();
    PyObject* sysPath = PySys_GetObject((char*)"path");
    PyObject* programName = PyString_FromString((_basedir).c_str());
    PyList_Append(sysPath, programName);
    Py_DECREF(programName);
    std::string cmd = "from " + _plugindir + ".main import main; main()";
    int res = PyRun_SimpleString(cmd.c_str());
    Py_Finalize();
    return !res;
}

bool Script::parseMetadata(const std::string& basedir, const std::string& plugindir) {
    QFile file;
    QString dir = QDir(basedir.c_str()).filePath(plugindir.c_str());
    QString jsonPath = QDir(dir).filePath("metadata.json");
    file.setFileName(jsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "Unable to find metadata.json in the directory:"<<dir;
        return false;
    }
    QString contents = file.readAll();
    QJsonDocument d = QJsonDocument::fromJson(contents.toUtf8());
    QJsonObject obj = d.object();
    QJsonValue nameVal = obj.value(QString("name"));
    if (nameVal.isUndefined()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a 'name' key.";
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << obj;
        return false;
    }
    _name = nameVal.toString().toStdString();
    
    QJsonValue descVal = obj.value(QString("description"));
    if (descVal.isUndefined()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a 'description' key.";
    } else {
        _description = descVal.toString().toStdString();
    }
    
    file.close();
    
    return true;
}
