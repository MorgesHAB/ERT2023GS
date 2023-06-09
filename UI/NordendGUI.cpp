/*!
 * \file Nordend_GUI.cpp
 *
 * \brief Nordend_GUI module implementation
 *
 * \author      ISOZ Lionel - EPFL EL BA3
 * \date        11.07.2023	
 */

#include "NordendGUI.h"
#include <ui_GS2023.h>
#include <iostream>
#include <QTimer>
#include <QGraphicsColorizeEffect>
//#include <cmath>

#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"

#define REFRESH_PERIOD_ms       1000   // refresh every second
#define TIME_TO_SEND_CMD_ms     1000   // refresh every second

#define PACKET_RATE_MAX_UI      30  // slider

#define RX_IMAGES_PATH          "../ImagesRx/ImageRx.jpg"


NordendGUI::NordendGUI() :
        ui(new Ui::nordend),
        serial(new QSerialPort(this)),
        capsule(&NordendGUI::handleSerialRxPacket, this),
        gse_cmd_status({STATUS_INACTIVE, STATUS_ACTIVE})
{

    ui->setupUi(this);

//    qtimer->start(REFRESH_PERIOD_ms);

    connect(serial, &QSerialPort::readyRead, this, &NordendGUI::readSerialData);
    connect(serial, &QSerialPort::errorOccurred, this, &NordendGUI::serialError);

    std::cout << "NordendGUI inited" << std::endl;

    serial->setPortName("ttyS26");
    serial->setBaudRate(9600); // Don't care if USB
    serial->setDataBits(QSerialPort::DataBits::Data8);
    /*   //We will chose the parity bits
       serial->setParity(QSerialPort::Parity);
       //We will chose the stop bits
       serial->setStopBits(QSerialPort::StopBits);
       //We will chose the Flow controls
       serial->setFlowControl(QSerialPort::FlowControl);*/
    on_open_serial_pressed(); // open program by auto-detecting serial-port !
}

void NordendGUI::readSerialData() {
    QByteArray data = serial->readAll();
    for (auto &&item: data) {
        capsule.decode(item);
    }
    //std::cout << "Received data " << data.size() << std::endl;
    /*for (int i = 0; i < data.size(); ++i) {
        if (data[i] != 'm') std::cout << data[i]; // << " | ";
        else ctr++;
    }
    std::cout <<std::endl;*/
}

NordendGUI::~NordendGUI() {
    if (serial->isOpen()) serial->close();
    delete ui;
    std::cout << "NordendGUI deleted" << std::endl;
}

void
NordendGUI::handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
//    static std::string filename_time = filename;
//    static float altitude_max = 0;
//    ui->send_cmd_available->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
//    packet_ctr++;
//    lastRxTime = std::time(nullptr); // for now
    std::cout << "Packet received, ID: " << +packetId << std::endl;
    switch (packetId) {
        case 0x00:
            std::cout << "Packet with ID 00 received : " << +packetId << std::endl;
            //Serial.write(dataIn,len);
            break;
        case CAPSULE_ID::AV_CMD_VALVE_FUEL: {
            std::cout << "AV_CMD_VALVE_FUEL received! " << std::endl;
//            ui->ping_pong_ack->setStyleSheet("image: url(:/assets/tennis.png);");
            break;
        case CAPSULE_ID::GSE_VENT: { // debug only, ACK should come from TelemetryAV cmd_status !!
            //AckPacket packet;
            //memcpy(&packet, dataIn, _size);
            std::cout << "Packet GSE vent received" << std::endl;
//            ui->vent_GSE->setCheckState(Qt::CheckState::Checked);
//            ui->fill_GSE->setCheckState(Qt::CheckState::Checked);
            set_valve_img(ui->valve_test, 2);
            break;
        }
        case CAPSULE_ID::GSE_TELEMETRY: {
            //AckPacket packet;
            memcpy(&gse_cmd_status, dataIn, GSE_cmd_status_size);
            std::cout << "Packet GSE vent received" << std::endl;
            set_valve_img(ui->GSE_fill, gse_cmd_status.fillingN2O);
            set_valve_img(ui->GSE_vent, gse_cmd_status.vent);
            break;
        }
        default:
            break;
        }
            // set callback for sending cmd available
//    qtimer_rximg->start(TIME_TO_SEND_CMD_ms);
//    qtimer_rximg->setSingleShot(true);
    }
}

void NordendGUI::sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size) {
    uint8_t *packetToSend = capsule.encode(packetId, packet, size);
    serial->write((char *) packetToSend,capsule.getCodedLen(size));
    delete[] packetToSend;
}

void NordendGUI::on_close_serial_pressed() {
    if (serial->isOpen()) {
        serial->close();
        std::cout << "Serial port closed" << std::endl;
        ui->serial_port_detected_name->setText("-");
        ui->serial_port_detected_name->setStyleSheet(""); // no color
    } else {
        std::cout << "Serial port already closed" << std::endl;
    }
    ui->serialport_status->setStyleSheet(
            "QLabel {image: url(:/assets/refresh.png);}");
}

