#include "SlotGame.h"

#include <string.h>

static const uint8_t kSlotPaylines[SLOT_MAX_PAYLINES][SLOT_MAX_REELS] = {
  {0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1},
  {2, 2, 2, 2, 2},
  {0, 1, 2, 1, 0},
  {2, 1, 0, 1, 2},
  {0, 0, 1, 0, 0},
  {2, 2, 1, 2, 2},
  {1, 0, 0, 0, 1},
  {1, 2, 2, 2, 1},
  {0, 1, 0, 1, 0},
  {2, 1, 2, 1, 2},
  {2, 2, 1, 0, 0},
  {0, 0, 1, 2, 2},
  {0, 1, 2, 2, 2},
  {2, 1, 0, 0, 0},
  {0, 1, 1, 1, 0},
  {2, 1, 1, 1, 2},
  {0, 1, 2, 1, 2},
  {2, 1, 0, 1, 0},
  {1, 0, 1, 2, 1}
};

static SlotSymbol randomSlotSymbolLocal() {
  return (SlotSymbol)random((int)SLOT_SYMBOL_COUNT);
}

static int payoutMultiplierForLength(uint8_t length) {
  if (length >= 5) {
    return 10;
  }
  if (length == 4) {
    return 5;
  }
  if (length == 3) {
    return 2;
  }
  return 0;
}

uint8_t slotModeReelCount(SlotMode mode) {
  return (mode == SLOT_MODE_5) ? 5 : 3;
}

uint8_t slotModeRowCount(SlotMode mode) {
  return (mode == SLOT_MODE_5) ? 3 : 1;
}

uint8_t slotPaylineCount() {
  return SLOT_MAX_PAYLINES;
}

const uint8_t* slotPayline(uint8_t paylineIndex) {
  if (paylineIndex >= SLOT_MAX_PAYLINES) {
    return nullptr;
  }
  return kSlotPaylines[paylineIndex];
}

SlotSpinResult determineSpinResult(SlotMode mode) {
  SlotSpinResult result;
  memset(&result, 0, sizeof(result));
  result.mode = mode;
  result.reelCount = slotModeReelCount(mode);
  result.rowCount = slotModeRowCount(mode);

  for (uint8_t row = 0; row < result.rowCount; row++) {
    for (uint8_t reel = 0; reel < result.reelCount; reel++) {
      result.symbols[row][reel] = randomSlotSymbolLocal();
    }
  }

  return result;
}

SlotOutcome evaluateSpinResult(const SlotSpinResult& result) {
  SlotOutcome outcome;
  memset(&outcome, 0, sizeof(outcome));

  if (result.mode == SLOT_MODE_3 || result.reelCount < 5 || result.rowCount < 3) {
    const bool threeEqual =
      result.symbols[0][0] == result.symbols[0][1] &&
      result.symbols[0][1] == result.symbols[0][2];

    outcome.isWin = threeEqual;
    outcome.isJackpot = threeEqual;
    if (threeEqual) {
      outcome.lineWinCount = 1;
      outcome.totalMatchCount = 3;
      outcome.payoutMultiplier = 10;
      outcome.lineWins[0].paylineIndex = 0;
      outcome.lineWins[0].length = 3;
      outcome.lineWins[0].symbol = result.symbols[0][0];
    }
    return outcome;
  }

  for (uint8_t lineIndex = 0; lineIndex < SLOT_MAX_PAYLINES; lineIndex++) {
    const uint8_t* line = kSlotPaylines[lineIndex];
    const SlotSymbol anchor = result.symbols[line[0]][0];
    uint8_t matched = 1;
    for (uint8_t reel = 1; reel < result.reelCount; reel++) {
      if (result.symbols[line[reel]][reel] != anchor) {
        break;
      }
      matched++;
    }

    if (matched >= 3 && outcome.lineWinCount < SLOT_MAX_PAYLINES) {
      SlotLineWin& lineWin = outcome.lineWins[outcome.lineWinCount++];
      lineWin.paylineIndex = lineIndex;
      lineWin.length = matched;
      lineWin.symbol = anchor;
      outcome.totalMatchCount += matched;
      outcome.payoutMultiplier += payoutMultiplierForLength(matched);
      if (matched >= 5) {
        outcome.isJackpot = true;
      }
    }
  }

  outcome.isWin = outcome.lineWinCount > 0;
  return outcome;
}
