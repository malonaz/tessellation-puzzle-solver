#include <iostream>
#include "types.h"

using std::ifstream;

bool read_coordinates_file(const char* file, ListOfShapes* list) {
  assert(list != NULL);
  ifstream input_file(file);
  if (input_file.fail()) {
    return false;
  }

  while (!input_file.eof()) {
    int num_points;
    input_file >> num_points;
    ListOfPoints* points = new ListOfPoints();
    for (int i = 0; i < num_points; ++i) {
      int x, y;
      input_file >> x;
      input_file >> y;
      points->push_back(Point(x, y));
    }
    list->push_back(points);
  }

  return true;
}
