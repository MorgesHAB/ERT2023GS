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
}

void PropGSGUI::init_valves(){

    ui->VentInC_v->setCheckState(Qt::Checked);
    ui->VentInF_v->setCheckState(Qt::Checked);
    ui->VentOut_v->setCheckState(Qt::Checked);



}
void PropGSGUI::sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size) {
    uint8_t *packetToSend = capsule.encode(packetId, packet, size);
    serial->write((char *) packetToSend,capsule.getCodedLen(size));
    delete[] packetToSend;
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
void PropGSGUI::CMDHandler(int valveNb) {

    PacketAV_uplink packet = {};
    //packet
    switch (valveNb) {
        case 1:
            break;
    }
}

void PropGSGUI::on_abort_p_clicked() {
    if (!aborting){
        ui->abort_p->setText("CANCEL");
        aborting = true;
        ui->ConfirmAbort_p->setEnabled(true);
        ui->confirmLaunch_p->setStyleSheet("background-color: rgb(255, 0, 0);");
        ui->abortTimer_n->setEnabled(true);
        ui->abortTimer_n->display(5.000);
        abortTimer.start(5000);
    }else{
        ui->abort_p->setText("ABORT?");
        aborting = false;
        ui->ConfirmAbort_p->setEnabled(true);
        ui->confirmLaunch_p->setStyleSheet("background-color: rgb(255, 0, 0);");
    }

}

void PropGSGUI::update_abortTimer_n() {
}



PropGSGUI::~PropGSGUI() {

    delete ui;
}




