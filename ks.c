#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  MU = 0,
  KING,
  KYO, TO,
  GIN, KAKU,
  KIN, KEI,
  HI, HU,
};

char *dirSyms1[3] = {"▲", " ", "▽"};
char *dirSyms2[3] = {"△", " ", "▼"};
char **dirSyms;
char *pieceSyms[10] = {"  ", "王", "香", "と", "銀", "角", "金", "桂", "飛", "歩"};

int pieceScores[10] = {
   0, 100000,
  20,     80,
  70,     75,
  80,     40,
  25,     10,
};

int pieces[5][5] = {
  {  HU,  KIN, KING,  GIN,   TO},
  {  MU,   MU,   MU,   MU,   MU},
  {  MU,   MU,   MU,   MU,   MU},
  {  MU,   MU,   MU,   MU,   MU},
  {  TO,  GIN, KING,  KIN,   HU},
};

int dirs[5][5] = {
  { 1,  1,  1,  1,  1},
  { 0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0},
  {-1, -1, -1, -1, -1},
};

int hands[2][5] = {
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
};

void displayBoard(void) {
  printf("  |");
  for (int x = 0; x < 5; x++) {
    printf(" %d |", x);
  }
  printf("\n");

  for (int y = 0; y < 5; y++) {
    printf("--+");
    for (int x = 0; x < 5; x++) {
      printf("---+");
    }
    printf("\n");

    printf("%d |", y);
    for (int x = 0; x < 5; x++) {
      int piece = pieces[y][x];
      int dir = dirs[y][x];
      printf("%s%s|", dirSyms[dir], pieceSyms[piece]);
    }
    if (y == 0) {
      printf(" ");
      for (int i = 1; i < 5; i++) {
        for (int j = 0; j < hands[1][i]; j++) {
          printf("%s", pieceSyms[i*2]);
        }
      }
    }
    if (y == 4) {
      printf(" ");
      for (int i = 1; i < 5; i++) {
        for (int j = 0; j < hands[0][i]; j++) {
          printf("%s", pieceSyms[i*2]);
        }
      }
    }
    printf("\n");
  }
}

void playerTurn(void) {
  for (;;) {
    printf(">>> "); fflush(stdout);
    char cmd[10];
    scanf("%s", cmd);

    if (strcmp(cmd, "move") == 0) {
      int fx, fy, tx, ty;
      scanf("%d%d%d%d", &fx, &fy, &tx, &ty);
      printf("%dx%d -> %dx%d\n", fx, fy, tx, ty);

      if (fx < 0 || fx >= 5 || fy < 0 || fy >= 5 || tx < 0 || tx >= 5 || ty < 0 || ty >= 5) {
        printf("無効な範囲です\n");
        continue;
      }

      int fpiece = pieces[fy][fx];
      int fdir = dirs[fy][fx];
      int tpiece = pieces[ty][tx];
      int tdir = dirs[ty][tx];

      if (fdir != -1) {
        printf("自分の駒がありません\n");
        continue;
      }

      if (tdir == -1) {
        printf("自分の駒がいるところへ指そうとしています\n");
        continue;
      }

      pieces[fy][fx] = MU;
      dirs[fy][fx] = 0;

      pieces[ty][tx] = fpiece + (fpiece == KING ? 0 : fpiece % 2 == 0 ? 1 : -1);
      dirs[ty][tx] = fdir;

      if (tpiece != MU) {
        hands[(fdir+2)/2][tpiece/2] += 1;
      }

      return;
    } else if (strcmp(cmd, "use") == 0) {
      int x, y;
      char pieceSym[20];
      scanf("%d%d%s", &x, &y, pieceSym);
      printf("%dx%d (%s)\n", x, y, pieceSym);

      if (x < 0 || x >= 5 || y < 0 || y >= 5) {
        printf("無効な位置です\n");
        continue;
      }

      int piece = 0;
      for (int i = 2; i < 10; i++) {
        if (strcmp(pieceSym, pieceSyms[i]) == 0) {
          piece = i;
          break;
        }
      }

      if (piece == 0) {
        printf("無効な駒名です\n");
        continue;
      }

      pieces[y][x] = piece;
      dirs[y][x] = -1;
      hands[0][piece/2] -= 1;

      return;
    }
  }
}