void NordendGUI::serialError() {
    //std::cout << "Serial port interrupt error" << std::endl;
    ui->serialport_status->setStyleSheet(
            "QLabel {image: url(:/assets/redCross.png);}");
    if (serial->isOpen()) serial->close();
    ui->serial_port_detected_name->setText("-");
    ui->serial_port_detected_name->setStyleSheet(""); // no color
}


void NordendGUI::on_open_serial_pressed() {
    int ctr = 0;
    QString serial_port_name = "";
    if (!serial->isOpen()) {
        do {
            serial_port_name = "ttyS" + QString::number(ctr++);
            serial->setPortName(serial_port_name);
        } while (!serial->open(QIODevice::ReadWrite) && ctr <= 30);
        if (serial->isOpen()) {
            std::cout << "Serial port open" << std::endl;
            ui->serialport_status->setStyleSheet(
                    "QLabel {image: url(:/assets/correct.png);}");
            ui->serial_port_detected_name->setText(serial_port_name);
            ui->serial_port_detected_name->textFormat();
            ui->serial_port_detected_name->setStyleSheet("color : green;");
        } else {
            std::cout << "Impossible to find valid serial port" << std::endl;
            ui->serialport_status->setStyleSheet(
                    "QLabel {image: url(:/assets/redCross.png);}");
            ui->serial_port_detected_name->setText("None!");
            ui->serial_port_detected_name->setStyleSheet("color : red;");
        }
    }
}

void NordendGUI::on_abort_cmd_pressed() {
    uint8_t x =  10;
    sendSerialPacket(CAPSULE_ID::ABORT, &x, sizeof(x));
}

void NordendGUI::on_ignition_cmd_pressed() {

}

void NordendGUI::on_disconnect_cmd_pressed() {

}

void NordendGUI::on_vent_GSE_clicked() { //stateChanged(int state) {
    std::cout << "Miaou state: " << "." << std::endl;
    Packet_cmd p;
    p.value = CMD_ACTIVE;
    sendSerialPacket(CAPSULE_ID::GSE_VENT, (uint8_t*) &p, packet_cmd_size);
    //ui->vent_GSE->setCheckState(Qt::CheckState::PartiallyChecked);
}

void NordendGUI::on_reset_valves_pressed() {
//    ui->vent_GSE->setCheckState(Qt::CheckState::Unchecked);
//    ui->fill_GSE->setCheckState(Qt::CheckState::Unchecked);
    set_valve_img(ui->valve_test, 0);
    set_valve_img(ui->GSE_fill, 0);
    set_valve_img(ui->GSE_vent, 0);
}

void NordendGUI::on_valve_test_pressed() {
    std::cout << "Miaou state: " << "." << std::endl;
    Packet_cmd p;
    p.value = CMD_ACTIVE;
    sendSerialPacket(CAPSULE_ID::GSE_VENT, (uint8_t*) &p, packet_cmd_size);
    set_valve_img(ui->valve_test, 1);
}

void NordendGUI::set_valve_img(QPushButton * valve, int i) {
    QString img_name = "";
    switch (i) {
        case STATUS_INACTIVE: img_name = "CloseH";
        break;
        case STATUS_ACTIVE: img_name = "OpenV";
        break;
        default: img_name = "Unknown";
        break;
    }
    valve->setStyleSheet("QPushButton {\n"
                         "\tbackground: transparent;\n"
                         "\tqproperty-icon: url(:/assets/GS_valve_V2_" + img_name + ".svg);\n"
                         "   qproperty-iconSize: 50px;\n"
                         "}\n"
                         "QPushButton:hover {\n"
                         "\tbackground-color: rgba(0, 0, 0, 50); \n"
                         "}");
}

void NordendGUI::on_GSE_fill_pressed() {
    Packet_cmd p;
    if (gse_cmd_status.fillingN2O == STATUS_ACTIVE) p.value = CMD_INACTIVE;
    else if (gse_cmd_status.fillingN2O == STATUS_INACTIVE) p.value = CMD_ACTIVE;
    sendSerialPacket(CAPSULE_ID::GSE_FILLING_N2O, (uint8_t*) &p, packet_cmd_size);
    set_valve_img(ui->GSE_fill, 1);
}

void NordendGUI::on_GSE_vent_pressed() {
    Packet_cmd p;
    if (gse_cmd_status.vent == STATUS_ACTIVE) p.value = CMD_INACTIVE;
    else if (gse_cmd_status.vent == STATUS_INACTIVE) p.value = CMD_ACTIVE;
    sendSerialPacket(CAPSULE_ID::GSE_VENT, (uint8_t*) &p, packet_cmd_size);
    set_valve_img(ui->GSE_vent, 1);
}











