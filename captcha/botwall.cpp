/*
flow
-> usr loads /CAPTCHA.zomg
<- gets back challenge img loaded from b64, plus invisible form field for "token"
    token is challenge soln+timestamp of generation rounded to 100 secs+captcha_secret+ipid all sha256 hashed

-> usr jets back the proposed soln + the token
<- if proposed+time of processing rounded to 100 secs+captcha secret+ipid == token, usr is heckin' cute and valid
    else, FUCK YUO!!!!1111one
*/

#include "./botwall.h"
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb/stb_image.h"
#include "../lib/stb/stb_image_write.h"
#include "../lib/women/women.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>

std::vector<std::string> opts = {"S", "C", "T", "D"};

void get_file_bytes(void* context, void* data, int size)
{
    std::stringstream& f = *((std::stringstream*)context);
    f.write(((char*)data), size);
}

std::pair<std::string, std::string> botwall::generate_captcha(unsigned int ipid, std::string secret)
{
    int x = 200;
    int y = 40;
    int n = 1;

    std::vector<std::string> challenge;
    for (int i = 0; i < ((int)(x/20))-2; i++)
        challenge.push_back(opts[rand() % 4]);

    std::vector<std::vector<unsigned char>> proto;
    for (int yi = 0; yi < y; yi++)
    {
        proto.push_back(std::vector<unsigned char>());
        for (int xi = 0; xi < x; xi++)
            proto[yi].push_back(0);
    }

    std::string challenge_str;

    int mod = rand() % 12;
    mod *= (mod%2) ? 1 : -1;
    for (int xi = 20; xi < x-20; xi += 20)
    {
        int idx = (xi-30)/20;
        int oldxi = xi;
        mod = rand() % 6;
        mod *= (mod%2) ? 1 : -1;
        xi += mod;
        std::string chal = challenge[idx];
        challenge_str.append(chal);

        int basey = (y/2);
        basey += mod;

        int r = 10;

        if (chal == "D")
        {
            for (int deg = 0; deg < 360; deg += 2)
            {
                int xt = r*std::pow(cos(deg), 3);
                int yt = r*std::pow(sin(deg), 3);
                proto[yt + basey][xt + xi] = 255;
            }
        }
        else if (chal == "C")
        {
            for (int deg = 0; deg < 360; deg += 2)
            {
                int xt = r*cos(deg);
                int yt = r*sin(deg);
                proto[yt + basey][xt + xi] = 255;
            }
        }
        else if (chal == "T")
        {
            for (int deg = 0; deg < 360; deg += 2)
            {
                int xt = -1 * r*std::pow(cos(deg), 3);
                int yt = -1 * r*std::pow(sin(deg), 2);
                proto[yt + basey][xt + xi] = 255;
            }
            for (int bar = 0; bar < 10; bar++)
                proto[basey][xi - 5 + bar] = 255;
        }
        else if (chal == "S")
        {
            for (int bar = 0; bar < 10; bar++)
                proto[basey-5][xi - 5 + bar] = 255;
            for (int bar = 0; bar < 10; bar++)
                proto[basey+5][xi - 5 + bar] = 255;
            for (int bar = 0; bar < 10; bar++)
                proto[basey-5 + bar][xi - 5] = 255;
            for (int bar = 0; bar < 10; bar++)
                proto[basey-5 + bar][xi + 5] = 255;
        }
        xi = oldxi;
    }

    for (int i = 0; i < 100; i++)
        proto[rand()%y][rand()%x] = 255;

    for (int i = 0; i < 200; i++)
        proto[rand()%y][rand()%x] = 0;

    std::vector<unsigned char> bytes;
    for (int yi = 0; yi < y; yi++)
    {
        for (int xi = 0; xi < x; xi++)
            bytes.push_back(proto[yi][xi]);
    }
    std::stringstream f {};
    stbi_write_bmp_to_func(&get_file_bytes, (void*)(&f), x, y, n, bytes.data());
    std::string aaa = "data:image/bmp;base64,";
    aaa.append(b64enc(f.str()));


    std::string token = challenge_str;
    token.append(std::to_string(ipid));
    time_t tim = time(0);
    tim = (time_t)(tim/(60*5));
    token.append(std::to_string(tim));
    token.append(secret);
    token = sha256(token);

    /*
    std::fstream F;
    F.open("static/imgs/captcha/aids.bmp");
    F.write(f.str().c_str(), f.str().length());
    F.close();
    */
    return std::pair<std::string, std::string>(token, aaa);
}

bool botwall::check_captcha(std::string ipid, std::string guess, std::string token, std::string secret)
{
    std::string proposed = guess;
    proposed.append(ipid);
    time_t tim = time(0);
    tim = (time_t)(tim/(60*5));
    proposed.append(std::to_string(tim));
    proposed.append(secret);
    proposed = sha256(proposed);

    return proposed == token;
}