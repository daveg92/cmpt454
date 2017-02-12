/*
 *Node.cpp
 *Implementation for the Node of a B+ tree. Nodes are split into inner nodes, which contain pointers
 *to other nodes in the tree or leaf nodes, which contain string values
 *
 *The maximum number of keys in a Node is given by the nodeSize. With the exception of the root of 
 *the tree, Nodes must be at least half-full, ie they must contain at least (nodeSize/2) keys
 *
 *Authors: Dave Gill, Jackie Lang
 */

#include "Node.h"
#include <string>
#include <iostream>

/*
 *Constructs an empty node with maximum size n
 */
Node::Node(int n){
	nodeSize = n;
	parent = nullptr;
	rightSibling = nullptr;
	leftSibling = nullptr;
}

Node::Node(Node& other){
	
}

Node::~Node(){

}

/*
 *Insertion to general Node. All real implementation is taken care of
 *in the subclasses InnerNode and LeafNode
 */
void* Node::insert(int key, std::string value){
 	return nullptr;
}

/*
 *search in general Node. All real implementation is taken care of
 *in the subclasses InnerNode and LeafNode
 */
std::string Node::find(int key){
	return "";
}

/*
 *General printNode. All real implementation is taken care of
 *in the subclasses InnerNode and LeafNode
 */
void Node::printNode() const{
	
}

/*
 *General removal. All real implementation is taken care of
 *in the subclasses InnerNode and LeafNode
 */
void Node::remove(int key){return;}

/*
 *General key lookup. All real implementation is taken care of
 *in the subclasses InnerNode and LeafNode
 */
int Node::getKey() const{return 0;}

//-----------------------------------
//InnerNode implementation
//-----------------------------------
InnerNode::InnerNode(int n) : Node(n){
	extra = nullptr;
}

/*
 *Inserts the key, value pair to the B+Tree
 *Values are stored in leaf nodes, so the InnerNode searches for the correct node
 *to store the value.
 *
 *Returns a pointer to the root of the B+tree
 */
void* InnerNode::insert(int key, std::string value){
	int i;
	for(i = 0; i < keyPointerIndex.size(); i++){
		if(key < keyPointerIndex.at(i).first){
			break;
		}
	}

	if(i == 0){
		return ((Node*)extra)->insert(key, value);
	}
	
	void* nextNode = keyPointerIndex.at(i-1).second;
	return ((Node*)nextNode)->insert(key, value);
}

/*
 *Returns a pointer to the leafNode that either contains the specified key, or would hold
 *the key after it was inserted to the Tree
 */
void* InnerNode::findLeaf(int key){
	int i;
	for(i = 0; i < keyPointerIndex.size(); i++){
		if(key < keyPointerIndex.at(i).first){
			continue;
		}
		else{
			break;
		}
	}

	if(i == 0){
		if(InnerNode* node = static_cast<InnerNode*> (extra)){ //extra is another InnerNode
			((InnerNode*)extra)->findLeaf(key);
		}
		else{
			return extra;
		}
	}
	else{
		void* nextNode = keyPointerIndex.at(i-1).second;
		if(InnerNode* node = static_cast<InnerNode*>(nextNode)){ //nextNode is another InnerNode
			((InnerNode*)extra)->findLeaf(key);	
		}
		else{
			return nextNode;
		}
	}
}

/*
 *returns true if the Node is full
 *a Node can only hold nodeSize keys and nodeSize+1 pointers
 *If the number of keys equals the nodeSize, the node is considered to be full
 *A full node will need to be split before new keys can be added to it
 */
bool InnerNode::isFull() const{
	return (int)keyPointerIndex.size() == nodeSize;
}

/*
 *If a child node has too many values inserted, it will split. The new Node created will then need to be
 *added to its parent.
 *Inserts the search key and Node pointer to the new child in the keyPointerIndex and returns the root
 *of the B+tree
 */
