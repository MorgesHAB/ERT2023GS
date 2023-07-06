//
// Created by marin on 17/04/2023.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PropGSGUI.h" resolved

#include "propgsgui.h"
#include "ui_PropGSGUI.h"
#include <QDebug>
#include <iostream>
#include <QThread>
#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"
#include "stopwatch.h"


PropGSGUI::PropGSGUI(QWidget *parent) :


        QMainWindow(parent), ui(new Ui::PropGSGUI_ui),serial(new QSerialPort(this)),
        capsule(&PropGSGUI::handleSerialRxPacket, this)/*, watch(new Stopwatch()), abortTimer(new QTimer(this)) */{

   /* QObject::connect(ui->abort_p, &QPushButton::clicked,
                     this, &PropGSGUI::startStopTimer);

    //signal

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);*/



    ui->setupUi(this);
    enable_abort(false);
    enable_lauch(false);
}

void PropGSGUI::init_valves(){

    ui->VentInC_v->setCheckState(Qt::Checked);
    ui->VentInF_v->setCheckState(Qt::Checked);
    ui->VentOut_v->setCheckState(Qt::Checked);

    enable_lauch(false);
    enable_abort(false);
}

void PropGSGUI::enable_lauch(bool b) {
    ui->confirmLaunch_p->setEnabled(b);
    ui->confirmLaunch_p->setVisible(b);

    ui->bit0_c->setVisible(b);
    ui->bit1_c->setVisible(b);
    ui->bit2_c->setVisible(b);
    ui->bit3_c->setVisible(b);
    ui->bit0_n->setVisible(b);
    ui->bit1_n->setVisible(b);
    ui->bit2_n->setVisible(b);
    ui->bit3_n->setVisible(b);

    ui->bit0_c->setEnabled(b);
    ui->bit1_c->setEnabled(b);
    ui->bit2_c->setEnabled(b);
    ui->bit3_c->setEnabled(b);
    ui->bit0_n->setEnabled(b);
    ui->bit1_n->setEnabled(b);
    ui->bit2_n->setEnabled(b);
    ui->bit3_n->setEnabled(b);

    launching = b;



}
void PropGSGUI::enable_abort(bool b) {
    ui->ConfirmAbort_p->setEnabled(b);
    ui->ConfirmAbort_p->setVisible(b);
    ui->abortTimer_n->setEnabled(b);
    aborting = b;

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
void PropGSGUI::on_Launch_p_clicked() {
    if (!launching){
        ui->Launch_p->setText("CANCEL");
        enable_lauch(true);
    }else{
        ui->Launch_p->setText("LAUNCH");
        enable_lauch(false);
    }
}



void PropGSGUI::on_abort_p_clicked() {
    if (!aborting){


        ui->abort_p->setText("CANCEL");
        enable_abort(true);



    }else{
        ui->abort_p->setText("ABORT?");
        enable_abort(false);
        ui->ConfirmAbort_p->setEnabled(false);
    }
}

void PropGSGUI::on_bit0_c_stateChanged(int arg1){
    codeArray[0] = !codeArray[0];
    ui->bit0_n->display(static_cast<int>(codeArray[0]));
}
void PropGSGUI::on_bit1_c_stateChanged(int arg1){
    codeArray[1] = !codeArray[1];
    ui->bit1_n->display(static_cast<int>(codeArray[1]));
}
void PropGSGUI::on_bit2_c_stateChanged(int arg1){
    codeArray[2] = !codeArray[2];
    ui->bit2_n->display(static_cast<int>(codeArray[2]));
}
void PropGSGUI::on_bit3_c_stateChanged(int arg1){
    codeArray[3] = !codeArray[3];
    ui->bit3_n->display(static_cast<int>(codeArray[3]));
}

void PropGSGUI::on_ConfirmAbort_p_clicked() {
    qDebug() << "abort-> send command";
}

void PropGSGUI::on_confirmLaunch_p_clicked() {
    qDebug() << "launch code sent is:";
    for (bool value : codeArray) {
        qDebug() << static_cast<int>(value);
    }
}

/*
void PropGSGUI::startStopTimer() {
    watch->start();
}

// Triggers when the "Reset" button is clicked
// Stops the watch, if it is running,
// and resets the "Pause"/"Restart" to "Start"
void PropGSGUI::resetTimer() {
    ui->abortTimer_n->display(0.0);
    watch->reset();
}

// Triggers every 10 milliseconds (every hundredth of a second)
// Updates the time displayed on the stopwatch.
void PropGSGUI::update()
{
    if(watch->isRunning()){
        ui->abortTimer_n->display(static_cast<double>(watch->getTime()));
    }
}*/




PropGSGUI::~PropGSGUI() {

    delete ui;
}







