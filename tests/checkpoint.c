/* Project-owned state regression; no game or BIOS input is used. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vecx.c"

int main(void)
{
   psg_t sound = {0}, restored = {0};
   int size;
   char *state;
   vecx_reset();
   sound.rlatch = 13;
   sound.nshift = 0x1abcd;
   sound.reg[15] = 0x6a;
   vecx_psg_state_load(&sound);
   alg_dx = 12345;
   alg_dy = -54321;
   fcycles = 42;
   bankswitchOffset = 32768;
   newbankswitchOffset = 32768;
   vectors_draw[0].x0 = 1700;
   vector_draw_cnt = 1;
   /* Suspend a visible vector immediately before the beam blanks. */
   alg_vectoring = 1;
   alg_vector_x0 = 2000;
   alg_vector_y0 = 3000;
   alg_vector_x1 = 4000;
   alg_vector_y1 = 5000;
   alg_vector_color = 77;
   via_acr = 0;
   via_cb2h = 0;
   size = vecx_statesz();
   state = calloc(1, size + 32);
   assert(state);
   memset(state + size, 0x5a, 32);
   assert(vecx_serialize(state, size));
   for (int i = 0; i < 32; ++i) assert((unsigned char)state[size + i] == 0x5a);
   assert(!vecx_serialize(state, size - 1));
   alg_sstep();
   assert(vector_draw_cnt == 2);
   vector_t continued = vectors_draw[1];
   assert(continued.color == 77);
   vecx_reset();
   alg_vector_color = 0;
   alg_dx = alg_dy = 0;
   bankswitchOffset = newbankswitchOffset = 0;
   vectors_draw[0].x0 = 0;
   assert(!vecx_deserialize(state, size - 1));
   assert(vecx_deserialize(state, size));
   vecx_psg_state_save(&restored);
   assert(restored.rlatch == 13);
   assert(restored.nshift == 0x1abcd);
   assert(restored.reg[15] == 0x6a);
   assert(alg_dx == 12345 && alg_dy == -54321);
   assert(fcycles == 42);
   assert(bankswitchOffset == 32768 && newbankswitchOffset == 32768);
   assert(vector_draw_cnt == 1 && vectors_draw[0].x0 == 1700);
   alg_sstep();
   assert(vector_draw_cnt == 2);
   assert(vectors_draw[1].x0 == continued.x0);
   assert(vectors_draw[1].y0 == continued.y0);
   assert(vectors_draw[1].x1 == continued.x1);
   assert(vectors_draw[1].y1 == continued.y1);
   assert(vectors_draw[1].color == continued.color);
   state[0] ^= 1;
   assert(!vecx_deserialize(state, size));
   assert(alg_dx == 12345);
   free(state);
   puts("checkpoint restores sound, analog execution, cartridge bank and vector display");
   return 0;
}
