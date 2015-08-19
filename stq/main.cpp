#include<iostream>
#include<cmath>
#include<vector>
#include<fstream>
#include<assert.h>

using namespace std;

/* For declaring size n vector with entries of type T */
template<class T> vector<T> init1D(int n)
{
	vector<T> result;
	result.resize(n);
	return result;
}

/* For declaring size n by m vector with entries of type T */
template<class T> vector<vector<T> > init2D(int n, int m)
{
	vector<vector<T> > result;
	result.resize(n);
	for (int i = 0; i < n; i++) {
		result[i].resize(m);
	}
	return result;
}

bool is_obstruction(vector<vector<int> > graph,
	int s1, int s2, int q1, int q2)
{
	if(graph[s1][q1] == 1 && graph[s2][q2] == 1) {
		if(graph[s1][q2] == 0 && graph[s2][q1] == 0) {
			return true;
		} else {
			return false;
		}
	}

	if(graph[s1][q2] == 1 && graph[s2][q1] == 1) {
		if(graph[s1][q1] == 0 && graph[s2][q2] == 0) {
			return true;
		} else {
			return false;
		}
	}
	
}

int compute_obstructions(vector<vector<int> > graph,
	int num_students, int num_questions)
{
	int s1, s2, q1, q2;
	int total_obstructions = 0;
	for(s1 = 0; s1 < num_students; s1++) {
		for(s2 = s1 + 1; s2 < num_students; s2++) {
			for(q1 = 0; q1 < num_questions; q1++) {
				for(q2 = q1 + 1; q2 < num_questions; q2++) {
					if(is_obstruction(graph, s1, s2, q1, q2)) {
						total_obstructions++;
					}
				}
			}
		}
	}

	return total_obstructions;	
}

int main()
{
	int i, j;
	int num_students, num_questions, num_edges;

	std::ifstream fin("input.txt");
	fin >> num_students;
	fin >> num_questions;
	fin >> num_edges;

	vector<vector<int> > graph = init2D<int>(num_students, num_questions);
	for(i = 0; i < num_students; i++) {
		for(j = 0; j < num_questions; j++) {
			graph[i][j] = 0;
		}
	}

	for(i = 0; i < num_edges; i++) {
		int s, q;
		fin >> s;
		fin >> q;
		assert(s < num_students && q < num_questions);
		graph[s][q] = 1;
	}
	
	int total_obstructions = compute_obstructions(graph,
		num_students, num_questions);

	cout << "Total obstructions " << total_obstructions << "\n";
}
