#include <fstream>
#include <iostream>
#include "common/coordinates_io.h"
#include "common/puzzle_board.h"
#include "common/shape_matrix_io.h"
#include "common/shape_matrix.h"
#include "common/types.h"
#include "common/point.h"
#include "discretizer/shape_rotate.h"
#include "discretizer/shape_translate.h"
#include "puzzle_solving/solver.h"

using namespace std;

int main(int argc, char** argv){ //  ./demo <input_filename>
  if (argc != 2){
    cout << "ERROR: There is not enough arguments! "<< endl;
    return 1;
  }

  /* READ FILE */
  ListOfShapes* puzzle_pieces = NULL;
  bool file_read = read_coordinates_file(argv[1], puzzle_pieces);

  if (file_read == false){
    cout << "ERROR: FILE ERROR" << endl;
    return 1;
  }

  /* SHAPE TRANSLATE MODULE */
  ListOfShapes* rotated_puzzle_pieces = NULL;
  rotate_shapes(puzzle_pieces, rotated_puzzle_pieces);

  /* DISCRETIZER MODULE */
  ListOfShapeMatrices* pieces = NULL;
  bool translate_success = shape_translate_all_shapes(rotated_puzzle_pieces, pieces);

  if (translate_success == false){
    cout << "INTERNAL ERROR: SHAPE TRANSLATE FAIL" <<endl;
    return 1;
  }

  /* SOLVER MODULE */
  int solve_success = 0;
  solve_success = puzzleSolver(pieces);

  /* Return Message */
  switch (solve_success) {
    case SOLVED:
      cout << endl;
      cout << "Puzzle is solved !!" << endl;
      break;
    case UNDERSIZED:
      cout << "Puzzle pieces cannot fit the container: potentially more pieces than required!!" << endl;
      break;
    case OVERSIZED:
      cout << "Container not fully filled: potentially less pieces than required!!" << endl;
      break;
    case UNSOLVED:
      cout << "Puzzle cannot be solved !!" << endl;
      break;
    default:
      cout << "INTERNAL ERROR: SOLVER ERROR" << endl;
  }

  delete puzzle_pieces;
  delete rotated_puzzle_pieces;
  delete pieces;

  return 0;
}