int tryUseHand(int depth, int fx, int fy, int piece, int dir, int alpha, int beta);
int tryAllMove(int depth, int fx, int fy, int dir, int *ok, int *tx, int *ty, int *piece, int alpha, int beta);
int alphabeta(int depth, int dir, int alpha, int beta);

#define MIN_INF -1000000
#define MAX_INF 1000000

void enemyTurn(void) {
  int max = MIN_INF, max_fx, max_fy, max_tx, max_ty, max_piece, max_use_hand = 0;

  for (int fy = 0; fy < 5; fy++) {
    for (int fx = 0; fx < 5; fx++) {
      int fpiece = pieces[fy][fx];
      int fdir = dirs[fy][fx];
      if (fdir == -1) {
        continue;
      } else if (fdir == 0) { // その位置に駒が無いとき
        for (int i = 1; i < 5; i++) {
          for (int j = 0; j < 2; j++) {
            if (hands[1][i] > 0) {
              int result = tryUseHand(0, fx, fy, i*2+j, 1, MIN_INF, MAX_INF);
              if (max < result) {
                max = result;
                max_tx = fx;
                max_ty = fy;
                max_piece = i*2+j;
                max_use_hand = 1;
              }
            }
          }
        }
      } else { // その位置に自分の駒があるとき
        int ok, tx, ty, piece;
        int result = tryAllMove(0, fx, fy, 1, &ok, &tx, &ty, &piece, MIN_INF, MAX_INF);
        if (ok && max < result) {
          max = result;
          max_fx = fx;
          max_fy = fy;
          max_tx = tx;
          max_ty = ty;
          max_piece = piece;
          max_use_hand = 0;
        }
      }
    }
  }

  if (max_use_hand) {
    printf("%dx%d (%s)\n", max_tx, max_ty, pieceSyms[max_piece]);
    hands[1][max_piece/2] -= 1;
  } else {
    printf("%dx%d -> %dx%d\n", max_fx, max_fy, max_tx, max_ty);
    pieces[max_fy][max_fx] = MU;
    dirs[max_fy][max_fx] = 0;
    int tpieces = pieces[max_ty][max_tx];
    int tdir = dirs[max_ty][max_tx];
    if (tdir == -1) {
      hands[1][tpieces/2] += 1;
    }
  }
  pieces[max_ty][max_tx] = max_piece;
  dirs[max_ty][max_tx] = 1;
}

int tryUseHand(int depth, int fx, int fy, int piece, int dir, int alpha, int beta) {
  hands[(dir+2)/2][piece/2] -= 1;
  pieces[fy][fx] = piece;
  dirs[fy][fx] = dir;
  int result = alphabeta(depth, dir == 1 ? -1 : 1, alpha, beta);
  hands[(dir+2)/2][piece/2] += 1;
  pieces[fy][fx] = MU;
  dirs[fy][fx] = 0;
  return result;
}

int canPut(int x, int y, int dir) {
  if (x < 0 || x >= 5) return 0;
  if (y < 0 || y >= 5) return 0;
  if (dirs[y][x] == dir) return 0; // 味方がいるマスには動けない
  return 1;
}

int tryPut(int depth, int x, int y, int dir, int piece, int alpha, int beta) {
  int tpiece = pieces[y][x];
  int tdir = dirs[y][x];
  pieces[y][x] = piece;
  dirs[y][x] = dir;

  int result;

  if (tdir != 0) {
    hands[(dir+2)/2][tpiece/2] += 1;
    result = alphabeta(depth, dir == 1 ? -1 : 1, alpha, beta);
    hands[(dir+2)/2][tpiece/2] -= 1;
  } else {
    result = alphabeta(depth, dir == 1 ? -1 : 1, alpha, beta);
  }

  pieces[y][x] = tpiece;
  dirs[y][x] = tdir;

  return result;
}

