# Chicken Coop Door

![Build badge](https://github.com/kivancsikert/chicken-coop-door/actions/workflows/build.yml/badge.svg)

## Dashboard

See [Galagonya Kert dashboards on MotherHen](https://motherhen.kertkaland.com/grafana/).

## Building

-   Place `data/mqtt-config.json` with the following contents used to access the MQTT server:

    ```json
    {
        "host": "broker",                 // MQTT broker host name
        "port": 1883,                     // MQTT broker port
        "clientId": "chicken-door",       // MQTT client ID
        "prefix": "devices/chicken-door"  // MQTT prefix
    }
    ```

-   Deploy file system image.
