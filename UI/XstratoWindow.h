/*!
 * \file XstratoWindow.h
 *
 * \brief XstratoWindow module interface
 *
 * \author      ISOZ Lionel - EPFL EL MA4
 * \date        13.04.2023	
 */

#ifndef XstratoWindow_H
#define XstratoWindow_H


#include <QMainWindow>
#include <QSerialPort>
//#include <capsule.h>
#include "../Capsule/src/capsule.h"
#include <fstream>

QT_BEGIN_NAMESPACE
namespace Ui { class xstrato_ui; } // named of object in ui file
QT_END_NAMESPACE

class XstratoWindow: public QMainWindow {
    Q_OBJECT
public:
    XstratoWindow(int* myvar = nullptr);

    virtual ~XstratoWindow();

public slots:
    void readSerialData();
    void on_send_color_cmd_pressed();
    void on_clear_image_pressed();
    void on_open_serial_pressed();
    void on_close_serial_pressed();
    void on_test_button_pressed();
    void on_lock_RF_param_pressed();
    void on_set_RF_param_pressed();
    void on_set_CAM_param_pressed();
    void on_send_transmission_settings_pressed();
    void on_ping_cmd_pressed();

    void serialError();
    void qtimer_callback();
    void qtimer_rximg_callback();
    void LoRa_RF_param_changed(int value);

public:
    void handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len);
    void sendRandomPacket();
private:
    uint32_t getBandwidthHz(int index);
    uint32_t getCamFrameSize(int index);
    std::string insert_time(std::string s);
    void sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size);

private:
    Ui::xstrato_ui *ui;
    int* myvar;

    QSerialPort *serial;

    Capsule<XstratoWindow> capsule;

    size_t ctr;

    std::ofstream fileOutImg;
    std::string filename;

    QTimer * qtimer; // for time since last rx packet
    QTimer * qtimer_rximg; // for time since last rx packet
    time_t lastRxTime;
    uint32_t packet_ctr; // for packet rate
};

#endif //XstratoWindow_H
