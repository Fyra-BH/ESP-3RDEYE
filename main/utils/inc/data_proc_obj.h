#ifndef DATA_PROC_OBJ_H
#define DATA_PROC_OBJ_H

#include "idata_proc.h"
#include "servor_input_adapter.h"


class DataProcObj : public IDataProc
{
public:
    std::string ProcessMessage(const std::string& message) override;
private:
    std::string HandleDiscoveryRequest(const std::string& message);
    std::string HandleMoveRequest(const std::string& message);
    std::string HandleVarHackerRequest(const std::string& message);

private:
    ServoInputAdapter m_servoInputAdapter;
};

#endif // DATA_PROC_OBJ_H
