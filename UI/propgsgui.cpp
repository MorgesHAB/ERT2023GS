//
// Created by marin on 17/04/2023.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PropGSGUI.h" resolved

#include "propgsgui.h"
#include "ui_PropGSGUI.h"
#include <QDebug>



PropGSGUI::PropGSGUI(QWidget *parent) :




        QMainWindow(parent), ui(new Ui::PropGSGUI_ui) {



    ui->setupUi(this);
    qDebug()<<ui->progressBar->orientation();
    ui->progressBar->setOrientation(Qt::Orientation::Vertical);
    //ui->progressBar->setGeometry(200, 150, 40, 200);
    qDebug()<<ui->progressBar->orientation();
    initLight();












}

void PropGSGUI::checkAvionics(){
    /*packetAVDownlink = CMD_ACTIVE;
       serialw*/
}
void PropGSGUI::setAvionics(int nb_button){
    // current state search
    int current_state = 0;
    bool tempBool = myMap[current_state][nb_button];
    switch (nb_button) {
        case  1:
            if((ui->checkBox->checkState() == Qt::Checked) == tempBool ){
                ui->label->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                ui->label->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }

            break;
        case  2:
            if((ui->checkBox_2->checkState() == Qt::Checked) == tempBool ){
                ui->label_2->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                ui->label_2->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }
            break;
        case  3:
            if((ui->checkBox_3->checkState() == Qt::Checked) == tempBool ){
                ui->label_3->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                ui->label_3->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }

            break;
        case  4:
            if((ui->checkBox_4->checkState() == Qt::Checked) == tempBool ){
                ui->label_4->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                ui->label_4->setStyleSheet("image: url(:/assets/Red_Light_Icon.svg.png);");
            }


            break;



    }

}

void PropGSGUI::initLight(){


            if((ui->checkBox->checkState() == Qt::Checked) == myMap[0][1]){
                ui->label->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                if (ui->checkBox->checkState() == Qt::Checked ){
                    ui->checkBox->setCheckState(Qt::Unchecked);
                }else{
                    ui->checkBox->setCheckState(Qt::Checked);
                }
            }


            if((ui->checkBox_2->checkState() == Qt::Checked) == myMap[0][2] ){
                ui->label_2->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else{
                if (ui->checkBox_2->checkState() == Qt::Checked ){
                    ui->checkBox_2->setCheckState(Qt::Unchecked);
                }else {
                    ui->checkBox_2->setCheckState(Qt::Checked);
                }}

            if((ui->checkBox_3->checkState() == Qt::Checked) == myMap[0][3] ){
                ui->label_3->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                if (ui->checkBox_3->checkState() == Qt::Checked ){
                    ui->checkBox_3->setCheckState(Qt::Unchecked);}
                else {
                    ui->checkBox_3->setCheckState(Qt::Checked);
                }
            }


            if((ui->checkBox_4->checkState() == Qt::Checked) == myMap[0][4] ){
                ui->label_4->setStyleSheet("image: url(:/assets/Green_Light_Icon.svg.png);");
            }else
            {
                if(ui->checkBox_4->checkState()== Qt::Checked){
                    ui->checkBox_4->setCheckState(Qt::Checked);}
                else{
                    ui->checkBox_4->setCheckState(Qt::Unchecked);
                }
            }





}

void PropGSGUI::on_checkBox_stateChanged(int arg1)
{

        qDebug()<<ui->checkBox->checkState();
        setAvionics(1);
        /*packetAVuplink.cmd.ventN2O = CMD_ACTIVE;
        serialw*/

    //ui->label->setPixmap(redIcon);






}

void PropGSGUI::on_checkBox_2_stateChanged(int arg1)
{

    qDebug()<<ui->checkBox_2->checkState();
    setAvionics(2);


}

void PropGSGUI::on_checkBox_3_stateChanged(int arg1)
{

    qDebug()<<ui->checkBox_3->checkState();
    setAvionics(3);


}
PropGSGUI::~PropGSGUI() {

    delete ui;
}



