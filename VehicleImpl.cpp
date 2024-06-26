#include "VehicleImpl.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <random>
#include <numeric>
#include <queue>
#include <spdlog/sinks/stdout_color_sinks.h>

VehicleImpl::VehicleImpl(std::shared_ptr<spdlog::logger> logger)
    : logger_(std::move(logger)) {
    isCruiseControl = false;
    running = true;
    subscribe_running = false;
    this->logger_->set_level(spdlog::level::info); 
    loadFromConfig();
    logFilePath        = "../logs/" + logFileName;
    historicCCFilePath = "../logs/" + historicCCFileName;
    std::ofstream logFile(logFilePath, std::ios::out | std::ios::trunc);
    logFile.close();
    std::ofstream historicFile(historicCCFilePath, std::ios::out | std::ios::trunc);
    historicFile.close();
    speedUpdateThread  = std::thread(&VehicleImpl::continuousSpeedUpdate, this);
    this->logger_->info("VehicleImpl initialized");
}

VehicleImpl::~VehicleImpl() {
    running = false;
    subscribe_running = false;
    if (subscriptionThread.joinable()) {
        subscriptionThread.join();
    }
    if (speedUpdateThread.joinable()) {
        speedUpdateThread.join();
    }
    this->logger_->info("VehicleImpl destroyed");
}
void VehicleImpl::loadFromConfig() {
    try {
        YAML::Node config = YAML::LoadFile("../settings/params.yaml");

        cruiseModeMaxSpeed = config["CRUISE_MODE_MAX_SPEED"].as<int>(90);
        cruiseModeMinSpeed = config["CRUISE_MODE_MIN_SPEED"].as<int>(20);
        currentSpeed       = config["INIT_SPEED"].as<int>(50);
        historicCCFileName = config["HISTORIC_CC_FILE_NAME"].as<std::string>("historic_cc_speeds.json");
        logFileName        = config["LOG_FILE_NAME"].as<std::string>("log.json");
        maxSpeed           = config["MANUAL_MODE_MAX_SPEED"].as<int>(100);
        minSpeed           = config["MANUAL_MODE_MIN_SPEED"].as<int>(10);
        simRate            = config["SIM_RATE"].as<double>(10.0);
        speedAvgWinSize    = config["SPEED_AVG_WIN_SIZE"].as<int>(1);

        this->logger_->info("Loaded configuration:");
        this->logger_->info("CRUISE_MODE_MAX_SPEED: {}", cruiseModeMaxSpeed);
        this->logger_->info("CRUISE_MODE_MIN_SPEED: {}", cruiseModeMinSpeed);
        this->logger_->info("INIT_SPEED: {}", currentSpeed);
        this->logger_->info("HISTORIC_CC_FILE_NAME: {}", historicCCFileName);
        this->logger_->info("LOG_FILE_NAME: {}", logFileName);
        this->logger_->info("MANUAL_MODE_MAX_SPEED: {}", maxSpeed);
        this->logger_->info("MANUAL_MODE_MIN_SPEED: {}", minSpeed);
        this->logger_->info("SIM_RATE: {}", simRate);
        this->logger_->info("SPEED_AVG_WIN_SIZE: {}", speedAvgWinSize);
    } catch (const YAML::BadFile& e) {
        this->logger_->error("YAML file error: {}", e.what());
    } catch (const std::exception& e) {
        this->logger_->error("Error: {}", e.what());

    }
}

void VehicleImpl::continuousSpeedUpdate() {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(minSpeed, maxSpeed);
    std::queue<int> speedQueue;  
    double sumSpeeds = 0;

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / simRate));
        if (!isCruiseControl) {
            int randomSpeed = distribution(generator);

            if (speedQueue.size() == speedAvgWinSize) {
                sumSpeeds -= speedQueue.front();
                speedQueue.pop();
            }

            speedQueue.push(randomSpeed);
            sumSpeeds += randomSpeed;

            currentSpeed = sumSpeeds / speedQueue.size();
            this->logger_->debug("Updated current speed: {}", currentSpeed);
        }
        logSpeedToJson();
    }
}

void VehicleImpl::getSpeed(IVehicleCallback* callback) {
    callback->onGetSpeed(getCurrentSpeed());
    this->logger_->info("Get speed: {}", getCurrentSpeed());
}

