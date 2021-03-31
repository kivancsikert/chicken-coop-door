#pragma once

#include <Arduino.h>
#include <Client.h>

class DebugClient : public Client {

public:
    DebugClient(Client* delegate = nullptr) {
        this->delegate = delegate;
    }

    void delegateTo(Client* delegate) {
        this->delegate = delegate;
    }

    void setLog(bool log) {
        this->log = log;
    }

    int connect(IPAddress ip, uint16_t port) {
        if (log) {
            Serial.print("> connect(");
            Serial.print(ip);
            Serial.print(", ");
            Serial.print(port);
            Serial.println();
        }
        int result = delegate->connect(ip, port);
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    int connect(const char* host, uint16_t port) {
        if (log) {
            Serial.print("> connect(");
            Serial.print(host);
            Serial.print(", ");
            Serial.print(port);
            Serial.println();
        }
        int result = delegate->connect(host, port);
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    size_t write(uint8_t data) {
        if (log) {
            Serial.printf("> write(%u)\n", data);
        }
        size_t result = delegate->write(data);
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    size_t write(const uint8_t* buf, size_t size) {
        if (log) {
            Serial.printf("> write([%u])\n", size);
        }
        size_t result = delegate->write(buf, size);
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    int available() {
        // if (log) {
        //     Serial.println("> available()");
        // }
        int result = delegate->available();
        // if (log) {
        //     Serial.print("< ");
        //     Serial.println(result);
        // }
        return result;
    }

    int read() {
        if (log) {
            Serial.println("> read()");
        }
        int result = delegate->read();
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    int read(uint8_t* buf, size_t size) {
        if (log) {
            Serial.printf("> read([%u])\n", size);
        }
        size_t result = delegate->read(buf, size);
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    int peek() {
        if (log) {
            Serial.println("> peek()");
        }
        int result = delegate->peek();
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    void flush() {
        if (log) {
            Serial.println("> flush()");
        }
        delegate->flush();
        if (log) {
            Serial.println("< (void)");
        }
    }

    void stop() {
        if (log) {
            Serial.println("> stop()");
        }
        delegate->stop();
        if (log) {
            Serial.println("< (void)");
        }
    }

    uint8_t connected() {
        if (log) {
            Serial.println("> connected()");
        }
        uint8_t result = delegate->connected();
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

    operator bool() {
        if (log) {
            Serial.println("> bool()");
        }
        bool result = (bool) delegate;
        if (log) {
            Serial.print("< ");
            Serial.println(result);
        }
        return result;
    }

private:
    Client* delegate;
    bool log = true;
};
