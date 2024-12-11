#include "data_proc_obj.h"
#include "servo_group.h"
#include <cstring>
#include "esp_log.h"

static const char* TAG = "DataProcObj";

std::string DataProcObj::ProcessMessage(const std::string& message)
{
    if (message == "SatoriEye_DISCOVERY_REQUEST") {
        return HandleDiscoveryRequest(message);
    }
    if (message.starts_with("CH1")) {
        return HandleMoveRequest(message);
    }
    return message;
}

std::string DataProcObj::HandleDiscoveryRequest(const std::string& message)
{
    int power = 50; // %50
    // TODO: power = PowerManager::GetPowerLevel();
    ESP_LOGI(TAG, "Discovery request received, power level: %d%%", power);
    return "SatoriEye_DISCOVERY_RESPONSE," + std::to_string(power);
}

std::string DataProcObj::HandleMoveRequest(const std::string& message)
{
    int pulseWidth[3];
    float angle[3];
    sscanf(message.c_str(), "CH1:%dCH2:%dCH3:%d", &pulseWidth[0], &pulseWidth[1], &pulseWidth[2]);
    // ESP_LOGI(TAG, "Move request received, pulse width: CH1:%d, CH2:%d, CH3:%d", pulseWidth[0], pulseWidth[1], pulseWidth[2]);
    for (int i = 0; i < 3; i++) {
        angle[i] = m_servoInputAdapter.PulseWidth2Angle(pulseWidth[i]);
    }
    auto str = "CH1:" + std::to_string(angle[0]) + ",CH2:" + std::to_string(angle[1]) + ",CH3:" + std::to_string(angle[2]);
    ESP_LOGI(TAG, "Move request response: %s", str.c_str());
    ServoGroup::GetInstance().SetAngle((int)SERVO_IDX_PITCH, angle[0]);
    ServoGroup::GetInstance().SetAngle((int)SERVO_IDX_ROLL, angle[1]);
    ServoGroup::GetInstance().SetAngle((int)SERVO_IDX_YAW, angle[2]);
    return str;
}
