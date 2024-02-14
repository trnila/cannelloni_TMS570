#pragma once

void vim_init();
void vim_register_irq(int channel, void (*handler)());
