#ifndef VEHICLEIMPL_H
#define VEHICLEIMPL_H

#include "IVehicle.h"

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <vector>
#include <yaml-cpp/yaml.h>

class VehicleImpl : public IVehicle {
public:
    VehicleImpl(std::shared_ptr<spdlog::logger> logger);
    ~VehicleImpl();  // Destructor

    void getSpeed(IVehicleCallback* callback) override;
    void setSpeed(int speed, IVehicleCallback* callback) override;
    void unsetSpeed(IVehicleCallback* callback) override;
    void getHistoricCCSpeed(const std::string& input, IVehicleCallback* callback) override;
    void subscribeSpeed(IVehicleCallback* callback) override;
    void unsubscribeSpeed(IVehicleCallback* callback) override;

private:
    void continuousSpeedUpdate();
    int getCurrentSpeed();
    void saveCruiseControlSpeed(int speed);
    void logSpeedToJson();
    void loadFromConfig();

    std::atomic<bool> running;
    std::condition_variable cv;
    bool isCruiseControlEnabled;
    bool subscribe_running;
    int cruiseModeMaxSpeed;
    int cruiseModeMinSpeed;
    int currentSpeed;
    int maxSpeed;
    int minSpeed;
    int simRate;
    int speedAvgWinSize;
    std::map<std::string, int> speedLog;
    std::mutex mtx;
    std::mutex mtxSpeed;
    std::shared_ptr<spdlog::logger> logger_;
    std::string historicCCFileName;
    std::string historicCCFilePath;
    std::string logFileName;
    std::string logFilePath;
    std::thread speedUpdateThread;
    std::thread subscriptionThread;
};

#endif // VEHICLEIMPL_H
