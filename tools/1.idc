#include <idc.idc>
static main(void) {
  auto ea;
  ea = LocByName("GameOpLevelData0x288_0");
  if (ea != BADADDR) MakeNameEx(ea, "updateAnimatedLvlObjectType0", SN_CHECK | SN_NOWARN);
  ea = LocByName("GameOpLevelData0x288_1");
  if (ea != BADADDR) MakeNameEx(ea, "updateAnimatedLvlObjectType1", SN_CHECK | SN_NOWARN);
  ea = LocByName("GameOpLevelData0x288_2");
  if (ea != BADADDR) MakeNameEx(ea, "updateAnimatedLvlObjectType2", SN_CHECK | SN_NOWARN);
}
