/*!
 *
 * \file Nordend.cpp
 *
 * \brief Nordend Software GS
 *
 *  The goal of this program is to ...
 *
 *
 * \author      ISOZ Lionel - EPFL EL MA4
 * \date        11.07.2023
 */

#include <iostream>
#include <NordendGUI.h>
#include <QApplication>

//#include "../Capsule/src/capsule.h"
#include <capsule.h>


int main(int argc, char** argv) {

    QApplication app(argc, argv); // QWidget: Must construct a QApplication before a QWidget

    NordendGUI nordendGui;

//    nordendGui.setWindowState(Qt::WindowMaximized);

    nordendGui.show();

    app.exec();

    std::cout << "program running" << std::endl;

    return 0;
}