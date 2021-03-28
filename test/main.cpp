#include "../estruct.h"

struct Demo
{
    int a;
    double b;
    float c;
    std::string d;
    bool e;

    std::map<int, std::string> f;

    ESTRUCT_TYPE_DEFINE(a, b,c,d,e, f)
};

int main()
{
    EStructPacker packer;

    Demo d, d2;
    d.a = 1;
    d.b = 2.2;
    d.c = 3.3;
    d.d = "tyrant EStringPacker ggo";
    d.e = true;
    d.f[332] = "ojbk";
    packer.serlize(d);

    EStructUnPacker up(packer.ptr(), packer.size());

    up.deserlize(d2);

    return 0;
}