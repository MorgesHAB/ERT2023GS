/*!
 * \file ProtocolDefine.h
 *
 * \brief User Interface - RF Telecom interface protocol definition
 *
 * \author      ISOZ Lionel - EPFL EL BA3
 * \author      KESKE Cem - EPFL EL BA3
 * \date        02.12.2019
 */

#ifndef ProtocolDefine_H
#define ProtocolDefine_H

#include <array>

namespace ui_interface {

    enum DataType {
        ///General

        RUNNING = 0, // bool             **Every thread listens this boolean and stop executing when it is false.

        /// Qt input informations

        ACTIVE_XBEE,        // bool             **This is set to true-false by the Activate-Xbee button.
        IGNITION_CLICKED,   // bool             **This is set to true when ignition button is clicked.
                            //                @attention !!! Listener should eat this boolean to be able to detect further clicks !!!
        SERIALPORT_STATUS,                  //int (enum?)

        SERIALPORT_INDEX,
        RSSI_READ_ORDER,
        RSSI_VALUE,         // uint8_t           **Reports the RSSI in -dBm of the last received RF data packet.

        // logging
        LOGGING_ACTIVE,             // bool - GUI button

        //////////////// RF modem
        /// Ignition
        IGNITION_STATUS,            // bool        ** true :  FIRE,  false : ABORTED
        IGNITION_KEY_1_ACTIVATED,   // bool
        IGNITION_KEY_2_ACTIVATED,   // bool
        IGNITION_RED_BUTTON_PUSHED, // bool
        IGNITION_SENT,              // bool to eat

        /// Packet Rate
        PACKET_CTR_ALL,
        PACKET_CTR_AV,
        PACKET_CTR_GSE,
        PACKET_CTR_PL,
        PACKET_CTR_PP,
        TIME_SINCE_LAST_RX_PACKET,

        PACKET_RX_RATE_CTR, // uint32_t      **This is incremented on each packet received, eated by guito find the packet rate.
        RX_PACKET_CTR,    // uint32_t        **This is the count of the received packets from the beginning of the program
        CORRUPTED_PACKET_CTR,    // uint64_t **The count of the corrupted packets received. Incremented by RX.

        /// Header
        DATAGRAM_ID,     // uint8_t            **The ID of the last packet received
        TX_PACKET_NR,    // uint32_t           **This is the last packet's number. // TODO delete (RF no longer use this)
        TIMESTAMP,       // time_t             **The Time of the last packet received
        TOTAL_RX_PACKET_CTR, //uint64_t         just increment when received a packet

        /// GPS Data Structure
        GPS_ALTITUDE,    // float                **The last altitude reading
        GPS_LONGITUDE,    // float               **The last longitude reading
        GPS_LATITUDE,    // float                **The last latitude reading
        GPS_HDOP,    // float
        GPS_SAT_NBR, // uint8_t

        // Telemetry Avionic Data
        T_ACCELEROMETER_X, T_ACCELEROMETER_Y, T_ACCELEROMETER_Z, // float
        T_EULER_X, T_EULER_Y, T_EULER_Z,                         // float
        T_TEMPERATURE,                                           // float
        T_PRESSURE,                                              // float
        T_SPEED,                                                 // float
        T_ALTITUDE,                                              // float

        // MetaData
        ALTITUDE_MAX,

        /// Pressure Data
        PP_PRESSURE,    // float           **Last pressure reading

        AIR_BRAKES_ANGLE, // float

        // TEST
        TEST_SENSOR_DATA, // print just for test it's a generic DataType

        /// Tx ignition code
        TX_IGNITION_CODE, // uint8_t     **Extract first 4 lsb for the code.


        /// File transmission
            SEND_FILE_REQUEST, // bool       **true if button on the gui is clicked, eated by Telecom thread
            // File transmitting States
            FILE_TRANSMISSION_TOTAL_PACKETS,  // uint64_t
            FILE_TRANSMISSION_CURRENT_PACKET, // uint64_t
            FILE_TRANSMISSION_MY_STATE,       // enum in DataStructures/File
            FILE_TRANSMISSION_RECEIVED_STATE,
            FILE_TRANSMISSION_ALL_RECEIVED, // bool to EAT - for activation of warning window
            FILE_TRANSMISSION_ABORT_ORDER, // Gui push button
            // Gui state    FTX = File Tx (Transmission)
            FTX_FILE_TX_SENT,
            FTX_PL_RESPONSE,
            FTX_MISSING_REQUEST_SENT,
            FTX_ALL_RECEIVED,
            FTX_ACK_SENT,
            // Gui command
            FTX_SEND_MISSING_REQUEST,   // bool eaten
            FTX_RX_PACKET_CTR,      // uint64_t Counter
            FTX_SAVE_FILE,  // bool eaten

