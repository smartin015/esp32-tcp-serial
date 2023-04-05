#ifndef VCP_H
#define VCP_H

#include <cstddef>

void setup_vcp(void (*cb)(uint8_t*, std::size_t, void*));
void loop_vcp();
void vcp_send(uint8_t* data, std::size_t len); //blocking?

#endif // VCP_H
