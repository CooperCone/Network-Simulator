#include "unitTest.h"
#include "layers/forward.h"
#include "layers/layer3.h"
#include "layers/layer3.c"

TEST_SUITE("IP Address") {

    TEST("Create IP Address") {
        IPAddress addr;
        ipAddr_fromStr("192.168.1.1", addr);
        assertEq(addr[0], 192);
        assertEq(addr[1], 168);
        assertEq(addr[2], 1);
        assertEq(addr[3], 1);
    }

    TEST("IP Address To String") {
        IPAddress addr;
        ipAddr_fromStr("192.168.1.1", addr);

        IPStr str;
        ipAddr_toStr(addr, str);
        assertEq(strcmp(str, "192.168.1.1"), 0);
    }

}
