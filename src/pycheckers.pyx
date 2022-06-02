# distutils: language = c++

from cython.operator cimport dereference
import numpy as np

def coords_2D_to_1D(x, y):
    return (7-y)*8 + x

cdef extern from "./utils.h":
    enum Color:
        WHITE = 0,
        BLACK = 1

    enum PieceType:
        NORMAL_PIECE = 1,
        KING_PIECE = 2

    enum Piece:
        NO_PIECE = 0,
        WHITE_PIECE = 1,
        WHITE_KING = 2,
        BLACK_PIECE = 5,
        BLACK_KING = 6 

    enum Flags:
        QUIET = 0,
        CAPTURE = 0x1000,
        PROMOTION = 0x2000,
        CAPTURE_PROMOTION = 0x3000

    cdef cppclass Move:
        Move() except +
        Move(int from_, int to) except +
        Move(int from_, int to, Flags flags) except +
        int from_ "from" ()
        int to()

cdef extern from "./engine.cpp": #Stuff from game.cpp, just grabbing them from engine.cpp instead since it's already included there
    cdef cppclass Checkers:
        Checkers() except +
        Color color
        Piece Board[64]
        void makeMove(Move move)

    cdef cppclass MoveList:
        MoveList(Checkers& game) except +
        Move* begin()
        Move* end()
        int size()
        Move element(int i)

    cdef cppclass engine:
        engine() except +
        Move search(int depth, Checkers game)

cdef class pyCheckers:
    cdef Checkers *cobj
    cdef engine *cengine
    def __cinit__(self):
        self.cobj = new Checkers()
        self.cengine = new engine()

    property color:
        def __get__(self):
            if self.cobj.color == WHITE:
                return 1
            else:
                return 0

    def engineMove(self, depth=5): #Lets the engine make a move
        cdef Move best_move = self.cengine.search(depth, dereference(self.cobj))
        self.cobj.makeMove(best_move)

    def PyMoveList(self): #Returns the move list as a python list
        py_list = []
        cdef MoveList* c_list = new MoveList(dereference(self.cobj))
        cdef int size = c_list.size()   
        cdef int i
        cdef Move move

        for i in range(size):
            move = c_list.element(i)
            py_list.append((move.from_(), move.to()))

        return py_list

    def gameBoard(self):
        cdef int [:] board = np.zeros(64, dtype="int32")
        cdef Piece piece
        cdef int i

        for i in range(64):
            piece = self.cobj.Board[i]
            if piece == NO_PIECE:
                continue
            elif piece == WHITE_PIECE:
                board[i] = 1
            elif piece == BLACK_PIECE:
                board[i] = -1
            elif piece == WHITE_KING:
                board[i] = 2
            elif piece == BLACK_KING:
                board[i] = -2

        return np.flipud(np.array(board).reshape(8, 8))
        
    def pieceMoves(self, x, y):
        piece_coord = coords_2D_to_1D(x, y)
        moves = []
        for move in self.PyMoveList():
            from_, to = move
            if piece_coord == from_:
                moves.append(to)

        return moves

    def makeMove(self, from_x, from_y, to_x, to_y):
        cdef int from_ = coords_2D_to_1D(from_x, from_y)
        cdef int to = coords_2D_to_1D(to_x, to_y)

        cdef MoveList* c_list = new MoveList(dereference(self.cobj))
        cdef int size = c_list.size()   
        cdef int i
        cdef Move move

        for i in range(size+1):
            if i == size:
                raise Exception("Couldn't find move")
            move = c_list.element(i)
            if (move.from_() == from_) and (move.to() == to):
                break

        self.cobj.makeMove(move)
        