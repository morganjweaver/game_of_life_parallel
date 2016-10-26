// CPSC 4600 / 5600 - Life
//
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
#include "tbb/blocked_range2d.h"
#include "tbb/tbb.h"
using namespace tbb;
class Matrix
{

public: 
    Matrix(int _rows, int _cols) {
    rows = _rows;
    cols = _cols;	
    Arr = new int[rows*cols];
    }
    int& operator() (int i, int j) {
      return Arr[i*cols + j];
    
}

void printMatrix(ofstream &out){
  for (int l = 0; l < rows; l++){
    for (int m = 0; m < cols; m++)
       { out << (*this)(l,m) << " "; }
    out << endl;
  }
}
int live_neighbors(int i, int j)
{
//check the entire 8-neighbor grid from upper left to lower right order:
//initialize sptial coordinate
  int lives = 0;
  int up = i - 1; //i value
  int left = j - 1; //j value
  int right = j + 1; //j
  int lower = i + 1; //i
//check and adjust edges:
  if (up < 0)
    up = rows - 1;
  if (lower >= rows)
    lower = 0;
  if (left < 0)
    left = rows - 1;
  if (right >= cols)
    right = 0;
  if (Arr[up*cols + left] == 1) //FIX MATRIX COORDINATE COMPARISONS
    lives++;
  if (Arr[up*cols + j] == 1)
    lives++;
  if (Arr[up*cols + right] == 1)
    lives++;
  if (Arr[i*cols + left] == 1)
    lives++;
  if (Arr[i*cols + right] == 1)
    lives++;
  if (Arr[lower*cols + left] == 1)
    lives++;
  if (Arr[lower*cols + j] == 1)
    lives++;
  if (Arr[lower*cols + right] == 1)
    lives++;
  return lives;
}
private:
  int* Arr;
  int rows;
  int cols;

friend class Life_Iterate; //let's be friends!
};

class Life_Iterate {
  Matrix &a;
  Matrix &b;
public:
  Life_Iterate(Matrix &_a, Matrix &_b) :
  a(_a), b(_b) {}
  void operator() (const blocked_range2d<int> &r) const {
  for (int i = r.rows().begin(); i != r.rows().end(); i++) {
    for (int j = r.cols().begin(); j != r.cols().end(); j++) {
      if (a(i,j) == 1 ){ //If cell is alive and lonely or overcrowded
        if ((a.live_neighbors(i,j) < 2) || a.live_neighbors(i, j) > 3){ //IMPLEMENT CELL LIVE/DIE STATE HERE
        b(i, j) = 0;
        }
        else { b(i,j) = a(i,j);}
        }
      else if ((a(i,j) == 0) && (a.live_neighbors(i,j) == 3)){
        b(i, j) = 1;
      }
      else {
	b(i,j) = a(i,j); }  //else do nothing.  else states: live with 2-3 neighbors goes unchanged
    }
  }
 } 
};

int main(int argc, char *argv[])
{
  if (argc != 3) {
    cerr << "Invalid command line - usage: <input file> <number of threads>" << endl;
    exit(-1);
  }

  // Extract parameters
  ifstream ifile(argv[1]);
  int num_threads = atoi(argv[2]);

  // Set the number of threads
  task_scheduler_init init(num_threads);

  // Get the size of the problem - on the first line of the input file.
  int size;
  ifile >> size;

  // X TODO: Create and initialize data structures
  Matrix first(size, size);
  Matrix second(size, size);
  for (int i = 0; i < size; i++){
    for (int j = 0; j < size; j++)
       { ifile >> first(i,j);} 
  }
         
  // Start the timer
  tick_count start = tick_count::now();

 ofstream outfile("output.txt");
 for (int k = 0; k < 10; k++)
 {
   if (k % 2 == 0) {
      parallel_for(blocked_range2d<int>(0, size, 0, size),
	      Life_Iterate(first, second));
    }
    else {
  parallel_for(blocked_range2d<int>(0, size, 0, size),
	     Life_Iterate(second, first));
  }
  }
  
first.printMatrix(outfile);
// Stop the timer
  tick_count end = tick_count::now();
  double run_time = (end-start).seconds();
  cout << "Time: " << (end-start).seconds() << endl;



  outfile.close();

  // Append the peformance results to the results file.
  ofstream ofile("life.csv", ios::app);
  ofile << size << "," << num_threads << "," << run_time << endl;
  ofile.close();

  return 0;
}