void* InnerNode::insertFromChild(int key, void* child){
	//newly created root node. First the left-most child is added to extra, 
	//all others will be added to the index vector
	if(extra == nullptr){
		extra = child;
		return this;
	}
	auto insertionPoint = keyPointerIndex.begin();

	if(keyPointerIndex.empty()){
		keyPointerIndex.insert(keyPointerIndex.begin(), std::pair<int, void*>(key, child) );
		//keyPointerIndex.push_back(std::pair<int, void*>(key, child));
		return this;
	}

	while(insertionPoint->first < key && insertionPoint != keyPointerIndex.end()){
		insertionPoint++;
	}

	//fail if inserting duplicate key
	if( insertionPoint->first == key){
		return this;
	}

	keyPointerIndex.insert(insertionPoint, std::pair<int, void*>(key, child));
	//check if node has become over-full
	if(keyPointerIndex.size() > nodeSize){
		return split();
	}
	if(parent == nullptr){
		return this;
	}
	return parent;
}

/*
 *Splits the Node into two 
 *Nodes can only contain maximum nodeSize number of keys. If an insertion
 *would cause this to be exceeded, the Node must be split
 *ceiling of (n+1)/2 keys will remain in the original Node, rest will be in new NOde
 *After splitting, reference pointer will be inserted in parent
 *If there is no parent (ie we are splitting the root) create a new root Node and return
 *pointer to the parent
 */
void* InnerNode::split(){
	void* temp = rightSibling;
	rightSibling = new InnerNode(nodeSize);

	//set pointers in new Node
	((InnerNode *)rightSibling)->leftSibling = this;
	((InnerNode *)rightSibling)->rightSibling = temp;
	((InnerNode *)rightSibling)->parent = parent;

	//Move last (n+1)/2 keys to the new node
	int key;
	void* child;
	for(int i = (nodeSize +2)/2; i  < keyPointerIndex.size(); i++){
		key = keyPointerIndex.at(i).first; //extract key
		child = keyPointerIndex.at(i).second; //extract sting value
		((InnerNode *)rightSibling)->insertFromChild(key, child);
	}
	for(int i = (nodeSize + 2)/2; i < nodeSize +1; i++){
		keyPointerIndex.pop_back(); //remove the entries we just copied over
	}
	//key, value pair of new node to insert to parent 
	std::pair<int, void*> p = ((InnerNode *)rightSibling)->keyPointerIndex.at(0);

	//Check if we split the root
	if(parent == nullptr){
		parent = new InnerNode(nodeSize);
		((InnerNode *)rightSibling)->parent = parent; //fix rightSibling's parent		
		//insert original Node pointer to parent
		((InnerNode *)parent)->insertFromChild(keyPointerIndex.at(0).first, this); 
		//insert new node pointer to parent
		((InnerNode *)parent)->insertFromChild( p.first, rightSibling);
		return parent;
	}

	//didn't create new parent, need to add new NOde to parent index
	((InnerNode *)parent)->insertFromChild( p.first, rightSibling);
	return parent;
}

/*
 *Searches through the B+tree to retrieve the value corresponding to the specified key
 *if the key is not found within the tree, returns the empty string ""
 */
std::string InnerNode::find(int key){
	int i;
	if(keyPointerIndex.empty()){
		if(extra != nullptr){
			return ((Node*)extra)->find(key);
		}
		return ""; //you searched in an empty tree, dumbass!
	}
	if(key < keyPointerIndex.at(0).first){
		return ((Node*)extra)->find(key);
	}

	for(i = 1; i < keyPointerIndex.size() && key > keyPointerIndex.at(i).first; i++){
	}

	void* nextNode = keyPointerIndex.at(i-1).second;
	return ((Node*)nextNode)->find(key);
}

/*
 *Prints the contents of the Node to standard output
 */
void InnerNode::printNode() const{
	std::cout << "[";
	if(keyPointerIndex.size() > 0){
		std::cout << keyPointerIndex.at(0).first;
		for(int i = 1; i < keyPointerIndex.size(); i++){
			std::cout << ", " << keyPointerIndex.at(i).first;
		}
	}
	std::cout << "] ";
}

/*
 *Removes the key and its corresponding value from the B+Tree
 *If this causes a Node or Nodes to be less than half full, keys and reference pointers may
 *need to be rearranged to maintain proper tree structure
 */
void InnerNode::remove(int key){
	int i;

	if(key < keyPointerIndex.at(0).first){
		return ((Node*)extra)->remove(key);
	}

	for(i = 1; i < keyPointerIndex.size() && key > keyPointerIndex.at(i).first; i++){
	}

	void* nextNode = keyPointerIndex.at(i-1).second;
	return ((Node*)nextNode)->remove(key);
}

