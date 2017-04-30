#include <iostream>

#include "tree.h"
#include "y.tab.h"

template<class T>
void print(T s, int indent) {
	for ( int i = 0; i < indent; ++i)
		std::cout << ' ';
	std::cout << s << std::endl;
}

void Node::Print(int indent) {
	print(this->op, indent);

	if (this->left) this->left->Print(indent+4);
	if (this->right) this->right->Print(indent+4);
}

void MultiNode::Print(int indent) {
	for (std::vector<IPrintable *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
		(*it)->Print(indent);
}

Variable::Variable(std::string name) : type(UNKNOWN_ID), name(name) {}

void Variable::Print(int indent) {
	print(this->name, indent);
}

Variable* Block::GetOrCreate(std::string name) {
	Variable *var = this->ids[name];
	if ( !var ) {
		var = new Variable(name);
		this->ids[name] = var;
	}
	return var;
}

/*
int main() {
	Node *plus = new Node('*');
	plus->left = new Node(23.4);
	plus->right = new Node(45.3);

	Node *node = new Node('+');
	node->left = plus;
	node->right = new Node(25);

	node->Print(0);

	return 0;
}
*/
