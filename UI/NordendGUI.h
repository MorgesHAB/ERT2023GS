/*!
 * \file Nordend_GUI.h
 *
 * \brief Nordend_GUI module interface
 *
 * \author      ISOZ Lionel - EPFL EL MA4
 * \date        11.07.2023	
 */

#ifndef Nordend_GUI_H
#define Nordend_GUI_H

#include <QMainWindow>
#include <QSerialPort>
#include "../Capsule/src/capsule.h"
#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"
#include <fstream>
#include <QPushButton> // valve function

QT_BEGIN_NAMESPACE
namespace Ui { class nordend; } // named of object "QMainWindow" in ui file
QT_END_NAMESPACE

class NordendGUI: public QMainWindow {
    Q_OBJECT
public:
    NordendGUI();

    virtual ~NordendGUI();

public slots:
    void readSerialData();
    void on_open_serial_pressed();
    void on_close_serial_pressed();
    void on_abort_cmd_pressed();
    void on_recover_cmd_pressed();
    void on_arm_cmd_pressed();
    void on_disarm_cmd_pressed();
    void on_ignition_cmd_pressed();
    void on_disconnect_cmd_pressed();
    void on_reset_valves_pressed();

    void on_GSE_fill_pressed();
    void on_GSE_vent_pressed();
    void on_AV_servo_N2O_pressed();
    void on_AV_servo_fuel_pressed();
    void on_AV_vent_N2O_pressed();
    void on_AV_vent_fuel_pressed();
    void on_AV_pressurization_pressed();

    void serialError();

    void qtimer_callback();

public:
    void handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len);
private:
    void sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size);
    void set_valve_img(QPushButton * valve, int i);

private:
    Ui::nordend *ui;

    QSerialPort *serial;

    Capsule<NordendGUI> capsule;

    PacketGSE_downlink packetGSE_downlink;

    av_downlink_t packetAV_downlink;

    QTimer * qtimer; // for time since last rx packet
    time_t lastRxTime;
};


#endif //Nordend_GUI_H
