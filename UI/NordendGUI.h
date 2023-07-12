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
    void on_ignition_cmd_pressed();
    void on_disconnect_cmd_pressed();
    void on_reset_valves_pressed();
    void on_valve_test_pressed();
    void on_GSE_fill_pressed();
    void on_GSE_vent_pressed();


    void on_vent_GSE_clicked();


    void serialError();

public:
    void handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len);
private:
    void sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size);
    void set_valve_img(QPushButton * valve, int i);

private:
    Ui::nordend *ui;

    QSerialPort *serial;

    Capsule<NordendGUI> capsule;

    GSE_cmd_status gse_cmd_status;
};


#endif //Nordend_GUI_H
