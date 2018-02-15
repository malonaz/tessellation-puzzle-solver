#include <iostream>
#include <vector>
#include <cmath>
#include <time.h> //may be removed later
#include <fstream>
#include "common/debugger.h"
#include "common/memory.h"
#include "common/puzzle_board.h"
#include "common/shape_matrix.h"
#include "common/shape_matrix_io.h"
#include "common/types.h"
#include "solver.h"
#include <string>
#include <fstream>
#include <thread>


using namespace std;

PuzzleBoard* createBoard(const vector<ShapeMatrix> &matrices,
    vector<ShapeMatrix> &pieces, int& containerArea, int& totalPieceArea) {
  int maxArea = matrices[0].getShapeArea();
  int maxAreaIdx = 0;
  int accumArea = 0;
  int matricesSize = (int)matrices.size();

  for (int i = 1; i < matricesSize; i++) {
    int tempArea = matrices[i].getShapeArea();
    int pushIdx = i;
    if (tempArea > maxArea) {
      //places the previous largest piece into the puzzle pieces vector
      pushIdx = maxAreaIdx;
      accumArea += maxArea;
      maxArea = tempArea;
      maxAreaIdx = i;
      accumArea -= tempArea;
    }
    accumArea += tempArea;
    pieces.push_back(matrices[pushIdx]);
  }
  ShapeMatrix container = matrices[maxAreaIdx];
  PuzzleBoard* board = new PuzzleBoard(container);
  containerArea = maxArea; // return container area as a reference
  totalPieceArea = accumArea; // return total area of puzzle pieces
  return board;
}

bool isShapeMatrixInList(const ShapeMatrix &shape,
    const vector<ShapeMatrix*> &list) {
  bool result = false;
  for (uint j = 0; j < list.size(); j++) {
    if (shape == *(list[j])) {
      result = true;
      break;
    }
  }
  return result;
}

/*Function creates a list containing the unique orientations of each shape temp*/
vector<ShapeMatrix*>* combinations(const ShapeMatrix &temp) {
  vector<ShapeMatrix*>* combi = new vector<ShapeMatrix*>();
  ShapeMatrix* r_temp = new ShapeMatrix(temp);

  for (uint i = 0; i < 8; i++) {
    if (!isShapeMatrixInList(*r_temp, *combi)) {
      combi->push_back(r_temp);
    }

    r_temp = r_temp->rotate();
    if (i == 3) {
      r_temp = r_temp->mirror();
    }
  }
  return combi;
}

/* function to get an adjacent block of empty cells */
int getAdjacentEmptyArea(int r, int c, uint height, uint width, int** copiedBoard) {
  if (r < 0 ||c < 0){
    return 0;
  }

  if (r >= (int)height || c >= (int)width) {
    return 0;
  }

  if (copiedBoard[r][c] != 0) {
    return 0;
  }

  copiedBoard[r][c] = -1;
  return 1 + getAdjacentEmptyArea(r, c + 1, height, width, copiedBoard)
    + getAdjacentEmptyArea(r, c - 1, height, width,copiedBoard)
    + getAdjacentEmptyArea(r + 1, c,height, width, copiedBoard)
    + getAdjacentEmptyArea(r - 1, c,height, width, copiedBoard);
}

/* function to generate all possible area combinations from remaining pieces */
void generatePossibleAreas(int* answerArray,
    long int maxCombinations,
    const vector<ShapeMatrix> &pieces,
    bool* currentPiecesInventory) {


  int sizeArray = 0;
  for (int i = 0; i < pieces.size(); i++) {
    if (currentPiecesInventory[i]) {
      sizeArray++;
    }
  }


  int* generativeArray = new int[sizeArray];
  for (long int sequencei = 0; sequencei < maxCombinations; sequencei++) {
    int answer=0;
    long int copyi = sequencei;
    for (int countj = sizeArray-1; countj >= 0; countj--) {
      generativeArray[countj] = copyi % 2 ;
      copyi /= 2;
    }

    for (int countk = 0; countk < sizeArray; countk++) {
      if (generativeArray[countk]) {
        //countk here means the countk'th item on the boolean array thats true,
        //for eg countk = 3, array is 000101100. searchCount should be 3, countNum shld be 6
        int searchCount=0;
        int countNum = 0;
        while ((searchCount < (countk+1)) && (countNum < pieces.size())) {
          if (currentPiecesInventory[countNum]) {
            searchCount++;
          }
          countNum++;
        }
        answer += pieces[(countNum-1)].getShapeArea(); // size instead}
      }
    }

    answerArray[sequencei] = answer;

    for (int countl = 0; countl < sizeArray; countl++) {
      generativeArray[countl] = 0;
    }
  }
  delete[] generativeArray;
}

