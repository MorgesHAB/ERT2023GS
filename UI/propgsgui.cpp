//
// Created by marin on 17/04/2023.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PropGSGUI.h" resolved

#include "propgsgui.h"
#include "ui_PropGSGUI.h"
#include <QDebug>
#include <iostream>
#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"


PropGSGUI::PropGSGUI(QWidget *parent) :


        QMainWindow(parent), ui(new Ui::PropGSGUI_ui),serial(new QSerialPort(this)),
        capsule(&PropGSGUI::handleSerialRxPacket, this) {


    ui->setupUi(this);
    qDebug() << ui->progressBar->orientation();
    ui->progressBar->setOrientation(Qt::Orientation::Vertical);
    //ui->progressBar->setGeometry(200, 150, 40, 200);
    qDebug() << ui->progressBar->orientation();
    initLight();


}

void PropGSGUI::telemetry_handler(PacketAV_downlink *packet) {
    current_state = packet->av_state;
    telemetry[0] = packet->pressionTankEthanol;
    telemetry[1] = packet->pressionTankO2;
    telemetry[2] = packet->pressionTankN2O;
}


void PropGSGUI::CMDHandler(int valveNb) {

    PacketAV_uplink packet = {};
    //packet
    switch (valveNb) {
        case 1:
            break;
    }

    /*uint8_t *packetToSend = capsule.encode(CAPSULE_ID::RF_cmd, (uint8_t *) &packet, packetAV_uplink_size);
    serial->write((char *) packetToSend, capsule.getCodedLen(packetAV_uplink_size));
    delete[] packetToSend;*/


}


void PropGSGUI::checkAvionics() {
    /*packetAVDownlink = CMD_ACTIVE;
       serialw*/
    ui->progressBar->setValue(telemetry[0]);
    ui->progressBar_2->setValue(telemetry[1]);
    ui->progressBar_3->setValue(telemetry[2]);
}
void PropGSGUI::handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
    switch (packetId) {
        case 0x00:
            std::cout << "Packet with ID 00 received : " << std::endl;
//Serial.write(dataIn,len);
            break;
        case 0x01:
            std::cout << "Packet with ID 01 received : " << std::endl;
            break;
        default:
            break;
    }
}

void PropGSGUI::setAvionics(int nb_button) {
    // current state search

    bool tempBool = myMap[current_state][nb_button];
    switch (nb_button) {
        case 1:
            if ((ui->checkBox->checkState() == Qt::Checked) == tempBool) {
                ui->label->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            } else {
                ui->label->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }

            break;
        case 2:
            if ((ui->checkBox_2->checkState() == Qt::Checked) == tempBool) {
                ui->label_2->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            } else {
                ui->label_2->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }
            break;
        case 3:
            if ((ui->checkBox_3->checkState() == Qt::Checked) == tempBool) {
                ui->label_3->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            } else {
                ui->label_3->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }

            break;
        case 4:
            if ((ui->checkBox_4->checkState() == Qt::Checked) == tempBool) {
                ui->label_4->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            } else {
                ui->label_4->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }


            break;
    }

}

void PropGSGUI::initLight() {
    if ((ui->checkBox->checkState() == Qt::Checked) == myMap[0][1]) {
        ui->label->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
    } else {
        if (ui->checkBox->checkState() == Qt::Checked) {
            ui->checkBox->setCheckState(Qt::Unchecked);
        } else {
            ui->checkBox->setCheckState(Qt::Checked);
        }
    }


    if ((ui->checkBox_2->checkState() == Qt::Checked) == myMap[0][2]) {
        ui->label_2->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
    } else {
        if (ui->checkBox_2->checkState() == Qt::Checked) {
            ui->checkBox_2->setCheckState(Qt::Unchecked);
        } else {
            ui->checkBox_2->setCheckState(Qt::Checked);
        }
    }

    if ((ui->checkBox_3->checkState() == Qt::Checked) == myMap[0][3]) {
        ui->label_3->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
    } else {
        if (ui->checkBox_3->checkState() == Qt::Checked) {
            ui->checkBox_3->setCheckState(Qt::Unchecked);
        } else {
            ui->checkBox_3->setCheckState(Qt::Checked);
        }
    }


    if ((ui->checkBox_4->checkState() == Qt::Checked) == myMap[0][4]) {
        ui->label_4->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
    } else {
        if (ui->checkBox_4->checkState() == Qt::Checked) {
            ui->checkBox_4->setCheckState(Qt::Checked);
        } else {
            ui->checkBox_4->setCheckState(Qt::Unchecked);
        }
    }
}

void PropGSGUI::on_checkBox_stateChanged(int arg1) {

    qDebug() << ui->checkBox->checkState();
    setAvionics(1);
    /*packetAVuplink.cmd.ventN2O = CMD_ACTIVE;
    serialw*/

    //ui->label->setPixmap(redIcon);

}

void PropGSGUI::on_checkBox_2_stateChanged(int arg1) {
    qDebug() << ui->checkBox_2->checkState();
    setAvionics(2);
}

void PropGSGUI::on_checkBox_3_stateChanged(int arg1) {
    qDebug() << ui->checkBox_3->checkState();
    setAvionics(3);
}

void PropGSGUI::on_checkBox_4_stateChanged(int arg1) {
    qDebug() << ui->checkBox_4->checkState();
    setAvionics(4);
}

void PropGSGUI::on_pushButton_pressed() {

    if (!abortIncurse) {
        ui->pushButton->setText("Cancel");
        ui->checkBox_5->setCheckable(true);
        ui->checkBox_5->setStyleSheet(abortSheet);
        abortIncurse = true;
    } else {
        ui->pushButton->setText("Abort!");
        ui->checkBox_5->setCheckable(false);
        ui->checkBox_5->setStyleSheet(transparentCheckBox);
        abortIncurse = false;
    }
}

void PropGSGUI::on_checkBox_5_stateChanged(int arg1) {
    PacketAV_uplink packet = {};
    qDebug() << "ABORT";
    packet.cmd.abort = CMD_ACTIVE;
    sendSerialPacket(0x01, (uint8_t *) &packet, packetAV_uplink_size);


}

void PropGSGUI::sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size) {
    uint8_t *packetToSend = capsule.encode(packetId, packet, size);
    serial->write((char *) packetToSend,capsule.getCodedLen(size));
    delete[] packetToSend;
}

PropGSGUI::~PropGSGUI() {

    delete ui;
}




