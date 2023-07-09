/*!
 * \file XstratoWindow.cpp
 *
 * \brief XstratoWindow module implementation
 *
 * \author      ISOZ Lionel - EPFL EL BA3
 * \date        13.04.2023	
 */

#include "XstratoWindow.h"
#include <ui_Xstrato.h>
#include <iostream>
#include <QTimer>
#include <QGraphicsColorizeEffect>
#include <cmath>

// RF protocol XSTRATO
#include "../XRF_interface/PacketDefinition.h"

#define REFRESH_PERIOD_ms       1000   // refresh every second
#define TIME_TO_SEND_CMD_ms     1000   // refresh every second

#define PACKET_RATE_MAX_UI      30  // slider

#define RX_IMAGES_PATH          "../ImagesRx/ImageRx.jpg"


XstratoWindow::XstratoWindow(int *myvar) :
        myvar(myvar),
        ui(new Ui::xstrato_ui),
        serial(new QSerialPort(this)),
        capsule(&XstratoWindow::handleSerialRxPacket, this),
        ctr(0),
        filename(RX_IMAGES_PATH),
        qtimer(new QTimer(this)),
        qtimer_rximg(new QTimer(this)),
        lastRxTime(std::time(nullptr)),
        packet_ctr(0)
        {

    ui->setupUi(this);

    qtimer->start(REFRESH_PERIOD_ms);

    connect(serial, &QSerialPort::readyRead, this, &XstratoWindow::readSerialData);
    connect(serial, &QSerialPort::errorOccurred, this, &XstratoWindow::serialError);

    connect(qtimer, SIGNAL(timeout()), this, SLOT(qtimer_callback()));
    connect(qtimer_rximg, SIGNAL(timeout()), this, SLOT(qtimer_rximg_callback()));

    // LoRa RF param
//    connect(ui->LoRa_SF_slider, SIGNAL(valueChanged(int)), this, SLOT(LoRa_RF_param_changed(int)));
//    connect(ui->LoRa_BW_slider, SIGNAL(valueChanged(int)), this, SLOT(LoRa_RF_param_changed(int)));
//    connect(ui->LoRa_CR_slider, SIGNAL(valueChanged(int)), this, SLOT(LoRa_RF_param_changed(int)));

    std::cout << "XstratoWindow inited" << std::endl;

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

void XstratoWindow::readSerialData() {
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

XstratoWindow::~XstratoWindow() {
    if (serial->isOpen()) serial->close();
    delete ui;
    std::cout << "XstratoWindow deleted" << std::endl;
}

void
XstratoWindow::handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
    static std::string filename_time = filename;
    static float altitude_max = 0;
    ui->send_cmd_available->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
    packet_ctr++;
    lastRxTime = std::time(nullptr); // for now
    switch (packetId) {
        case 0x00:
            std::cout << "Packet with ID 00 received : " << std::endl;
            //Serial.write(dataIn,len);
            break;
        case CAPSULE_ID::IMAGE_DATA: {
            Xstrato_img_info p{};
            if (len != Xstrato_img_info_size)
                std::cout << "Size problem !!" << std::endl;
            memcpy(&p, dataIn, Xstrato_img_info_size);
            //std::cout << "Infoimg: " << p.nbr_rx_packet << "/" << p.nbr_tot_packet << std::endl;
            ui->nbr_rx_packet->setText(QString::number(p.nbr_rx_packet));
            ui->nbr_tot_packet->setText(QString::number(p.nbr_tot_packet));
            ui->rxtx_ratio->setText(QString::number((100.0* p.nbr_rx_packet) / (p.nbr_tx_packet+1)));
            ui->file_transmission_progress_bar->setMaximum((p.nbr_tot_packet));
            ui->file_transmission_progress_bar->setValue((p.nbr_rx_packet));
            break;
        }
        case CAPSULE_ID::IMAGE_START: {
            std::cout << "IMAGE START received" << std::endl;
            filename_time = insert_time(filename);
            fileOutImg = std::ofstream(filename_time, std::ios::trunc | std::ios::binary);
            fileOutImg.write((char *) dataIn, len);
            break;
        }
        case CAPSULE_ID::IMAGE_MIDDLE:
            // put in file
            fileOutImg.write((char *) dataIn, len);
            break;
        case CAPSULE_ID::IMAGE_END: {
            fileOutImg.write((char *) dataIn, len);
            fileOutImg.close();
            std::cout << "IMAGE STOP received" << std::endl;
            ui->image_display->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            QPixmap img(QString::fromStdString(filename_time));
            ui->image_display->setPixmap(img);
            break;
        }
        case CAPSULE_ID::TELEMETRY: {
            SerialTelemetryPacket packet;
            memcpy(&packet, dataIn, SerialTelemetryPacketSize);
            std::cout << "NEW Packet, time: " << packet.telemetry.packetTime << std::endl;
            std::cout << "GNSS: alt: " << packet.telemetry.position.alt << " |lat: " << packet.telemetry.position.lat
                      << "|lon: " << packet.telemetry.position.lon << " | speed: V " << packet.telemetry.verticalSpeed << " |H " <<  packet.telemetry.horizontalSpeed << std::endl;
            std::cout << "BME press: " << packet.telemetry.barometer.pressure << " temp: " <<packet.telemetry.barometer.temperature
                      << " hum: " << packet.telemetry.barometer.humidity << std::endl;
            std::cout << "RFinfo Balloon: rssi: " << packet.telemetry.balloon.rssi << " snr: " << packet.telemetry.balloon.snr
                      << " f_err: " << packet.telemetry.balloon.frequencyError << std::endl;
            std::cout << "RFinfo GS: rssi: " << packet.ground.rssi << " snr: " << packet.ground.snr
                      << " f_err: " << packet.ground.frequencyError << std::endl;

            // RF info
            ui->rssi_balloon_bar->setValue(packet.telemetry.balloon.rssi);
            ui->rssi_balloon_value->setText(QString::number(packet.telemetry.balloon.rssi));
            ui->rssi_gs_bar->setValue(packet.ground.rssi);
            ui->rssi_gs_value->setText(QString::number(packet.ground.rssi));

            // RF panel
            ui->rssi_bal->setText(QString::number(packet.telemetry.balloon.rssi));
            ui->snr_bal->setText(QString::number(packet.telemetry.balloon.snr));
            ui->freqErr_bal->setText(QString::number(packet.telemetry.balloon.frequencyError));
            ui->rssi_gs->setText(QString::number(packet.ground.rssi));
            ui->snr_gs->setText(QString::number(packet.ground.snr));
            ui->freqErr_gs->setText(QString::number(packet.ground.frequencyError));

            // GPS data
            ui->latitude_label->setText(QString::number(packet.telemetry.position.lat));
            ui->longitude_label->setText(QString::number(packet.telemetry.position.lon));
            ui->altitude_lcd_gps->display(QString::number(packet.telemetry.position.alt));
            if (packet.telemetry.position.alt > altitude_max) altitude_max = packet.telemetry.position.alt;
            ui->altitude_max_lcd_m->display(QString::number(altitude_max));
            ui->speed_vertical->setText(QString::number(packet.telemetry.verticalSpeed));
            ui->speed_horizontal->setText(QString::number(packet.telemetry.horizontalSpeed));

            // BME data
            ui->temperature->setText(QString::number(packet.telemetry.barometer.temperature));
            ui->humidity->setText(QString::number(packet.telemetry.barometer.humidity));
            ui->pressure->setText(QString::number(packet.telemetry.barometer.pressure));
            float sea_level_pressure_hPa = ui->sea_level_pressure_edit->text().toFloat();
            float altitude = 44330.0 * (1.0 - pow(packet.telemetry.barometer.pressure / sea_level_pressure_hPa, 0.1903));
            ui->altitude_sensor->setText(QString::number(altitude));

        }
            break;
        case CAPSULE_ID::ACK: {
            std::cout << "ACK received! " << std::endl;
            ui->ping_pong_ack->setStyleSheet("image: url(:/assets/tennis.png);");
        }
            break;
        default:
            break;
    }
    // set callback for sending cmd available
    qtimer_rximg->start(TIME_TO_SEND_CMD_ms);
    qtimer_rximg->setSingleShot(true);
}

void XstratoWindow::sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size) {
    uint8_t *packetToSend = capsule.encode(packetId, packet, size);
    serial->write((char *) packetToSend,capsule.getCodedLen(size));
    delete[] packetToSend;
}

