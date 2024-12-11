#ifndef I_DATA_PROC_H
#define I_DATA_PROC_H

#include <string>

class IDataProc {
public:
    virtual std::string ProcessMessage(const std::string& message) = 0; // Process the message and return the response
};


#endif // I_DATA_PROC_H
