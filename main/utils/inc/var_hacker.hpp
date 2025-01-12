#ifndef VAR_HACKER_HPP
#define VAR_HACKER_HPP

#include <map>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <functional>


namespace VH{
struct DataTarget {
    void *ptr;
    const char *type;
};

namespace {
const char *TAG = "VarHacker";

const auto SUPORTED_TYPES = {
    typeid(int).name(),
    typeid(float).name(),
    typeid(bool).name(),
};

std::string GetVarInString(std::shared_ptr<DataTarget> target)
{
    std::string str;
    if (std::strcmp(target->type, typeid(int).name()) == 0) {
        str = std::to_string(*(int*)target->ptr);
    } else if (std::strcmp(target->type, typeid(float).name()) == 0) {
        str = std::to_string(*(float*)target->ptr);
    } else if (std::strcmp(target->type, typeid(bool).name()) == 0) {
        str = *(bool*)target->ptr? "true" : "false";
    } else {
        str = "Unsupported type";
    }
    return str;
}
} // namespace

/**
 * @brief VarHacker is a singleton class that allows you to register variables and access them from anywhere in the code.
 *        The class is implemented as a singleton, so you can access it from anywhere in the code.
 */
class VarHacker {
public:
    static VarHacker& GetInstance()
    {
        static VarHacker instance;
        return instance;
    }

    template <typename T>
    bool RegisterVar(std::string name, T* var)
    {
        const char *type = typeid(T).name();
        if(m_vars.find(name) != m_vars.end()) {
            return false;
        }
        ESP_LOGI(TAG, "Registering var: %s, type: %s, val: %s", name.c_str(), type,
            GetVarInString(std::make_shared<DataTarget>(reinterpret_cast<void *>(var), type)).c_str());
        m_vars[name] = std::make_shared<DataTarget>(reinterpret_cast<void *>(var), type);
        return true;
    }

    template <typename T>
    T AddVal(std::string name, T* val)
    {
        this->RegisterVar(name, val);
        return val;
    }

    std::shared_ptr<DataTarget> GetVar(std::string name)
    {
        if(m_vars.find(name)!= m_vars.end()) {
            return m_vars[name];
        }
        return nullptr;
    }
    
    template <typename T>
    bool SetVar(const char *name, T&& var)
    {
        if(m_vars.find(name)!= m_vars.end()) {
            *(T*)m_vars[name]->ptr = var;
            return true;
        }
        return false;
    }
    
    const std::vector<std::string> GetVarNames()
    {
        std::vector<std::string> names;
        for(auto &var: m_vars) {
            names.push_back(var.first);
        }
        return names;
    }

    ~VarHacker(){}

private:
    VarHacker(){}
    std::map<std::string, std::shared_ptr<DataTarget>> m_vars; // <name, <ptr, type>>
};

} // namespace VH

#endif
