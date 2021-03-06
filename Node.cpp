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


/*
 *returns a pointer to the parent of the node
 */
void* Node::getParent() const{
	return parent;
}

/*
 *Updates the parent pointer of the node
 *used when the parent has split and needs to inform its new children of the change
 */
void Node::setParent(void* newParent){

}

/*
 *Prints the values of the tree to standard output
 */
void Node::printValues(){

}

/*
 *Returns true if the keyPointerIndex is empty, false otherwise
 *Used to determine if the root node of the tree has become empty, if so, may promote next node as new root
 */
bool Node::isEmpty() const {return true;}

/*
 *Helper function to delete the tree
 *Recursively deletes all nodes of the subtree
 */
void Node::fullDeletion(){}

/*
 *Returns the node that does or would contain the key
 */
void* Node::findLeaf(int key){ return this;}

/*
 *Helper function to create a copy of the tree
 *returns pointer to a new copy of the subtree
 *The copied subtree will have the same node structure as the original,
 *but sibling and parent pointers will not be assigned. Call fixSiblings() on the copied
 *tree to correctly connect the nodes
 */
void* Node::copySubTree(){return this;}

/*
 *Helper function in copying a tree
 *Fixes the sibling and parent pointers of a newly created tree so that regular operations
 *can function correctly
 */
void Node::fixSiblings(){}

//-----------------------------------
//InnerNode implementation
//-----------------------------------
InnerNode::InnerNode(int n) : Node(n){
	isLeaf = false;
	extra = nullptr;
}

InnerNode::~InnerNode(){

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
	//check that the vector isn't empty before proceeding
	if(keyPointerIndex.empty()){
		return ((Node*)extra)->insert(key, value);
	}
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
	if(keyPointerIndex.empty() || key < keyPointerIndex.at(0).first){
		return ((Node*)extra)->findLeaf(key);
	}
	int i;
	for(i = 1; i < keyPointerIndex.size() ; i++ ){
		if(key < keyPointerIndex.at(i).first){
			break;
		}
	}
	return ((Node*)keyPointerIndex.at(i-1).second)->findLeaf(key);
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

	if(temp != nullptr){
		((InnerNode*)temp)->leftSibling = rightSibling;
	}

	//Move last (n+1)/2 keys to the new node
	int key;
	void* child;
	for(int i = (nodeSize +2)/2; i  < keyPointerIndex.size(); i++){
		key = keyPointerIndex.at(i).first; //extract key
		child = keyPointerIndex.at(i).second; //extract sting value
		((InnerNode *)rightSibling)->insertFromChild(key, child);
		((Node*)child)->setParent(rightSibling);
	}
	for(int i = (nodeSize + 2)/2; i < nodeSize +1; i++){
		if(!keyPointerIndex.empty()){
			keyPointerIndex.pop_back(); //remove the entries we just copied over
		}
	}
	//key, pointer pair of new node to insert to parent 
	std::pair<int, void*> p = ((InnerNode *)rightSibling)->keyPointerIndex.at(0);

	//Check if we split the root
	if(parent == nullptr){
		parent = new InnerNode(nodeSize);
		((InnerNode *)rightSibling)->parent = parent; //fix rightSibling's parent		
		//insert original Node pointer to parent
		((InnerNode *)parent)->insertFromChild(keyPointerIndex.at(0).first, this); 
		//insert new node pointer to parent
		((InnerNode *)parent)->insertFromChild( ((Node*)rightSibling)->getKey(), rightSibling);
		return parent;
	}

	//didn't create new parent, need to add new NOde to parent index
	((InnerNode *)parent)->insertFromChild( ((InnerNode*)rightSibling)->getKey(), rightSibling);
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

	for(i = 1; i < keyPointerIndex.size() && key >= keyPointerIndex.at(i).first; i++){
	}

	void* nextNode = keyPointerIndex.at(i-1).second;
	((Node*)nextNode)->printNode();
	return ((Node*)nextNode)->find(key);
}

/*
 *Prints the contents of the B+Tree to standard output
 */
