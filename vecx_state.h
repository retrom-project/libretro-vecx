/* Retrom VecX checkpoint v1. Included after the machine's private state.
 * The format is local to the pinned architecture/core; no process pointers
 * are persisted. The browser target is wasm32 little endian. */
static unsigned *const state_unsigned[] = {
   &via_ora, &via_orb, &via_ddra, &via_ddrb, &via_t1on, &via_t1int,
   &via_t1c, &via_t1ll, &via_t1lh, &via_t1pb7, &via_t2on, &via_t2int,
   &via_t2c, &via_t2ll, &via_sr, &via_srb, &via_src, &via_srclk,
   &via_acr, &via_pcr, &via_ifr, &via_ier, &via_ca2, &via_cb2h, &via_cb2s,
   &alg_rsh, &alg_xsh, &alg_ysh, &alg_zsh, &alg_jch0, &alg_jch1,
   &alg_jch2, &alg_jch3, &alg_jsh, &alg_compare, &alg_vectoring,
   &newbankswitchOffset, &bankswitchOffset, &bankswitchstate,
   &dacsamps, &dacset, &psgsamps, &psgcycs,
};
static long *const state_long[] = {
   &alg_dx, &alg_dy, &alg_curr_x, &alg_curr_y, &alg_vector_x0,
   &alg_vector_y0, &alg_vector_x1, &alg_vector_y1, &alg_vector_dx,
   &alg_vector_dy, &vector_draw_cnt, &vector_erse_cnt, &fcycles,
};
#define STATE_UNSIGNED_COUNT (sizeof(state_unsigned) / sizeof(state_unsigned[0]))
#define STATE_LONG_COUNT (sizeof(state_long) / sizeof(state_long[0]))

typedef struct {
   uint32_t magic;
   uint32_t version;
   unsigned cpu[10];
   psg_t sound;
   unsigned registers[STATE_UNSIGNED_COUNT];
   long analog[STATE_LONG_COUNT];
   uint32_t draw_offset;
   char large_cart;
   unsigned char vector_color;
   unsigned char ram[sizeof(vecx_ram)];
   vector_t vectors[2 * VECTOR_CNT];
   long hash[VECTOR_HASH];
} vecx_snapshot;

int vecx_statesz(void)
{
   return sizeof(vecx_snapshot);
}

int vecx_serialize(char *dst, int size)
{
   vecx_snapshot *state;
   unsigned i;
   if (!dst || size != vecx_statesz()) return 0;
   state = calloc(1, sizeof(*state));
   if (!state) return 0;
   state->magic = 0x31584356; /* VCX1 */
   state->version = 1;
   e6809_serialize((char *)state->cpu);
   vecx_psg_state_save(&state->sound);
   for (i = 0; i < STATE_UNSIGNED_COUNT; ++i)
      state->registers[i] = *state_unsigned[i];
   for (i = 0; i < STATE_LONG_COUNT; ++i)
      state->analog[i] = *state_long[i];
   state->draw_offset = (unsigned)(vectors_draw - vectors_set);
   state->large_cart = big;
   state->vector_color = alg_vector_color;
   memcpy(state->ram, vecx_ram, sizeof(vecx_ram));
   memcpy(state->vectors, vectors_set, sizeof(vectors_set));
   memcpy(state->hash, vector_hash, sizeof(vector_hash));
   memcpy(dst, state, sizeof(*state));
   free(state);
   return 1;
}

int vecx_deserialize(char *src, int size)
{
   vecx_snapshot *state;
   unsigned i;
   if (!src || size != vecx_statesz()) return 0;
   state = malloc(sizeof(*state));
   if (!state) return 0;
   memcpy(state, src, sizeof(*state));
   if (state->magic != 0x31584356 || state->version != 1 ||
       (state->draw_offset != 0 && state->draw_offset != VECTOR_CNT) ||
       state->sound.rlatch >= 16 || state->analog[10] < 0 ||
       state->analog[10] > VECTOR_CNT || state->analog[11] < 0 ||
       state->analog[11] > VECTOR_CNT) {
      free(state);
      return 0;
   }
   e6809_deserialize((char *)state->cpu);
   vecx_psg_state_load(&state->sound);
   vecx_psg_reset_buffer();
   for (i = 0; i < STATE_UNSIGNED_COUNT; ++i)
      *state_unsigned[i] = state->registers[i];
   for (i = 0; i < STATE_LONG_COUNT; ++i)
      *state_long[i] = state->analog[i];
   big = state->large_cart;
   alg_vector_color = state->vector_color;
   memcpy(vecx_ram, state->ram, sizeof(vecx_ram));
   memcpy(vectors_set, state->vectors, sizeof(vectors_set));
   memcpy(vector_hash, state->hash, sizeof(vector_hash));
   vectors_draw = vectors_set + state->draw_offset;
   vectors_erse = vectors_set + (state->draw_offset ? 0 : VECTOR_CNT);
   free(state);
   return 1;
}
