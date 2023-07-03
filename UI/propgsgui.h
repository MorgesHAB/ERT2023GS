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
    Ui::PropGSGUI_ui *ui;

    Capsule<PropGSGUI> capsule;
    QSerialPort *serial;
    bool aborting = false;
    bool launching = false;
    QTimer abortTimer;



private slots:
    void on_abort_p_clicked();
    void update_abortTimer_n();
    void sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size);
    void CMDHandler(int valveNb);
    //void telemetry_handler(PacketAV_downlink *packet);
};


#endif //ERT2023GS_PROPGSGUI_H
