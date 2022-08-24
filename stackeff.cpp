#include<iostream>
#include<vector>
using namespace std;
class Stack{
	private:
		
	static const int max = 100001;
		int v[max];
		int v2[max];
		int top;

	public:
		Stack() {top = -1;}
		bool isEmpty();
		bool isFull();
		void pop();
		void push(int x);
		void get_max();
};
bool Stack::isEmpty(){
	if(top == -1)
		return true;
	return false;
}


bool Stack::isFull(){
	if(top == max-1)
		return true;
	return false;
	
}
void Stack::pop(){
	if(isEmpty()){
		cout << "error" << endl;
	}
	else
	    top--;
}
void Stack::push(int x){
	top++;
	v[top] = x;
	if(top==0) v2[top]=x;
	else {
	    int y = v2[top-1];
	    if(x>y)
		    v2[top]=x;
	    else
		    v2[top]=y;
	}
}
void  Stack::get_max(){
	if(isEmpty()){
		cout << "None" << endl;
		return;
	}
	cout <<  v2[top] << endl;
}

int main() {
    Stack s;
    int n, a;
	cin >> n;
	string com;
	while(n!=0){
		cin >> com;
		if(com == "push"){
			cin >> a;
			s.push(a);
		}
		if(com == "pop"){
			s.pop();
		}
		if(com == "get_max"){
			s.get_max();
		}
		n--;
	}


}