void XstratoWindow::on_close_serial_pressed() {
    if (serial->isOpen()) {
        serial->close();
        std::cout << "Serial port closed" << std::endl;
    } else {
        std::cout << "Serial port already closed" << std::endl;
    }
    ui->serialport_status->setStyleSheet(
            "QLabel {image: url(:/assets/refresh.png);}");
}

void XstratoWindow::on_send_color_cmd_pressed() {
    uint32_t color = rand() >> 8; // 0 RGB
    std::cout << "Send color" << std::endl;
    uint8_t packetData[4];
    uint32_t len = sizeof(packetData);
    memcpy(packetData, &color, len);
    sendSerialPacket(CAPSULE_ID::LED, packetData, len);
}

void XstratoWindow::serialError() {
    //std::cout << "Serial port interrupt error" << std::endl;
    ui->serialport_status->setStyleSheet(
            "QLabel {image: url(:/assets/redCross.png);}");
    if (serial->isOpen()) serial->close();
    ui->serial_port_detected_name->setText("-");
    ui->serial_port_detected_name->setStyleSheet(""); // no color
}

void XstratoWindow::on_clear_image_pressed() {
    ui->image_display->clear();
}

void XstratoWindow::qtimer_callback() {
    ui->packet_rate_bar->setMaximum(
            (packet_ctr > PACKET_RATE_MAX_UI) ? packet_ctr : PACKET_RATE_MAX_UI);
    ui->packet_rate_bar->setValue(packet_ctr);
    packet_ctr = 0;

    // Time since last received packet
    time_t t = difftime(std::time(nullptr), lastRxTime);
    struct tm *tt = gmtime(&t);
    char buf[32];
    std::strftime(buf, 32, "%T", tt);
    ui->time_since_last_Rx->setText(buf);

    // random stuff
    //ui->tank_N2O->setValue(rand() % 100);
}


