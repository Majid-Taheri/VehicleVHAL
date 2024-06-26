#include "IVehicle.h"
#include "VehicleImpl.h"

#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

class MyVehicleCallback : public IVehicleCallback {
public:
    void onGetSpeed(int speed) override {
        spdlog::info("Current speed: {}", speed);
    }

    void onSetSpeed(bool success) override {
        spdlog::info("Set speed success: {}", success);
    }

    void onUnsetSpeed(bool success) override {
        spdlog::info("Unset speed success: {}", success);
    }

    void onGetHistoricCCSpeed(int speed, int speed_vec_size = 0) override {
        if (speed == -1) {
            spdlog::warn("No cruise control was set for this input, max size is: {}", speed_vec_size);
        } else {
            spdlog::info("Historic speed: {}", speed);
        }
    }

    void onSubscribeSpeed(int speed) override {
        spdlog::info("Subscribed speed: {}", speed);
    }

    void onUnsubscribeSpeed(bool success) override {
        spdlog::info("Unsubscribe success: {}", success);
    }
};

void handleCommand(IVehicle* vehicleService, const std::string& command, MyVehicleCallback& callback) {
    spdlog::info("Received command: {}", command);
    if (command == "get speed") {
        vehicleService->getSpeed(&callback);
    } else if (command.substr(0, 9) == "set speed") {
        int speed = std::stoi(command.substr(10));
        vehicleService->setSpeed(speed, &callback);
    } else if (command == "unset speed") {
        vehicleService->unsetSpeed(&callback);
    } else if (command.substr(0, 21) == "get historic_cc_speed") {
        if (command.length() > 21) {
            std::string timestamp = command.substr(22);
            vehicleService->getHistoricCCSpeed(timestamp, &callback);
        } else {
            spdlog::error("Invalid command format for get historic_cc_speed");
        }
    } else if (command == "subscribe speed") {
        vehicleService->subscribeSpeed(&callback);
    } else if (command == "unsubscribe speed") {
        vehicleService->unsubscribeSpeed(&callback);
    } else {
        spdlog::error("Unknown command");
    }
}

int main(int argc, char* argv[]) {
    auto console_logger = spdlog::stdout_color_mt("VehicleImpl");
    spdlog::set_default_logger(console_logger);
    spdlog::set_level(spdlog::level::info);  

    auto vehicleService = std::make_shared<VehicleImpl>(console_logger);
    MyVehicleCallback callback;
    std::string command;
    
    spdlog::info("Enter commands: ");

    while (std::getline(std::cin, command)) {
        if (command == "exit") {
            break;
        }
        handleCommand(vehicleService.get(), command, callback);
    }

    spdlog::info("Exiting application");
    return 0;
}
