#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>

class IPrintable {
public:
	virtual void Print(int indent) = 0;
};

template <class T>
class Object : public IPrintable {
public:
	T val;

	Object(T v) : val(v) {}
	virtual void Print(int indent);
};

template <class T>
void Object<T>::Print(int indent) {
	for (int i = 0; i < indent; ++i)
		std::cout << " ";
	std::cout << val << std::endl;
}

class Node : public IPrintable {
public:
	std::string op;
	int val;

	IPrintable *left;
	IPrintable *right;

	Node(std::string o, IPrintable *l, IPrintable *r) : op(o), left(l), right(r) {};

	virtual void Print(int indent);
};

class MultiNode : public IPrintable {
private:
	std::vector<IPrintable *> nodes;
public:
	MultiNode() : nodes() {}

	void Add(IPrintable *node) { nodes.push_back(node); }
	int Len() { return nodes.size(); }
	virtual void Print(int indent);
};

class Variable : public IPrintable {
public:
	Variable(std::string name);

	unsigned type;
	int val;
	std::string name;

	virtual void Print(int indent);
};

class Block {

public:
	Block(): ids() {}

	Variable* GetOrCreate(std::string name);
private:
	std::map<std::string, Variable *> ids;

};