void XstratoWindow::on_test_button_pressed() {
    std::string filename_time = filename;
    std::cout << "Counter " << ctr << std::endl;
    ctr = 0;
    filename_time = insert_time(filename);
    std::cout << filename_time << std::endl;
}

void XstratoWindow::on_open_serial_pressed() {
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

void XstratoWindow::on_lock_RF_param_pressed() {
    static bool locked = true;
    locked = !locked;
    ui->lock_RF_param->setStyleSheet((locked) ? "QPushButton{\n"
                                                "qproperty-icon: url(:/assets/keyOFF.png);\n"
                                                "qproperty-iconSize: 50px;\n"
                                                "border-radius: 25px;\n"
                                                "}"
                                              : "QPushButton{\n"
                                                "qproperty-icon: url(:/assets/keyON.png);\n"
                                                "qproperty-iconSize: 50px;\n"
                                                "border-radius: 25px;\n"
                                                "}");
    ui->LoRa_SF_slider->setDisabled(locked);
    ui->LoRa_BW_slider->setDisabled(locked);
    ui->LoRa_CR_slider->setDisabled(locked);
}

void XstratoWindow::LoRa_RF_param_changed(int value) {
    std::cout << "miaou: " << value << std::endl;
    //ui->LoRa_SF_slider->value()
}

void XstratoWindow::on_set_RF_param_pressed() {
    RFsettingsPacket packet = {};
    packet.SF = ui->LoRa_SF_slider->value();
    packet.BW = getBandwidthHz(ui->LoRa_BW_slider->value());
    packet.CR = ui->LoRa_CR_slider->value();
    std::cout << "SF: " << +packet.SF << " BW: " << +packet.BW << " CR: "
              << +packet.CR << " Psize: " << RFsettingsPacket_size << std::endl;

    sendSerialPacket(CAPSULE_ID::RF_PARAM, (uint8_t *) &packet, RFsettingsPacket_size);
}

uint32_t XstratoWindow::getBandwidthHz(int index) {
    switch (index) {
        case 1:
            return 7.8E3;
        case 2:
            return 125E3;
        case 3:
            return 250E3;
        case 4:
            return 500E3;
        default:
            return 125E3;
    }
}

void XstratoWindow::on_set_CAM_param_pressed() {
    CameraSettingsPacket packet = {};
    packet.framesize = (uint8_t) ui->cam_framsize_selector->currentIndex();
    packet.quality = ui->CAM_png_quality_slider->value();
    packet.whiteBalanceEnable = ui->whiteBalanceEnable->isChecked();
    packet.awbGainEnable = ui->awbGainEnable->isChecked();
    packet.wbMode = ui->CAM_wbmode_slider->value();
    packet.exposureEnable = ui->exposureEnable->isChecked();
    packet.exposureValue = ui->CAM_exposure_slider->value();
    packet.aec2Enable = ui->aec2Enable->isChecked();
    packet.rawGmaEnable = ui->rawGmaEnable->isChecked();

    sendSerialPacket(CAPSULE_ID::CAM_PARAM, (uint8_t *) &packet, CameraSettingsPacketSize);
}


std::string XstratoWindow::insert_time(std::string s) {
    std::time_t now = std::time(0);
    std::tm *now_tm = std::gmtime(&now);
    char buf[50];
    std::strftime(buf, 50, "_%Y-%m-%d_%H-%M-%S", now_tm);
    return s.replace(0, s.rfind('.'), s.substr(0, s.rfind('.')) + buf);
}

void XstratoWindow::on_send_transmission_settings_pressed() {
    TransmissionSettingsPacket packet = {};
    packet.transmissionEnable = ui->tx_image_checkbox->isChecked();
    packet.marginRate = ui->margin_rate_edit->text().toFloat();
    packet.silenceTime = ui->silence_time_edit->text().toInt();

    sendSerialPacket(CAPSULE_ID::TRANSMISSION_PARAM, (uint8_t *) &packet, TransmissionSettingsPacketSize);
}

void XstratoWindow::qtimer_rximg_callback() {
    ui->send_cmd_available->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
    ui->ping_pong_ack->setStyleSheet("");
}

void XstratoWindow::on_ping_cmd_pressed() {
    uint8_t tmp = 0; // don't care
    sendSerialPacket(CAPSULE_ID::PING, &tmp, 1);
    std::cout << "ping" << std::endl;
}
