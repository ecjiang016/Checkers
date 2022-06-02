#include "utils.h"
#include "masks.h"

class Checkers {
  private:
    U64 Bitboards [7];

    inline U64& get_bitboard(Color c, PieceType p) {
        return Bitboards[make_piece(c, p)];
    }

    int chain_move_pos; //Keeps track of the piece last moved for chained captures
                        //Set to 1 for no chain move since pieces can't end up in that position

    Flags history [100];

  public:
    Color color;
    Piece Board [64];

    template <Color Us> Move* getMoves(Move* list);

    Checkers() {
        Bitboards[WHITE_PIECE] = C64(0xAA55AA);
        Bitboards[WHITE_KING] = C64(0);
        Bitboards[BLACK_PIECE] = C64(0x55AA550000000000);
        Bitboards[BLACK_KING] = C64(0);

        for (int i = 0; i < 64; i++) {
            Board[i] = NO_PIECE;
        }

        for (int i = 0; i < 64; i++) {
            if (check_bit(Bitboards[WHITE_PIECE], i)) {
                Board[i] = WHITE_PIECE;
            }
        }

        for (int i = 0; i < 64; i++) {
            if (check_bit(Bitboards[WHITE_KING], i)) {
                Board[i] = WHITE_KING;
            }
        }

        for (int i = 0; i < 64; i++) {
            if (check_bit(Bitboards[BLACK_PIECE], i)) {
                Board[i] = BLACK_PIECE;
            }
        }

        for (int i = 0; i < 64; i++) {
            if (check_bit(Bitboards[BLACK_KING], i)) {
                Board[i] = BLACK_KING;
            }
        }

        color = WHITE;
        chain_move_pos = 1;
    }

    void makeMove(Move move) {
        U64 from_to;
        Piece capture;
        int capture_pos;
        Move list [4];

        switch (move.flags()) {
        
            case QUIET:
                from_to = single_bitboard[move.from()] | single_bitboard[move.to()];
                Bitboards[Board[move.from()]] ^= from_to;
                Board[move.to()] = Board[move.from()];
                Board[move.from()] = NO_PIECE;

                color = ~color;
                chain_move_pos = 1;
                break;

            case CAPTURE:
                from_to = single_bitboard[move.from()] | single_bitboard[move.to()];
                capture_pos = (move.to() + move.from()) >> 1;
                capture = Board[capture_pos];
                Bitboards[Board[move.from()]] ^= from_to;
                Bitboards[capture] ^= single_bitboard[capture_pos];
                Board[move.to()] = Board[move.from()];
                Board[move.from()] = NO_PIECE;
                Board[capture_pos] = NO_PIECE;

                //Check if there are any chained capture moves
                chain_move_pos = move.to();
                if (((color == WHITE ? getMoves<WHITE>(list) : getMoves<BLACK>(list)) - list) == 0) {
                    //If there are not chain moves, reset the chained move position
                    chain_move_pos = 1;
                    color = ~color;
                }
                break;

            case PROMOTION:
                Bitboards[make_piece(color, NORMAL_PIECE)] ^= single_bitboard[move.from()];
                Bitboards[make_piece(color, KING_PIECE)] ^= single_bitboard[move.to()];
                Board[move.to()] = make_piece(color, KING_PIECE);
                Board[move.from()] = NO_PIECE;

                color = ~color;
                chain_move_pos = 1;
                break;

            case CAPTURE_PROMOTION:
                capture_pos = (move.to() + move.from()) >> 1;
                capture = Board[capture_pos];
                Bitboards[make_piece(color, NORMAL_PIECE)] ^= single_bitboard[move.from()];
                Bitboards[make_piece(color, KING_PIECE)] ^= single_bitboard[move.to()];
                Bitboards[capture] ^= single_bitboard[capture_pos];
                Board[move.to()] = make_piece(color, KING_PIECE);
                Board[move.from()] = NO_PIECE;
                Board[capture_pos] = NO_PIECE;

                //Check if there are any chained capture moves
                chain_move_pos = move.to();
                if (((color == WHITE ? getMoves<WHITE>(list) : getMoves<BLACK>(list)) - list) == 0) {
                    //If there are not chain moves, reset the chained move position
                    chain_move_pos = 1;
                    color = ~color;
                }
                break;
                
        }

        void undoMove(Move move) {

        }

    }

};

