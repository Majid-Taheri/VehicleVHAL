#ifndef IVEHICLE_H
#define IVEHICLE_H

#include <string>

class IVehicleCallback {
public:
    virtual void onGetSpeed(int speed) = 0;
    virtual void onSetSpeed(bool success) = 0;
    virtual void onUnsetSpeed(bool success) = 0;
    virtual void onGetHistoricCCSpeed(int speed, int speed_vec_size = 0) = 0;
    virtual void onSubscribeSpeed(int speed) = 0;
    virtual void onUnsubscribeSpeed(bool success) = 0;
    virtual ~IVehicleCallback() = default;
};

class IVehicle {
public:
    virtual void getSpeed(IVehicleCallback* callback) = 0;
    virtual void setSpeed(int speed, IVehicleCallback* callback) = 0;
    virtual void unsetSpeed(IVehicleCallback* callback) = 0;
    virtual void getHistoricCCSpeed(const std::string& input, IVehicleCallback* callback) = 0; // Changed parameter type
    virtual void subscribeSpeed(IVehicleCallback* callback) = 0;
    virtual void unsubscribeSpeed(IVehicleCallback* callback) = 0;
    virtual ~IVehicle() = default;
};

#endif // IVEHICLE_H
