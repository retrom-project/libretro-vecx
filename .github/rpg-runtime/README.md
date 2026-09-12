# Retrom VecX Web build

Upstream `libretro/libretro-vecx` is pinned at `8f671cc9d737f2890c3ce19e177e2984dcae121f`.
`master` is an upstream mirror. Retrom changes belong to `retrom/g8f671cc9d737`,
with development on `feat/*`, `fix/*` or `build/*` branches.

Run `.github/rpg-runtime/build-candidate.sh /absolute/empty/directory` through
Retrom's `pfb-core-build CORE=vecx`. It runs the native checkpoint regression,
uses the pinned Emscripten image and EmulatorJS RetroArch commit, and produces
`vecx-wasm.data`, `LICENSE.md`, `source.tar.gz`, and a content-addressed
`retrom-core-candidate.json`. Runtime only consumes these artifacts.
The Web core uses software vector rendering and the existing single-threaded
EmulatorJS 4.2.3 loader. Supported cartridges are `.vec` and `.bin`.

Retrom checkpoint v1 replaces upstream's incomplete serializer. It preserves
CPU, RAM, VIA, PSG (including register latch and full noise generator), bank
switching, analog integration, frame timing, both vector lists and vector hash.
Pointers are reconstructed in the new instance. The format is private to the
pinned wasm32 core and is not compatible with upstream VecX save files. The
Provider supplies the outer RetroArch container and one gzip storage layer.

No game files belong in this repository or the candidate. The upstream BIOS
headers remain upstream source; no external BIOS download is required.
The core workflow builds and verifies PRs into the maintenance branch. After
product validation, publish annotated `retrom-core-g8f671cc9d737-rN` tags from
that branch; the same workflow publishes the immutable assets and descriptor.
