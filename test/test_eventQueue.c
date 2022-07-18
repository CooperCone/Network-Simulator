#include "unitTest.h"
#include "event.h"
#include "event.c"

EventNode nodeFixture(u64 time) {
    return (EventNode) {
        .event=NULL,
        .time=time
    };
}

TEST_SUITE("Event Queue") {

    TEST("Push Single Element") {
        EventQueue q = {0};
        eventQueue_push(&q, nodeFixture(7));

        assertEq(q.maxNodes, 8);
        assertEq(q.numNodes, 1);
        assertEq(q.startNode, 0);
        assertEq(q.nodes[0].time, 7);
    }

    TEST("Push Two Elements") {
        EventQueue q = {0};
        eventQueue_push(&q, nodeFixture(7));

        assertEq(q.maxNodes, 8);
        assertEq(q.numNodes, 1);
        assertEq(q.startNode, 0);
        assertEq(q.nodes[0].time, 7);

        eventQueue_push(&q, nodeFixture(10));

        assertEq(q.maxNodes, 8);
        assertEq(q.numNodes, 2);
        assertEq(q.startNode, 0);
        assertEq(q.nodes[1].time, 10);
    }

    TEST("Push To Resize") {
        EventQueue q = {0};
        eventQueue_push(&q, nodeFixture(7));
        eventQueue_push(&q, nodeFixture(8));
        eventQueue_push(&q, nodeFixture(9));
        eventQueue_push(&q, nodeFixture(11));
        eventQueue_push(&q, nodeFixture(12));
        eventQueue_push(&q, nodeFixture(13));
        eventQueue_push(&q, nodeFixture(14));
        eventQueue_push(&q, nodeFixture(15));
        eventQueue_push(&q, nodeFixture(16));

        assertEq(q.maxNodes, 16);
        assertEq(q.numNodes, 9);
        assertEq(q.startNode, 0);
    }

    TEST("Sorting") {
        EventQueue q = {0};
        eventQueue_push(&q, nodeFixture(7));
        eventQueue_push(&q, nodeFixture(12));
        eventQueue_push(&q, nodeFixture(9));
        eventQueue_push(&q, nodeFixture(11));
        eventQueue_push(&q, nodeFixture(8));

        assertEq(q.nodes[0].time, 7);
        assertEq(q.nodes[1].time, 8);
        assertEq(q.nodes[2].time, 9);
        assertEq(q.nodes[3].time, 11);
        assertEq(q.nodes[4].time, 12);
    }

    TEST("Circular Push and Pop") {
        EventQueue q = {0};
        eventQueue_push(&q, nodeFixture(7));
        eventQueue_push(&q, nodeFixture(8));
        eventQueue_push(&q, nodeFixture(9));
        eventQueue_push(&q, nodeFixture(11));
        eventQueue_push(&q, nodeFixture(12));
        eventQueue_push(&q, nodeFixture(13));
        eventQueue_push(&q, nodeFixture(14));
        eventQueue_push(&q, nodeFixture(15));

        eventQueue_pop(&q);
        assertEq(q.numNodes, 7);

        eventQueue_pop(&q);
        assertEq(q.numNodes, 6);

        eventQueue_push(&q, nodeFixture(17));
        assertEq(q.nodes[0].time, 17);

        eventQueue_push(&q, nodeFixture(6));
        assertEq(q.nodes[2].time, 6);
    }

}
