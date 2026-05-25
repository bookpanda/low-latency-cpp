#include <cstddef>
#include <cstdint>
#include <cstdio>

struct PoorlyAlignedData {
  char c;     // 0 (0 + 1 char + 1 pad)
  uint16_t u; // 2 (2 + 2 uint16_t + 4 pad)
  double d;   // 8 (8 + 8 double)
  int16_t i;  // 16 (16 + 2 int16_t + 6 pad)
}; // 24 (extra 11 bytes of padding)

struct WellAlignedData {
  double d;   // 0 (0 + 8 double)
  uint16_t u; // 8 (8 + 2 uint16_t)
  int16_t i;  // 10 (10 + 2 int16_t)
  char c;     // 12 (12 + 1 char + 3 pad)
}; // 16 (3 bytes of padding)

// eliminate all padding
#pragma pack(push, 1)
struct PackedData {
  double d;
  uint16_t u;
  int16_t i;
  char c;
}; // 8 + 2 + 2 + 1 = 13
#pragma pack(pop)

int main() {
  printf("PoorlyAlignedData c:%lu u:%lu d:%lu i:%lu size:%lu\n",
         offsetof(struct PoorlyAlignedData, c),
         offsetof(struct PoorlyAlignedData, u),
         offsetof(struct PoorlyAlignedData, d),
         offsetof(struct PoorlyAlignedData, i), sizeof(PoorlyAlignedData));
  printf("WellAlignedData d:%lu u:%lu i:%lu c:%lu size:%lu\n",
         offsetof(struct WellAlignedData, d),
         offsetof(struct WellAlignedData, u),
         offsetof(struct WellAlignedData, i),
         offsetof(struct WellAlignedData, c), sizeof(WellAlignedData));
  printf("PackedData d:%lu u:%lu i:%lu c:%lu size:%lu\n",
         offsetof(struct PackedData, d), offsetof(struct PackedData, u),
         offsetof(struct PackedData, i), offsetof(struct PackedData, c),
         sizeof(PackedData));
}