void VehicleImpl::setSpeed(int speed, IVehicleCallback* callback) {
    if (speed < cruiseModeMinSpeed || speed > cruiseModeMaxSpeed) {
        this->logger_->error("Invalid speed: {}. Please provide a speed between {} and {}. Cruise control disabled!", speed, cruiseModeMinSpeed, cruiseModeMaxSpeed);
        callback->onSetSpeed(false);
        return;
    }
    isCruiseControl = true;
    currentSpeed = speed;

    saveCruiseControlSpeed(speed);
    callback->onSetSpeed(true);
    this->logger_->info("Set speed: {} with cruise control", speed);
}

void VehicleImpl::unsetSpeed(IVehicleCallback* callback) {
    isCruiseControl = false;
    logSpeedToJson();
    callback->onUnsetSpeed(true);
    this->logger_->info("Unset speed, cruise control deactivated");
}

void VehicleImpl::getHistoricCCSpeed(const std::string& criteria, IVehicleCallback* callback) {
    nlohmann::json j;
    std::ifstream inFile(historicCCFilePath);
    if (inFile.peek() != std::ifstream::traits_type::eof()) {
        inFile >> j;
    }
    inFile.close();

    if (!j.contains("speeds")) {
        callback->onGetHistoricCCSpeed(-1);
        logger_->warn("No historic speeds found");
        return;
    }

    const auto& speeds = j["speeds"];

    if (criteria == "min" || criteria == "max") {
        if (criteria == "max") {
            int maxSpeed = 0;
            for (const auto& speed : speeds) {
                if (speed > maxSpeed) {
                    maxSpeed = speed;
                }
            }
            callback->onGetHistoricCCSpeed(maxSpeed);
            logger_->info("Max historic speed: {}", maxSpeed);
        } else {
            int minSpeed = std::numeric_limits<int>::max();
            for (const auto& speed : speeds) {
                if (speed < minSpeed) {
                    minSpeed = speed;
                }
            }
            callback->onGetHistoricCCSpeed(minSpeed);
            logger_->info("Min historic speed: {}", minSpeed);
        }
    } else {
        try {
            int index = std::stoi(criteria);
            if (index > 0 && index <= speeds.size()) {
                int reversedIndex = index - 1;
                callback->onGetHistoricCCSpeed(speeds[speeds.size() - 1 - reversedIndex]);
                logger_->info("Historic speed at index {}: {}", index, speeds[speeds.size() - 1 - reversedIndex].dump());
            } else {
                callback->onGetHistoricCCSpeed(-1, speeds.size());
                logger_->warn("Index out of range: {}", index);
            }
        } catch (const std::exception& e) {
            logger_->error("Invalid criteria: {}", e.what());
            callback->onGetHistoricCCSpeed(-1);
        }
    }
}

void VehicleImpl::subscribeSpeed(IVehicleCallback* callback) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        subscribe_running = true;
    }
    subscriptionThread = std::thread([callback, this]() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (!subscribe_running) {
                    break;
                }
            }
            callback->onSubscribeSpeed(getCurrentSpeed());
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    this->logger_->info("Subscribed to speed updates");
}

void VehicleImpl::unsubscribeSpeed(IVehicleCallback* callback) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        subscribe_running = false;
    }
    cv.notify_all();
    if (subscriptionThread.joinable()) {
        subscriptionThread.join();
    }
    callback->onUnsubscribeSpeed(true);
    this->logger_->info("Unsubscribed from speed updates");
}

int VehicleImpl::getCurrentSpeed() {
    return currentSpeed;
}

void VehicleImpl::logSpeedToJson() {
    nlohmann::json logEntry;
    auto now = std::chrono::system_clock::now();

    // Convert time_point to nanoseconds since epoch
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    logEntry["timestamp"] = nanoseconds;
    logEntry["speed"] = currentSpeed;
    logEntry["cruise_control"] = isCruiseControl ? "active" : "inactive";

    std::ofstream file(logFilePath, std::ios::app);
    file << logEntry.dump() << std::endl;

    this->logger_->debug("Logged speed to JSON: {}", logEntry.dump());
}

void VehicleImpl::saveCruiseControlSpeed(int speed) {
    nlohmann::json j;
    std::ifstream inFile(historicCCFilePath);
    if (inFile.peek() != std::ifstream::traits_type::eof()) {
        inFile >> j;
    }
    inFile.close();

    if (!j.contains("speeds")) {
        j["speeds"] = nlohmann::json::array();
    }

    j["speeds"].push_back(speed);

    std::ofstream outFile(historicCCFilePath, std::ios::out | std::ios::trunc);
    if (outFile.is_open()) {
        outFile << j.dump(4);
        outFile.close();
    }

    logger_->info("Saved cruise control speed: {}", speed);
}

