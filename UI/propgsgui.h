//
// Created by marin on 17/04/2023.
//

#ifndef ERT2023GS_PROPGSGUI_H
#define ERT2023GS_PROPGSGUI_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"
#include "../Capsule/src/capsule.h"
#include "stopwatch.h"


QT_BEGIN_NAMESPACE
namespace Ui { class PropGSGUI_ui; }
QT_END_NAMESPACE

class PropGSGUI : public QMainWindow {
Q_OBJECT

    void on_pushButton_clicked();


public:
    explicit PropGSGUI(QWidget *parent = nullptr);
    void handleSerialRxPacket(uint8_t packetId, uint8_t *dataIn, uint32_t len);
    ~PropGSGUI() override;

    void init_valves();

private:
    //Stopwatch *watch;



    Ui::PropGSGUI_ui *ui;

    Capsule<PropGSGUI> capsule;
    QSerialPort *serial;
    bool aborting = false;
    bool launching = false;
    QTimer *abortTimer = new QTimer(this);
    double timeRemaining = 0;
    bool codeArray[4]={false,false,false,false};



private slots:
    void enable_abort(bool b);
    void enable_lauch(bool b);
    void on_bit0_c_stateChanged(int arg1);
    void on_bit1_c_stateChanged(int arg1);
    void on_bit2_c_stateChanged(int arg1);
    void on_bit3_c_stateChanged(int arg1);

    void on_ConfirmAbort_p_clicked();
    void on_confirmLaunch_p_clicked();
    void on_abort_p_clicked();
    void on_Launch_p_clicked();

    void sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size);
    void CMDHandler(int valveNb);

    //void telemetry_handler(PacketAV_downlink *packet);

    //void update();

    //void resetTimer();

    //void startStopTimer();
};


#endif //ERT2023GS_PROPGSGUI_H