/* helper function to copy current state of board into 2D array */
int** copyBoard(PuzzleBoard* const board) {
  uint height = board->getHeight();
  uint width = board->getWidth();
  int** copiedBoard = new int*[height];
  for (uint i = 0; i < height; i++) {
      copiedBoard[i] = new int[width];
  }
  for (uint i = 0; i < height; i++) {
    for (uint j = 0; j < width; j++ ) {
      copiedBoard[i][j]= (board->getCurrentBoard())[i][j];
    }
  }
  return copiedBoard;
}

/* helper function to delete dynamically allocated 2D array */
void deleteCopy(uint height, int** copyBoard) {
  for (uint i = 0; i < height; i++){
    delete[] copyBoard[i];
  }
  delete[] copyBoard;
}

/* pruning function: scans through board for every adjacent block of empty
cells and checks if a combination of the remaining pieces can be fit into each empty block */
bool** solvableConfig(PuzzleBoard* board,
    const vector<ShapeMatrix> &pieces,
    bool* currentPiecesInventory,
    bool& solvable,
    int& possibleCombis) {
  uint b_height = board->getHeight();
  uint b_width = board->getWidth();


  //preparation to generate an array all possible area combinations.
  int numRemainingPieces = 0;

  for (int i = 0; i < pieces.size(); i++) {
    cout<<"number is "<<currentPiecesInventory[i]<<endl;
    if (currentPiecesInventory[i]) {
      numRemainingPieces++;
    }
  }
  cout<<endl<<endl;

  long int maxCombinations = pow(2, numRemainingPieces);
  int* answerArray = new int[maxCombinations];
  /*helper function which returns an int array of all possible areas that
  can be formed with the remaining pieces*/
  generatePossibleAreas(answerArray, maxCombinations, pieces, currentPiecesInventory);

  int** copiedBoard = copyBoard(board);

  for (uint r = 0; r < b_height; r++) {
    for (uint c = 0; c < b_width; c++) {
      //GETTING ADJACENT AREA OF CURRENT SLOT
      int area = getAdjacentEmptyArea(r, c, b_height, b_width, copiedBoard);
      bool areaImpossible = true;

      //IF AREA IS ZERO, JUST BREAK OUT OF THIS FOR LOOP
      if (area == 0) {
        break;
      }

      for (long int sequencei = 0; sequencei < maxCombinations; sequencei++) {
        //cout<< "for area: "<<area<<". Sequencei is "<<sequencei<<", and ansArray[seqi] is "<<answerArray[sequencei]<<endl;
        if (area == answerArray[sequencei]) {
          areaImpossible = false;
          break; //BREAK OUT OF SEARCH LOOP IF POSSIBLE COMBINATION OF SHAPES FOUND
        }
      }

      if (areaImpossible) {
        deleteCopy(b_height, copiedBoard);
        delete[] answerArray;
        solvable = false;
        return nullptr;
      }
    }
  }

  //Make bool[][] array. THIS IS A DUMMY PLACEHOLDER\ AND NEEDS TO BE DESTROYED LATER
  possibleCombis = 1;
  bool** tempConfig = new bool*[possibleCombis];
  for (uint i = 0; i < possibleCombis; i++) {
      tempConfig[i] = new bool[pieces.size()];
  }
  for (uint i = 0; i < possibleCombis; i++) {
    for (uint j = 0; j < pieces.size(); j++ ) {
      tempConfig[i][j] = currentPiecesInventory[j]; //THIS NEEDS TO BE AMENDED, UNCONVERT THE ANSWERS
    }
  }

  deleteCopy(b_height, copiedBoard);

  delete[] answerArray;
  solvable = true;

  return tempConfig;
}