        /// Avionics status variables
        STATUS_AV_ID,
        STATUS_AV_VALUE,
        STATUS_AV_STATE, // enum in DataStructures/Avionics/StateValues.h

        /// Avionics state reach
        SLEEP_REACHED,          //bool
        CALIBRATION_REACHED,    //bool
        IDLE_REACHED,           //bool
        FILLING_REACHED,        //bool
        LIFTOFF_REACHED,        //bool
        COAST_REACHED,          //bool
        PRIMARY_EVENT_REACHED,  //bool
        SECONDARY_EVENT_REACHED,//bool
        TOUCH_DOWN_REACHED,     //bool

        /// GSE orders
        GSE_ORDER_VALUE, // enum in DataStructures/GSE/GSEOrderValues.h

        /// GSE sensors
        GSE_HOSE_PRESSURE,
        GSE_HOSE_TEMP,
        GSE_HOSE_STATUS,
        GSE_MOTOR_SPEED,
        GSE_TANK_WEIGHT,

        /// Payload data
        PL_GPS_ALTITUDE,    // float                **The last altitude reading
        PL_GPS_LONGITUDE,    // float               **The last longitude reading
        PL_GPS_LATITUDE,    // float                **The last latitude reading
        PL_GPS_HDOP,    // float
        PL_GPS_SAT_NBR, // uint8_t,
        PL_TEMPERATURE,
        PL_STATE_UI,
        PL_BME_ALTITUDE,
        PL_BME_TEMPERATURE,
        PL_BME_PRESSURE,
        PL_BME_HUMIDITY,


        /// !!! THIS MUST BE THE LAST LINE !!!
        /// The size of the connected data array
        ARRAY_SIZE
    };

    enum StringType {
        PL_RX_FILENAME,
        /// !!! THIS MUST BE THE LAST LINE !!!
        /// The size of the connected data array
        STRING_ARRAY_SIZE
    };

/**
*   The values put here will be reset to 0 when the connector resets.
*
*/
    constexpr DataType dataToReset[] = {
                   IGNITION_STATUS,
                   IGNITION_KEY_1_ACTIVATED,
                   IGNITION_KEY_2_ACTIVATED,
                   IGNITION_RED_BUTTON_PUSHED,
                   IGNITION_SENT,

                   /// PacketNbr
                   PACKET_RX_RATE_CTR,
                   RX_PACKET_CTR,
                   CORRUPTED_PACKET_CTR,

                   /// Header
                   DATAGRAM_ID, // uint8_t
                   TX_PACKET_NR,// uint32_t
                   TIMESTAMP, // time_t

                   /// GPS Data Structure
                   GPS_ALTITUDE,// float
                   GPS_LONGITUDE,// float
                   GPS_LATITUDE,// float
                   GPS_HDOP,    // float
                   GPS_SAT_NBR, // uint8_t

                   // Telemetry Avionic Data
                   T_ACCELEROMETER_X, T_ACCELEROMETER_Y,
                   T_ACCELEROMETER_Z,
                   T_EULER_X, T_EULER_Y, T_EULER_Z,
                   T_TEMPERATURE,
                   T_PRESSURE,
                   T_SPEED,
                   T_ALTITUDE,

                   // MetaData
                   ALTITUDE_MAX,

                   /// Pressure Data
                   PP_PRESSURE,

                   AIR_BRAKES_ANGLE,

                   // TEST
                   TEST_SENSOR_DATA,

                   /// Tx ignition code
                   TX_IGNITION_CODE,


                   /// File transmission
                   SEND_FILE_REQUEST,
                   // File transmitting States
                   FILE_TRANSMISSION_TOTAL_PACKETS,
                   FILE_TRANSMISSION_CURRENT_PACKET,
                   FILE_TRANSMISSION_MY_STATE,
                   FILE_TRANSMISSION_RECEIVED_STATE,
                   FILE_TRANSMISSION_ALL_RECEIVED,

                   /// Avionics status variables
                   STATUS_AV_ID,
                   STATUS_AV_VALUE,
                   STATUS_AV_STATE,

                   SLEEP_REACHED,          //bool
                   CALIBRATION_REACHED,    //bool
                   IDLE_REACHED,           //bool
                   FILLING_REACHED,        //bool
                   LIFTOFF_REACHED,        //bool
                   COAST_REACHED,          //bool
                   PRIMARY_EVENT_REACHED,  //bool
                   SECONDARY_EVENT_REACHED,//bool
                   TOUCH_DOWN_REACHED,     //bool

                   /// GSE orders
                   GSE_ORDER_VALUE,

                   /// GSE sensors
                   GSE_HOSE_PRESSURE,
                   GSE_HOSE_TEMP,
                   GSE_HOSE_STATUS,
                   GSE_MOTOR_SPEED,
                   GSE_TANK_WEIGHT
};

} // namespace
#endif // ProtocolDefine_H
