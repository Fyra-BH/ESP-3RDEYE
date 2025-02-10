#include "data_proc_obj.h"
#include "servo_group.h"
#include <cstring>
#include <sstream>
#include <algorithm>
#include "esp_log.h"
#include "var_hacker.hpp"
#include "esp_thread_helper.h"

static const char* TAG = "DataProcObj";

std::string DataProcObj::ProcessMessage(const std::string& message)
{
    if (message == "SatoriEye_DISCOVERY_REQUEST") {
        return HandleDiscoveryRequest(message);
    }
    if (message.starts_with("CH1")) {
        return HandleMoveRequest(message);
    }
    if (message.starts_with("SMOOTH")) {
        return HandleMoveRequestSmooth(message);
    }
    if (message.starts_with("VH:")) {
        return HandleVarHackerRequest(message);
    }
    return message;
}

std::string DataProcObj::HandleDiscoveryRequest(const std::string& message)
{
    int power = 50; // %50
    // TODO: power = PowerManager::GetPowerLevel();
    ESP_LOGI(TAG, "Discovery request received, power level: %d%%", power);
    return "SatoriEye_DISCOVERY_RESPONSE," + std::to_string(power) +  "\n";
}

std::string DataProcObj::HandleMoveRequest(const std::string& message)
{
    int pulseWidth[3];
    float theta[3];
    sscanf(message.c_str(), "CH1:%dCH2:%dCH3:%d", &pulseWidth[0], &pulseWidth[1], &pulseWidth[2]);
    // ESP_LOGI(TAG, "Move request received, pulse width: CH1:%d, CH2:%d, CH3:%d", pulseWidth[0], pulseWidth[1], pulseWidth[2]);
    for (int i = 0; i < 3; i++) {
        theta[i] = m_servoInputAdapter.PulseWidth2Angle(pulseWidth[i]);
    }
    auto str = "CH1:" + std::to_string(theta[0]) + ",CH2:" + std::to_string(theta[1]) + ",CH3:" + std::to_string(theta[2]);
    ESP_LOGI(TAG, "Move request response: %s", str.c_str());
    ServoGroup::GetInstance().SetAngle((int)SERVO_IDX_CH1, theta[0]);
    ServoGroup::GetInstance().SetAngle((int)SERVO_IDX_CH2, theta[1]);
    const float affectAngle = (theta[1] - 90.0f) * 0.8f;
    ServoGroup::GetInstance().SetAngle((int)SERVO_IDX_CH3, theta[2] - affectAngle); // TODO: magic number
    return str;
}

std::string DataProcObj::HandleMoveRequestSmooth(const std::string& message)
{
    int pulseWidth[3];
    float theta[3];
    int durationMs = 0;
    sscanf(message.c_str(), "SMOOTH:CH1:%dCH2:%dCH3:%dMS:%d", &pulseWidth[0], &pulseWidth[1], &pulseWidth[2], &durationMs);
    float duration = durationMs / 1000.0f;

    // ESP_LOGI(TAG, "Move request received, pulse width: CH1:%d, CH2:%d, CH3:%d", pulseWidth[0], pulseWidth[1], pulseWidth[2]);
    for (int i = 0; i < 3; i++) {
        theta[i] = m_servoInputAdapter.PulseWidth2Angle(pulseWidth[i]);
    }
    auto str = "CH1:" + std::to_string(theta[0]) + ",CH2:" + std::to_string(theta[1]) + ",CH3:" + std::to_string(theta[2]);
    theta[3] = theta[2] - (theta[1] - 90.0f) * 0.8f; // TODO: magic number (90.0f is the center angle)
    ESP_LOGI(TAG, "Move request response: %s", str.c_str());
    ServoGroup::GetInstance().SetAngleSmooth((int)SERVO_IDX_CH1, theta[0], duration);
    ServoGroup::GetInstance().SetAngleSmooth((int)SERVO_IDX_CH2, theta[1], duration);
    ServoGroup::GetInstance().SetAngleSmooth((int)SERVO_IDX_CH3, theta[2], duration);
    return str;
}

// message format: VH:operation,varName, duration,[value]
std::string DataProcObj::HandleVarHackerRequest(const std::string& message)
{
    std::string msg(message);
    size_t pos = message.find('\n');  // 查找第一个换行符的位置
    if (pos != std::string::npos) {
        msg = message.substr(0, pos);  // 截取从开始到换行符之前的部分
    }
    std::stringstream ss(msg);
    std::string operation;
    std::string varName;
    std::string value;
    std::getline(ss, operation, ',');
    std::getline(ss, varName, ',');
    std::getline(ss, value, ',');

    operation = std::string(operation.c_str());
    ESP_LOGI(TAG, "VarHacker request received, operation: %s, varName: %s, value: %s",
        operation.c_str(), varName.c_str(), value.c_str());

    std::string retMsg;

    if (operation == "VH:get") {
        auto data = VH::VarHacker::GetInstance().GetVar(varName);
        if (data == nullptr) {
            return "VH:get null";
        }
        std::string res = VH::VarHacker::GetInstance().GetVarInString(data);
        retMsg = "VH:get," + varName + "," + res;
        return res;
    }
    if (operation == "VH:showall") {
        auto varNames = VH::VarHacker::GetInstance().GetVarNames();
        for (auto &name : varNames) {
            auto data = VH::VarHacker::GetInstance().GetVar(name);
            std::string res = VH::VarHacker::GetInstance().GetVarInString(data);
            retMsg += name + ":" + res + "\n";
        }
    }
    if (operation == "VH:set") {
        auto data = VH::VarHacker::GetInstance().GetVar(varName);
        if (data == nullptr) {
            return "VH:set null";
        }
        VH::VarHacker::GetInstance().SetVarInString(varName, value);
        retMsg = "VH:set," + varName + "," + value;
    }
    ESP_LOGI(TAG,"%s", retMsg.c_str());
    return retMsg;
}