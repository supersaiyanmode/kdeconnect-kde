#include <cstdlib>

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QProcess>

#include "script.h"

Script::Script(const std::string& p): _path(p) {

}

Script::~Script() {

}

std::string Script::name() const {
    return "script" + _path;
}

std::string Script::guid() const {
    return "guid" + _path;
}


QString Script::invoke(const std::map<std::string, std::string>& params) const {
  ::system(("python " + _path).c_str());
  return QString("ran the script");
}