/*
 *updates the keys to match a modified child node
 */
void InnerNode::updateChildKey(int old, int newKey){
	if(old < keyPointerIndex.at(0).first){
		return;
	}

	for(int i = 0; i <keyPointerIndex.size(); i++){
		if(old == keyPointerIndex.at(i).first){
			keyPointerIndex.at(i).first = newKey;
			return;
		}
	}
}

/*
 *Removes the references to the deleted child Node from the InnerNode
 */
void* InnerNode::removeLeftChild(void* deadChild){
	int i;
	if((Node*)extra == (Node*)deadChild){
		i=0;
		extra = keyPointerIndex.at(0).second;
	}
	else{ //looking for which pointer index to remove
		for(i=0; i < keyPointerIndex.size() && keyPointerIndex.at(i).second != deadChild; i++){
		}
	}

	//shifting the pointers
	for(int j=i; j < keyPointerIndex.size() - 1; j++){
		keyPointerIndex.at(j).second = keyPointerIndex.at(j+1).second;
	}
	keyPointerIndex.pop_back(); 

	//need to update corresponding keys
	for(int j = i; j <keyPointerIndex.size(); j++){
		keyPointerIndex.at(j).first = ((Node*)keyPointerIndex.at(j).second)->getKey();
	}

	//check if innerNOde is now to small AND this is not the root
	if(keyPointerIndex.size() < nodeSize/2 && parent != nullptr){
		//panic
		//implement
	}
}

/*
 *Returns the first key of the Node for indexing
 */
int InnerNode::getKey() const{
	return keyPointerIndex.at(0).first;
}

//-----------------------------------
//LeafNode implementation
//-----------------------------------
LeafNode::LeafNode(int n) : Node(n){

}

/*
 *Inserts the key, value pair to the leaf of the B+Tree
 *Duplicates are forbidden in the B+tree, so if they key already exists in the tree, insertion will fail
 *Splits the node if insertion would cause it to be overfull
 *
 */
void* LeafNode::insert(int key, std::string value){
	std::vector< std::pair<int, std::string> >::iterator insertionPoint = keyValueIndex.begin();

	if(keyValueIndex.size() == 0){
		keyValueIndex.push_back(std::pair<int, std::string>(key, value));
		return this;
	}

	while((*insertionPoint).first < key && insertionPoint != keyValueIndex.end()){
		insertionPoint++;
	}

	//check if the key already exists, if so, terminate insertion
	if( (*insertionPoint).first == key){
		return parent;
	}

	//Need to check if split is needed after insertion

	keyValueIndex.insert(insertionPoint, std::pair<int, std::string>(key, value));

	if(keyValueIndex.size() > nodeSize){
		return split();
	}

	if(parent == nullptr){
		return this;
	}
	return parent;

}

/*
 *Prints the keys of the Node to standard output
 */
void LeafNode::printNode() const{
	std::cout << "[";
	if(keyValueIndex.size() > 0){
		std::cout << keyValueIndex.at(0).first;
		for(int i = 1; i < keyValueIndex.size(); i++){
			std::cout << ", " << keyValueIndex.at(i).first;
		}
	}
	std::cout << "] ";
}

/*
 *Splits the Node into two 
 *Nodes can only contain maximum nodeSize number of keys. If an insertion
 *would cause this to be exceeded, the Node must be split
 *ceiling of (n+1)/2 keys will remain in the original Node, rest will be in new NOde
 *After splitting, reference pointer will be inserted in parent
 *If there is no parent (ie we are splitting the root) create a new root Node and return
 *pointer to the parent
 */
