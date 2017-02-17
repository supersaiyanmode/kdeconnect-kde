#ifndef PYEXT_INTERPRETER_H
#define PYEXT_INTERPRETER_H

#include <string>
#include <map>

class Interpreter {
public:
    Interpreter(){}
    virtual ~Interpreter(){}
    
    virtual void addSysPath(const std::string&) {}
    
    virtual void initialize()=0;
  
    virtual std::map<std::string, std::string> callFunction(
        const std::string&, const std::string&,
        const std::map<std::string, std::string>&) = 0;
};

class Py3xSimpleInterpreter: public Interpreter {
public:
    Py3xSimpleInterpreter();
    virtual ~Py3xSimpleInterpreter();
    
    void addSysPath(const std::string&);
    void initialize();
    
    std::map<std::string, std::string> callFunction(
        const std::string&, const std::string&,
        const std::map<std::string, std::string>&);
};

#endif
