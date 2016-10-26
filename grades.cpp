// CPSC 4600 / 5600 - Grades (map, reduce, scan)
//
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "tbb/tbb.h"
using namespace tbb;

// TODO: Complete these six functions:
//
void
adjust_seq(int *src, int *dest, int size)
{
  for ( int i = 0; i < size; i++){
    if (src[i] >= 95)
	{
		dest[i] = 100;
	}
    else { dest[i] = (src[i]+5); }
   }
}

void
adjust_para(int *src, int *dest, int size)
{
    parallel_for(blocked_range<int>(0,size),
        [&] (blocked_range<int> r) {
          for (int i = r.begin(); i<r.end();i++){
	    if (src[i]>=95)
		dest[i] = 100;
	    else 
	        dest[i] = src[i]+5;
          }	 
        }
    );
}


int 
count_seq(int *src, int size, int low, int high)
{
	int count = 0;
	for (int i = 0; i < size; i++) {
		if ((src[i] >= low) && (src[i] <= high)) {
			count++;
		}
	}
	return count;
}

int count_para(int *src, int size, int low, int high)
{
	int total = parallel_reduce(blocked_range<int>(0, size),
		0,
		[&](blocked_range<int> r, int total)->int {
			for (int i = r.begin(); i < r.end(); i++) {
				if (src[i] >= low && src[i] <= high) {
					total++;
				}
			}
			return total;
		},
		[](int x, int y) {return x + y; }
	);
	std::cout << total << endl;
	return total;
}

void
scan_seq(int *src, int *dest, int size, int low, int high)
{ 
//dest must be allocated prior to call
    int count = 0;
    for (int i = 0; i<size; i++){
        if (src[i] >= low && src[i] <= high){
            count++;
        }
	dest[i] = count;
    } 
}

class Para {
	int* src;
	int* dest;
	int size;
	int low;
	int high;
	int count;
public:
	Para(int* _src, int* _dest, int _size, int _low, int _high)
		: src(_src), dest(_dest), size(_size), low(_low), high(_high) { count = 0; }
	Para(Para &p, split)
		:src(p.src), dest(p.dest), size(p.size), low(p.low), high(p.high) { count = 0; }
	int getTotal() const { return count; }
	template<typename Tag>
	void operator()
		(const blocked_range<int> r, Tag tag) {
		for (int i = r.begin(); i < r.end(); i++) {
			if ((src[i] >= low) && src[i] <= high)
				count++;
			if (tag.is_final_scan()) dest[i] = count;
		}
	}
	void reverse_join(Para& p) { count += p.count; }
	void assign(Para& p) { count = p.count; }
};

void
scan_para(int *src, int *dest, int size, int low, int high)
{
	Para range_counter(src, dest, size, low, high);
	parallel_scan(blocked_range<int>(0, size), range_counter);
}

// NOTE: No modifications are permitted / needed beyond this point

int
main(int argc, char *argv[])
{
  if (argc != 5) {
    cerr << "Invalid command line - usage: <input file> <number of threads> <low> <high>" << endl;
    exit(-1);
  }

  // Extract parameters
  ifstream ifile(argv[1]);
  int num_threads = atoi(argv[2]);
  int low = atoi(argv[3]);
  int high = atoi(argv[4]);

  // Set the number of threads
  task_scheduler_init init(num_threads);
  
  // Read in the file
  int size;
  ifile >> size;
  int *src = new int[size];
  for (int i = 0; i < size; i++) {
    ifile >> src[i];
  }

  // Run adjust_seq
  int *adjust_seq_dest = new int[size];
  tick_count start = tick_count::now();
  adjust_seq(src, adjust_seq_dest, size);
  tick_count end = tick_count::now();
  double adjust_seq_time = (end-start).seconds();
  cout << "adjust_seq output (last 15): ";
  for (int i = size - 15; i < size; i++) {
    cout << adjust_seq_dest[i] << " ";
  }
  cout << endl;
  cout << "time: " << adjust_seq_time << endl;

  // Run adjust_para
  int *adjust_para_dest = new int[size];
  start = tick_count::now();
  adjust_para(src, adjust_para_dest, size);
  end = tick_count::now();
  double adjust_para_time = (end-start).seconds();
  cout << "adjust_para output (last 15): ";
  for (int i = size - 15; i < size; i++) {
    cout << adjust_para_dest[i] << " ";
  }
  cout << endl;
  cout << "time: " << adjust_para_time << endl;

  // Run count_seq
  start = tick_count::now();
  int count = count_seq(src, size, low, high);
  end = tick_count::now();
  double count_seq_time = (end-start).seconds();
  cout << "count_seq output: " << count << endl;
  cout << "time: " << count_seq_time << endl;

  // Run count_para
  start = tick_count::now();
  count = count_para(src, size, low, high);
  end = tick_count::now();
  double count_para_time = (end-start).seconds();
  cout << "count_para output: " << count << endl;
  cout << "time: " << count_para_time << endl;

  // Run scan_seq
  int *scan_seq_dest = new int[size];
  start = tick_count::now();
  scan_seq(src, scan_seq_dest, size, low, high);
  end = tick_count::now();
  double scan_seq_time = (end-start).seconds();
  cout << "scan_seq output (last 15): ";
  for (int i = size - 15; i < size; i++) {
    cout << scan_seq_dest[i] << " ";
  }
  cout << endl;
  cout << "time: " << scan_seq_time << endl;

  // Run scan_para
  int *scan_para_dest = new int[size];
  start = tick_count::now();
  scan_para(src, scan_para_dest, size, low, high);
  end = tick_count::now();
  double scan_para_time = (end-start).seconds();
  cout << "scan_para output (last 15): ";
  for (int i = size - 15; i < size; i++) {
    cout << scan_para_dest[i] << " ";
  }
  cout << endl;
  cout << "time: " << scan_para_time << endl;

  // Append the peformance results to the results file.
  ofstream ofile("grades.csv", ios::app);
  ofile << size << "," << num_threads << ",";
  ofile << adjust_seq_time << "," << adjust_para_time << ",";
  ofile << count_seq_time << "," << count_para_time << ",";
  ofile << scan_seq_time << "," << scan_para_time << endl;
  ofile.close();

  return 0;
}
