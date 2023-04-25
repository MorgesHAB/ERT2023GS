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

#define IMAGE_START     0x42
#define IMAGE_MIDDLE    0x53
#define IMAGE_END       0x64

#define REFRESH_PERIOD_ms       1000   // refresh every second

#define PACKET_RATE_MAX_UI      30


XstratoWindow::XstratoWindow(int* myvar) :
        myvar(myvar),
        ui(new Ui::xstrato_ui),
        serial(new QSerialPort(this)),
        capsule(&XstratoWindow::handleSerialRxPacket, this),
        ctr(0),
        filename("output.jpg"),
        qtimer(new QTimer(this)),
        lastRxTime(std::time(nullptr)),
        packet_ctr(0)
        {

    ui->setupUi(this);

    qtimer->start(REFRESH_PERIOD_ms);

    connect(serial, &QSerialPort::readyRead, this, &XstratoWindow::readSerialData);
    connect(serial, &QSerialPort::errorOccurred, this, &XstratoWindow::serialError);

    connect(qtimer, SIGNAL(timeout()), this, SLOT(qtimer_callback()));

    std::cout << "XstratoWindow inited" << std::endl;

    int i =0;
    /*while(true) {
        std::cout << "thread running val: " << i++ << std::endl;
        //std::this_thread::sleep_for(std::chrono::milliseconds (1))
    }*/

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
    for (auto && item : data) {
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

void XstratoWindow::handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
    packet_ctr++;
    lastRxTime = std::time(nullptr); // for now
    switch (packetId) {
        case 0x00:
            std::cout << "Packet with ID 00 received : " << std::endl;
            //Serial.write(dataIn,len);
            break;
        case 0x01:
            std::cout << "Packet with ID 01 received : " << std::endl;
            //Serial.write(dataIn,len);
            break;
        case 0x12: {
            Xstrato_img_info p{};
            if (len != 4) std::cout << "Size problem !!" << std::endl;
            memcpy(&p, dataIn, 4);
            //std::cout << "Infoimg: " << p.nbr_rx_packet << "/" << p.nbr_tot_packet << std::endl;
            ui->nbr_rx_packet->setText(QString::number(p.nbr_rx_packet));
            ui->nbr_tot_packet->setText(QString::number(p.nbr_tot_packet));
            ui->file_transmission_progress_bar->setMaximum((p.nbr_tot_packet));
            ui->file_transmission_progress_bar->setValue((p.nbr_rx_packet));
            break;
        }
        case IMAGE_START:
            std::cout << "IMAGE START received" << std::endl;
            fileOutImg = std::ofstream(filename, std::ios::trunc | std::ios::binary);
            fileOutImg.write((char*) dataIn, len);
            /*for (int i(0); i < len; i++) {
                std::cout << +dataIn[i];
            }*/
            break;
        case IMAGE_MIDDLE:
            // put in file
            fileOutImg.write((char*) dataIn, len);
            break;
        case IMAGE_END: {
            fileOutImg.write((char*) dataIn, len);
            fileOutImg.close();
            std::cout << "IMAGE STOP received" << std::endl;
            ui->image_display->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            QPixmap img(QString::fromStdString(filename));
            ui->image_display->setPixmap(img);
            break;
        }
        default:
            break;
    }
}

void XstratoWindow::sendRandomPacket() {
    uint8_t packetData[4];
    uint8_t packetId = 0x01;
    uint32_t len = 4;

    for (int i = 0; i < 4; i++) {
        packetData[i] = i;
    }

    uint8_t* packetToSend = capsule.encode(packetId,packetData,len);
    serial->write((char*) packetToSend, capsule.getCodedLen(len));
    delete[] packetToSend;
}

void XstratoWindow::on_close_serial_pressed() {
    if (serial->isOpen()) {
        serial->close();
        std::cout << "Serial port closed" << std::endl;
    } else {
        std::cout << "Serial port already closed" << std::endl;
    }
    ui->serialport_status->setStyleSheet("QLabel {image: url(:/assets/refresh.png);}");
}

void XstratoWindow::on_send_color_cmd_pressed() {
    uint32_t color = rand() >> 8; // 0 RGB
    std::cout << "Send color" << std::endl;
    uint8_t packetData[4];
    uint8_t packetId = 0x99;
    uint32_t len = sizeof(packetData);
    memcpy(packetData, &color, len);
    uint8_t* packetToSend = capsule.encode(packetId,packetData,len);
    serial->write((char*) packetToSend, capsule.getCodedLen(len));
    delete[] packetToSend;
}

void XstratoWindow::serialError() {
    //std::cout << "Serial port interrupt error" << std::endl;
    ui->serialport_status->setStyleSheet("QLabel {image: url(:/assets/redCross.png);}");
    if (serial->isOpen()) serial->close();
    ui->serial_port_detected_name->setText("-");
    ui->serial_port_detected_name->setStyleSheet(""); // no color
}

void XstratoWindow::on_clear_image_pressed() {
    ui->image_display->clear();
}

void XstratoWindow::qtimer_callback() {
    ui->packet_rate_bar->setMaximum((packet_ctr > PACKET_RATE_MAX_UI) ? packet_ctr : PACKET_RATE_MAX_UI);
    ui->packet_rate_bar->setValue(packet_ctr);
    packet_ctr = 0;

    // Time since last received packet
    time_t t = difftime(std::time(nullptr), lastRxTime);
    struct tm* tt = gmtime(&t);
    char buf[32];
    std::strftime(buf, 32, "%T", tt);
    ui->time_since_last_Rx->setText(buf);

    // random stuff
    //ui->tank_N2O->setValue(rand() % 100);
}


void XstratoWindow::on_test_button_pressed() {
    /*serial->close();
    if (serial->open(QIODevice::ReadWrite)) {
        std::cout << "Serial port open" << std::endl;
    } else {
        std::cout << "Serial port error" << std::endl;
    }*/
    std::cout << "Counter " << ctr << std::endl;
    ctr = 0;
    /*
    *myvar = (*myvar)+1;
    std::cout << "button clicked! val: " << *myvar << std::endl;

    if(serial->isOpen()) {
        serial->write("Miaou: ");
        sendRandomPacket();
        //serial->waitForBytesWritten();
    }
    else
        std::cout << "Write error" << std::endl;
        */
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
            ui->serialport_status->setStyleSheet("QLabel {image: url(:/assets/correct.png);}");
            ui->serial_port_detected_name->setText(serial_port_name);
            ui->serial_port_detected_name->textFormat();
            ui->serial_port_detected_name->setStyleSheet("color : green;");
        } else {
            std::cout << "Impossible to find valid serial port" << std::endl;
            ui->serialport_status->setStyleSheet("QLabel {image: url(:/assets/redCross.png);}");
            ui->serial_port_detected_name->setText("None!");
            ui->serial_port_detected_name->setStyleSheet("color : red;");
        }
    }
}
