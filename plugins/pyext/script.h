#ifndef PYEXT_SCRIPT_H
#define PYEXT_SCRIPT_H

#include "Interpreter/interpreter.h"

#include <string>
#include <map>
#include <vector>

class Script {
  std::string _basedir;
  std::string _plugindir;
  std::string _name;
  std::string _description;
  std::string _guid;
  bool _valid;
  bool _background;
  
  Interpreter& interpreter;
  
  std::map<std::string, std::string> entry_points;
  std::vector<std::string> _capabilities;
  
  bool parseMetadata(const std::string&, const std::string&);
public:
  Script(Interpreter&, const std::string&, const std::string&);
  ~Script();
  
  bool valid() const;
  std::string name() const;
  std::string guid() const;
  std::string description() const;
  
  const std::vector<std::string>& capabilities() const;
  
  bool invoke(const std::string& name, const std::map<std::string, std::string>& params);
};

#endif
