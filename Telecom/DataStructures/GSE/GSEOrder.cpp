//
// Created by stephanie on 13/02/20.
//

#include "GSEOrder.h"

void GSEOrder::write(Packet &packet) {
    packet.write(order);
}

void GSEOrder::parse(Packet &packet) {
    packet.parse(order);
}

void GSEOrder::print() const {
    using namespace gse;
    switch (order) {
        case START_FILLING:
            std::cout << "Start filling order requested" << std::endl;
            break;
        case STOP_FILLING:
            std::cout << "Stop filling order requested" << std::endl;
            break;
        case PURGE_HOSE:
            std::cout << "Purge of hose requested" << std::endl;
            break;
        case DISCONNECT_HOSE:
            std::cout << "Disconnection of hose requested" << std::endl;
            break;
        case OPEN_PURGE_VALVE:
            std::cout << "Opening of purge valve requested" << std::endl;
            break;
        case SENSOR_VALUES_REQUEST:
            std::cout << "Request for sensor values" << std::endl;
            break;
        default:
            break;
    }

}

bool GSEOrder::updateTx(std::shared_ptr<Connector> connector) {
    //order = connector->getData<uint8_t>(ui_interface::GSE_ORDER_VALUE);
    order = gse::OPEN_PURGE_VALVE;
    return true;
}

bool GSEOrder::updateRx(std::shared_ptr<Connector> connector) {
    return true;
}