void* LeafNode::split(){
	void* temp = rightSibling;
	rightSibling = new LeafNode(nodeSize);

	//set pointers in new Node
	((LeafNode *)rightSibling)->leftSibling = this;
	((LeafNode *)rightSibling)->rightSibling = temp;
	((LeafNode *)rightSibling)->parent = parent;

	//Move last (n+1)/2 keys to the new node
	auto iterator = keyValueIndex.end();
	iterator--;
	int key;
	std::string value;
	for(int i = 0; i < (nodeSize +1)/2; i++){
		key = iterator->first; //extract key
		value = iterator->second; //extract sting value
		iterator--; //move backwards through vector
		keyValueIndex.pop_back(); //delete old key,string pair
		((LeafNode *)rightSibling)->insert(key, value);
	}
	//key, value pair of new node to insert to parent 
	std::pair<int, std::string> p = ((LeafNode *)rightSibling)->keyValueIndex.at(0);

	//Check if we split the root
	if(parent == nullptr){
		parent = new InnerNode(nodeSize);
		((LeafNode *)rightSibling)->parent = parent; //fix rightSibling's parent		
		//insert original Node pointer to parent
		((InnerNode *)parent)->insertFromChild(keyValueIndex.at(0).first, this); 
		//insert new node pointer to parent
		((InnerNode *)parent)->insertFromChild( p.first, rightSibling);
		return parent;
	}

	//didn't create new parent, need to add new NOde to parent index
	((InnerNode *)parent)->insertFromChild( p.first, rightSibling);
	return parent;

}

/*
 *Returns the string value corresonding with key
 *If key is not in the tree, returns the empty string ""
 */
std::string LeafNode::find(int key){
	for(int i = 0; i < keyValueIndex.size(); i++){
		if(keyValueIndex.at(i).first == key){
			return keyValueIndex.at(i).second;
		}
	}
	return "";
}

/*
 *Removes the key from the leaf Node. If this would cause the node to be less than half full
 *will rearrange keys and values from sibling nodes to maintain structure of the tree
 */
void LeafNode::remove(int key){
	bool found = false;
	auto iterator = keyValueIndex.begin();
	while(iterator != keyValueIndex.end() &&!found){
		if(iterator->first == key){
			found = true;
			keyValueIndex.erase(iterator);
		}
		iterator++;
	}
	if(!found){ //key was not found
		return;
	}

	if(keyValueIndex.size() > nodeSize/2){ //still more than half full
		return;
	}

	//The node is less than half full
	//Case 1 -- Try to borrow from left sibling if it exists
	if(leftSibling != nullptr && ((LeafNode*)leftSibling)->parent == parent){
		if( ((LeafNode*)leftSibling)->keyValueIndex.size()-1 > nodeSize/2){
			int newKey = ((LeafNode*)leftSibling)->keyValueIndex.back().first;
			std::string value = ((LeafNode*)leftSibling)->keyValueIndex.back().second;
			insert(newKey, value);
			((LeafNode*)leftSibling)->keyValueIndex.pop_back(); //delete value from sibling
			//update parent key
			((InnerNode*)parent)->updateChildKey(key, newKey);
			return;
		}
	}

	//Case 2 -- Try to borrow from right sibling if it exists
	if(rightSibling != nullptr && ((LeafNode*)rightSibling)->parent == parent){
		if( ((LeafNode*)rightSibling)->keyValueIndex.size() - 1 > nodeSize/2){
			int newKey = ((LeafNode*)rightSibling)->keyValueIndex.front().first;
			std::string value = ((LeafNode*)rightSibling)->keyValueIndex.front().second;
			insert(newKey,value);
			//delete value from sibling
			((LeafNode*)rightSibling)->keyValueIndex.erase(((LeafNode*)rightSibling)->keyValueIndex.begin());
			//update parent key
			((InnerNode*)parent)->updateChildKey(newKey, ((LeafNode*)rightSibling)->keyValueIndex.front().first);
			return;
		}
	}

	//Case 3 -- Try to coalese with left
	if(leftSibling != nullptr){
		//insert all values from the left sibling into current node
		auto iterator = ((LeafNode*)leftSibling)->keyValueIndex.begin();
		while(iterator != ((LeafNode*)leftSibling)->keyValueIndex.end()){
			insert(iterator->first, iterator->second);
			iterator++;
		}
		
		LeafNode* temp = (LeafNode*)leftSibling;
		leftSibling = ((LeafNode*)leftSibling)->leftSibling; //update left sibling of current
		if(leftSibling != nullptr){
			((LeafNode*)leftSibling)->rightSibling = this;
		}

		void* updatedParent = ((InnerNode*)parent)->removeLeftChild(temp);
		//finally, delete leftSibling
		delete temp;
		
		return; //updatedParent;
	}
}

/*
 *Returns the first key of the Node for indexing
 */
int LeafNode::getKey() const{
	return keyValueIndex.at(0).first;
}