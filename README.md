# Chicken Coop Door

https://github.com/kivancsikert/chicken-coop-door/actions/workflows/build.yml/badge.svg

## Dashboard

See [Galagonya Kert dashboards on Google Data Studio](https://datastudio.google.com/u/0/explorer/2f1ac65f-5839-4393-8eb5-25d90e34bad6).

## Building

* Place `data/iot-config.json` with the following contents used to access Google Cloud IoT Core:

    ```json
    {
        "projectId": "<project>",
        "location": "<location>",
        "registryId": "<registry>",
        "deviceId": "<device>",
        "privateKey": "6e:b8:17:35:c7:fc:6b:d7:a9:cb:cb:49:7f:a0:67:63:38:b0:90:57:57:e0:c0:9a:e8:6f:06:0c:d9:ee:31:41"
    }
    ```

    Private key must be exactly 32 bytes, pad front with `00:` if necessary.
* Deploy file system image.