/* function for recursive solving */
bool recursiveSolver (PuzzleBoard* board,
    const vector<ShapeMatrix> pieces,
    bool* currentPiecesInventory, /*NOTE:: the binary code for currently available pieces*/
    int& iterations,
    int& solutionNum,
    time_t start,
    string folderName) {
  iterations++;
  uint height = board->getHeight();
  uint width = board->getWidth();
  /*base case: if the board is already fully filled*/
  if (board->getRemainingArea() == 0) { //} || currentIndex >= pieces.size()) {
    //the board is complete, and no more remaining pieces
    solutionNum++;
    time_t elapsed = time(0) - start;
    std::cout<<"Solution number "<<solutionNum;
    std::cout<<". After "<< iterations<<" number of iterations. ";
    std::cout<<"Elapsed time: " << elapsed<<" seconds."<<std::endl;
    int** board_solution = copyBoard(board);

    string strbase1("/Solution ");
    string strbase2(".txt");
    ofstream output(folderName+strbase1+ to_string(solutionNum)+strbase2);// to_string() need c++11
    for (uint i = 0; i<height; i++){
      for (uint j = 0; j<width; j++){
        output<<board_solution[i][j]<<"\t";
      }
      output<<endl;
    }

    output.close();
    return true;
  }

  /*pruning function: checks if there are "unsolvable empty blocks" on the board
  for example, if there is an empty block of 3-unit squares, but there is no way
  we can form an area of 3 using any combination of the remaining pieces.
  NEW 1a) If everything is 0, mark out new smallest area.
  NEW 2) Search within defined smallest area whether there is any new smallest area
  NEW 3) Check whether there is a subset within the current subset to solve the smallest area
  */
  bool solvable = false;
  int possibleCombis = 0;
  cout<<"HERE 1"<<endl;
  bool** newPiecesInventory = solvableConfig(board, pieces, currentPiecesInventory, solvable, possibleCombis);
  cout<<"HERE 2"<<endl;
  if (!solvable) { //NOTE:: NEEED TO FIND A COMBINATION OF INVENTORY ARRAYS should output current smallest empty area, and the confined possible
    int** board_solution = copyBoard(board);

    string strbase1("/UnsolvableConfig ");
    string strbase2(".txt");
    ofstream output(folderName+strbase1+ to_string(solutionNum)+strbase2);// to_string() need c++11
    for (uint i = 0; i<height; i++){
      for (uint j = 0; j<width; j++){
        output<<board_solution[i][j]<<"\t";
      }
      output<<endl;
    }
    return false;
  }

  /*For the current piece that we are trying to place into the board,
  1) first try to find a suitable location in the board that we might be able to
  place it, 2) try different orientations
  */
  for (int testInventoryCount=0; testInventoryCount< possibleCombis; testInventoryCount) {
    //NOTE:: bool array piecesInventory find the first available piece within the
    bool firstNotFound = true;

    bool nextPiecesInventory[(pieces.size())] = {false};
    int currentIndex = 0;
    //finds first available piece within the current inventory that we are using
    for (int firstPieceFind = 0; firstPieceFind < pieces.size(); firstPieceFind++){
      //copying nextPiecesInventory, which will be used as next recursive call in the meanwhile
      nextPiecesInventory[firstPieceFind]= newPiecesInventory[testInventoryCount][firstPieceFind];
      //cout<<"For newPiecesInventory: "<<newPiecesInventory[firstPieceFind]<<endl;
      if (firstNotFound && newPiecesInventory[testInventoryCount][firstPieceFind]){
        currentIndex = firstPieceFind;
        firstNotFound = false;
        nextPiecesInventory[firstPieceFind] = 0;
      }
    }

    for (int i = 0; i < pieces.size(); i++) {
      //cout<<"For nextPiecesInventory: "<<nextPiecesInventory[i]<<endl;
    }

    //Extract the current piece that we will try to place, and find the different
    //orientations for that shape
    ShapeMatrix temp = pieces[currentIndex];
    vector<ShapeMatrix*>* shapesList = combinations(temp);
    int shapeAppear = currentIndex + 1;

    //try 1) by row and column, 2) by shape orientation
    for (uint r = 0; r < height; r++) {
      for (uint c = 0; c < width; c++) {
        //try differnet orientations of the same shapeSolution
        for (uint counteri = 0; counteri < shapesList->size(); counteri++) {
          ShapeMatrix* r_temp = (*shapesList)[counteri];
            cout<<"here before place piece rc"<<r<<c<<"; shapematrix, "<< shapeAppear<< "orientation: "<< counteri<<endl;
              int** board_solution = copyBoard(board);

          if (board->placePiece(c, r, shapeAppear, *r_temp)) {
            cout<<"HERE AFTER PLACE PIECCEC"<<endl;
            if (recursiveSolver(board, pieces, nextPiecesInventory, iterations, solutionNum, start, folderName)) {
            //TO CHANGE: SAVE THIS SOLUTION AND FIND NEXT, INSTEAD OF RETURN

            //return true;
            }
            print_solution_board(board_solution, height, width);
            deleteCopy(height, board_solution);
            board->removePiece(c, r, shapeAppear, *r_temp); // revert
          }
        }
      }
    }

    delete nextPiecesInventory;
    cleanup_list(shapesList);
  }
  delete [] newPiecesInventory;
  int** board_solution = copyBoard(board);

  string strbase1("/Iterations_failed ");
  string strbase2(".txt");
  ofstream output(folderName+strbase1+ to_string(solutionNum)+strbase2);
  for (uint i = 0; i<height; i++){
    for (uint j = 0; j<width; j++){
      output<<board_solution[i][j]<<"\t";
    }
    output<<endl;
  }


  return false;
}

