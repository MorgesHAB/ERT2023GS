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
#include <QMessageBox>

#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"

#define REFRESH_PERIOD_ms       1000   // refresh every second
#define TIME_TO_SEND_CMD_ms     1000   // refresh every second

#define PACKET_RATE_MAX_UI      30  // slider

#define UNKNOWN                 0  // valve state yellow

#define FORMAT                  "background: transparent; font: 9pt \"Verdana\";"

NordendGUI::NordendGUI() :
        ui(new Ui::nordend),
        serial(new QSerialPort(this)),
        capsule(&NordendGUI::handleSerialRxPacket, this),
        qtimer(new QTimer(this)),
        lastRxTime_AV(std::time(nullptr)),
        lastRxTime_GSE(std::time(nullptr)),
        tare_val(0.0), gain_val(1.0)
        {

//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
//    QCoreApplication::setAttribute(Qt::AA_Use96Dpi);

    ui->setupUi(this);
    last_state = ui->st_init;

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
    static int altitude_max_r = 0;

    // std::cout << "Packet received, ID: " << +packetId << " len: "  << len << std::endl;
    switch (packetId) {
        case 0x00:
            std::cout << "Packet with ID 00 received : " << +packetId << std::endl;
            break;
        case CAPSULE_ID::AV_TELEMETRY: {
            lastRxTime_AV = std::time(nullptr); // for now
            std::cout << "Packet AV_TELEMETRY received " << packetAV_downlink.packet_nbr << " time: " << packetAV_downlink.timestamp << std::endl;
            memcpy(&packetAV_downlink, dataIn, av_downlink_size);

            static uint32_t t = 0;
            if (packetAV_downlink.timestamp < t) ui->AV_timestamp->setStyleSheet("color: red;");
            t = packetAV_downlink.timestamp;
            ui->AV_timestamp->setText(QString::number(packetAV_downlink.timestamp));

            // sensor debug
            static bool first_time = true;
            if (packetAV_downlink.N2O_pressure < 0 && first_time) {
                time_t t = std::time(nullptr);
                struct tm *tt = gmtime(&t);
                char buf[32];
                std::strftime(buf, 32, "%T", tt);
                ui->sensor_crash_time->setText(buf);
            }

            // Set the valves states
            set_valve_img(ui->AV_servo_N2O, packetAV_downlink.engine_state.servo_N2O+10);
            set_valve_img(ui->AV_servo_fuel, packetAV_downlink.engine_state.servo_fuel+10);
            set_valve_img(ui->AV_vent_N2O, packetAV_downlink.engine_state.vent_N2O+10, true, true);
            set_valve_img(ui->AV_vent_fuel, packetAV_downlink.engine_state.vent_fuel+10, true, true);
            set_valve_img(ui->AV_pressurization, packetAV_downlink.engine_state.pressurize+10, false, true);

            // Set telemetry data box
            ui->N2O_pressure->setText(QString::number(packetAV_downlink.N2O_pressure, (char)103, 4) + " bar");
            ui->N2O_pressure->setStyleSheet(((packetAV_downlink.N2O_pressure < 0)?QString(FORMAT)+"color: red;":QString(FORMAT)+"color:white"));
            ui->N2O_temp->setText(QString::number(packetAV_downlink.tank_temp, (char)103, 4) + " °C");
            ui->fuel_pressure->setText(QString::number(packetAV_downlink.fuel_pressure, (char)103, 4) + " bar");
            ui->fuel_pressure->setStyleSheet(((packetAV_downlink.fuel_pressure < 0)?QString(FORMAT)+"color: red;":QString(FORMAT)+"color:white"));
            ui->chamber_pressure->setText(QString::number(packetAV_downlink.chamber_pressure, (char)103, 4) + " bar");
            ui->chamber_pressure->setStyleSheet(((packetAV_downlink.chamber_pressure < 0)?QString(FORMAT)+"color: red;":QString(FORMAT)+"color:white"));

            ui->AV_temp->setText(QString::number(packetAV_downlink.baro_temp));
//            ui->AV_humidity->setText(QString::number(packetAV_downlink.humidity));
            ui->AV_pressure->setText(QString::number(packetAV_downlink.baro_press));
            float sea_level_pressure_hPa = ui->sea_level_pressure_edit->text().toFloat();
            float altitude = 44330.0 * (1.0 - pow(packetAV_downlink.baro_press / sea_level_pressure_hPa, 0.1903));
            ui->AV_altitude_baro->setText(QString::number(altitude));

            // GPS data
            ui->AV_latitude->setText(QString::number(packetAV_downlink.gnss_lat));
            ui->AV_longitude->setText(QString::number(packetAV_downlink.gnss_lon));
            ui->altitude_lcd_gps->display(QString::number((int) packetAV_downlink.gnss_alt));
            if (packetAV_downlink.gnss_alt > altitude_max) altitude_max =packetAV_downlink.gnss_alt;
            ui->altitude_max_lcd_max->display(QString::number(altitude_max));
            // GPS reserve
            ui->AV_latitude_r->setText(QString::number(packetAV_downlink.gnss_lat_r));
            ui->AV_longitude_r->setText(QString::number(packetAV_downlink.gnss_lon_r));
            ui->altitude_lcd_gps_r->display(QString::number((int) packetAV_downlink.gnss_alt_r));
            if (packetAV_downlink.gnss_alt > altitude_max) altitude_max_r =packetAV_downlink.gnss_alt_r;
            ui->altitude_max_lcd_max_r->display(QString::number(altitude_max_r));
//            ui->speed_vertical->setText(QString::number(packet.telemetry.verticalSpeed));
//            ui->speed_horizontal->setText(QString::number(packet.telemetry.horizontalSpeed));
            update_AV_states((FLIGHTMODE) packetAV_downlink.av_state);

            //std::cout << "Servo fuel " << +packetAV_downlink.engine_state.servo_fuel << std::endl; 
            //std::cout << "Servo N2O " << +packetAV_downlink.engine_state.servo_N2O << std::endl; 
            //std::cout << "vent fuel " << +packetAV_downlink.engine_state.vent_fuel << std::endl; 
            //std::cout << "vent n2o " << +packetAV_downlink.engine_state.vent_N2O<< std::endl; 
            break;
        }
        case CAPSULE_ID::GSE_TELEMETRY: {
            lastRxTime_GSE = std::time(nullptr); // for now
            std::cout << "Packet GSE_TELEMETRY received" << std::endl;
            memcpy(&packetGSE_downlink, dataIn, packetGSE_downlink_size);
            set_valve_img(ui->GSE_fill, packetGSE_downlink.status.fillingN2O);
            set_valve_img(ui->GSE_vent, packetGSE_downlink.status.vent, true);
            if (packetGSE_downlink.disconnectActive) { // for 20sec
                ui->prop_diagram->setStyleSheet("QPushButton{background: transparent;qproperty-icon: url(:/assets/Prop_background_disconnect.png);qproperty-iconSize: 700px;}");
            } else {
                ui->prop_diagram->setStyleSheet("QPushButton{background: transparent;qproperty-icon: url(:/assets/Prop_background_V1.png);qproperty-iconSize: 700px;}");
            }
            ui->GSE_pressure->setText(QString::number(packetGSE_downlink.tankPressure, (char)103, 3) + " bar");
            ui->GSE_temp->setText(QString::number(packetGSE_downlink.tankTemperature, (char)103, 4) + " °C");
            ui->filling_pressure->setText(QString::number(packetGSE_downlink.fillingPressure, (char)103, 3) + " bar");
            ui->safe_config_label->setVisible(packetGSE_downlink.status.vent == INACTIVE && packetGSE_downlink.status.fillingN2O == INACTIVE && !packetAV_downlink.engine_state.pressurize && !packetAV_downlink.engine_state.vent_fuel && !packetAV_downlink.engine_state.vent_N2O && !packetAV_downlink.engine_state.servo_fuel && !packetAV_downlink.engine_state.servo_N2O);
            
            ui->load_cell->setText(QString::number(packetGSE_downlink.loadcellRaw));
            ui->load_cell_kg->setText(QString::number(packetGSE_downlink.loadcellRaw - tare_val));
            break;
        }
        default:
            break;
        }
}

