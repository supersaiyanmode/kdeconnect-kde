#ifndef PYEXT_SCRIPT_H
#define PYEXT_SCRIPT_H

#include <string>
#include <map>


class Script {
  std::string _dir;
  std::string _name;
  
public:
  Script(const std::string&, const std::string&);
  ~Script();
  
  std::string name() const;
  std::string guid() const;
  
  QString invoke(const std::map<std::string, std::string>& params) const;
};

#endif