int accum = 0;

int square(int x) {
  accum += x * x;
  return 0;
}

int** puzzleSolver(const vector<ShapeMatrix> &matrices, int& returnCode,
      uint& board_height, uint& board_width, string folderName) {
  returnCode = 0;
  vector<ShapeMatrix> shapes;
  int** board_solution = NULL;
  int containerArea = 0;
  int totalPieceArea = 0;
  PuzzleBoard* board = createBoard(matrices, shapes,
      containerArea, totalPieceArea);
  if (totalPieceArea > containerArea) { // case of undersized container
    returnCode = UNDERSIZED;
    return board_solution;
  }
  if (totalPieceArea < containerArea) { // case of oversized container
    returnCode = OVERSIZED;
    return board_solution;
  }
  // if puzzle pieces area == container area

/*********************EXPERIMENTING THREADS**********************/
  vector<thread> ths;
  for (int i = 1; i <= 20; i++) {
      ths.push_back(thread(&square, i));
  }

  for (auto& th : ths) {
      th.join();
  }
  cout << "accum = " << accum << endl;
/***********************************************************/

  //beginning solver
  int iterations =0;
  int solutionNum = 0;
  time_t start = time(0);
  bool currentPiecesInventory[shapes.size()];
  for (int i =0; i < shapes.size(); i++) {
    currentPiecesInventory[i]=true;

  }
  bool success = recursiveSolver(board, shapes, currentPiecesInventory, iterations, solutionNum, start, folderName);

  if (success) {
    returnCode = SOLVED;
    board_height = board->getHeight(); //returns height of board
    board_width = board->getWidth(); // returns width of board
    board_solution = copyBoard(board); // returns a 2D int array of board (w soln)
  } else {
    returnCode = UNSOLVED;
  }
  delete board;
  return board_solution;
}
