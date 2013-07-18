#ifndef AY8910_H
#define AY8910_H

#define MAX_8910 5
#define ALL_8910_CHANNELS -1

#include "neocdredux.h"

typedef u32(*mem_read_handler) (u32 offset);
typedef void (*mem_write_handler) (u32 offset, u32 data);

struct AY8910interface {
    int num;			/* total number of 8910 in the machine */
    int baseclock;
    int mixing_level[MAX_8910];
    mem_read_handler portAread[MAX_8910];
    mem_read_handler portBread[MAX_8910];
    mem_write_handler portAwrite[MAX_8910];
    mem_write_handler portBwrite[MAX_8910];
    void (*handler[MAX_8910]) (int irq);	/* IRQ handler for the YM2203 */
};

void AY8910_reset( /*int chip */ void);

void AY8910_set_clock( /*int chip, */ int _clock);
void AY8910_set_volume( /*int chip, */ int channel, int volume);


void AY8910Write( /*int chip, */ int a, int data);
int AY8910Read( /*int chip */ void);


u32 AY8910_read_port_0_r(u32 offset);
u32 AY8910_read_port_1_r(u32 offset);
u32 AY8910_read_port_2_r(u32 offset);
u32 AY8910_read_port_3_r(u32 offset);
u32 AY8910_read_port_4_r(u32 offset);

void AY8910_control_port_0_w(u32 offset, u32 data);
void AY8910_control_port_1_w(u32 offset, u32 data);
void AY8910_control_port_2_w(u32 offset, u32 data);
void AY8910_control_port_3_w(u32 offset, u32 data);
void AY8910_control_port_4_w(u32 offset, u32 data);

void AY8910_write_port_0_w(u32 offset, u32 data);
void AY8910_write_port_1_w(u32 offset, u32 data);
void AY8910_write_port_2_w(u32 offset, u32 data);
void AY8910_write_port_3_w(u32 offset, u32 data);
void AY8910_write_port_4_w(u32 offset, u32 data);

int AY8910_sh_start( /*const struct MachineSound *msound */ void);
void AY8910Update(int param, s16 ** buffer, int length);

#endif
