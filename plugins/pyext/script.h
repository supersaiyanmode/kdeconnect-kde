#ifndef PYEXT_SCRIPT_H
#define PYEXT_SCRIPT_H

#include <string>
#include <map>


class Script {
  std::string _basedir;
  std::string _plugindir;
  std::string _name;
  std::string _description;
  std::string _guid;
  bool _valid;
  
  bool parseMetadata(const std::string&, const std::string&);
public:
  Script(const std::string&, const std::string&);
  ~Script();
  
  bool valid() const;
  std::string name() const;
  std::string guid() const;
  std::string description() const;
  
  bool invoke(const std::map<std::string, std::string>& params) const;
};

#endif