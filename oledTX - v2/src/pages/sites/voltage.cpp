#include "pages/pages.h"
#include "hardware/OLED_screen/OLED.h"
#include "hardware/OLED_screen/perm/topbar.h"

static void render(TXState* s) {
    oledUpdateVoltage(s->voltage, s->voltageRaw, pageGetCurrent());
}

static Page voltagePage = {
    .render = render,
    .onBtn2 = NULL,
    .name = "VOLTAGE"
};

__attribute__((constructor))
static void registerVoltagePage() {
    pageRegister(voltagePage);
}