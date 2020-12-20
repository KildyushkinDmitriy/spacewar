#include <assert.h>

#include "game.h"

static void testWrap()
{
    assert(floatEq(floatWrap(0.f, 1.f), 0.f));
    assert(floatEq(floatWrap(0.5f, 1.f), 0.5f));
    assert(floatEq(floatWrap(-0.5f, 1.f), 0.5f));
    assert(floatEq(floatWrap(-22.1f, 1.f), 0.9f));
    assert(floatEq(floatWrap(22.2f, 1.f), 0.2f));
}


void runTests()
{
    testWrap();
}
