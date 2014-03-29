#include<iostream>
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
	B* bp = new B(1);
	func(bp);
}