void InnerNode::printNode() const{
	//first print self to console
	std::cout << "[";
	if(keyPointerIndex.size() > 0){
		std::cout << keyPointerIndex.at(0).first;
		for(int i = 1; i < keyPointerIndex.size(); i++){
			std::cout << ", " << keyPointerIndex.at(i).first;
		}
	}
	std::cout << "] ";

	//have all sibling to the right print themselves
	if(rightSibling != nullptr){
		((InnerNode*)rightSibling)->printNode();
	}

	//if this is the left-most node in the level, tell the next level to begin printing on a new line
	if(leftSibling == nullptr){
		std::cout << std::endl;
		((Node*)extra)->printNode();
	}
}

/*
 *Removes the key and its corresponding value from the B+Tree
 *If this causes a Node or Nodes to be less than half full, keys and reference pointers may
 *need to be rearranged to maintain proper tree structure
 */
void InnerNode::remove(int key){
	if(keyPointerIndex.empty()){
		((Node*)extra)->remove(key);
		return;
	}
	int i;

	if(key < keyPointerIndex.at(0).first){
		return ((Node*)extra)->remove(key);
	}

	for(i = 1; i < keyPointerIndex.size() && key >= keyPointerIndex.at(i).first; i++){
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
void InnerNode::removeLeftChild(void* deadChild){
	int i;
	if((Node*)extra == (Node*)deadChild){
		i=0;
		extra = keyPointerIndex.at(0).second;
	}
	else{ //looking for which pointer index to remove
		for(i=0; i < keyPointerIndex.size() && keyPointerIndex.at(i).second != deadChild; i++){
		}
	}

	//shifting the pointers and updating keys as we go
	for(int j=i; j < keyPointerIndex.size() - 1; j++){
		keyPointerIndex.at(j).second = keyPointerIndex.at(j+1).second;
		keyPointerIndex.at(j).first = ((Node*)keyPointerIndex.at(j).second)->getKey();
	}

	if(!keyPointerIndex.empty()){
		keyPointerIndex.pop_back();
	}
	//printNode();

	//check if innerNOde is now to small AND this is not the root
	if(keyPointerIndex.size() < nodeSize/2 && parent != nullptr){
		//modularise with restructure();
		//Case 1 -- try to borrow from left
		if(leftSibling != nullptr && ((InnerNode*)leftSibling)->parent == parent){
			//InnerNodes can contain one less key than leaves, because of the extra pointer that does not have 
			//an explicitly corresponding key
			if(((InnerNode*)leftSibling)->keyPointerIndex.size()-1 >= nodeSize/2 ){
				borrowLeft();
				return;
			}
		}
		//Case 2 -- try to borrow from right
		if(rightSibling != nullptr && ((InnerNode*)rightSibling)->parent == parent){
			if( ((InnerNode*)rightSibling)->keyPointerIndex.size() - 1 >= nodeSize/2){
				borrowRight();
				return;
			}
		}
		//Case 3 -- try to coalese with left
		if(leftSibling != nullptr && ((InnerNode*)leftSibling)->parent == parent){
			coaleseLeft();
			return;
		}
		//case 4 -- try to coalese with right
		if(rightSibling != nullptr && ((InnerNode*)rightSibling)->parent == parent){
			coaleseRight();
		}
	}
}

/*
 *Helper function for removal
 *If a node becomes less than half full, it may try borrowing from its
 *sibling to maintain the correct structure of the tree
 */
void InnerNode::borrowLeft(){
	int updateKey;
	auto iterator = ((InnerNode*)parent)->keyPointerIndex.begin();
	int keyToUpdate = iterator->first;
	
	//copy extra into front of the IndexPointer vector
	keyPointerIndex.insert(keyPointerIndex.begin(), std::pair<int, void*>(((Node*)extra)->getKey(), extra));
	//overwrite extra with right-most pointer from sibling
	extra = ((InnerNode*)leftSibling)->keyPointerIndex.back().second;
	//remove key, pointer pair from the sibling
	if( !((InnerNode*)leftSibling)->keyPointerIndex.empty()){
		((InnerNode*)leftSibling)->keyPointerIndex.pop_back();
	}
	//Luke, i am your father
	((Node*)extra)->setParent(this);
	//Update the key in the parent node
	while (iterator != ((InnerNode*)parent)->keyPointerIndex.end()){
		if(iterator->first >= getKey()){
			keyToUpdate = iterator->first;
			break;
		}
	}
	((InnerNode*)parent)->updateChildKey(keyToUpdate, getKey());
}

/*
 *Helper function for removal
 *If a node becomes less than half full, it may try borrowing from its
 *sibling to maintain the correct structure of the tree
 */
void InnerNode::borrowRight(){
	//copy extra pointer and corresponding key from right sibling
	void* newPointer = ((InnerNode*)rightSibling)->extra;
	keyPointerIndex.push_back( std::pair<int, void*>(((Node*)newPointer)->getKey(), newPointer) );

	//have right sibling shift all key, pointer pairs left
	((InnerNode*)rightSibling)->shiftPointersLeft();
	((InnerNode*)keyPointerIndex.back().second)->setParent(this);
}

/*
 *If we cannot borrow from siblings, need to merge the siblings
 *together
 */
void InnerNode::coaleseLeft(){
	//copy extra from this node into the vector
	insertFromChild( ((Node*)extra)->getKey(), extra);
	//begin copying in the left vector contents
	auto iterator = ((InnerNode*)leftSibling)->keyPointerIndex.begin();
	while (iterator != ((InnerNode*)leftSibling)->keyPointerIndex.end()){
		insertFromChild(iterator->first, iterator->second);
		iterator++;
	}
	//copy over left sibling's extra pointer
	extra = ((InnerNode*)leftSibling)->extra;
	//fix sibling pointers
	InnerNode* temp = (InnerNode*)leftSibling;
	leftSibling = temp->leftSibling;
	if(leftSibling != nullptr){
		((InnerNode*)leftSibling)->rightSibling = this;
	}
	//update parent and delete old sibling
	((InnerNode*)parent)->removeLeftChild(temp);
	delete temp;
	//fix parent pointers of new children
	((Node*)extra)->setParent(this);
	for(int i = 0; i < keyPointerIndex.size(); i++){
		((Node*)keyPointerIndex.at(i).second)->setParent(this);
	}

}

/*
 *If coalesing left is not possible, merge with the right sibling to maintain proper
 *tree structure
 */
void InnerNode::coaleseRight(){
	//insert all values from right sibling into current node
	//start with the extra pointer
	void* firstElement = ((InnerNode*)rightSibling)->extra;
	insertFromChild( ((InnerNode*)firstElement)->getKey(), firstElement);
	//copy everything from the vector
	auto iterator = ((InnerNode*)rightSibling)->keyPointerIndex.begin();
	while (iterator != ((InnerNode*)rightSibling)->keyPointerIndex.end()){
		insertFromChild(iterator->first, iterator->second);
		iterator++;
	}
	//update parent pointers of the children
	for(int i = 0; i < keyPointerIndex.size(); i++){
		((Node*)keyPointerIndex.at(i).second)->setParent(this);
	}

	//fix sibling pointers
	InnerNode* temp = (InnerNode*)rightSibling;
	rightSibling = temp->rightSibling;
	if(rightSibling != nullptr){
		((InnerNode*)rightSibling)->leftSibling = this;
	}
	//remove temp from parent and delete temp
	((InnerNode*)parent)->removeRightChild(temp);
	delete temp;
}

/*
 *Called when borrowing key from right sibling
 *after the key, pointer has been copied, need to shift everything
 *down
 */
void InnerNode::shiftPointersLeft(){
	//overwrite extra
	extra = keyPointerIndex.at(0).second;
	//shift pointers left
	for(int i = 0; i <keyPointerIndex.size() - 1; i++){
		keyPointerIndex.at(i) = keyPointerIndex.at(i+1);
		keyPointerIndex.at(i).first = ((Node*)keyPointerIndex.at(i).second)->getKey();
	}
	//remove last element
	if(!keyPointerIndex.empty()){
		keyPointerIndex.pop_back();
	}
}

/*
 *Removes the reference to the deleted child Node from the InnerNOde
*/
void InnerNode::removeRightChild(void* deadChild){
	int i;
	//don't need to check extra ptr, because we're coalesing from right into left sibling, therefore
	//there exists at least one sibling to the left of right
	//search for pointer index to removes
	for(i=0; i <keyPointerIndex.size() && keyPointerIndex.at(i).second != deadChild; i++){}

	//shift pointers and update keys as we go
	for(int j = i; j < keyPointerIndex.size() - 1; j++){
		keyPointerIndex.at(j).second = keyPointerIndex.at(j+1).second;
		keyPointerIndex.at(j).first = ((Node*)keyPointerIndex.at(j).second)->getKey();
	}
	if(!keyPointerIndex.empty()){
		keyPointerIndex.pop_back();
	}

	//check if innerNOde is now to small AND this is not the root
	if(keyPointerIndex.size() < nodeSize/2 && parent != nullptr){
		//modularise with restructure();
		//Case 1 -- try to borrow from left
		if(leftSibling != nullptr && ((InnerNode*)leftSibling)->parent == parent){
			//InnerNodes can contain one less key than leaves, because of the extra pointer that does not have 
			//an explicitly corresponding key
			if(((InnerNode*)leftSibling)->keyPointerIndex.size()-1 >= nodeSize/2 ){
				borrowLeft();
				return;
			}
		}
		//Case 2 -- try to borrow from right
		if(rightSibling != nullptr && ((InnerNode*)rightSibling)->parent == parent){
			if( ((InnerNode*)rightSibling)->keyPointerIndex.size() - 1 >= nodeSize/2){
				borrowRight();
				return;
			}
		}
		//Case 3 -- try to coalese with left
		if(leftSibling != nullptr && ((InnerNode*)leftSibling)->parent == parent){
			coaleseLeft();
			return;
		}
		//case 4 -- try to coalese with right
		if(rightSibling != nullptr && ((InnerNode*)rightSibling)->parent == parent){
			coaleseRight();
		}
	}
}

/*
 *Returns the indexing key for the Node
 *When building the nodes for higher levels, they need to reference the smallest possible key
 *that may be searched in this branch. This key will be the first key in the left-most leafNode
 *of the branch
 */
int InnerNode::getKey() const{
	return ((Node*)extra)->getKey();
}

/*
 *returns a pointer to the parent of the node
 */
void* InnerNode::getParent() const{
	return parent;
}

/*
 *Updates the parent pointer of the node
 *used when the parent has split and needs to inform its new children of the change
 */
void InnerNode::setParent(void* newParent){
	parent = newParent;
}

/*
 *Prints the values of the tree to standard output, each on a separate line
 */
void InnerNode::printValues(){
	//search for the left-most leafNode and begin printing values
	((Node*)extra)->printValues();
}

/*
 *Returns true if the keyPointerIndex is empty, false otherwise
 *Used to determine if the root node of the tree has become empty, if so, may promote next node as new root
 */
bool InnerNode::isEmpty() const {
	return keyPointerIndex.empty();
}

/*
 *Helper function to delete the tree
 *Recursively deletes all nodes of the subtree
 */
void InnerNode::fullDeletion(){

	auto iterator = keyPointerIndex.begin();

	while(iterator != keyPointerIndex.end()){
		((Node*)iterator->second)->fullDeletion();
		delete (Node*)(iterator->second);
		iterator++;
	}

	((Node*)extra)->fullDeletion();
	delete (Node*)extra;
}

/*
 *Helper function to create a copy of the tree
 *returns pointer to a new copy of the subtree
 *The copied subtree will have the same node structure as the original,
 *but sibling and parent pointers will not be assigned. Call fixSiblings() on the copied
 *tree to correctly connect the nodes
 */
void* InnerNode::copySubTree(){
	void* subTree = new InnerNode(nodeSize);

	//copy extra ptr
	((InnerNode*)subTree)->extra = ((Node*)extra)->copySubTree();

	//copy trees in the vector
	std::pair<int, void*> p;
	for(int i = 0; i < keyPointerIndex.size(); i++){
		p = keyPointerIndex.at(i);
		((InnerNode*)subTree)->keyPointerIndex.push_back(std::pair<int, void*>(p.first, ((Node*)p.second)->copySubTree()));
	}
	return subTree;
}

/*
 *Helper function in copying a tree
 *Fixes the sibling and parent pointers of a newly created tree so that regular operations
 *can function correctly
 */
void InnerNode::fixSiblings(){
	//fix sibling/parent pointers for the extra child
	if(leftSibling != nullptr){
		((Node*)extra)->leftSibling = ((InnerNode*)leftSibling)->keyPointerIndex.back().second;
	}
	((Node*)extra)->rightSibling = keyPointerIndex.at(0).second;
	((Node*)extra)->parent = this;
	((Node*)extra)->fixSiblings(); //fix sibling for the subtree

	//fix sibling/parent pointers for the child in position 0
	((Node*)keyPointerIndex.at(0).second)->leftSibling = extra;
	if(keyPointerIndex.size() > 1){
		((Node*)keyPointerIndex.at(0).second)->rightSibling = keyPointerIndex.at(1).second;
	}
	((Node*)keyPointerIndex.at(0).second)->parent = this;
	((Node*)keyPointerIndex.at(0).second)->fixSiblings();
	//fix sibling/parent pointers for middle children
 	int i;
	for(i = 1; i < keyPointerIndex.size()-1; i++){
		((Node*)keyPointerIndex.at(i).second)->leftSibling = keyPointerIndex.at(i-1).second;
		((Node*)keyPointerIndex.at(i).second)->rightSibling = keyPointerIndex.at(i+1).second;
		((Node*)keyPointerIndex.at(i).second)->parent = this;
		((Node*)keyPointerIndex.at(i).second)->fixSiblings();
	}
	//fix sibling/parent pointers for final child
	((Node*)keyPointerIndex.at(i).second)->leftSibling = keyPointerIndex.at(i-1).second;
	if(rightSibling != nullptr){
		((Node*)keyPointerIndex.at(i).second)->rightSibling = ((InnerNode*)rightSibling)->extra;
	}
	((Node*)keyPointerIndex.at(i).second)->parent = this;
	((Node*)keyPointerIndex.at(i).second)->fixSiblings();

}
//-----------------------------------
//LeafNode implementation
//-----------------------------------
LeafNode::LeafNode(int n) : Node(n){
	isLeaf = true;
}

/*
 *Inserts the key, value pair to the leaf of the B+Tree
 *Duplicates are forbidden in the B+tree, so if they key already exists in the tree, insertion will fail
 *Splits the node if insertion would cause it to be overfull
 *
 */
void* LeafNode::insert(int key, std::string value){

 	auto insertionPoint = keyValueIndex.begin();
	//set returnValue to be the root of the tree
	//this is overridden only in the case of splitting a node, which may cause there to be a new root
	void* returnValue = this;
	while (((Node*)returnValue)->getParent() != nullptr){
		returnValue = ((Node*)returnValue)->getParent();
	}

	if(keyValueIndex.size() == 0){
		keyValueIndex.push_back(std::pair<int, std::string>(key, value));
		return returnValue;
	}

	while((*insertionPoint).first < key && insertionPoint != keyValueIndex.end()){
		insertionPoint++;
	}

	//check if the key already exists, if so, terminate insertion
	if( (*insertionPoint).first == key){
		return returnValue;
	}

	//Need to check if split is needed after insertion
	keyValueIndex.insert(insertionPoint, std::pair<int, std::string>(key, value));
	if(keyValueIndex.size() > nodeSize){
		split();
	}

	while (((Node*)returnValue)->getParent() != nullptr){
		returnValue = ((Node*)returnValue)->getParent();
	}

	return returnValue;
}

/*
 *Prints the keys of the B+Tree to standard output
 */
void LeafNode::printNode() const{
	//print self to console
	std::cout << "[";
	if(keyValueIndex.size() > 0){
		std::cout << keyValueIndex.at(0).first;
		for(int i = 1; i < keyValueIndex.size(); i++){
			std::cout << ", " << keyValueIndex.at(i).first;
		}
	}
	std::cout << "] ";

	//have siblings to the right print themselves
	if(rightSibling != nullptr){
		((LeafNode*)rightSibling)->printNode();
	}

	//if this is the first node in leaf-level, print new line to finish printing
	if(leftSibling == nullptr){
		std::cout << std::endl;
	}
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
	//tell the old rightSibling it has a new baby brother
	if(temp != nullptr){
		((LeafNode*)temp)->leftSibling = rightSibling;
	}

	//Move last (n+1)/2 keys to the new node
	auto iterator = keyValueIndex.end();
	iterator--;
	int key;
	std::string value;
	for(int i = 0; i < (nodeSize +1)/2; i++){
		key = iterator->first; //extract key
		value = iterator->second; //extract sting value
		iterator--; //move backwards through vector
		if(!keyValueIndex.empty()){
			keyValueIndex.pop_back(); //delete old key,string pair
		}
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
		return ((InnerNode *)parent)->insertFromChild( p.first, rightSibling);
		return parent;
	}

	//didn't create new parent, need to add new NOde to parent index
	return ((InnerNode *)parent)->insertFromChild( p.first, rightSibling);
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
void LeafNode::remove(int keyToRemove){
	bool found = false;
	auto iterator = keyValueIndex.begin();
	while(iterator != keyValueIndex.end() &&!found){
		if(iterator->first == keyToRemove){
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
			borrowLeft(keyToRemove);
			return;
		}
	}
	//Case 2 -- Try to borrow from right sibling if it exists
	if(rightSibling != nullptr && ((LeafNode*)rightSibling)->parent == parent){
		if( ((LeafNode*)rightSibling)->keyValueIndex.size() - 1 > nodeSize/2){
			borrowRight();
			return;
		}
	}
	//Case 3 -- Try to coalese with left
	if(leftSibling != nullptr && ((LeafNode*)leftSibling)->parent == parent){
		coaleseLeft();
		return;
	}
	//Case 4 -- coalese with right
	//try telling rightSibling to coalese left
	if(rightSibling != nullptr && ((LeafNode*)rightSibling)->parent == parent){
		coaleseRight();
	}
}

/*
 *Returns the first key of the Node for indexing
 */
int LeafNode::getKey() const{
	return keyValueIndex.at(0).first;
}

/*
 *returns a pointer to the parent of the node
 */
void* LeafNode::getParent() const{
	return parent;
}

/*
 *Updates the parent pointer of the node
 *used when the parent has split and needs to inform its new children of the change
 */
void LeafNode::setParent(void* newParent){
	parent = newParent;
}

/*
 *Helper function for removal
 *If a node becomes less than half full, it may try borrowing from its
 *sibling to maintain the correct structure of the tree
 */
void LeafNode::borrowLeft(int oldKey){
	int replacementKey = getKey();
	int newKey = ((LeafNode*)leftSibling)->keyValueIndex.back().first;
	std::string newValue = ((LeafNode*)leftSibling)->keyValueIndex.back().second;
	insert(newKey, newValue);
	if( !((LeafNode*)leftSibling)->keyValueIndex.empty()){
		((LeafNode*)leftSibling)->keyValueIndex.pop_back(); //delete value from sibling
	}
	//update parent key
	((InnerNode*)parent)->updateChildKey(replacementKey, getKey());
}

/*
 *Helper function for removal
 *If a node becomes less than half full, it may try borrowing from its
 *sibling to maintain the correct structure of the tree
 */
void LeafNode::borrowRight(){
	int newKey = ((LeafNode*)rightSibling)->keyValueIndex.front().first;
	std::string value = ((LeafNode*)rightSibling)->keyValueIndex.front().second;
	insert(newKey,value);
	//delete value from sibling
	((LeafNode*)rightSibling)->keyValueIndex.erase(((LeafNode*)rightSibling)->keyValueIndex.begin());
	//update parent key
	// the old value we are replacing is now the right most key in the leaf --> keyValueIndex.back().first
	((InnerNode*)parent)->updateChildKey(keyValueIndex.back().first, ((LeafNode*)rightSibling)->keyValueIndex.front().first);	
}

/*
 *If we cannot borrow from siblings, need to merge the siblings
 *together
 */
void* LeafNode::coaleseLeft(){
	void* returnValue = this;
	//insert all values from the left sibling into current node
	auto iterator = ((LeafNode*)leftSibling)->keyValueIndex.begin();
	while(iterator != ((LeafNode*)leftSibling)->keyValueIndex.end()){
		insert(iterator->first, iterator->second);
		iterator++;
	}

	LeafNode* temp = (LeafNode*)leftSibling;
	leftSibling = temp->leftSibling; //update left sibling of current

	temp->leftSibling = nullptr;//completely detach the temporary node from the tree
	temp->rightSibling = nullptr;
	temp->parent=nullptr;

	if(leftSibling != nullptr){
		((LeafNode*)leftSibling)->rightSibling = this;
	}

	((InnerNode*)parent)->removeLeftChild(temp);
	//finally, delete leftSibling
	//delete temp;
	//walk up the tree to return the root in case it changed
	while(((Node*)returnValue)->getParent() != nullptr){
		returnValue = ((Node*)returnValue)->getParent();
	}
	return returnValue;
}

/*
 *If coalesing left is not possible, merge with the right sibling to maintain proper
 *tree structure
 */
void* LeafNode::coaleseRight(){
	void* returnValue = this;
	//insert all values from right sibling into current node
	auto iterator = ((LeafNode*)rightSibling)->keyValueIndex.begin();

	while(iterator != ((LeafNode*)rightSibling)->keyValueIndex.end() ){
		insert(iterator->first, iterator->second);
		iterator++;
	}
	LeafNode* temp = (LeafNode*)rightSibling;
	rightSibling = temp->rightSibling; //update right sibling of current

	if(rightSibling != nullptr){
		((LeafNode*)rightSibling)->leftSibling = this;
	}

	((InnerNode*)parent)->removeRightChild(temp);
	delete temp;

	while(((Node*)returnValue)->getParent() != nullptr){
		returnValue = ((Node*)returnValue)->getParent();
	}
	return returnValue;
}

/*
 *Prints the values of the tree to standard output, each on a separate line
 */
void LeafNode::printValues(){
	//print all values in the node on a new line
	for(int i = 0; i < keyValueIndex.size(); i++){
		std::cout << keyValueIndex.at(i).second << std::endl;
	}
	if(rightSibling != nullptr){
		//print all values in subsequent nodes
		((Node*)rightSibling)->printValues();
	}
}

/*
 /returns true if the node contains the key, false otherwise
 */
bool LeafNode::contains(int key){
	auto iterator = keyValueIndex.begin();
	while(iterator != keyValueIndex.end()){
		if(iterator->first == key){
			return true;
		}
		iterator++;
	}
	return false;
}

/*
 *Returns true if the keyPointerIndex is empty, false otherwise
 *Used to determine if the root node of the tree has become empty, if so, may promote next node as new root
 *In the case of leafNodes, there are no children to promote, so it does not make sense to return true
 */
bool LeafNode::isEmpty() const {
	return false;
}

/*
 *Helper function to delete the tree
 *Recursively deletes all nodes of the subtree
 */
void LeafNode::fullDeletion(){
	//no dynamically allocated memory in leaves, do nothing
	return;
}

/*
 *Returns the node that does or would contain the key
 */
void* LeafNode::findLeaf(int key){
	return this; 
}

/*
 *Helper function to create a copy of the tree
 *returns pointer to a new copy of the subtree
 *The copied subtree will have the same node structure as the original,
 *but sibling and parent pointers will not be assigned. Call fixSiblings() on the copied
 *tree to correctly connect the nodes
 */
void* LeafNode::copySubTree(){
	void* subTree = new LeafNode(nodeSize);
	int key;
	std::string value;
	for(int i = 0; i < keyValueIndex.size(); i++){
		key = keyValueIndex.at(i).first;
		value = keyValueIndex.at(i).second;
		((Node*)subTree)->insert(key, value);
	}
	return subTree;
}

/*
 *Helper function in copying a tree
 *Fixes the sibling and parent pointers of a newly created tree so that regular operations
 *can function correctly
 */
void LeafNode::fixSiblings(){
	//don't have any children to fix, does nothing
	return;
}