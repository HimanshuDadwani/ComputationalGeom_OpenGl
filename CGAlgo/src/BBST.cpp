#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

template <typename T>
class BBST
{
	class node {
	public:
		T data;
		node* left;
		node* right;
		node(T data)
		{
			this->data = data;
			left = right = nullptr;
		}
	};
public:
	node* root = NULL;
	void insert(T data, node* iRootNode = NULL)
	{
		if (root == nullptr)
		{
			root = new node(data);
			return;
		}
		if (iRootNode == nullptr)
			iRootNode = root;
		if (data < iRootNode->data)
		{
			if (iRootNode->left == nullptr)
				iRootNode->left = new node(data);
			else
				insert(data, iRootNode->left);
		}
		else
		{
			if (iRootNode->right == nullptr)
				iRootNode->right = new node(data);
			else
				insert(data, iRootNode->right);
		}

	}

	void inOrder(node* inode = NULL)
	{
		if (inode == nullptr)
			return;
		inOrder(inode->left);
		cout << inode->data << " ";
		inOrder(inode->right);
	}
	void preOrder(node* inode = NULL)
	{
		if (inode == nullptr)
			return;
		cout << inode->data << " ";
		preOrder(inode->left);
		preOrder(inode->right);
	}
	void postOrder(node* inode = NULL)
	{
		if (inode == nullptr)
			return;
		postOrder(inode->left);
		postOrder(inode->right);
		cout << inode->data << " ";
	}
};

int main_1()
{
	BBST<int> sortedIntList;
	sortedIntList.insert(5);
	sortedIntList.insert(3);
	sortedIntList.insert(6);
	sortedIntList.insert(8);
	sortedIntList.insert(1);
	sortedIntList.insert(9);
	cout << "Inorder: ";
	sortedIntList.inOrder(sortedIntList.root);
	cout << endl;
	cout << "PreOrder: ";
	sortedIntList.preOrder(sortedIntList.root);
	cout << endl;
	cout << "PostOrder: ";
	sortedIntList.postOrder(sortedIntList.root);
	cout << endl;
	return 0;
}