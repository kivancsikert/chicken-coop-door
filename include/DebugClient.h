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

    int connect(IPAddress ip, uint16_t port) {
        Serial.print("> connect(");
        Serial.print(ip);
        Serial.print(", ");
        Serial.print(port);
        Serial.println();
        int result = delegate->connect(ip, port);
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    int connect(const char* host, uint16_t port) {
        Serial.print("> connect(");
        Serial.print(host);
        Serial.print(", ");
        Serial.print(port);
        Serial.println();
        int result = delegate->connect(host, port);
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    size_t write(uint8_t data) {
        Serial.printf("> write(%u)\n", data);
        size_t result = delegate->write(data);
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    size_t write(const uint8_t* buf, size_t size) {
        Serial.printf("> write([%u])\n", size);
        size_t result = delegate->write(buf, size);
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    int available() {
        // Serial.println("> available()");
        int result = delegate->available();
        // Serial.print("< ");
        // Serial.println(result);
        return result;
    }

    int read() {
        Serial.println("> read()");
        int result = delegate->read();
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    int read(uint8_t* buf, size_t size) {
        Serial.printf("> read([%u])\n", size);
        size_t result = delegate->read(buf, size);
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    int peek() {
        Serial.println("> peek()");
        int result = delegate->peek();
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    void flush() {
        Serial.println("> flush()");
        delegate->flush();
        Serial.println("< (void)");
    }

    void stop() {
        Serial.println("> stop()");
        delegate->stop();
        Serial.println("< (void)");
    }

    uint8_t connected() {
        Serial.println("> connected()");
        uint8_t result = delegate->connected();
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

    operator bool() {
        Serial.println("> bool()");
        bool result = (bool) delegate;
        Serial.print("< ");
        Serial.println(result);
        return result;
    }

private:
    Client* delegate;
};
