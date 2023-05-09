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

// RF protocol XSTRATO
#include "../XRF_interface/PacketDefinition.h"

#define REFRESH_PERIOD_ms       1000   // refresh every second

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
        lastRxTime(std::time(nullptr)),
        packet_ctr(0)
        {

    ui->setupUi(this);

    qtimer->start(REFRESH_PERIOD_ms);

    connect(serial, &QSerialPort::readyRead, this, &XstratoWindow::readSerialData);
    connect(serial, &QSerialPort::errorOccurred, this, &XstratoWindow::serialError);

    connect(qtimer, SIGNAL(timeout()), this, SLOT(qtimer_callback()));

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
            PositionPacket packet{};
            memcpy(&packet, dataIn, Xstrato_img_info_size);
            std::cout << "GNSS: alt: " << packet.alt << " |lat: " << packet.lat
                      << "|lon: " << packet.lon << std::endl;
        }
            break;
        default:
            break;
    }
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
    packet.whiteBalanceEnable = ui->whiteBalanceEnable->isEnabled();
    packet.awbGainEnable = ui->awbGainEnable->isEnabled();
    packet.wbMode = ui->CAM_wbmode_slider->value();
    packet.exposureEnable = ui->exposureEnable->isEnabled();
    packet.exposureValue = ui->CAM_exposure_slider->value();
    packet.aec2Enable = ui->aec2Enable->isEnabled();
    packet.rawGmaEnable = ui->rawGmaEnable->isEnabled();

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
    packet.transmissionEnable = ui->tx_image_checkbox->isEnabled();
    packet.marginRate = ui->margin_rate_edit->text().toFloat();
    packet.silenceTime = ui->silence_time_edit->text().toInt();

    sendSerialPacket(CAPSULE_ID::TRANSMISSION_PARAM, (uint8_t *) &packet, TransmissionSettingsPacketSize);
}
