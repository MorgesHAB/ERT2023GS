//
// Created by disoz on 05.05.2023.
//
//  Doc: https://www.influxdata.com/blog/getting-started-c-influxdb/

#include <iostream>
#include <InfluxDBFactory.h>

int main(int argc, char *argv[]) {
    auto db = influxdb::InfluxDBFactory::Get(
            "http://localhost:8086?db=temperature_db");
    db->createDatabaseIfNotExists();
    for (auto i: db->query("SHOW DATABASES")) std::cout << i.getTags() << std::endl;
    return 0;
}