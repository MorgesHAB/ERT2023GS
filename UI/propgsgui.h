//
// Created by marin on 17/04/2023.
//

#ifndef ERT2023GS_PROPGSGUI_H
#define ERT2023GS_PROPGSGUI_H

#include <QMainWindow>
#include <QSerialPort>
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

private:
    Ui::PropGSGUI_ui *ui;
    int32_t current_state = 0;
    float *telemetry = new float[4];
    bool myMap[6][4] = {
            {true, true,  true,  true},
            {true, true,  false, true},
            {true, true,  false, false},
            {true, false, true,  true},
            {true, false, true,  false},
            {true, false, false, false}
    };
    bool *CMDState = new bool[7]; //every cmd except abort
    bool abortIncurse = false;
    QString transparentCheckBox = "QCheckBox {"
                                  "    color: transparent;"
                                  "    background-color: transparent;"
                                  "    border: none;"
                                  "    padding: 0;"
                                  "    margin: 0;"
                                  "    width: 0;"
                                  "    height: 0;"
                                  "}"
                                  "QCheckBox::indicator {"
                                  "    color: transparent;"
                                  "    background-color: transparent;"
                                  "    border: none;"
                                  "    padding: 0;"
                                  "    margin: 0;"
                                  "    width: 0;"
                                  "    height: 0;"
                                  "}";
    QString abortSheet = "QCheckBox {"
                         "    color: #333;"
                         "    font-weight: bold;"
                         "}"
                         ""
                         "QCheckBox::indicator {"
                         "    width: 16px;"
                         "    height: 16px;"
                         "    border: 2px solid #333;"
                         "    border-radius: 2px;"
                         "}"
                         ""
                         "QCheckBox::indicator:checked {"
                         "    background-color: #333;"
                         "}"
                         ""
                         "QCheckBox::indicator:checked:disabled {"
                         "    background-color: #bbb;"
                         "    border-color: #999;"
                         "}";

    Capsule<PropGSGUI> capsule;
    QSerialPort *serial;








    /*PacketAV packet;
    packet.valveN2O*/



private slots:

    void on_checkBox_stateChanged(int arg1);

    void on_checkBox_2_stateChanged(int arg1);

    void on_checkBox_3_stateChanged(int arg1);

    void on_checkBox_5_stateChanged(int arg1);

    void initLight();

    void setAvionics(int nb_button);

    void checkAvionics();


    void CMDHandler(int valveNb);

    void telemetry_handler(PacketAV_downlink *packet);

    void on_checkBox_4_stateChanged(int arg1);

    void on_pushButton_pressed();

    void sendSerialPacket(uint8_t packetId, uint8_t *packet, uint32_t size);
};


#endif //ERT2023GS_PROPGSGUI_H
