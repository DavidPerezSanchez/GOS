//
// Created by Roger Generoso Masós on 20/04/2020.
//

#ifndef CSP2SAT_PARAM_H
#define CSP2SAT_PARAM_H

#include <regex>
#include <iostream>
#include "../../Errors/GOSException.h"
#include "../../Errors/GOSInputExceptionsRepository.h"
#include <string>
#include <vector>
#include <map>

namespace GOS {

class Param {
public:
    Param(const std::string &name) : name(name) {}
    virtual ~Param() {};

    virtual bool isValuable() = 0;

public:
    std::string name;
};

class ParamValuable : public Param {
public:
    ParamValuable(const std::string &name) : Param(name) {}
    virtual ~ParamValuable() {};

    bool isValuable() override {
        return true;
    }

    virtual int getValue() = 0;
};

class ParamBool : public ParamValuable {
public:
    ParamBool(const std::string &name, bool val) : ParamValuable(name) {
        this->value = val;
    }
    int getValue() override {
        return value;
    }
    bool value;
};


class ParamInt : public ParamValuable {
public:
    ParamInt(const std::string &name, int val) : ParamValuable(name) {
        this->value = val;
    }
    int getValue() override {
        return value;
    }
    int value;
};

class ParamScoped : public Param {
public:
    ParamScoped(const std::string &name) : Param(name) {}
    virtual ~ParamScoped() {};
    
    bool isValuable() override {
        return false;
    }

    virtual void add(Param * a) = 0;
    virtual Param * get(std::string name) = 0;
};

class ParamArray : public ParamScoped {
public:
    ParamArray(const std::string &name) : ParamScoped(name) {
        elements = std::vector<Param*>();
    }

    void add(Param *a) override {
        elements.push_back(a);
    }

    Param *get(std::string name) override {

        int index = stoi(name);
        if(index < elements.size()){
            return elements[index];
        }
        return nullptr;
    }

    std::vector<Param*> elements;
};


class ParamJSON : public ParamScoped {
public:
    ParamJSON(const std::string &name) : ParamScoped(name) {
        elements = std::map<std::string, Param*>();
    }

    void add(Param *a) override {
        elements[a->name] = a;
    }

    Param *get(std::string name) override {
        if(elements.find(name) != elements.end())
            return elements[name];
        return nullptr;
    }

    std::map<std::string, Param*> elements;

    int resolve(std::string attrAccess) {
        std::vector<std::string> splitted = Utils::splitVarAccessNested(attrAccess);

        std::string currAccess = "";

        bool first = true;

        ParamScoped * currentScope = this;
        for(std::string attr : splitted){
            Param * currentParam = nullptr;
            if(attr.back() == ']'){
                currentParam = currentScope->get(attr.substr(0, attr.size()-1));
                currAccess += "[" + attr.substr(0, attr.size()-1) + "]";
            }
            else {
                currentParam = currentScope->get(attr);
                currAccess += (first ? "" : ".") + attr;
            }
            if(currentParam == nullptr) {
                throw CSP2SATInputNotFoundValue(currAccess);
            };
            if(currentParam->isValuable())
                return ((ParamValuable*) currentParam)->getValue();

            currentScope = (ParamScoped*) currentParam;
            first = false;
        }
        return -1;
    }
};

}

#endif //CSP2SAT_PARAM_H