int tryAllMove(int depth, int fx, int fy, int dir, int *ok, int *tx, int *ty, int *piece, int alpha, int beta) {
  int fpiece = pieces[fy][fx];
  pieces[fy][fx] = MU;
  dirs[fy][fx] = 0;

#define TRY_MOVE(p, ctx, cty) { \
  int vctx = ctx; \
  int vcty = cty; \
  if (canPut(vctx, vcty, dir)) { \
    int result = tryPut(depth, vctx, vcty, dir, p, alpha, beta); \
    if (dir == 1) { \
      if (alpha < result) { \
        alpha = result; \
        *ok = 1; \
        *tx = vctx; \
        *ty = vcty; \
        *piece = p; \
      } \
    } else { \
      if (beta > result) { \
        beta = result; \
        *ok = 1; \
        *tx = vctx; \
        *ty = vcty; \
        *piece = p; \
      } \
    } \
    if (alpha >= beta) { \
      goto finish; \
    } \
  } \
}

  switch (fpiece) {
  case KING:
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dx == 0 && dy == 0) continue;
        TRY_MOVE(KING, fx + dx, fy + dy);
      }
    }
    break;
  case KYO:
    for (int i = 1; i <= 4; i++) {
      int nty = fy + i * dir;
      TRY_MOVE(TO, fx, nty);
      if (0 <= nty && nty < 5 && dirs[nty][fx] != 0) {
        break;
      }
    }
    break;
  case TO:
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if ((dx == 0 && dy == 0) || (dy == -1 && dx != 0)) continue;
        TRY_MOVE(KYO, fx + dx, fy + dy * dir);
      }
    }
    break;
  case GIN:
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dy == 0 || (dy == -1 && dx == 0)) continue;
        TRY_MOVE(KAKU, fx + dx, fy + dy * dir);
      }
    }
    break;
  case KAKU:
    /*
    for (int d = -4; d <= 4; d++) {
      if (d == 0) continue;
      TRY_MOVE(GIN, fx + d, fy + d);
      TRY_MOVE(GIN, fx + d * -1, fy + d);
    }
    */
    for (int d = -1; d >= -4; d--) {
      TRY_MOVE(GIN, fx + d, fy + d);
      if (0 <= fx + d && fx + d < 5 && 0 <= fy + d && fy + d < 5 && dirs[fy + d][fx + d] != 0) {
        break;
      }
    }
    for (int d = 1; d <= 4; d++) {
      TRY_MOVE(GIN, fx + d, fy + d);
      if (0 <= fx + d && fx + d < 5 && 0 <= fy + d && fy + d < 5 && dirs[fy + d][fx + d] != 0) {
        break;
      }
    }
    for (int d = -1; d >= -4; d--) {
      TRY_MOVE(GIN, fx - d, fy + d);
      if (0 <= fx - d && fx - d < 5 && 0 <= fy + d && fy + d < 5 && dirs[fy + d][fx - d] != 0) {
        break;
      }
    }
    for (int d = 1; d <= 4; d++) {
      TRY_MOVE(GIN, fx - d, fy + d);
      if (0 <= fx - d && fx - d < 5 && 0 <= fy + d && fy + d < 5 && dirs[fy + d][fx - d] != 0) {
        break;
      }
    }
    break;
  case KIN:
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if ((dx == 0 && dy == 0) || (dy == -1 && dx != 0)) continue;
        TRY_MOVE(KEI, fx + dx, fy + dy * dir);
      }
    }
    break;
  case KEI:
    TRY_MOVE(KIN, fx - 1, fy + 2 * dir);
    TRY_MOVE(KIN, fx + 1, fy + 2 * dir);
    break;
  case HI:
    /*
    for (int d = -4; d <= 4; d++) {
      if (d == 0) continue;
      TRY_MOVE(HU, fx + d, fy);
      TRY_MOVE(HU, fx, fy + d);
    }
    */
    for (int d = -1; d >= -4; d--) {
      TRY_MOVE(HU, fx + d, fy);
      if (0 <= fx + d && fx + d < 5 && dirs[fy][fx + d] != 0) {
        break;
      }
    }
    for (int d = 1; d <= 4; d++) {
      TRY_MOVE(HU, fx + d, fy);
      if (0 <= fx + d && fx + d < 5 && dirs[fy][fx + d] != 0) {
        break;
      }
    }
    for (int d = -1; d >= -4; d--) {
      TRY_MOVE(HU, fx, fy + d);
      if (0 <= fy + d && fy + d < 5 && dirs[fy + d][fx] != 0) {
        break;
      }
    }
    for (int d = 1; d <= 4; d++) {
      TRY_MOVE(HU, fx, fy + d);
      if (0 <= fy + d && fy + d < 5 && dirs[fy + d][fx] != 0) {
        break;
      }
    }
    break;
  case HU:
    TRY_MOVE(HI, fx, fy + dir);
    break;
  }