///////////////////////////////////////////////////
// CMD button handling

void NordendGUI::on_arm_cmd_pressed() {
//    QMessageBox::StandardButton reply = QMessageBox::question(this,"ARM MODE","Arm mode confirmation request", QMessageBox::Yes | QMessageBox::No);
//    if (reply == QMessageBox::Yes) {
        av_uplink_t p;
        p.prefix = ERT_PREFIX;
        p.order_id = CMD_ID::AV_CMD_ARM;
        p.order_value = ACTIVE;
        sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
//    } else {
//        std::cout << "Arm mode rejected" << std::endl;
//    }
}

void NordendGUI::on_disarm_cmd_pressed() {
    av_uplink_t p;
    p.prefix = ERT_PREFIX;
    p.order_id = CMD_ID::AV_CMD_ARM;
    p.order_value = INACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_abort_cmd_pressed() {
    av_uplink_t p;
    p.prefix = ERT_PREFIX;
    p.order_id = CMD_ID::AV_CMD_ABORT;
    p.order_value = ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_recover_cmd_pressed() {
    av_uplink_t p;
    p.prefix = ERT_PREFIX;
    p.order_id = CMD_ID::AV_CMD_RECOVER;
    p.order_value = ACTIVE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_ignition_cmd_pressed() {
    av_uplink_t p;
    p.prefix = ERT_PREFIX;
    p.order_id = CMD_ID::AV_CMD_IGNITION;
    p.order_value = IGNITION_CODE;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
}

void NordendGUI::on_disconnect_cmd_pressed() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,"DISCONNECT","GSE Disconnect confirmation request", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        av_uplink_t p;
        p.prefix = ERT_PREFIX;
        p.order_id = CMD_ID::GSE_CMD_DISCONNECT;
        p.order_value = ACTIVE;
        sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    } else {
        std::cout << "Disconnect cmd rejected" << std::endl;
    }
}


void NordendGUI::on_tare_button_pressed() {
    tare_val = packetGSE_downlink.loadcellRaw;
}

/////////////////////////////////////////////////////

void NordendGUI::set_valve_img(QPushButton * valve, int i, bool normally_open, bool horizontal_bar) {
    QString img_name = "";
    switch (i) {
        case 10: // 0 => INACTIVE
        case INACTIVE:
            if (!normally_open) { // => normally close, classic => red
                img_name = (horizontal_bar)?"CloseV":"CloseH";
            } else {
                img_name = (horizontal_bar)?"OpenH":"OpenV";
            }
            break;
        case 11: // 1 => ACTIVE
        case ACTIVE:
            if (!normally_open) { // => normally close, classic => green
                img_name = (horizontal_bar)?"OpenH":"OpenV";
            } else { // => normally close, NO => red
                img_name = (horizontal_bar)?"CloseV":"CloseH";
            }
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
    time_t t1 = difftime(std::time(nullptr), lastRxTime_AV);
    ui->time_since_last_Rx_AV->setStyleSheet(((t1 > 3) ? "color : red;" : "color : white;"));
    struct tm *tt1 = gmtime(&t1);
    char buf1[32];
    std::strftime(buf1, 32, "%T", tt1);
    ui->time_since_last_Rx_AV->setText(buf1);

    time_t t2 = difftime(std::time(nullptr), lastRxTime_GSE);
    ui->time_since_last_Rx_GSE->setStyleSheet(((t2 > 3) ? "color : red;" : "color : white;"));
    struct tm *tt2 = gmtime(&t2);
    char buf2[32];
    std::strftime(buf2, 32, "%T", tt2);
    ui->time_since_last_Rx_GSE->setText(buf2);
}

//////////////////////////////////////////////
// Serial stuff

void NordendGUI::sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size) {
    uint8_t *packetToSend = capsule.encode(packetId, packet, size);
    serial->write((char *) packetToSend,capsule.getCodedLen(size));
    delete[] packetToSend;
}

void NordendGUI::send_cmd(uint8_t order_id, uint8_t order_value, QPushButton *button) {
    av_uplink_t p;
    p.prefix = ERT_PREFIX;
    p.order_id = order_id;
    p.order_value = order_value;
    sendSerialPacket(CAPSULE_ID::GS_CMD, (uint8_t*) &p, av_uplink_size);
    set_valve_img(button, UNKNOWN);
    std::cout << "CMD_ID: " << +order_id << " " << ((order_value==ACTIVE)?"ACTIVE":"INACTIVE") << " sent!" << std::endl;
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

void NordendGUI::set_AV_state(QLabel* st_label) {//, control_state_copy_t state) {
    st_label->setText("NOW");
    st_label->setStyleSheet("color: green; ; font: 10pt \"Segoe UI\";");
    last_state = st_label;
}

void NordendGUI::update_AV_states(FLIGHTMODE state) {
    last_state->setStyleSheet("color: green");
    last_state->setText("X");
    switch (state) {
        case INITIALIZE_MODE:
            set_AV_state(ui->st_init);
            std::cout << "INITIALIZE_MODE: Wait for arming or calibration" << std::endl;
            break;
        case CALIBRATION_MODE:
            set_AV_state(ui->st_calibration);
            std::cout << "CALIBRATION_MODE: Calibrate sensors and actuators" << std::endl;
            break;
        case MANUAL_MODE:
            set_AV_state(ui->st_manual_operation);
            std::cout << "MANUAL_MODE: Manual Servo movement" << std::endl;
            break;
        case ARMED_MODE:
            set_AV_state(ui->st_armed);
            std::cout << "ARMED_MODE: System is armed and ready to pressure" << std::endl;
            break;
        case PRESSURED_MODE:
            set_AV_state(ui->st_pressured);
            std::cout << "PRESSURED_MODE: System is pressured" << std::endl;
            break;
        case IGNITER_MODE:
            set_AV_state(ui->st_fire_igniter);
            std::cout << "IGNITER_MODE: Fire igniter" << std::endl;
            break;
        case IGNITION_MODE:
            set_AV_state(ui->st_ignition);
            std::cout << "IGNITION_MODE: Partially open valves" << std::endl;
            break;
        case THRUST_MODE:
            set_AV_state(ui->st_thrust);
            std::cout << "THRUST_MODE: Fully open valves" << std::endl;
            break;
        case SHUTDOWN_MODE:
            set_AV_state(ui->st_shutdown);
            std::cout << "SHUTDOWN_MODE: Close ethanol valve" << std::endl;
            break;
        case GLIDING_MODE:
            set_AV_state(ui->st_glide);
            std::cout << "GLIDING_MODE: Glide" << std::endl;
            break;
        case DESCENT_MODE:
            set_AV_state(ui->st_descent);
            std::cout << "DESCENT_MODE: Descent" << std::endl;
            break;
        case ABORT_MODE:
            set_AV_state(ui->st_abort);
            std::cout << "ABORT_MODE: User triggered abort" << std::endl;
            break;
        default:
            std::cout << "Unknown state" << std::endl;
    }
}


// // Function to print the state text
// void NordendGUI::update_AV_states(control_state_copy_t state) {
//     last_state->setStyleSheet("color: green");
//     last_state->setText("X");
//     switch (state) {
//         case AV_CONTROL_IDLE:
//             set_AV_state(ui->st_init);
//             std::cout << "AV_CONTROL_IDLE: Wait for arming or calibration"
//                       << std::endl;
//             break;
//         case AV_CONTROL_CALIBRATION:
//             set_AV_state(ui->st_calibration);
//             std::cout << "AV_CONTROL_CALIBRATION: Calibrate sensors and actuators"
//                       << std::endl;
//             break;
//         case AV_CONTROL_MANUAL_OPERATION:
//             set_AV_state(ui->st_manual_operation);
//             std::cout << "AV_CONTROL_MANUAL_OPERATION: Manual Servo movement"
//                       << std::endl;
//             break;
//         case AV_CONTROL_ARMED:
//             set_AV_state(ui->st_armed);
//             std::cout << "AV_CONTROL_ARMED: System is armed and ready to pressure"
//                       << std::endl;
//             break;
//         case AV_CONTROL_PRESSURED:
//             set_AV_state(ui->st_pressured);
//             std::cout << "AV_CONTROL_PRESSURED: System is pressured" << std::endl;
//             break;
//         case AV_CONTROL_IGNITER:
//             set_AV_state(ui->st_fire_igniter);
//             std::cout << "AV_CONTROL_IGNITER: Fire igniter" << std::endl;
//             break;
//         case AV_CONTROL_IGNITION:
//             set_AV_state(ui->st_ignition);
//             std::cout << "AV_CONTROL_IGNITION: Partially open valves" << std::endl;
//             break;
//         case AV_CONTROL_THRUST:
//             set_AV_state(ui->st_thrust);
//             std::cout << "AV_CONTROL_THRUST: Fully open valves" << std::endl;
//             break;
//         case AV_CONTROL_SHUTDOWN:
//             set_AV_state(ui->st_shutdown);
//             std::cout << "AV_CONTROL_SHUTDOWN: Close ethanol valve" << std::endl;
//             break;
//         case AV_CONTROL_GLIDE:
//             set_AV_state(ui->st_glide);
//             std::cout << "AV_CONTROL_GLIDE: Glide" << std::endl;
//             break;
//         case AV_CONTROL_DESCENT:
//             set_AV_state(ui->st_descent);
//             std::cout << "AV_CONTROL_DESCENT: Descent" << std::endl;
//             break;
//         case AV_CONTROL_ABORT:
//             set_AV_state(ui->st_abort);
//             std::cout << "AV_CONTROL_ABORT: User triggered abort" << std::endl;
//             break;
//         default:
//             std::cout << "Unknown state" << std::endl;
//     }
// }

void NordendGUI::on_debug_button_pressed() {
    static int ctr = 0;
    update_AV_states((FLIGHTMODE) ctr);
    ctr+=2;
    if (ctr==8) ctr=3;
    if (ctr > 13) ctr = 0;
}

////////////////////////////////////////////////////////
// Valve clicked

void NordendGUI::on_AV_servo_N2O_pressed() {
    send_cmd(CMD_ID::AV_CMD_SERVO_N2O, (packetAV_downlink.engine_state.servo_N2O)?INACTIVE:ACTIVE, ui->AV_servo_N2O);
}

void NordendGUI::on_AV_servo_fuel_pressed() {
    send_cmd(CMD_ID::AV_CMD_SERVO_FUEL, (packetAV_downlink.engine_state.servo_fuel)?INACTIVE:ACTIVE, ui->AV_servo_fuel);
}

void NordendGUI::on_AV_vent_N2O_pressed() {
    send_cmd(CMD_ID::AV_CMD_VENT_N2O, (packetAV_downlink.engine_state.vent_N2O)?INACTIVE:ACTIVE, ui->AV_vent_N2O);
}

void NordendGUI::on_AV_vent_fuel_pressed() {
    send_cmd(CMD_ID::AV_CMD_VENT_FUEL, (packetAV_downlink.engine_state.vent_fuel)?INACTIVE:ACTIVE, ui->AV_vent_fuel);
}

void NordendGUI::on_AV_pressurization_pressed() {
    send_cmd(CMD_ID::AV_CMD_PRESSURIZE, (packetAV_downlink.engine_state.pressurize)?INACTIVE:ACTIVE, ui->AV_pressurization);
}


void NordendGUI::on_GSE_fill_pressed() {
    send_cmd(CMD_ID::GSE_CMD_FILLING_N2O, (packetGSE_downlink.status.fillingN2O == ACTIVE)?INACTIVE:ACTIVE, ui->GSE_fill);
}

void NordendGUI::on_GSE_vent_pressed() {
    send_cmd(CMD_ID::GSE_CMD_VENT, (packetGSE_downlink.status.vent == ACTIVE)?INACTIVE:ACTIVE, ui->GSE_vent);
}

//////////////////////////////////////////////
// Full manual cmd

void NordendGUI::on_cmd_active_pressurization_pressed() {
    send_cmd(CMD_ID::AV_CMD_MAN_PRESSURE, ACTIVE, ui->AV_pressurization);
}

void NordendGUI::on_cmd_inactive_pressurization_pressed() {
    send_cmd(CMD_ID::AV_CMD_MAN_PRESSURE, INACTIVE, ui->AV_pressurization);
}

void NordendGUI::on_cmd_active_N2O_servo_pressed() {
    send_cmd(CMD_ID::AV_CMD_SERVO_N2O, ACTIVE, ui->AV_servo_N2O);
}

void NordendGUI::on_cmd_inactive_N2O_servo_pressed() {
    send_cmd(CMD_ID::AV_CMD_SERVO_N2O, INACTIVE, ui->AV_servo_N2O);
}

void NordendGUI::on_cmd_active_fuel_servo_pressed() {
    send_cmd(CMD_ID::AV_CMD_SERVO_FUEL, ACTIVE, ui->AV_servo_fuel);
}

void NordendGUI::on_cmd_inactive_fuel_servo_pressed() {
    send_cmd(CMD_ID::AV_CMD_SERVO_FUEL, INACTIVE, ui->AV_servo_fuel);
}

void NordendGUI::on_cmd_active_N2O_vent_pressed() {
    send_cmd(CMD_ID::AV_CMD_VENT_N2O, ACTIVE, ui->AV_vent_N2O);
}

void NordendGUI::on_cmd_inactive_N2O_vent_pressed() {
    send_cmd(CMD_ID::AV_CMD_VENT_N2O, INACTIVE, ui->AV_vent_N2O);
}

void NordendGUI::on_cmd_active_fuel_vent_pressed() {
    send_cmd(CMD_ID::AV_CMD_VENT_FUEL, ACTIVE, ui->AV_vent_fuel);
}

void NordendGUI::on_cmd_inactive_fuel_vent_pressed() {
    send_cmd(CMD_ID::AV_CMD_VENT_FUEL, INACTIVE, ui->AV_vent_fuel);
}

void NordendGUI::on_cmd_active_N2O_fill_pressed() {
    send_cmd(CMD_ID::GSE_CMD_FILLING_N2O, ACTIVE, ui->GSE_fill);
}

void NordendGUI::on_cmd_inactive_N2O_fill_pressed() {
    send_cmd(CMD_ID::GSE_CMD_FILLING_N2O, INACTIVE, ui->GSE_fill);
}

void NordendGUI::on_cmd_active_GSE_vent_pressed() {
    send_cmd(CMD_ID::GSE_CMD_VENT, ACTIVE, ui->GSE_vent);
}

void NordendGUI::on_cmd_inactive_GSE_vent_pressed() {
    send_cmd(CMD_ID::GSE_CMD_VENT, INACTIVE, ui->GSE_vent);
}


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

    ui->N2O_pressure->setText("0 bar");
    ui->N2O_temp->setText("0 °C");
    ui->chamber_pressure->setText("0 bar");
    ui->fuel_pressure->setText("0 bar");
    ui->filling_pressure->setText("0 bar");
    ui->GSE_pressure->setText("0 bar");
    ui->GSE_temp->setText("0 °C");


    // reset AV state table
    ui->st_init->setText("X");
    ui->st_init->setStyleSheet("color: red;");
    ui->st_calibration->setText("X");
    ui->st_calibration->setStyleSheet("color: red;");
    ui->st_manual_operation->setText("X");
    ui->st_manual_operation->setStyleSheet("color: red;");
    ui->st_armed->setText("X");
    ui->st_armed->setStyleSheet("color: red;");
    ui->st_pressured->setText("X");
    ui->st_pressured->setStyleSheet("color: red;");
    ui->st_fire_igniter->setText("X");
    ui->st_fire_igniter->setStyleSheet("color: red;");
    ui->st_ignition->setText("X");
    ui->st_ignition->setStyleSheet("color: red;");
    ui->st_thrust->setText("X");
    ui->st_thrust->setStyleSheet("color: red;");
    ui->st_shutdown->setText("X");
    ui->st_shutdown->setStyleSheet("color: red;");
    ui->st_glide->setText("X");
    ui->st_glide->setStyleSheet("color: red;");
    ui->st_descent->setText("X");
    ui->st_descent->setStyleSheet("color: red;");
    ui->st_abort->setText("X");
    ui->st_abort->setStyleSheet("color: red;");
}




