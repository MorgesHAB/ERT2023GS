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
#include <cmath> // pow, alt(pressure)

#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"

#define REFRESH_PERIOD_ms       1000   // refresh every second
#define TIME_TO_SEND_CMD_ms     1000   // refresh every second

#define PACKET_RATE_MAX_UI      30  // slider

#define UNKNOWN                 0  // valve state yellow


NordendGUI::NordendGUI() :
        ui(new Ui::nordend),
        serial(new QSerialPort(this)),
        capsule(&NordendGUI::handleSerialRxPacket, this),
        qtimer(new QTimer(this)),
        lastRxTime(std::time(nullptr))
        {

//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
//    QCoreApplication::setAttribute(Qt::AA_Use96Dpi);

    ui->setupUi(this);

    qtimer->start(REFRESH_PERIOD_ms);

    connect(serial, &QSerialPort::readyRead, this, &NordendGUI::readSerialData);
    connect(serial, &QSerialPort::errorOccurred, this, &NordendGUI::serialError);

    connect(qtimer, SIGNAL(timeout()), this, SLOT(qtimer_callback()));

    std::cout << "NordendGUI inited" << std::endl;

    on_open_serial_pressed(); // open program by auto-detecting serial-port !
}

NordendGUI::~NordendGUI() {
    if (serial->isOpen()) serial->close();
    delete ui;
    std::cout << "NordendGUI deleted" << std::endl;
}

//////////////////////////////////////////////////////
// Telemetry receiver

void NordendGUI::handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
//    packet_ctr++;
    static int altitude_max = 0;
    lastRxTime = std::time(nullptr); // for now
    std::cout << "Packet received, ID: " << +packetId << std::endl;
    switch (packetId) {
        case 0x00:
            std::cout << "Packet with ID 00 received : " << +packetId << std::endl;
            break;
        case CAPSULE_ID::AV_TELEMETRY: {
            std::cout << "Packet AV_TELEMETRY received" << std::endl;
            memcpy(&packetAV_downlink, dataIn, av_downlink_size);
            // Set the valves states
            set_valve_img(ui->AV_servo_N2O, packetAV_downlink.valves_state); // TODO update !!!! with real value
            set_valve_img(ui->AV_servo_fuel, packetAV_downlink.valves_state);
            set_valve_img(ui->AV_vent_N2O, packetAV_downlink.valves_state);
            set_valve_img(ui->AV_vent_fuel, packetAV_downlink.valves_state);
            set_valve_img(ui->AV_pressurization, packetAV_downlink.valves_state);

            // Set telemetry data box
            ui->N2O_pressure->setText(QString::number(packetAV_downlink.N2O_pressure) + " hPa");
            ui->N2O_temp->setText(QString::number(packetAV_downlink.N2O_temp) + " °C");
            ui->fuel_pressure->setText(QString::number(packetAV_downlink.fuel_pressure) + " hPa");
            ui->chamber_pressure->setText(QString::number(packetAV_downlink.chamber_pressure) + " hPa");

            ui->AV_temp->setText(QString::number(packetAV_downlink.baro_temp));
//            ui->AV_humidity->setText(QString::number(packetAV_downlink.humidity));
            ui->AV_humidity->setText(QString::number(packetAV_downlink.baro_press));
            float sea_level_pressure_hPa = ui->sea_level_pressure_edit->text().toFloat();
            float altitude = 44330.0 * (1.0 - pow(packetAV_downlink.baro_press / sea_level_pressure_hPa, 0.1903));
            ui->AV_altitude_baro->setText(QString::number(altitude));

            // GPS data
            // GPS data
            ui->AV_latitude->setText(QString::number(packetAV_downlink.gnss_lat));
            ui->AV_longitude->setText(QString::number(packetAV_downlink.gnss_lon));
            ui->altitude_lcd_gps->display(QString::number((int) packetAV_downlink.gnss_alt));
            if (packetAV_downlink.gnss_alt > altitude_max) altitude_max =packetAV_downlink.gnss_alt;
            ui->altitude_max_lcd_max->display(QString::number(altitude_max));
//            ui->speed_vertical->setText(QString::number(packet.telemetry.verticalSpeed));
//            ui->speed_horizontal->setText(QString::number(packet.telemetry.horizontalSpeed));

            break;
        }
        case CAPSULE_ID::GSE_TELEMETRY: {
            std::cout << "Packet GSE_TELEMETRY received" << std::endl;
            memcpy(&packetGSE_downlink, dataIn, packetGSE_downlink_size);
            set_valve_img(ui->GSE_fill, packetGSE_downlink.status.fillingN2O);
            set_valve_img(ui->GSE_vent, packetGSE_downlink.status.vent);
            ui->GSE_pressure->setText(QString::number(packetGSE_downlink.tankPressure) + " hPa");
            break;
        }
        default:
            break;
        }
}

///////////////////////////////////////////////////
// CMD button handling

void NordendGUI::on_arm_cmd_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_ARM;
    p.order_value = ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_disarm_cmd_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_ARM;
    p.order_value = INACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_abort_cmd_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_ABORT;
    p.order_value = ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_recover_cmd_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_RECOVER;
    p.order_value = ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_ignition_cmd_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_IGNITION;
    p.order_value = IGNITION_CODE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_disconnect_cmd_pressed() {
    ui->prop_diagram->setStyleSheet("QPushButton{background: transparent;qproperty-icon: url(:/assets/Prop_background_disconnect.png);qproperty-iconSize: 700px;}");
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_DISCONNECT;
    p.order_value = ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

////////////////////////////////////////////////////////
// Valve clicked

void NordendGUI::on_AV_servo_N2O_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_SERVO_N2O;
    p.order_value = (packetAV_downlink.engine_state == ACTIVE)?INACTIVE:ACTIVE; // TODO update !!!!!!!!!!!!!!!!!!
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->AV_servo_N2O, UNKNOWN);
}

