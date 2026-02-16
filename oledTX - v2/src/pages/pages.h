#ifndef PAGES_H
#define PAGES_H

#include "state.h"

typedef void (*PageRenderFunc)(TXState* s);
typedef void (*PageActionFunc)(TXState* s);

struct Page {
    PageRenderFunc render;
    PageActionFunc onBtn2;  // NULL if no action
    const char* name;
};

#define MAX_PAGES 8

// Registration (called by page files at startup)
void pageRegister(Page page);

// Navigation
void pageNext();
uint8_t pageGetCurrent();
uint8_t pageGetCount();

// Execute
void pageRender(TXState* s);
void pageBtn2(TXState* s);

#endif