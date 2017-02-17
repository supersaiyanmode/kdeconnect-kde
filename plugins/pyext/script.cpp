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

namespace {
    template<typename K, typename V>
    std::vector<K> extractKeys(const std::map<K, V>& map) {
        std::vector<K> result;
        result.reserve(map.size());
        
        for (auto const& entry: map) {
            result.push_back(entry.first);
        }
        
        return result;
    }
}

Script::Script(Interpreter& interpreter, const std::string& d,
               const std::string& p): _basedir(d), _plugindir(p),
               interpreter(interpreter) {
    _valid = parseMetadata(_basedir, _plugindir);
    if (!_valid) {
        return;
    }
    _capabilities = extractKeys(entry_points);
}

Script::~Script() {
    if (_background) {
        //terminate interpreter here.
    }   
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

const std::vector<std::string>& Script::capabilities() const {
    return _capabilities;
}

bool Script::valid() const {
    return _valid;
}

bool Script::invoke(const std::string& name, 
                    const std::map<std::string, std::string>& params) {
    std::map<std::string, std::string>::const_iterator it = entry_points.find(name);
    if (it == entry_points.end()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT()) << "Unable to find entry point: "<<QString::fromStdString(name)
        <<" for script: "<<QString::fromStdString(_name);
        return false;
    }
    
    interpreter.callFunction(_plugindir, it->second, params);
    return true;
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
    
    //Handle "name" - mandatory
    QJsonValue nameVal = obj.value(QString("name"));
    if (nameVal.isUndefined() || !nameVal.isString()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a 'name' key holding a string.";
        return false;
    }
    _name = nameVal.toString().toStdString();
    qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "Processing Plugin: "<<_name.c_str();
    
    
    //"description" - not mandatory
    QJsonValue descVal = obj.value(QString("description"));
    if (descVal.isUndefined() || !descVal.isString()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a 'description' key holding a string.";
    } else {
        _description = descVal.toString().toStdString();
    }

    //"persistent" - not mandatory
    QJsonValue persistentVal = obj.value(QString("persistent"));
    if (persistentVal.isUndefined() || !persistentVal.isBool()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a 'persistent' key holding a bool. Defaulting to false.";
        _background = false;
    } else {
        _background = persistentVal.toBool();
    }
    
    //"entry_points" - mandatory
    QJsonValue entryPointsVal = obj.value(QString("entry_points"));
    if (entryPointsVal.isUndefined() || !entryPointsVal.isObject()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a 'entry_points' key holding an object.";
        return false;
    }
    QJsonObject entryPointsObj = entryPointsVal.toObject();
    QStringList entryPointsKeys = entryPointsObj.keys();;
    QRegExp regex("[a-ZA-Z_][a-ZA-Z_]*");
    
    for (auto it = entryPointsKeys.begin(); it != entryPointsKeys.end(); it++) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "Processing EntryPoint: "<<entryPointsObj.value(*it).toString();
        QJsonValue entryVal = entryPointsObj.value(*it);
        if (entryVal.isUndefined() || !entryVal.isString()) {
            qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "JSON does not define a string value key against key: "<<*it;
            return false;
        }
        std::string entryPointStr = entryVal.toString().toStdString();
        if (!regex.exactMatch(QString::fromStdString(entryPointStr))) {
            qCWarning(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "Value against 'name' is not a valid Python identifier: "<<QString::fromStdString(entryPointStr);
            //TODO: For some reason the string is enclosed within double quotes.
            //return false;
        }
        entry_points[it->toStdString()] = entryPointStr;
    }
    
    if (entry_points.empty()) {
        qCDebug(KDECONNECT_PLUGIN_PYEXT_SCRIPT) << "No valid entry points found.";
        return false;
    }
    
    file.close();
    
    return true;
}