template <Color Us>
Move* Checkers::getMoves(Move* list) {
    const U64 occupancy_us = get_bitboard(Us, NORMAL_PIECE) | get_bitboard(Us, KING_PIECE);
    const U64 occupancy_them = get_bitboard(~Us, NORMAL_PIECE) | get_bitboard(~Us, KING_PIECE);
    const U64 occupancy = occupancy_us | occupancy_them;

    if (chain_move_pos == 1) {
        if (Us == WHITE) {
            U64 quiet_left_forward = ((occupancy_us & ~LEFT_COLUMN & ~TOP_ROW) << 7);
            U64 capture_left_forward = ((quiet_left_forward & occupancy_them & ~LEFT_COLUMN & ~TOP_ROW) << 7) & ~occupancy;
            quiet_left_forward &= ~occupancy;
            U64 promotion_left_forward = quiet_left_forward & TOP_ROW;
            quiet_left_forward &= ~TOP_ROW;
            U64 promotion_capture_left_forward = capture_left_forward & TOP_ROW & ~get_bitboard(Us, KING_PIECE);

            U64 quiet_right_forward = ((occupancy_us & ~RIGHT_COLUMN & ~TOP_ROW) << 9);
            U64 capture_right_forward = ((quiet_right_forward & occupancy_them & ~RIGHT_COLUMN & ~TOP_ROW) << 9) & ~occupancy;
            quiet_right_forward &= ~occupancy;
            U64 promotion_right_forward = quiet_right_forward & TOP_ROW;
            quiet_right_forward &= ~TOP_ROW;
            U64 promotion_capture_right_forward = capture_right_forward & TOP_ROW & ~get_bitboard(Us, KING_PIECE);

            //For kings who can move backwards
            U64 quiet_left_backward = ((get_bitboard(Us, KING_PIECE) & ~LEFT_COLUMN & ~BOTTOW_ROW) >> 9);
            U64 capture_left_backward = ((quiet_left_backward & occupancy_them & ~LEFT_COLUMN & ~BOTTOW_ROW) >> 9) & ~occupancy;
            quiet_left_backward &= ~occupancy;

            U64 quiet_right_backward = ((get_bitboard(Us, KING_PIECE) & ~RIGHT_COLUMN & ~BOTTOW_ROW) >> 7);
            U64 capture_right_backward = ((quiet_right_backward & occupancy_them & ~RIGHT_COLUMN & ~BOTTOW_ROW) >> 7) & ~occupancy;
            quiet_right_backward &= ~occupancy;


            if (!(capture_left_forward | capture_right_forward | capture_left_backward | capture_right_backward)) {
                //Forces captures when captures are possible

                if (quiet_left_forward) do {
                    int to = bitScanForward(quiet_left_forward);
                    int from = to - 7;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_left_forward &= quiet_left_forward-1);

                if (quiet_right_forward) do {
                    int to = bitScanForward(quiet_right_forward);
                    int from = to - 9;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_right_forward &= quiet_right_forward-1);

                if (quiet_left_backward) do {
                    int to = bitScanForward(quiet_left_backward);
                    int from = to + 9;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_left_backward &= quiet_left_backward-1);

                if (quiet_right_backward) do {
                    int to = bitScanForward(quiet_right_backward);
                    int from = to + 7;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_right_backward &= quiet_right_backward-1);

                if (promotion_left_forward) do {
                    int to = bitScanForward(promotion_left_forward);
                    int from = to - 7;
                    *list++ = Move(from, to, PROMOTION);
                } while (promotion_left_forward &= promotion_left_forward-1);

                if (promotion_right_forward) do {
                    int to = bitScanForward(promotion_right_forward);
                    int from = to - 9;
                    *list++ = Move(from, to, PROMOTION);
                } while (promotion_right_forward &= promotion_right_forward-1);

            } else {

                capture_left_forward &= ~TOP_ROW;
                capture_right_forward &= ~TOP_ROW;

                if (capture_left_forward) do {
                    int to = bitScanForward(capture_left_forward);
                    int from = to - 14;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_left_forward &= capture_left_forward-1);

                if (capture_right_forward) do {
                    int to = bitScanForward(capture_right_forward);
                    int from = to - 18;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_right_forward &= capture_right_forward-1);

                if (promotion_capture_left_forward) do {
                    int to = bitScanForward(promotion_capture_left_forward);
                    int from = to - 14;
                    *list++ = Move(from, to, CAPTURE_PROMOTION);
                } while (promotion_capture_left_forward &= promotion_capture_left_forward-1);

                if (promotion_capture_right_forward) do {
                    int to = bitScanForward(promotion_capture_right_forward);
                    int from = to - 18;
                    *list++ = Move(from, to, CAPTURE_PROMOTION);
                } while (promotion_capture_right_forward &= promotion_capture_right_forward-1);

                if (capture_left_backward) do {
                    int to = bitScanForward(capture_left_backward);
                    int from = to + 18;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_left_backward &= capture_left_backward-1);

                if (capture_right_backward) do {
                    int to = bitScanForward(capture_right_backward);
                    int from = to + 14;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_right_backward &= capture_right_backward-1);

            }

        } else {
            U64 quiet_left_forward = ((occupancy_us & ~LEFT_COLUMN & ~BOTTOW_ROW) >> 9);
            U64 capture_left_forward = ((quiet_left_forward & occupancy_them & ~LEFT_COLUMN & ~BOTTOW_ROW) >> 9) & ~occupancy;
            quiet_left_forward &= ~occupancy;
            U64 promotion_left_forward = quiet_left_forward & BOTTOW_ROW;
            quiet_left_forward &= ~BOTTOW_ROW;
            U64 promotion_capture_left_forward = capture_left_forward & TOP_ROW & ~get_bitboard(Us, KING_PIECE);

            U64 quiet_right_forward = ((occupancy_us & ~RIGHT_COLUMN & ~BOTTOW_ROW) >> 7);
            U64 capture_right_forward = ((quiet_right_forward & occupancy_them & ~RIGHT_COLUMN & ~BOTTOW_ROW) >> 7) & ~occupancy;
            quiet_right_forward &= ~occupancy;
            U64 promotion_right_forward = quiet_right_forward & BOTTOW_ROW;
            quiet_right_forward &= ~BOTTOW_ROW;
            U64 promotion_capture_right_forward = capture_right_forward & BOTTOW_ROW & ~get_bitboard(Us, KING_PIECE);

            //For kings who can move backwards
            U64 quiet_left_backward = ((get_bitboard(Us, KING_PIECE) & ~LEFT_COLUMN & ~TOP_ROW) << 7);
            U64 capture_left_backward = ((quiet_left_backward & occupancy_them & ~LEFT_COLUMN & ~TOP_ROW) << 7) & ~occupancy;
            quiet_left_backward &= ~occupancy;

            U64 quiet_right_backward = ((get_bitboard(Us, KING_PIECE) & ~RIGHT_COLUMN & ~TOP_ROW) << 9);
            U64 capture_right_backward = ((quiet_right_backward & occupancy_them & ~RIGHT_COLUMN & ~TOP_ROW) << 9) & ~occupancy;
            quiet_right_backward &= ~occupancy;

            if (!(capture_left_forward | capture_right_forward | capture_left_backward | capture_right_backward)) {
                //Forces captures when captures are possible
                if (quiet_left_forward) do {
                    int to = bitScanForward(quiet_left_forward);
                    int from = to + 9;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_left_forward &= quiet_left_forward-1);

                if (quiet_right_forward) do {
                    int to = bitScanForward(quiet_right_forward);
                    int from = to + 7;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_right_forward &= quiet_right_forward-1);

                if (quiet_left_backward) do {
                    int to = bitScanForward(quiet_left_backward);
                    int from = to - 7;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_left_backward &= quiet_left_backward-1);

                if (quiet_right_backward) do {
                    int to = bitScanForward(quiet_right_backward);
                    int from = to - 9;
                    *list++ = Move(from, to, QUIET);
                } while (quiet_right_backward &= quiet_right_backward-1);

                if (promotion_left_forward) do {
                    int to = bitScanForward(promotion_left_forward);
                    int from = to + 9;
                    *list++ = Move(from, to, PROMOTION);
                } while (promotion_left_forward &= promotion_left_forward-1);

                if (promotion_right_forward) do {
                    int to = bitScanForward(promotion_right_forward);
                    int from = to + 7;
                    *list++ = Move(from, to, PROMOTION);
                } while (promotion_right_forward &= promotion_right_forward-1);

            } else {
                //Mask out promotion moves for them to be handled seperately
                capture_left_forward &= ~BOTTOW_ROW;
                capture_right_forward &= ~BOTTOW_ROW;

                if (capture_left_forward) do {
                    int to = bitScanForward(capture_left_forward);
                    int from = to + 18;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_left_forward &= capture_left_forward-1);

                if (capture_right_forward) do {
                    int to = bitScanForward(capture_right_forward);
                    int from = to + 14;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_right_forward &= capture_right_forward-1);

                if (promotion_capture_left_forward) do {
                    int to = bitScanForward(promotion_capture_left_forward);
                    int from = to + 18;
                    *list++ = Move(from, to, CAPTURE_PROMOTION);
                } while (promotion_capture_left_forward &= promotion_capture_left_forward-1);

                if (promotion_capture_right_forward) do {
                    int to = bitScanForward(promotion_capture_right_forward);
                    int from = to + 14;
                    *list++ = Move(from, to, CAPTURE_PROMOTION);
                } while (promotion_capture_right_forward &= promotion_capture_right_forward-1);

                if (capture_left_backward) do {
                    int to = bitScanForward(capture_left_backward);
                    int from = to - 14;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_left_backward &= capture_left_backward-1);

                if (capture_right_backward) do {
                    int to = bitScanForward(capture_right_backward);
                    int from = to - 18;
                    *list++ = Move(from, to, CAPTURE);
                } while (capture_right_backward &= capture_right_backward-1);
            }
        }

    } else if (check_bit(get_bitboard(Us, NORMAL_PIECE) | get_bitboard(Us, KING_PIECE), chain_move_pos)) {
        //AHHHHHHHHHHH
        //Chain captures cause of course it can't just be easy
        U64 piece_bb = single_bitboard[chain_move_pos];

        if (Us == WHITE) {
            U64 moves = (((((piece_bb & ~LEFT_TWO & ~TOP_TWO) << 7) & occupancy_them) << 7) & ~occupancy) | //Left forward capture
                        (((((piece_bb & ~RIGHT_TWO & ~TOP_TWO) << 9) & occupancy_them) << 9) & ~occupancy); //Right forward capture

                if (check_bit(get_bitboard(Us, KING_PIECE), chain_move_pos)) {
                    //King only moves
                    moves |= (((((piece_bb & ~LEFT_TWO & ~BOTTOW_TWO) >> 9) & occupancy_them) >> 9) & ~occupancy) | //Left backward capture
                             (((((piece_bb & ~RIGHT_TWO & ~BOTTOW_TWO) >> 7) & occupancy_them) >> 7) & ~occupancy); //Right backward capture
                }

            if (moves) do {
                if (!(TOP_ROW & get_bitboard(Us, KING_PIECE) & moves & -moves)) {
                    //Is not a promotion
                    *list++ = Move(chain_move_pos, bitScanForward(moves), CAPTURE);
                } else {
                    *list++ = Move(chain_move_pos, bitScanForward(moves), CAPTURE_PROMOTION);
                }

            } while (moves &= moves-1);

        } else {
            U64 moves = (((((piece_bb & ~LEFT_TWO & ~BOTTOW_TWO) >> 9) & occupancy_them) >> 9) & ~occupancy) | //Left backward capture
                        (((((piece_bb & ~RIGHT_TWO & ~BOTTOW_TWO) >> 7) & occupancy_them) >> 7) & ~occupancy); //Right backward capture

                if (check_bit(get_bitboard(Us, KING_PIECE), chain_move_pos)) {
                    //King only moves
                    moves |= (((((piece_bb & ~LEFT_TWO & ~TOP_TWO) << 7) & occupancy_them) << 7) & ~occupancy) | //Left forward capture
                             (((((piece_bb & ~RIGHT_TWO & ~TOP_TWO) << 9) & occupancy_them) << 9) & ~occupancy); //Right forward capture
                }

            if (moves) do {
                if (!(TOP_ROW & get_bitboard(Us, KING_PIECE) & moves & -moves)) {
                    //Is not a promotion
                    *list++ = Move(chain_move_pos, bitScanForward(moves), CAPTURE);
                } else {
                    *list++ = Move(chain_move_pos, bitScanForward(moves), CAPTURE_PROMOTION);
                }

            } while (moves &= moves-1);
        }

    }

    return list;
    
}

struct MoveList {
  private:
    //Move list [96]; //Using the upper bound of maximum moves being 12 pieces * 4 directions * 2 (regular move or capture)
    Move list [256];
    Move* last;
  public:
    MoveList(Checkers& game) {
        last = game.color == WHITE ? game.getMoves<WHITE>(list) : game.getMoves<BLACK>(list);
    }

    Move* begin() {
        return list;
    }

    Move* end() {
        return last;
    }

    size_t size() {
        return last - list;
    }

    Move element(int i) {
        return list[i];
    }
};