finish:
  pieces[fy][fx] = fpiece;
  dirs[fy][fx] = dir;
  if (dir == 1) {
    return alpha;
  } else {
    return beta;
  }
}

int evaluate(void) {
  int score = 0;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      score += dirs[y][x] * pieceScores[pieces[y][x]];
    }
  }
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 5; j++) {
      score += (i == 0 ? -1 : 1) * hands[i][j] * (pieceScores[j*2] + pieceScores[j*2+1]) / 2;
    }
  }
  return score;
}

int alphabeta(int depth, int dir, int alpha, int beta) {
  if (depth == 7 || hands[0][0] > 0 || hands[1][0] > 0) {
    int result = evaluate();
    //printf("evaluate: %d\n", result);
    return result;
  }

  for (int fy = 0; fy < 5; fy++) {
    for (int fx = 0; fx < 5; fx++) {
      int fpiece = pieces[fy][fx];
      int fdir = dirs[fy][fx];
      if (fdir != dir) {
        continue;
      } else if (fdir == 0) { // その位置に駒が無いとき
        for (int i = 1; i < 5; i++) {
          for (int j = 0; j < 2; j++) {
            if (hands[(dir+2)/2][i] > 0) {
              int result = tryUseHand(depth+1, fx, fy, i*2+j, dir, alpha, beta);
              if (dir == 1) {
                if (alpha < result) {
                  alpha = result;
                }
              } else {
                if (beta > result) {
                  beta = result;
                }
              }
              if (alpha >= beta) {
                goto finish;
              }
            }
          }
        }
      } else { // その位置に自分の駒があるとき
        int ok, tx, ty, piece;
        int result = tryAllMove(depth+1, fx, fy, dir, &ok, &tx, &ty, &piece, alpha, beta);
        if (ok) {
          if (dir == 1) {
            if (alpha < result) {
              alpha = result;
            }
          } else {
            if (beta > result) {
              beta = result;
            }
          }
          if (alpha >= beta) {
            goto finish;
          }
        }
      }
    }
  }

finish:
  if (dir == 1) {
    return alpha;
  } else {
    return beta;
  }
}

int main(int argc, char **argv) {
  dirSyms = dirSyms1 + 1;
  int dir = -1;
  if (argc > 1) {
    if (strcmp(argv[1], "-1") == 0) {
      dirSyms = dirSyms1 + 1;
      dir = -1;
    }
    if (strcmp(argv[1], "-2") == 0) {
      dirSyms = dirSyms2 + 1;
      dir = 1;
    }
  }

  displayBoard();
  for (;;) {
    if (dir == -1) {
      playerTurn();
      dir = 1;
    } else {
      enemyTurn();
      dir = -1;
    }
    displayBoard();
    if (hands[0][0] > 0) {
      printf("YOU WIN!\n");
      break;
    }
    if (hands[1][0] > 0) {
      printf("CPU WIN!\n");
      break;
    }
  }

  return 0;
}
