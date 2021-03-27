#include "../estruct.h"

struct Demo
{
    int a;
    double b;
    float c;
    std::string d;
    bool e;

    ESTRUCT_TYPE_DEFINE(a, b,c,d,e)
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
    packer.serlize(d);

    EStructUnPacker up(packer.ptr(), packer.size());

    up.deserlize(d2);

    return 0;
}