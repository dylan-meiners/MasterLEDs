//#define NO_ARDI
#define SERIALPORT_H_DEBUG_WRITE

#include <SerialPort/SerialPort.h>
#include <Socket/Socket.h>
#include <ConfigParser/ConfigParser.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>

typedef unsigned char uchar;
typedef unsigned int uint;

const uint BUF_LEN = 18;

void OnMessage(sio::event &);
uchar hex_to_byte_two(const char* ptr);
uint uchar_star_to_uint(const char*, uint);
float parse_double_to_float(const char* start);
void debug_buf(const uint);

#ifndef NO_ARDI
SerialPort ardi(true, true);
#endif
uchar buf[BUF_LEN] = { 0 };
const uchar NOTHING = 0;
const uchar SOMETHING = 1;

bool F_SEND = 0;
bool F_EXT_SEND = 0;
uint ext = 0;
uchar* ptr_to_ext_buf = nullptr;
uchar ok;

int main() {

    ConfigParser config("CONFIG.txt");
    std::string port = config.GetMatchingChoice("port");
    if (port == "") return -1;
    uint len = strlen(port.c_str()) + 1;
    wchar_t* wportstr = new wchar_t[len];
    size_t convchars = 0;
    mbstowcs_s(&convchars, wportstr, len, port.c_str(), _TRUNCATE);

#ifndef NO_ARDI
    if (!ardi.Setup(wportstr)) {

        printf("FATAL: ERROR | UNABLE TO SETUP ARDUINO\n");
        return false;
    }
#endif

    Socket frontend("http://localhost:3000");
    frontend.Connect();
    frontend.socket()->on("message", &OnMessage);

    bool running = true;
    char c;
    while (running) {

        if (_kbhit()) {
            
            c = _getch();
            switch (c) {

            case 27:
                running = false;
                break;

            case 'q':
                running = false;
                break;

            case 't':
                buf[0] = 's'; //solid color incoming
                buf[1] = 255;
                buf[2] = 0;
                buf[3] = 255;
                F_SEND = true;

            default:
                break;
            }
        }

        if (!ardi.ReadBytes(&ok, 1)) {

            printf("Could not read ok: %d\n", GetLastError());
        }
        else {
            
            if (ok == 127) {

                if (F_SEND) {

                    ardi.WriteBytes(&SOMETHING, 1);
                    Sleep(100);
                    if (!F_EXT_SEND) ardi.WriteBytes(buf, BUF_LEN);
                    else {

                        ardi.WriteBytes(ptr_to_ext_buf, BUF_LEN + ext);
                        F_EXT_SEND = false;
                    }
                    F_SEND = false;
                }
                else ardi.WriteBytes(&NOTHING, 1);
            }
            else printf("not 127... it is actually: %d\n", ok);
        }
    }

    return 0;
}

