#include "pages.h"

static Page pages[MAX_PAGES];
static uint8_t pageCount = 0;
static uint8_t currentPage = 0;

void pageRegister(Page page) {
    if (pageCount < MAX_PAGES) {
        pages[pageCount++] = page;
    }
}

void pageNext() {
    if (pageCount > 0) {
        currentPage = (currentPage + 1) % pageCount;
    }
}

uint8_t pageGetCurrent() {
    return currentPage;
}

uint8_t pageGetCount() {
    return pageCount;
}

void pageRender(TXState* s) {
    if (pageCount > 0 && pages[currentPage].render) {
        pages[currentPage].render(s);
    }
}

void pageBtn2(TXState* s) {
    if (pageCount > 0 && pages[currentPage].onBtn2) {
        pages[currentPage].onBtn2(s);
    }
}