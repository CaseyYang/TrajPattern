#include<iostream>
#include<list>
#include<set>
#include<map>
#include<vector>
#include <algorithm> 
#include <iterator>
using namespace std;

class A{
public:
	int a;
	A(){
		a = 3;
	}
	A(int a) :a(a){
	}
};

class B:public A{
public:
	int b;
	B(int a) :b(a){
	}
};

void func(A* a){
	cout << a->a << endl;
}

void main(){

	//int first[] = { 5, 10, 15, 20, 25 };
	//int second[] = { 50, 40, 30, 20, 10 };
	//std::vector<int> v(10);                      // 0  0  0  0  0  0  0  0  0  0
	//std::vector<int>::iterator it;

	//std::sort(first, first + 5);     //  5 10 15 20 25
	//std::sort(second, second + 5);   // 10 20 30 40 50

	//it = std::set_intersection(first, first + 5, second, second + 5, v.begin());
	//// 10 20 0  0  0  0  0  0  0  0
	//v.resize(it - v.begin());                      // 10 20

	//std::cout << "The intersection has " << (v.size()) << " elements:\n";
	//for (it = v.begin(); it != v.end(); ++it)
	//	std::cout << ' ' << *it;
	//std::cout << '\n';


	
	//set<int> set1 = set<int>();	
	//set<int> set2 = set<int>();
	//set<int> set3 = set<int>();
	//set1.insert(1);
	//set1.insert(3);
	//set1.insert(2);
	//list<int> list1 = list<int>(set1.begin(), set1.end());
	//set2.insert(2);
	//set2.insert(3);
	//set2.insert(0);
	//set_difference(set2.begin(), set2.end(), set1.begin(), set1.end(),inserter(set3,set3.begin()));
	//list1.insert(list1.end(), set3.begin(), set3.end());
	//for each (int var in list1)
	//{
	//	cout << var << " ";
	//}

	list<int> l1 = list<int>();
	l1.push_back(1);
	l1.push_back(2);
	l1.push_back(3);
	l1.push_back(4);
	l1.push_back(5);
	for (auto i : l1){
		if (i % 2 == 0){
			//l1.remove(i);
		}
		else{
			i *= 2;
		}
	}
	for (auto i : l1){
		cout << i << " ";
	}
	cout << endl;
}