void NordendGUI::on_AV_servo_fuel_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_SERVO_FUEL;
    p.order_value = (packetAV_downlink.engine_state == ACTIVE)?INACTIVE:ACTIVE; // TODO update !!!!!!!!!!!!!!!!!!
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->AV_servo_fuel, UNKNOWN);
}

void NordendGUI::on_AV_vent_N2O_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_VENT_N2O;
    p.order_value = (packetAV_downlink.engine_state == ACTIVE)?INACTIVE:ACTIVE; // TODO update !!!!!!!!!!!!!!!!!!
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->AV_vent_N2O, UNKNOWN);
}

void NordendGUI::on_AV_vent_fuel_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_VENT_FUEL;
    p.order_value = (packetAV_downlink.engine_state == ACTIVE)?INACTIVE:ACTIVE; // TODO update !!!!!!!!!!!!!!!!!!
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->AV_vent_fuel, UNKNOWN);
}

void NordendGUI::on_AV_pressurization_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::AV_CMD_PRESSURIZE;
    p.order_value = (packetAV_downlink.engine_state == ACTIVE)?INACTIVE:ACTIVE; // TODO update !!!!!!!!!!!!!!!!!!
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->AV_pressurization, UNKNOWN);
}


void NordendGUI::on_GSE_fill_pressed() {
    av_uplink_t p;
    p.order_id = CMD_ID::GSE_FILLING_N2O;
    p.order_value = (packetGSE_downlink.status.fillingN2O == ACTIVE)?INACTIVE:ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->GSE_fill, UNKNOWN);
}

void NordendGUI::on_GSE_vent_pressed() {
    av_uplink_t p;
    p.order_value = (packetGSE_downlink.status.vent == ACTIVE)?INACTIVE:ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(ui->GSE_vent, UNKNOWN);
}

/////////////////////////////////////////////////////

void NordendGUI::on_reset_valves_pressed() {
//    ui->vent_GSE->setCheckState(Qt::CheckState::Unchecked);
//    ui->fill_GSE->setCheckState(Qt::CheckState::Unchecked);
    set_valve_img(ui->GSE_fill, UNKNOWN);
    set_valve_img(ui->GSE_vent, UNKNOWN);
    set_valve_img(ui->AV_vent_N2O, UNKNOWN);
    set_valve_img(ui->AV_vent_fuel, UNKNOWN);
    set_valve_img(ui->AV_servo_N2O, UNKNOWN);
    set_valve_img(ui->AV_servo_fuel, UNKNOWN);
    set_valve_img(ui->AV_pressurization, UNKNOWN);
    ui->prop_diagram->setStyleSheet("QPushButton{background: transparent;qproperty-icon: url(:/assets/Prop_background_V1.png);qproperty-iconSize: 700px;}");
}

void NordendGUI::set_valve_img(QPushButton * valve, int i) {
    QString img_name = "";
    switch (i) {
        case INACTIVE: img_name = "CloseH";
        break;
        case ACTIVE: img_name = "OpenV";
        break;
        default: img_name = "Unknown";
        break;
    }
    valve->setStyleSheet("QPushButton {\n"
                         "\tbackground-color: transparent;\n"
                         "\tqproperty-icon: url(:/assets/GS_valve_V2_" + img_name + ".svg);\n"
                         "   qproperty-iconSize: 50px;\n"
                         "}\n"
                         "QPushButton:hover {\n"
                         "\tbackground-color: rgba(0, 0, 0, 50); \n"
                         "}");
}

void NordendGUI::qtimer_callback() {
    // Time since last received packet
    time_t t = difftime(std::time(nullptr), lastRxTime);
    struct tm *tt = gmtime(&t);
    char buf[32];
    std::strftime(buf, 32, "%T", tt);
    ui->time_since_last_Rx->setText(buf);
}

//////////////////////////////////////////////
// Serial stuff

void NordendGUI::sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size) {
    uint8_t *packetToSend = capsule.encode(packetId, packet, size);
    serial->write((char *) packetToSend,capsule.getCodedLen(size));
    delete[] packetToSend;
}


void NordendGUI::on_open_serial_pressed() {
    int ctr = 0;
    QString serial_port_name = "";
    if (!serial->isOpen()) {
        do {
            serial_port_name = "ttyACM" + QString::number(ctr++);
            serial->setPortName(serial_port_name);
        } while (!serial->open(QIODevice::ReadWrite) && ctr <= 50);
        if (!serial->isOpen()) { // opening on WSL => ttyS
            ctr = 0;
            do {
                serial_port_name = "ttyS" + QString::number(ctr++);
                serial->setPortName(serial_port_name);
            } while (!serial->open(QIODevice::ReadWrite) && ctr <= 50);
        }
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

void NordendGUI::on_close_serial_pressed() {
    if (serial->isOpen()) {
        serial->close();
        std::cout << "Serial port closed" << std::endl;
    } else {
        std::cout << "Serial port already closed" << std::endl;
    }
    ui->serial_port_detected_name->setText("-");
    ui->serial_port_detected_name->setStyleSheet(""); // no color
    ui->serialport_status->setStyleSheet("QLabel {image: url(:/assets/refresh.png);}");
}

void NordendGUI::serialError() {
    //std::cout << "Serial port interrupt error" << std::endl;
    ui->serialport_status->setStyleSheet(
            "QLabel {image: url(:/assets/redCross.png);}");
    if (serial->isOpen()) serial->close();
    ui->serial_port_detected_name->setText("-");
    ui->serial_port_detected_name->setStyleSheet(""); // no color
}






