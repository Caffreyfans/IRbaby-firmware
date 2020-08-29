#ifndef IRBABYHA_H
#define IRBABYHA_H
#include <WString.h>
#include "../lib/Irext/include/ir_ac_control.h"
void returnACStatus(String filename, t_remote_ac_status ac_status);
void registAC(String filename, bool flag);
#endif