void OnMessage(sio::event &event) {

    sio::message::ptr message = event.get_message();
    std::string m = message->get_string();
    const char* m_as_c_str = m.c_str();
    const char* ptr = m_as_c_str + 1;
    const char ident = m[0];
    if (ident == 's') {

        buf[0] = 's'; //solid color incoming
        buf[1] = hex_to_byte_two(m_as_c_str + 2);
        buf[2] = hex_to_byte_two(m_as_c_str + 4);
        buf[3] = hex_to_byte_two(m_as_c_str + 6);
#ifndef NO_ARDI
        F_SEND = true;
#else
        debug_buf(4);
#endif
    }
    else if (ident == 'w') {

        buf[0] = 'w'; //wave data incoming
        float wave_speed = parse_double_to_float(m_as_c_str + 1);
        memcpy(buf + 1, &wave_speed, sizeof(float));
#ifndef NO_ARDI
        F_SEND = true;
#else
        debug_buf(2);
#endif
    }
    else if (ident == 't') {

        buf[0] = 't'; //trail data incoming
        float trail_speed = parse_double_to_float(m_as_c_str + 1);
        memcpy(buf + 1, &trail_speed, sizeof(float));
        while (*++ptr != ':');
        ptr += 2;
        buf[5] = hex_to_byte_two(ptr);
        buf[6] = hex_to_byte_two(ptr + 2);
        buf[7] = hex_to_byte_two(ptr + 4);
        ptr += 7;
        buf[8] = (*ptr == '1') ? 1 : 0;
#ifndef NO_ARDI
        F_SEND = true;
#else
        debug_buf(13);
#endif
    }
    else if (ident == 'm') {

        uint acc = 1;
        while (*++ptr != ':') acc++; //count how many chars the text is
        uchar* stendo_buf = (uchar*)malloc((acc + BUF_LEN) * sizeof(uchar));
        stendo_buf[0] = 'm'; //morse data incoming
        stendo_buf[1] = acc; //let ardi know how many bytes are coming
        ptr = m_as_c_str + 1; //point to text
        memcpy(stendo_buf + 2, ptr, acc); //copy text to buf
        ptr = m_as_c_str + acc + 2; //point to floats
        float dot = parse_double_to_float(ptr);
        while (*++ptr != ':');
        ptr++;
        float dash = parse_double_to_float(ptr);
        while (*++ptr != ':');
        ptr++;
        float space = parse_double_to_float(ptr);
        while (*++ptr != ':');
        ptr++;
        float letter = parse_double_to_float(ptr);
        memcpy(stendo_buf + acc + 2, &dot, sizeof(float));
        memcpy(stendo_buf + acc + 6, &dash, sizeof(float));
        memcpy(stendo_buf + acc + 10, &space, sizeof(float));
        memcpy(stendo_buf + acc + 14, &letter, sizeof(float));
#ifndef NO_ARDI
        F_SEND = true;
        F_EXT_SEND = true;
        ext = acc;
        ptr_to_ext_buf = stendo_buf;
#else
        printf("DEBUG STENDO BUF: ");
        for (uint i = 0; i < 34 + acc; i++) {

            printf("%d ", stendo_buf[i]);
        }
        printf("\n");
#endif
    }
    else if (ident == 'i') {

        buf[0] = 'i';
        buf[1] = uchar_star_to_uint(m_as_c_str + 1, 3);
#ifndef NO_ARDI
        F_SEND = true;
#else
        debug_buf(2);
#endif
    }
    else if (ident == 0) {

        buf[0] = 0;
#ifndef NO_ARDI
        F_SEND = true;
#else
        debug_buf(1);
#endif
    }
    else if (ident == 1) {

        buf[0] = 1;
#ifndef NO_ARDI
        F_SEND = true;
#else
        debug_buf(1);
#endif
    }
}

uchar hex_to_byte_two(const char * ptr) {

    uchar toReturn = 0;
    uint dec_form[2] = { 0 };
    for (uint i = 0; i < 2; i++) {

        if (*ptr >= '0' && *ptr <= '9') dec_form[i] = *ptr - 48; //if a digit, convert form ascii to int
        else if (*ptr >= 'a' && *ptr <= 'f') dec_form[i] = *ptr - 97 + 10;
        else if (*ptr >= 'A' && *ptr <= 'F') dec_form[i] = *ptr - 65 + 10;

        toReturn += dec_form[i] * (uint)pow(16, 1 - i);
        ptr++;
    }
    return toReturn;
}

uint uchar_star_to_uint(const char* buffer, uint digits) {

    const char* ptr = buffer;
    uint acc = 0;
    for (uint i = 0; i < digits; i++) {

        acc += (*ptr - 48) * (uint)pow(10, digits - 1 - i);
        ptr++;
    }
    return acc;
}

float parse_double_to_float(const char* start) {

    const char* ptr = start;
    float res = 0.0;
    uint acc = 1;
    while (*++ptr != '.') { acc++; }
    ptr -= acc;
    res = uchar_star_to_uint(ptr, acc);
    ptr += acc;
    acc = 0;
    while (*++ptr != ':') { acc++; }
    ptr -= acc;
    float temp = uchar_star_to_uint(ptr, acc);
    while (temp > 1) {

        temp /= 10.0;
        acc--;
    }
    if (acc > 0) { for (uint i = 0; i < acc; i++) { temp /= 10.0; } }
    return res + temp;
}