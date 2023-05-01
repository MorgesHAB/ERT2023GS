//
// Created by marin on 17/04/2023.
//

#ifndef ERT2023GS_PROPGSGUI_H
#define ERT2023GS_PROPGSGUI_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class PropGSGUI_ui; }
QT_END_NAMESPACE

class PropGSGUI : public QMainWindow {
    Q_OBJECT


public:
    explicit PropGSGUI(QWidget *parent = nullptr);

    ~PropGSGUI() override;

private:
    Ui::PropGSGUI_ui *ui;
    bool myMap[6][4] = {
            {true, true, true, true},
            {true, true, false, true},
            {true, true, false, false},
            {true, false, true, true},
            {true, false, true, false},
            {true, false, false, false}
    };





    /*PacketAV packet;
    packet.valveN2O*/



private slots:
    void on_checkBox_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);
    void on_checkBox_3_stateChanged(int arg1);
    void initLight();

    void setAvionics(int nb_button);
    void checkAvionics();

};


#endif //ERT2023GS_PROPGSGUI_H
