# ECGR3101-VirtualPet

Slime pet??

Required Systems:
- Menu
  - Runs Idle in background
- Idle
  - Default Behavior
- Mood
  - Always running
  - Signed int: negative = sad, positive = happy
  - If pet is too sad, it runs away
- Hunger (food level)
  - Always running
  - Unsigned int: 0% = emtpy stomach, 100% = full stomach
  - lower hunger makes mood decrease faster
  - If stomach is empty, pet dies
- Interact
  - For editing Mood and Hunger
  - Walk: Increases mood, slightly decreases food level
  - Feed: Increases food level
  - Pet?: Slightly increases happiness every so often, done during idle with S2 button

Remember animations for all the above
