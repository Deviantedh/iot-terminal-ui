#ifndef SLOT_GAME_H
#define SLOT_GAME_H

#include <Arduino.h>

enum SlotSymbol : uint8_t {
  SLOT_SYMBOL_CHERRY = 0,
  SLOT_SYMBOL_LEMON,
  SLOT_SYMBOL_BELL,
  SLOT_SYMBOL_STAR,
  SLOT_SYMBOL_DIAMOND,
  SLOT_SYMBOL_SEVEN,
  SLOT_SYMBOL_COUNT
};

enum SlotMode : uint8_t {
  SLOT_MODE_3 = 0,
  SLOT_MODE_5
};

static const uint8_t SLOT_MAX_REELS = 5;
static const uint8_t SLOT_MAX_ROWS = 3;
static const uint8_t SLOT_MAX_PAYLINES = 20;

struct SlotSpinResult {
  SlotMode mode;
  uint8_t reelCount;
  uint8_t rowCount;
  SlotSymbol symbols[SLOT_MAX_ROWS][SLOT_MAX_REELS];
};

struct SlotLineWin {
  uint8_t paylineIndex;
  uint8_t length;
  SlotSymbol symbol;
};

struct SlotOutcome {
  bool isWin;
  bool isJackpot;
  uint8_t lineWinCount;
  uint8_t totalMatchCount;
  int payoutMultiplier;
  SlotLineWin lineWins[SLOT_MAX_PAYLINES];
};

uint8_t slotModeReelCount(SlotMode mode);
uint8_t slotModeRowCount(SlotMode mode);
uint8_t slotPaylineCount();
const uint8_t* slotPayline(uint8_t paylineIndex);
SlotSpinResult determineSpinResult(SlotMode mode);
SlotOutcome evaluateSpinResult(const SlotSpinResult& result);

#endif
